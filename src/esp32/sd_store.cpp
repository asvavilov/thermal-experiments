#include "sd_store.h"

#include <SD.h>
#include <SPI.h>

#include "config.h"
#include "printer.h"

namespace {

bool mounted = false;

bool isAsciiNameChar(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
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

  String out;
  for (size_t i = 0; i < base.length() && out.length() < 48; i++) {
    const char c = base[i];
    out += isAsciiNameChar(c) ? c : '_';
  }
  if (out.length() == 0) out = "file";

  String lower = out;
  lower.toLowerCase();
  if (!lower.endsWith(".bin") && !lower.endsWith(".raw") &&
      !lower.endsWith(".prn")) {
    out += ".bin";
  }
  return out;
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
  String lower = name;
  lower.toLowerCase();
  return lower.endsWith(".bin") || lower.endsWith(".raw") ||
         lower.endsWith(".prn");
}

bool sdIsRawFile(const String& name) {
  String lower = name;
  lower.toLowerCase();
  return lower.endsWith(".prn");
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
  if (!mounted || !sdIsAllowedName(name) || sdIsRawFile(name)) return false;

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

bool sdPrintBitmap(const String& name, uint8_t feed, String& error) {
  error = "";
  if (!mounted) {
    error = "sd not mounted";
    return false;
  }
  if (!sdIsAllowedName(name) || sdIsRawFile(name)) {
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

  if (!printerBeginJob()) {
    f.close();
    error = "printer not ready";
    return false;
  }
  printerBitmapHeader(width, height);

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

  if (feed && !printerFeed(feed)) {
    error = "printer not ready";
    return false;
  }
  return true;
}

bool sdPrintRaw(const String& name, String& error) {
  error = "";
  if (!mounted) {
    error = "sd not mounted";
    return false;
  }
  if (!sdIsAllowedName(name) || !sdIsRawFile(name)) {
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
