#include "printer.h"

#include <Preferences.h>
#include <WiFi.h>

#include "config.h"

namespace {

Preferences prefs;
bool prefsReady = false;
uint8_t heatValue = DEFAULT_HEAT;
bool dtrPresent = false;
bool dtrEnabled = DEFAULT_USE_DTR;
uint8_t printMode = 0;
uint32_t baudValue = PRINTER_BAUD;
uint8_t codePageValue = DEFAULT_CODEPAGE;
// Принтер принимает ~256 байт буфера; пишем крупными кусками, чтобы не
// терять время на переключениях и не рвать поток.
constexpr size_t WRITE_CHUNK = 128;

// Лог реально отправленных в принтер байт (для диагностики).
uint8_t txLog[512];
size_t txLogLen = 0;

// ---------------------------------------------------------------------------
// UTF-8 -> CP1251 / CP866 / ASCII
// ---------------------------------------------------------------------------

void appendCp1251(String& out, uint32_t cp) {
  if (cp < 0x80) {
    out += (char)cp;
    return;
  }
  if (cp >= 0x0410 && cp <= 0x044F) {  // А..я
    out += (char)(cp - 0x0410 + 0xC0);
    return;
  }
  switch (cp) {
    case 0x0401: out += (char)0xA8; return;  // Ё
    case 0x0451: out += (char)0xB8; return;  // ё
    case 0x00AB: out += (char)0xAB; return;  // «
    case 0x00BB: out += (char)0xBB; return;  // »
    case 0x2013: out += (char)0x96; return;  // –
    case 0x2014: out += (char)0x97; return;  // —
    case 0x2018: out += (char)0x91; return;  // ‘
    case 0x2019: out += (char)0x92; return;  // ’
    case 0x201C: out += (char)0x93; return;  // “
    case 0x201D: out += (char)0x94; return;  // ”
    case 0x2026: out += (char)0x85; return;  // …
    case 0x2116: out += (char)0xB9; return;  // №
    case 0x20AC: out += (char)0x88; return;  // €
    case 0x20BD:                            // ₽ — в CP1251 нет, печатаем «руб.»
      out += (char)0xF0;  // р
      out += (char)0xF3;  // у
      out += (char)0xE1;  // б
      out += '.';
      return;
    case 0x00A0: out += (char)0xA0; return;  // NBSP
    case 0x00B0: out += (char)0xB0; return;  // °
    case 0x00B7: out += (char)0xB7; return;  // ·
    case 0x2022: out += (char)0x95; return;  // •
    default:     out += '?'; return;
  }
}

void appendCp866(String& out, uint32_t cp) {
  if (cp < 0x80) {
    out += (char)cp;
    return;
  }
  if (cp >= 0x0410 && cp <= 0x043F) {  // А..п
    out += (char)(cp - 0x0410 + 0x80);
    return;
  }
  if (cp >= 0x0440 && cp <= 0x044F) {  // р..я
    out += (char)(cp - 0x0440 + 0xE0);
    return;
  }
  switch (cp) {
    case 0x0401: out += (char)0xF0; return;  // Ё
    case 0x0451: out += (char)0xF1; return;  // ё
    case 0x2013:
    case 0x2014: out += '-'; return;
    case 0x00AB:
    case 0x00BB:
    case 0x2018:
    case 0x2019:
    case 0x201C:
    case 0x201D: out += '"'; return;
    case 0x2026: out += '.'; return;
    case 0x2022: out += '-'; return;  // • — в CP866 безопаснее дефис
    case 0x20BD:                            // ₽ — в CP866 нет, печатаем «руб.»
      out += (char)0xE0;  // р
      out += (char)0xE3;  // у
      out += (char)0xA1;  // б
      out += '.';
      return;
    case 0x00A0: out += ' '; return;
    default:     out += '?'; return;
  }
}

void appendAscii(String& out, uint32_t cp) {
  out += (cp < 0x80) ? (char)cp : '?';
}

typedef void (*CpAppend)(String&, uint32_t);

String decodeUtf8(const String& src, CpAppend append) {
  String out;
  out.reserve(src.length());
  const uint8_t* s = (const uint8_t*)src.c_str();
  const size_t n = src.length();

  for (size_t i = 0; i < n;) {
    const uint8_t c = s[i];
    uint32_t cp = 0;
    size_t len = 1;

    if (c < 0x80) {
      cp = c;
    } else if ((c & 0xE0) == 0xC0 && i + 1 < n) {
      cp = ((c & 0x1F) << 6) | (s[i + 1] & 0x3F);
      len = 2;
    } else if ((c & 0xF0) == 0xE0 && i + 2 < n) {
      cp = ((c & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
      len = 3;
    } else if ((c & 0xF8) == 0xF0 && i + 3 < n) {
      cp = ((c & 0x07) << 18) | ((s[i + 1] & 0x3F) << 12) |
           ((s[i + 2] & 0x3F) << 6) | (s[i + 3] & 0x3F);
      len = 4;
    } else {
      out += '?';
      i++;
      continue;
    }

    append(out, cp);
    i += len;
  }
  return out;
}

String utf8ToCp1251(const String& src) { return decodeUtf8(src, appendCp1251); }
String utf8ToCp866(const String& src) { return decodeUtf8(src, appendCp866); }
String utf8ToAscii(const String& src) { return decodeUtf8(src, appendAscii); }

// ---------------------------------------------------------------------------
// Низкоуровневая передача
// ---------------------------------------------------------------------------

bool waitReady() {
#if USE_DTR
  if (!dtrEnabled || !dtrPresent) return true;
  const uint32_t start = millis();
  while (digitalRead(PIN_PRINTER_DTR) != DTR_READY_LEVEL) {
    if (millis() - start > DTR_WAIT_TIMEOUT_MS) {
      Serial.println("[printer] DTR timeout: printer is not ready");
      return false;
    }
    delay(1);
  }
#endif
  return true;
}

bool writeBytes(const uint8_t* data, size_t len) {
  size_t off = 0;
  while (off < len) {
    if (!waitReady()) return false;
    size_t n = len - off;
    if (n > WRITE_CHUNK) n = WRITE_CHUNK;
    const size_t written = Serial2.write(data + off, n);
    if (written == 0) {
      delay(1);
      continue;
    }
    for (size_t i = 0; i < written && txLogLen < sizeof(txLog); i++) {
      txLog[txLogLen++] = data[off + i];
    }
    off += written;
    delay(1);
  }
  return true;
}

bool writeByte(uint8_t b) { return writeBytes(&b, 1); }

int readByte(uint32_t timeoutMs) {
  const uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (Serial2.available()) return Serial2.read();
    delay(1);
  }
  return -1;
}

void flushInput() {
  while (Serial2.available()) Serial2.read();
}

void writePrintMode() {
  const uint8_t cmd[] = {0x1B, 0x21, printMode};  // ESC ! n
  writeBytes(cmd, sizeof(cmd));
}

void setPrintModeBit(uint8_t mask, bool on) {
  if (on) {
    printMode |= mask;
  } else {
    printMode &= (uint8_t)~mask;
  }
  writePrintMode();
}

}  // namespace

// ---------------------------------------------------------------------------
// Публичный API
// ---------------------------------------------------------------------------

void printerBegin() {
  Serial2.setTxBufferSize(1024);
  Serial2.begin(baudValue, SERIAL_8N1, PIN_PRINTER_RX, PIN_PRINTER_TX);
  pinMode(PIN_PRINTER_DTR, INPUT_PULLUP);
  printerLoadSettings();
  delay(100);

  // Определяем, подключён ли DTR. Если линия не подключена, INPUT_PULLUP
  // держит HIGH; тогда работаем с паузами, а не ждём готовности.
  if (dtrEnabled) {
    const uint32_t probeStart = millis();
    while (digitalRead(PIN_PRINTER_DTR) != DTR_READY_LEVEL &&
           millis() - probeStart < DTR_BOOT_PROBE_MS) {
      delay(1);
    }
    dtrPresent = digitalRead(PIN_PRINTER_DTR) == DTR_READY_LEVEL;
  } else {
    dtrPresent = false;
  }
  if (!dtrEnabled) {
    Serial.println("[printer] DTR disabled in settings, using delays");
  } else {
    Serial.println(dtrPresent ? "[printer] DTR detected"
                              : "[printer] DTR not detected, using delays");
  }
  Serial.printf("[printer] baud %u, codepage %u\n", (unsigned)baudValue,
                (unsigned)codePageValue);

  // Сброс и включение аппаратного управления потоком по DTR.
  // GS a 0x20 — так делает Adafruit_Thermal при использовании DTR.
  const uint8_t reset[] = {0x1B, 0x40};
  writeBytes(reset, sizeof(reset));
  delay(100);
  if (dtrEnabled) {
    const uint8_t dtrOn[] = {0x1D, 0x61, 0x20};
    writeBytes(dtrOn, sizeof(dtrOn));
    delay(50);
    if (!dtrPresent) {
      dtrPresent = digitalRead(PIN_PRINTER_DTR) == DTR_READY_LEVEL;
      if (dtrPresent) Serial.println("[printer] DTR detected after init");
    }
  }
}

void printerLoadSettings() {
  prefs.begin("printer", false);
  prefsReady = true;
  heatValue = prefs.getUChar("heat", DEFAULT_HEAT);
  if (heatValue == 0) heatValue = DEFAULT_HEAT;

  baudValue = prefs.getUInt("baud", PRINTER_BAUD);
  if (baudValue < 1200 || baudValue > 500000) baudValue = PRINTER_BAUD;

  codePageValue = prefs.getUChar("cp", DEFAULT_CODEPAGE);
  // Старые версии сохраняли 46/17 (это Hebrew/Greek); переносим на 6/7.
  if (codePageValue == 46) codePageValue = 6;
  if (codePageValue == 17) codePageValue = 7;
  if (codePageValue > 47) codePageValue = DEFAULT_CODEPAGE;

  dtrEnabled = prefs.getBool("dtr", DEFAULT_USE_DTR != 0);
}

bool printerBeginJob() {
  const uint8_t reset[] = {0x1B, 0x40};  // ESC @ — сброс и выход из сна
  if (!writeBytes(reset, sizeof(reset))) return false;
  delay(50);

  printerOnline(true);  // ESC = 1 — выйти из offline, если предыдущий тег выключил

  printMode = 0;
  writePrintMode();  // ESC ! 0 — сбросить режим печати

  // Включить аппаратное управление потоком по DTR (как в Adafruit_Thermal).
  if (dtrEnabled) {
    const uint8_t dtrOn[] = {0x1D, 0x61, 0x20};  // GS a 0x20
    if (!writeBytes(dtrOn, sizeof(dtrOn))) return false;
  }

  const uint8_t codepage[] = {0x1B, 0x74, codePageValue};  // ESC t n
  if (!writeBytes(codepage, sizeof(codepage))) return false;

  const uint8_t heatCmd[] = {0x1B, 0x37, 11, heatValue, 40};  // ESC 7 dots,time,interval
  if (!writeBytes(heatCmd, sizeof(heatCmd))) return false;

  const uint8_t lineSpacing[] = {0x1B, 0x32};  // ESC 2 — интервал по умолчанию
  return writeBytes(lineSpacing, sizeof(lineSpacing));
}

void printerReset() {
  printMode = 0;
  const uint8_t cmd[] = {0x1B, 0x40};
  writeBytes(cmd, sizeof(cmd));
}

bool printerFeed(uint8_t lines) {
  if (lines == 0) return true;
  // ESC d (0x1B 0x64 n) на T7-US игнорируется, поэтому подача — переводами
  // строки: их понимает любая ESC/POS-прошивка.
  uint8_t buf[32];
  memset(buf, '\n', sizeof(buf));
  while (lines > 0) {
    const uint8_t n = lines > sizeof(buf) ? (uint8_t)sizeof(buf) : lines;
    if (!writeBytes(buf, n)) return false;
    lines -= n;
  }
  return true;
}

void printerSetHeat(uint8_t heat) {
  if (heat < 1) heat = 1;
  heatValue = heat;
  if (prefsReady) prefs.putUChar("heat", heatValue);
}

uint8_t printerHeat() { return heatValue; }

bool printerDtrActive() { return dtrEnabled && dtrPresent; }
bool printerDtrEnabled() { return dtrEnabled; }

void printerSetDtrEnabled(bool on) {
  dtrEnabled = on;
  if (prefsReady) prefs.putBool("dtr", on);
}

uint32_t printerBaud() { return baudValue; }

void printerSetBaud(uint32_t baud) {
  if (baud < 1200 || baud > 500000) return;
  baudValue = baud;
  if (prefsReady) prefs.putUInt("baud", baudValue);
  Serial2.end();
  Serial2.setTxBufferSize(1024);
  Serial2.begin(baudValue, SERIAL_8N1, PIN_PRINTER_RX, PIN_PRINTER_TX);
  Serial.printf("[printer] baud changed to %u\n", (unsigned)baudValue);
}

uint8_t printerCodePage() { return codePageValue; }

void printerSetCodePageValue(uint8_t page) {
  if (page > 47) return;
  codePageValue = page;
  if (prefsReady) prefs.putUChar("cp", codePageValue);
  Serial.printf("[printer] codepage set to %u\n", (unsigned)codePageValue);
}

// Печатает по строке на каждую кодовую страницу 0..47 с текстом в CP1251,
// плюс контрольную строку с CP866 на странице 7. По бумаге видно, какая
// страница даёт читаемую кириллицу (для T7-US это 6).
bool printerCodePageTest() {
  if (!printerBeginJob()) return false;

  const char* text = "Привет, мир!";
  const String cp1251 = utf8ToCp1251(text);
  const String cp866 = utf8ToCp866(text);

  for (uint8_t page = 0; page <= 47; page++) {
    const uint8_t pageCmd[] = {0x1B, 0x74, page};  // ESC t n
    if (!writeBytes(pageCmd, sizeof(pageCmd))) return false;

    char label[8];
    snprintf(label, sizeof(label), "%02u:", (unsigned)page);
    if (!writeBytes((const uint8_t*)label, strlen(label))) return false;
    if (!writeBytes((const uint8_t*)cp1251.c_str(), cp1251.length())) return false;
    if (!writeByte('\n')) return false;
  }

  const uint8_t page7[] = {0x1B, 0x74, 7};  // ESC t 7 = CP866
  if (!writeBytes(page7, sizeof(page7))) return false;
  const char* label866 = "07/866:";
  if (!writeBytes((const uint8_t*)label866, strlen(label866))) return false;
  if (!writeBytes((const uint8_t*)cp866.c_str(), cp866.length())) return false;
  if (!writeByte('\n')) return false;

  // Возвращаемся к сохранённой странице.
  const uint8_t restore[] = {0x1B, 0x74, codePageValue};
  if (!writeBytes(restore, sizeof(restore))) return false;
  return printerFeed(3);
}

// Диагностика: печатает по строке на каждую команду, которую прошивка
// отправляет в начале задания. Символ, вылезший после метки, показывает,
// какая команда на этой плате печатается.
bool printerCommandTest() {
  struct Case {
    const char* label;
    uint8_t len;
    uint8_t bytes[8];
  };
  const Case cases[] = {
      {"FF:", 1, {0xFF}},
      {"ESC8:", 4, {0x1B, 0x38, 0x00, 0x00}},
      {"ESC@:", 2, {0x1B, 0x40}},
      {"ESC=:", 3, {0x1B, 0x3D, 0x01}},
      {"ESC!:", 3, {0x1B, 0x21, 0x00}},
      {"GSa:", 3, {0x1D, 0x61, 0x20}},
      {"ESCt:", 3, {0x1B, 0x74, 0x06}},
      {"ESC7:", 5, {0x1B, 0x37, 0x0B, 0x78, 0x28}},
      {"ESC2:", 2, {0x1B, 0x32}},
      {"ESCd:", 3, {0x1B, 0x64, 0x02}},
      {"NUL:", 2, {0x00, 0x00}},
  };

  for (const Case& c : cases) {
    if (!writeBytes((const uint8_t*)c.label, strlen(c.label))) return false;
    if (!writeBytes(c.bytes, c.len)) return false;
    if (!writeByte('\n')) return false;
  }

  // Варианты полного старта задания (printerBeginJob). Если символ вылезет
  // после метки — виновата команда, которая в этом варианте есть.
  const uint8_t wake[] = {0x1B, 0x38, 0x00, 0x00};   // ESC 8 0 0
  const uint8_t reset[] = {0x1B, 0x40};              // ESC @
  const uint8_t online[] = {0x1B, 0x3D, 0x01};       // ESC = 1
  const uint8_t mode[] = {0x1B, 0x21, 0x00};         // ESC ! 0
  const uint8_t asb[] = {0x1D, 0x61, 0x20};          // GS a 0x20
  const uint8_t page[] = {0x1B, 0x74, 0x06};         // ESC t 6
  const uint8_t heat[] = {0x1B, 0x37, 0x0B, 0x78, 0x28};  // ESC 7
  const uint8_t spacing[] = {0x1B, 0x32};            // ESC 2

  auto run = [&](const char* label, bool useWake, bool useAsb, bool usePage,
                 bool useHeat) -> bool {
    if (!writeBytes((const uint8_t*)label, strlen(label))) return false;
    if (useWake && !writeBytes(wake, sizeof(wake))) return false;
    if (!writeBytes(reset, sizeof(reset))) return false;
    if (!writeBytes(online, sizeof(online))) return false;
    if (!writeBytes(mode, sizeof(mode))) return false;
    if (useAsb && !writeBytes(asb, sizeof(asb))) return false;
    if (usePage && !writeBytes(page, sizeof(page))) return false;
    if (useHeat && !writeBytes(heat, sizeof(heat))) return false;
    if (!writeBytes(spacing, sizeof(spacing))) return false;
    return writeByte('\n');
  };

  delay(1000);  // пауза, как между заданиями
  if (!run("JA:", true, true, true, true)) return false;
  if (!run("JB:", false, true, true, true)) return false;
  if (!run("JC:", true, false, true, true)) return false;
  if (!run("JD:", true, true, false, true)) return false;
  if (!run("JE:", true, true, true, false)) return false;
  if (!run("JF:", true, false, false, false)) return false;

  return printerFeed(3);
}

bool printerText(const String& utf8, uint8_t size, bool bold, bool underline,
                 char align, uint8_t feed) {
  // Убираем CR, чтобы не было двойных переводов строк.
  String normalized;
  normalized.reserve(utf8.length());
  for (size_t i = 0; i < utf8.length(); i++) {
    if (utf8[i] != '\r') normalized += utf8[i];
  }

  if (!printerBeginJob()) return false;

  const uint8_t a = (align == 'C') ? 1 : (align == 'R') ? 2 : 0;
  const uint8_t alignCmd[] = {0x1B, 0x61, a};  // ESC a n
  if (!writeBytes(alignCmd, sizeof(alignCmd))) return false;

  uint8_t s = size;
  if (s < 1) s = 1;
  if (s > 3) s = 3;
  const uint8_t gs = (uint8_t)(((s - 1) << 4) | (s - 1));
  const uint8_t sizeCmd[] = {0x1D, 0x21, gs};  // GS ! n
  if (!writeBytes(sizeCmd, sizeof(sizeCmd))) return false;

  if (bold) {
    const uint8_t cmd[] = {0x1B, 0x45, 1};
    if (!writeBytes(cmd, sizeof(cmd))) return false;
  }
  if (underline) {
    const uint8_t cmd[] = {0x1B, 0x2D, 1};
    if (!writeBytes(cmd, sizeof(cmd))) return false;
  }

  const String cp = printerEncodeText(normalized, codePageValue);
  if (!writeBytes((const uint8_t*)cp.c_str(), cp.length())) return false;

  if (cp.length() == 0 || cp[cp.length() - 1] != '\n') {
    if (!writeByte('\n')) return false;
  }

  if (feed) {
    const uint8_t feedCmd[] = {0x1B, 0x64, feed};
    if (!writeBytes(feedCmd, sizeof(feedCmd))) return false;
  }
  return true;
}

String printerEncodeText(const String& utf8, uint8_t codePage) {
  // Номера — как в Adafruit_Thermal: 6 = WPC1251, 7 = CP866.
  if (codePage == 6) return utf8ToCp1251(utf8);
  if (codePage == 7) return utf8ToCp866(utf8);
  return utf8ToAscii(utf8);
}

void printerSetSize(char size) {
  switch (toupper(size)) {
    case 'M':
      setPrintModeBit(1 << 4, true);   // double height
      setPrintModeBit(1 << 5, false);  // normal width
      break;
    case 'L':
      setPrintModeBit(1 << 4, true);
      setPrintModeBit(1 << 5, true);
      break;
    default:  // S
      setPrintModeBit(1 << 4, false);
      setPrintModeBit(1 << 5, false);
      break;
  }
}

void printerSetJustify(char align) {
  const uint8_t a = (toupper(align) == 'C') ? 1 : (toupper(align) == 'R') ? 2 : 0;
  const uint8_t cmd[] = {0x1B, 0x61, a};  // ESC a n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetBold(bool on) { setPrintModeBit(1 << 3, on); }

void printerSetUnderline(uint8_t weight) {
  if (weight > 2) weight = 2;
  const uint8_t cmd[] = {0x1B, 0x2D, weight};  // ESC - n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetInverse(bool on) {
  const uint8_t cmd[] = {0x1D, 0x42, (uint8_t)(on ? 1 : 0)};  // GS B n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetDoubleHeight(bool on) { setPrintModeBit(1 << 4, on); }
void printerSetDoubleWidth(bool on) { setPrintModeBit(1 << 5, on); }
void printerSetStrike(bool on) { setPrintModeBit(1 << 6, on); }

void printerSetUpsideDown(bool on) {
  const uint8_t cmd[] = {0x1B, 0x7B, (uint8_t)(on ? 1 : 0)};  // ESC { n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetFont(char font) {
  setPrintModeBit(1 << 0, toupper(font) == 'B');
}

void printerSetLineHeight(uint8_t dots) {
  if (dots < 24) dots = 24;
  const uint8_t cmd[] = {0x1B, 0x33, dots};  // ESC 3 n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetCharSpacing(uint8_t dots) {
  const uint8_t cmd[] = {0x1B, 0x20, dots};  // ESC SP n
  writeBytes(cmd, sizeof(cmd));
}

void printerTab() { writeByte(0x09); }

void printerFeedRows(uint8_t dots) {
  const uint8_t cmd[] = {0x1B, 0x4A, dots};  // ESC J n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetCodePage(uint8_t page) {
  if (page > 47) page = 47;
  const uint8_t cmd[] = {0x1B, 0x74, page};  // ESC t n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetCharset(uint8_t charset) {
  if (charset > 15) charset = 15;
  const uint8_t cmd[] = {0x1B, 0x52, charset};  // ESC R n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetBarcodeHeight(uint8_t height) {
  if (height < 1) height = 1;
  const uint8_t cmd[] = {0x1D, 0x68, height};  // GS h n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetPrintDensity(uint8_t density, uint8_t breakTime) {
  if (density > 31) density = 31;
  if (breakTime > 7) breakTime = 7;
  const uint8_t cmd[] = {0x12, 0x23,
                         (uint8_t)(((breakTime & 0x07) << 5) | (density & 0x1F))};
  writeBytes(cmd, sizeof(cmd));
}

void printerSetHeatConfig(uint8_t dots, uint8_t time, uint8_t interval) {
  const uint8_t cmd[] = {0x1B, 0x37, dots, time, interval};  // ESC 7
  writeBytes(cmd, sizeof(cmd));
}

void printerSleepAfter(uint16_t seconds) {
  const uint8_t cmd[] = {0x1B, 0x38, (uint8_t)(seconds & 0xFF),
                         (uint8_t)(seconds >> 8)};  // ESC 8 nL nH
  writeBytes(cmd, sizeof(cmd));
}

void printerWake() {
  // ESC 8 0 0 (как в Adafruit_Thermal) на этой плате усыпляет принтер и
  // даёт лишний символ при пробуждении, а 0xFF печатается как «я».
  // Будим сбросом — он же выводит принтер из сна.
  const uint8_t reset[] = {0x1B, 0x40};  // ESC @
  writeBytes(reset, sizeof(reset));
}

// Диагностика: последние байты, реально ушедшие в UART принтера.
void printerTxLogReset() { txLogLen = 0; }

size_t printerTxLog(uint8_t* out, size_t cap) {
  const size_t n = txLogLen < cap ? txLogLen : cap;
  memcpy(out, txLog, n);
  return n;
}

void printerOnline(bool on) {
  const uint8_t cmd[] = {0x1B, 0x3D, (uint8_t)(on ? 1 : 0)};  // ESC = n
  writeBytes(cmd, sizeof(cmd));
}

void printerSetDefault() {
  printMode = 0;
  writePrintMode();
  printerOnline(true);
  printerSetJustify('L');
  printerSetInverse(false);
  printerSetUnderline(0);
  printerSetLineHeight(30);
  printerSetCharSpacing(0);
  printerSetBarcodeHeight(50);
  printerSetCharset(0);
  printerSetUpsideDown(false);
}

void printerBitmapHeader(uint16_t width, uint16_t height) {
  if (width == 0 || width > PRINTER_WIDTH) return;
  const uint16_t bytesPerRow = (width + 7) / 8;
  const uint8_t cmd[] = {
      0x1D, 0x76, 0x30, 0x00,                  // GS v 0 m=0
      (uint8_t)(bytesPerRow & 0xFF), (uint8_t)(bytesPerRow >> 8),  // xL, xH
      (uint8_t)(height & 0xFF), (uint8_t)(height >> 8)  // yL, yH
  };
  writeBytes(cmd, sizeof(cmd));
}

bool printerWrite(const uint8_t* data, size_t len) {
  return writeBytes(data, len);
}

bool printerQrInline(const String& data, uint8_t moduleSize) {
  if (data.length() == 0) return true;
  if (moduleSize < 1) moduleSize = 1;
  if (moduleSize > 8) moduleSize = 8;

  const uint8_t model[] = {0x1D, 0x28, 0x6B, 0x04, 0x00, 0x31, 0x41, 0x32, 0x00};
  if (!writeBytes(model, sizeof(model))) return false;

  const uint8_t msize[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x43, moduleSize};
  if (!writeBytes(msize, sizeof(msize))) return false;

  const uint8_t ecc[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x45, 48};  // M
  if (!writeBytes(ecc, sizeof(ecc))) return false;

  const size_t len = data.length();
  const uint16_t p = (uint16_t)(len + 3);
  const uint8_t store[] = {0x1D, 0x28, 0x6B, (uint8_t)(p & 0xFF),
                           (uint8_t)(p >> 8), 0x31, 0x50, 0x30};
  if (!writeBytes(store, sizeof(store))) return false;
  if (!writeBytes((const uint8_t*)data.c_str(), len)) return false;

  const uint8_t printCmd[] = {0x1D, 0x28, 0x6B, 0x03, 0x00, 0x31, 0x51, 0x30};
  return writeBytes(printCmd, sizeof(printCmd));
}

bool printerQr(const String& data, uint8_t moduleSize, uint8_t feed) {
  if (data.length() == 0) return true;

  if (!printerBeginJob()) return false;
  if (!printerQrInline(data, moduleSize)) return false;

  if (feed) {
    const uint8_t feedCmd[] = {0x1B, 0x64, feed};
    if (!writeBytes(feedCmd, sizeof(feedCmd))) return false;
  }
  return true;
}

bool printerBarcode(const String& type, const String& data, uint8_t height,
                    uint8_t width, uint8_t feed) {
  if (data.length() == 0) return true;
  if (height < 1) height = 60;
  if (width < 1) width = 2;
  if (width > 6) width = 6;

  String t = type;
  t.toUpperCase();

  uint8_t m = 0;
  String d = data;

  if (t == "CODE128" || t == "CODE-128") {
    m = 73;
    if (!d.startsWith("{")) d = "{B" + d;
  } else if (t == "EAN13" || t == "EAN-13") {
    m = 67;
  } else if (t == "EAN8" || t == "EAN-8") {
    m = 68;
  } else if (t == "CODE39" || t == "CODE-39") {
    m = 4;
  } else if (t == "ITF") {
    m = 5;
  } else if (t == "CODABAR") {
    m = 6;
  } else if (t == "UPCA" || t == "UPC-A") {
    m = 65;
  } else if (t == "UPCE" || t == "UPC-E") {
    m = 66;
  } else {
    return false;
  }

  if (d.length() > 255) return false;

  if (!printerBeginJob()) return false;

  const uint8_t h[] = {0x1D, 0x68, height};  // GS h n
  if (!writeBytes(h, sizeof(h))) return false;
  const uint8_t w[] = {0x1D, 0x77, width};   // GS w n
  if (!writeBytes(w, sizeof(w))) return false;
  const uint8_t hri[] = {0x1D, 0x48, 2};     // GS H 2 — цифры под штрихкодом
  if (!writeBytes(hri, sizeof(hri))) return false;
  const uint8_t font[] = {0x1D, 0x66, 0};    // GS f 0
  if (!writeBytes(font, sizeof(font))) return false;

  const uint8_t header[] = {0x1D, 0x6B, m, (uint8_t)d.length()};  // GS k m n
  if (!writeBytes(header, sizeof(header))) return false;
  if (!writeBytes((const uint8_t*)d.c_str(), d.length())) return false;

  if (feed) {
    const uint8_t feedCmd[] = {0x1B, 0x64, feed};
    if (!writeBytes(feedCmd, sizeof(feedCmd))) return false;
  }
  return true;
}

bool printerTestPage() {
  if (!printerText("Thermal58", 3, true, false, 'C', 1)) return false;
  if (!printerText("Wi-Fi print server", 1, false, false, 'C', 1)) return false;
  if (!printerText("--------------------------------", 1, false, false, 'L', 0)) return false;
  if (!printerText("Русский текст: Привет, мир!", 1, false, false, 'L', 1)) return false;
  if (!printerText("384 точки, ESC/POS, CP1251", 1, false, false, 'L', 1)) return false;
  if (!printerText("http://" + WiFi.localIP().toString(), 1, false, false, 'L', 2)) return false;
  return printerQr("http://" + WiFi.localIP().toString(), 4, 3);
}

PrinterStatus printerQueryStatus() {
  PrinterStatus st;

  flushInput();
  const uint8_t cmd1[] = {0x10, 0x04, 0x01};  // DLE EOT 1
  writeBytes(cmd1, sizeof(cmd1));
  const int b1 = readByte(300);

  flushInput();
  const uint8_t cmd2[] = {0x10, 0x04, 0x02};  // DLE EOT 2
  writeBytes(cmd2, sizeof(cmd2));
  const int b2 = readByte(300);

  st.ok = (b1 >= 0) || (b2 >= 0);
  st.status = (b1 >= 0) ? (uint8_t)b1 : 0;
  st.paperRaw = (b2 >= 0) ? (uint8_t)b2 : 0;
  // Бит 3 у DLE EOT 2 обычно означает «нет бумаги».
  st.paper = (b2 >= 0) ? ((b2 & 0x08) == 0) : true;
  return st;
}
