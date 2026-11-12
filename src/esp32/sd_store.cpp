#include "sd_store.h"

#include <SD.h>
#include <SPI.h>

#include "config.h"
#include "printer.h"
#include "rich_text.h"

namespace {

bool mounted = false;

bool isAsciiNameChar(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
}

bool isKnownExtension(const String& ext) {
  return ext == ".bin" || ext == ".raw" || ext == ".prn" || ext == ".txt" ||
         ext == ".md" || ext == ".markdown" || ext == ".csv" || ext == ".log" ||
         ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" ||
         ext == ".bmp" || ext == ".webp";
}

bool isSourceImageExtension(const String& ext) {
  return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" ||
         ext == ".bmp" || ext == ".webp";
}

String lowerExt(const String& name) {
  const int dot = name.lastIndexOf('.');
  if (dot < 0) return String();
  String ext = name.substring(dot);
  ext.toLowerCase();
  return ext;
}

// Простая транслитерация, чтобы русские имена файлов не превращались в "_".
const char* translitCyrillic(uint32_t cp) {
  if (cp >= 0x410 && cp <= 0x42F) cp += 0x20;  // А..Я -> а..я
  switch (cp) {
    case 0x430: return "a";
    case 0x431: return "b";
    case 0x432: return "v";
    case 0x433: return "g";
    case 0x434: return "d";
    case 0x435: return "e";
    case 0x451: return "e";  // ё
    case 0x401: return "e";  // Ё
    case 0x436: return "zh";
    case 0x437: return "z";
    case 0x438: return "i";
    case 0x439: return "y";
    case 0x43A: return "k";
    case 0x43B: return "l";
    case 0x43C: return "m";
    case 0x43D: return "n";
    case 0x43E: return "o";
    case 0x43F: return "p";
    case 0x440: return "r";
    case 0x441: return "s";
    case 0x442: return "t";
    case 0x443: return "u";
    case 0x444: return "f";
    case 0x445: return "kh";
    case 0x446: return "ts";
    case 0x447: return "ch";
    case 0x448: return "sh";
    case 0x449: return "shch";
    case 0x44A: return "";
    case 0x44B: return "y";
    case 0x44C: return "";
    case 0x44D: return "e";
    case 0x44E: return "yu";
    case 0x44F: return "ya";
    default: return nullptr;
  }
}

void appendNameChar(String& out, uint32_t cp) {
  if (cp < 0x80) {
    out += isAsciiNameChar((char)cp) ? (char)cp : '_';
    return;
  }
  const char* t = translitCyrillic(cp);
  out += t && t[0] ? t : "_";
}

void appendSanitizedStem(String& out, const String& stem) {
  for (size_t i = 0; i < stem.length();) {
    const uint8_t c = (uint8_t)stem[i];
    uint32_t cp = 0;
    size_t len = 0;
    if (c < 0x80) {
      cp = c;
      len = 1;
    } else if ((c & 0xE0) == 0xC0 && i + 1 < stem.length()) {
      cp = ((c & 0x1F) << 6) | ((uint8_t)stem[i + 1] & 0x3F);
      len = 2;
    } else if ((c & 0xF0) == 0xE0 && i + 2 < stem.length()) {
      cp = ((c & 0x0F) << 12) | (((uint8_t)stem[i + 1] & 0x3F) << 6) |
           ((uint8_t)stem[i + 2] & 0x3F);
      len = 3;
    } else {
      out += '_';
      i++;
      continue;
    }
    appendNameChar(out, cp);
    i += len;
  }
}

}  // namespace

bool sdBegin() {
  if (mounted) return true;
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  mounted = SD.begin(PIN_SD_CS, SPI, SD_SPI_HZ, SD_MOUNT, 5);
  return mounted;
}

void sdEnd() {
  if (!mounted) return;
  SD.end();
  mounted = false;
}

bool sdMounted() { return mounted; }

uint64_t sdTotalBytes() { return mounted ? SD.totalBytes() : 0; }
uint64_t sdUsedBytes() { return mounted ? SD.usedBytes() : 0; }
uint8_t sdCardType() { return mounted ? (uint8_t)SD.cardType() : 0; }

