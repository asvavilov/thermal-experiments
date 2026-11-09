#include "printer.h"

#include <Preferences.h>
#include <WiFi.h>

#include "config.h"

namespace {

Preferences prefs;
bool prefsReady = false;
uint8_t heatValue = DEFAULT_HEAT;
bool dtrPresent = false;

// ---------------------------------------------------------------------------
// UTF-8 -> CP1251
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
    case 0x00A0: out += (char)0xA0; return;  // NBSP
    case 0x00B0: out += (char)0xB0; return;  // °
    case 0x00B7: out += (char)0xB7; return;  // ·
    default:     out += '?'; return;
  }
}

String utf8ToCp1251(const String& src) {
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

    appendCp1251(out, cp);
    i += len;
  }
  return out;
}

// ---------------------------------------------------------------------------
// Низкоуровневая передача
// ---------------------------------------------------------------------------

bool waitReady() {
#if USE_DTR
  if (!dtrPresent) return true;
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
    if (n > 64) n = 64;
    const size_t written = Serial2.write(data + off, n);
    if (written == 0) {
      delay(1);
      continue;
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

}  // namespace

// ---------------------------------------------------------------------------
// Публичный API
// ---------------------------------------------------------------------------

void printerBegin() {
  Serial2.begin(PRINTER_BAUD, SERIAL_8N1, PIN_PRINTER_RX, PIN_PRINTER_TX);
  pinMode(PIN_PRINTER_DTR, INPUT_PULLUP);
  printerLoadSettings();
  delay(100);

  // Определяем, подключён ли DTR. Если линия не подключена, INPUT_PULLUP
  // держит HIGH; тогда работаем с паузами, а не ждём готовности.
  const uint32_t probeStart = millis();
  while (digitalRead(PIN_PRINTER_DTR) != DTR_READY_LEVEL &&
         millis() - probeStart < DTR_BOOT_PROBE_MS) {
    delay(1);
  }
  dtrPresent = digitalRead(PIN_PRINTER_DTR) == DTR_READY_LEVEL;
  Serial.println(dtrPresent ? "[printer] DTR detected"
                            : "[printer] DTR not detected, using delays");

  // Сброс и включение аппаратного управления потоком по DTR.
  // GS a 0x20 — так делает Adafruit_Thermal при использовании DTR.
  const uint8_t reset[] = {0x1B, 0x40};
  writeBytes(reset, sizeof(reset));
  delay(100);
  const uint8_t dtrOn[] = {0x1D, 0x61, 0x20};
  writeBytes(dtrOn, sizeof(dtrOn));
  delay(50);
  if (!dtrPresent) {
    dtrPresent = digitalRead(PIN_PRINTER_DTR) == DTR_READY_LEVEL;
    if (dtrPresent) Serial.println("[printer] DTR detected after init");
  }
}

void printerLoadSettings() {
  prefs.begin("printer", false);
  prefsReady = true;
  heatValue = prefs.getUChar("heat", DEFAULT_HEAT);
  if (heatValue == 0) heatValue = DEFAULT_HEAT;
}

bool printerBeginJob() {
  const uint8_t reset[] = {0x1B, 0x40};  // ESC @
  if (!writeBytes(reset, sizeof(reset))) return false;
  delay(50);

  // Включить аппаратное управление потоком по DTR (как в Adafruit_Thermal).
  const uint8_t dtrOn[] = {0x1D, 0x61, 0x20};  // GS a 0x20
  if (!writeBytes(dtrOn, sizeof(dtrOn))) return false;

  const uint8_t codepage[] = {0x1B, 0x74, 46};  // ESC t 46 = WPC1251
  if (!writeBytes(codepage, sizeof(codepage))) return false;

  const uint8_t heatCmd[] = {0x1B, 0x37, 11, heatValue, 40};  // ESC 7 dots,time,interval
  if (!writeBytes(heatCmd, sizeof(heatCmd))) return false;

  const uint8_t lineSpacing[] = {0x1B, 0x32};  // ESC 2 — интервал по умолчанию
  return writeBytes(lineSpacing, sizeof(lineSpacing));
}

void printerReset() {
  const uint8_t cmd[] = {0x1B, 0x40};
  writeBytes(cmd, sizeof(cmd));
}

bool printerFeed(uint8_t lines) {
  if (lines == 0) return true;
  const uint8_t cmd[] = {0x1B, 0x64, lines};  // ESC d n
  return writeBytes(cmd, sizeof(cmd));
}

void printerSetHeat(uint8_t heat) {
  if (heat < 1) heat = 1;
  heatValue = heat;
  if (prefsReady) prefs.putUChar("heat", heatValue);
}

uint8_t printerHeat() { return heatValue; }

bool printerDtrActive() { return dtrPresent; }

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

  const String cp = utf8ToCp1251(normalized);
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

bool printerQr(const String& data, uint8_t moduleSize, uint8_t feed) {
  if (data.length() == 0) return true;
  if (moduleSize < 1) moduleSize = 1;
  if (moduleSize > 8) moduleSize = 8;

  if (!printerBeginJob()) return false;

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
  if (!writeBytes(printCmd, sizeof(printCmd))) return false;

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