String sdSanitizeName(const String& input) {
  String base = input;
  int slash = base.lastIndexOf('/');
  if (slash >= 0) base = base.substring(slash + 1);
  slash = base.lastIndexOf('\\');
  if (slash >= 0) base = base.substring(slash + 1);
  base.trim();

  // Сохраняем известное расширение, чтобы .txt не превратился в .bin.
  String stem = base;
  String ext = lowerExt(base);
  if (isKnownExtension(ext)) {
    stem = base.substring(0, base.length() - ext.length());
  } else {
    ext = "";
  }

  String out;
  appendSanitizedStem(out, stem);
  if (out.length() == 0) out = "file";
  if (ext.length() == 0) ext = ".bin";

  const size_t maxStem = 48 > ext.length() ? 48 - ext.length() : 8;
  if (out.length() > maxStem) out = out.substring(0, maxStem);
  return out + ext;
}

bool sdIsAllowedName(const String& name) {
  if (name.length() == 0 || name.length() > 64) return false;
  if (name.indexOf('/') >= 0 || name.indexOf('\\') >= 0 ||
      name.indexOf("..") >= 0) {
    return false;
  }
  for (size_t i = 0; i < name.length(); i++) {
    if (!isAsciiNameChar(name[i])) return false;
  }
  return isKnownExtension(lowerExt(name));
}

SdFileKind sdFileKind(const String& name) {
  if (!sdIsAllowedName(name)) return SdFileKind::Invalid;
  const String ext = lowerExt(name);
  if (ext == ".prn") return SdFileKind::Raw;
  if (ext == ".txt") return SdFileKind::Text;
  if (ext == ".md" || ext == ".markdown" || ext == ".csv" || ext == ".log") {
    return SdFileKind::Plain;
  }
  if (isSourceImageExtension(ext)) return SdFileKind::Source;
  return SdFileKind::Image;
}

const char* sdKindName(SdFileKind kind) {
  switch (kind) {
    case SdFileKind::Image: return "image";
    case SdFileKind::Source: return "source";
    case SdFileKind::Raw: return "raw";
    case SdFileKind::Text: return "text";
    case SdFileKind::Plain: return "plain";
    default: return "invalid";
  }
}

bool sdIsRawFile(const String& name) {
  return sdFileKind(name) == SdFileKind::Raw;
}

bool sdIsTextFile(const String& name) {
  return sdFileKind(name) == SdFileKind::Text;
}

String sdPath(const String& name) { return "/" + name; }

bool sdExists(const String& name) {
  if (!mounted || !sdIsAllowedName(name)) return false;
  return SD.exists(sdPath(name));
}

bool sdRemove(const String& name) {
  if (!mounted || !sdIsAllowedName(name)) return false;
  return SD.remove(sdPath(name));
}

size_t sdFileSize(const String& name) {
  if (!mounted || !sdIsAllowedName(name)) return 0;
  File f = SD.open(sdPath(name), FILE_READ);
  if (!f) return 0;
  const size_t size = f.size();
  f.close();
  return size;
}

bool sdReadBitmapHeader(const String& name, uint16_t& width, uint16_t& height,
                        uint32_t& dataBytes) {
  width = 0;
  height = 0;
  dataBytes = 0;
  if (!mounted || sdFileKind(name) != SdFileKind::Image) return false;

  File f = SD.open(sdPath(name), FILE_READ);
  if (!f) return false;

  uint8_t header[4];
  if (f.read(header, 4) != 4) {
    f.close();
    return false;
  }

  width = (uint16_t)header[0] | ((uint16_t)header[1] << 8);
  height = (uint16_t)header[2] | ((uint16_t)header[3] << 8);
  if (width == 0 || width > PRINTER_WIDTH || height == 0 ||
      height > MAX_BITMAP_HEIGHT) {
    f.close();
    return false;
  }

  dataBytes = (uint32_t)((width + 7) / 8) * height;
  if (f.size() < (size_t)(4 + dataBytes)) {
    f.close();
    return false;
  }

  f.close();
  return true;
}

bool printBitmapFile(const String& name, uint8_t feed, bool beginJob,
                     String& error) {
  error = "";
  if (!mounted) {
    error = "sd not mounted";
    return false;
  }
  if (sdFileKind(name) != SdFileKind::Image) {
    error = "bad file name";
    return false;
  }

  File f = SD.open(sdPath(name), FILE_READ);
  if (!f) {
    error = "file not found";
    return false;
  }

  uint8_t header[4];
  if (f.read(header, 4) != 4) {
    f.close();
    error = "short header";
    return false;
  }

  const uint16_t width =
      (uint16_t)header[0] | ((uint16_t)header[1] << 8);
  const uint16_t height =
      (uint16_t)header[2] | ((uint16_t)header[3] << 8);

  if (width == 0 || width > PRINTER_WIDTH || height == 0 ||
      height > MAX_BITMAP_HEIGHT) {
    f.close();
    error = "bad image size";
    return false;
  }

  const uint32_t dataBytes = (uint32_t)((width + 7) / 8) * height;
  if (f.size() < (size_t)(4 + dataBytes)) {
    f.close();
    error = "short file";
    return false;
  }

  if (beginJob && !printerBeginJob()) {
    f.close();
    error = "printer not ready";
    return false;
  }
  printerBitmapHeader(width, height);
  const uint32_t startedAt = millis();

  uint8_t buffer[256];
  uint32_t left = dataBytes;
  while (left > 0) {
    const size_t want = left < sizeof(buffer) ? (size_t)left : sizeof(buffer);
    const size_t got = f.read(buffer, want);
    if (got == 0) {
      f.close();
      error = "short file";
      return false;
    }
    if (!printerWrite(buffer, got)) {
      f.close();
      error = "printer not ready";
      return false;
    }
    left -= (uint32_t)got;
  }

  f.close();

  Serial.printf("[print] sd bitmap %s %ux%u, %u ms\n", name.c_str(),
                (unsigned)width, (unsigned)height,
                (unsigned)(millis() - startedAt));

  if (feed && !printerFeed(feed)) {
    error = "printer not ready";
    return false;
  }
  return true;
}

bool sdPrintBitmap(const String& name, uint8_t feed, String& error) {
  return printBitmapFile(name, feed, true, error);
}

bool sdPrintBitmapInline(const String& name, String& error) {
  return printBitmapFile(name, 0, false, error);
}

bool sdPrintRaw(const String& name, String& error) {
  error = "";
  if (!mounted) {
    error = "sd not mounted";
    return false;
  }
  if (sdFileKind(name) != SdFileKind::Raw) {
    error = "bad file name";
    return false;
  }

  File f = SD.open(sdPath(name), FILE_READ);
  if (!f) {
    error = "file not found";
    return false;
  }

  uint8_t buffer[256];
  while (f.available()) {
    const size_t got = f.read(buffer, sizeof(buffer));
    if (got == 0) break;
    if (!printerWrite(buffer, got)) {
      f.close();
      error = "printer not ready";
      return false;
    }
  }

  f.close();
  return true;
}

bool sdPrintText(const String& name, uint8_t feed, String& error) {
  error = "";
  if (!mounted) {
    error = "sd not mounted";
    return false;
  }
  if (sdFileKind(name) != SdFileKind::Text) {
    error = "bad file name";
    return false;
  }

  File f = SD.open(sdPath(name), FILE_READ);
  if (!f) {
    error = "file not found";
    return false;
  }

  if (f.size() > MAX_SD_TEXT_BYTES) {
    f.close();
    error = "text file too large";
    return false;
  }

  RichTextPrinter printer;
  if (!printer.begin(error)) {
    f.close();
    return false;
  }

  uint8_t buffer[256];
  while (f.available()) {
    const size_t got = f.read(buffer, sizeof(buffer));
    if (got == 0) break;
    if (!printer.write((const char*)buffer, got, error)) {
      f.close();
      return false;
    }
  }

  f.close();
  return printer.end(feed, error);
}

bool sdPrintPlainText(const String& name, uint8_t feed, String& error) {
  error = "";
  if (!mounted) {
    error = "sd not mounted";
    return false;
  }
  if (sdFileKind(name) != SdFileKind::Plain) {
    error = "bad file name";
    return false;
  }

  File f = SD.open(sdPath(name), FILE_READ);
  if (!f) {
    error = "file not found";
    return false;
  }

  if (f.size() > MAX_SD_TEXT_BYTES) {
    f.close();
    error = "text file too large";
    return false;
  }

  RichTextPrinter printer;
  if (!printer.beginPlain(error)) {
    f.close();
    return false;
  }

  uint8_t buffer[256];
  while (f.available()) {
    const size_t got = f.read(buffer, sizeof(buffer));
    if (got == 0) break;
    if (!printer.write((const char*)buffer, got, error)) {
      f.close();
      return false;
    }
  }

  f.close();
  return printer.end(feed, error);
}
