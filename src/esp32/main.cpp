#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>

#include "config.h"
#include "printer.h"
#include "rich_text.h"
#include "sd_store.h"
#include "web_ui.h"

WebServer server(80);
WiFiManager wm;

// ---------------------------------------------------------------------------
// Состояние потоковых загрузок (multipart/form-data)
// ---------------------------------------------------------------------------

struct BitmapJob {
  bool active = false;
  bool started = false;
  bool completed = false;
  bool error = false;
  char errorMsg[96] = {0};
  uint8_t header[4] = {0};
  uint8_t headerCount = 0;
  uint16_t width = 0;
  uint16_t height = 0;
  uint32_t expected = 0;
  uint32_t received = 0;
};

struct RawJob {
  bool active = false;
  bool completed = false;
  bool error = false;
  uint32_t bytes = 0;
  char errorMsg[96] = {0};
};

struct SdUploadJob {
  bool active = false;
  bool completed = false;
  bool error = false;
  bool opened = false;
  char errorMsg[96] = {0};
  File file;
  String name;
  String tmpName;
  uint32_t bytes = 0;
  uint32_t expected = 0;
  uint8_t header[4] = {0};
  uint8_t headerCount = 0;
  bool headerChecked = false;
  SdFileKind kind = SdFileKind::Invalid;
};

BitmapJob bmp;
RawJob rawJob;
SdUploadJob sdJob;

// ---------------------------------------------------------------------------
// Вспомогательные функции
// ---------------------------------------------------------------------------

bool parseJsonBody(JsonDocument& doc, String& body) {
  body = server.arg("plain");
  if (body.length() == 0 || body.length() > MAX_JSON_BODY) return false;
  return deserializeJson(doc, body) == DeserializationError::Ok;
}

uint8_t sizeFromString(const String& value, uint8_t def) {
  String s = value;
  s.trim();
  s.toUpperCase();
  if (s == "S") return 1;
  if (s == "M") return 2;
  if (s == "L") return 3;
  int n = s.toInt();
  if (n >= 1 && n <= 3) return n;
  return def;
}

char alignFromString(const String& value, char def) {
  String s = value;
  s.trim();
  s.toUpperCase();
  if (s == "L") return 'L';
  if (s == "C") return 'C';
  if (s == "R") return 'R';
  return def;
}

bool boolFromString(const String& value) {
  String s = value;
  s.trim();
  s.toLowerCase();
  return s == "1" || s == "true" || s == "on" || s == "yes";
}

bool barcodeSupported(const String& type) {
  String t = type;
  t.toUpperCase();
  return t == "CODE128" || t == "CODE-128" || t == "EAN13" || t == "EAN-13" ||
         t == "EAN8" || t == "EAN-8" || t == "CODE39" || t == "CODE-39" ||
         t == "ITF" || t == "CODABAR" || t == "UPCA" || t == "UPC-A" ||
         t == "UPCE" || t == "UPC-E";
}

String applyInitialStyle(const String& text, uint8_t size, char align,
                         bool bold, bool underline) {
  String markup;
  markup.reserve(text.length() + 48);
  if (size >= 2) {
    markup += "[setSize,";
    markup += (size == 2) ? 'M' : 'L';
    markup += "]";
  }
  if (align == 'C' || align == 'R') {
    markup += "[justify,";
    markup += align;
    markup += "]";
  }
  if (bold) markup += "[boldOn]";
  if (underline) markup += "[underlineOn]";
  markup += text;
  return markup;
}

void sendJson(int code, JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json; charset=utf-8", out);
}

void sendOk(const char* message = nullptr) {
  JsonDocument doc;
  doc["ok"] = true;
  if (message) doc["message"] = message;
  sendJson(200, doc);
}

void sendError(int code, const char* message) {
  JsonDocument doc;
  doc["ok"] = false;
  doc["error"] = message;
  sendJson(code, doc);
}

// ---------------------------------------------------------------------------
// HTTP: страница и информация
// ---------------------------------------------------------------------------

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleStatus() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["name"] = FW_NAME;
  doc["version"] = FW_VERSION;
  doc["ip"] = WiFi.localIP().toString();
  doc["ssid"] = WiFi.SSID();
  doc["rssi"] = WiFi.RSSI();
  doc["hostname"] = HOSTNAME;
  doc["mdns"] = String(HOSTNAME) + ".local";
  doc["mac"] = WiFi.macAddress();
  doc["uptime"] = (uint32_t)(millis() / 1000);
  doc["heap"] = ESP.getFreeHeap();
  doc["baud"] = PRINTER_BAUD;
  doc["width"] = PRINTER_WIDTH;
  doc["maxHeight"] = MAX_BITMAP_HEIGHT;
  doc["heat"] = printerHeat();
  doc["dtr"] = printerDtrActive();
  sendJson(200, doc);
}

void handleInfo() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["name"] = FW_NAME;
  doc["version"] = FW_VERSION;
  doc["width"] = PRINTER_WIDTH;
  doc["maxHeight"] = MAX_BITMAP_HEIGHT;
  doc["baud"] = PRINTER_BAUD;
  doc["bitmapFormat"] = "uint16 width LE, uint16 height LE, packed 1-bit rows (MSB first), 1 = black";
  JsonArray endpoints = doc["endpoints"].to<JsonArray>();
  endpoints.add("GET  /api/status");
  endpoints.add("GET  /api/info");
  endpoints.add("GET  /api/config");
  endpoints.add("POST /api/config");
  endpoints.add("GET  /api/printer/status");
  endpoints.add("POST /api/text");
  endpoints.add("POST /api/bitmap (multipart file)");
  endpoints.add("POST /api/qr");
  endpoints.add("POST /api/barcode");
  endpoints.add("POST /api/feed");
  endpoints.add("POST /api/raw (multipart file)");
  endpoints.add("POST /api/test");
  endpoints.add("GET  /api/print?text=...");
  endpoints.add("POST /api/restart");
  endpoints.add("POST /api/wifi/reset");
  endpoints.add("GET  /api/sd/list");
  endpoints.add("GET  /api/sd/file?name=...");
  endpoints.add("POST /api/sd/upload (multipart file)");
  endpoints.add("POST /api/sd/delete");
  endpoints.add("POST /api/sd/print");
  endpoints.add("POST /api/sd/remount");
  sendJson(200, doc);
}

void handleGetConfig() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["heat"] = printerHeat();
  doc["baud"] = PRINTER_BAUD;
  doc["width"] = PRINTER_WIDTH;
  doc["maxHeight"] = MAX_BITMAP_HEIGHT;
  doc["dtr"] = printerDtrActive();
  sendJson(200, doc);
}

void handlePostConfig() {
  JsonDocument doc;
  String body;
  if (!parseJsonBody(doc, body)) return sendError(400, "invalid json");

  if (doc["heat"].is<int>()) {
    int heat = doc["heat"].as<int>();
    if (heat < 1) heat = 1;
    if (heat > 255) heat = 255;
    printerSetHeat((uint8_t)heat);
  }

  JsonDocument out;
  out["ok"] = true;
  out["heat"] = printerHeat();
  out["baud"] = PRINTER_BAUD;
  out["dtr"] = printerDtrActive();
  sendJson(200, out);
}

void handlePrinterStatus() {
  const PrinterStatus st = printerQueryStatus();
  JsonDocument doc;
  doc["ok"] = true;
  doc["answered"] = st.ok;
  doc["paper"] = st.paper;
  doc["status"] = st.status;
  doc["paperRaw"] = st.paperRaw;
  doc["dtr"] = printerDtrActive();
  sendJson(200, doc);
}

// ---------------------------------------------------------------------------
// HTTP: печать
// ---------------------------------------------------------------------------

void handleText() {
  String body = server.arg("plain");
  if (body.length() == 0) return sendError(400, "empty body");
  if (body.length() > MAX_JSON_BODY) return sendError(413, "body too large");

  String text;
  uint8_t size = 1;
  bool bold = false;
  bool underline = false;
  char align = 'L';
  uint8_t feed = DEFAULT_FEED;

  String trimmed = body;
  trimmed.trim();
  if (trimmed.startsWith("{")) {
    JsonDocument doc;
    if (deserializeJson(doc, trimmed) != DeserializationError::Ok) {
      return sendError(400, "invalid json");
    }
    text = doc["text"].is<const char*>() ? doc["text"].as<String>() : String();
    size = sizeFromString(doc["size"].as<String>(), 1);
    bold = doc["bold"] | false;
    underline = doc["underline"] | false;
    align = alignFromString(doc["align"].as<String>(), 'L');
    int f = doc["feed"] | DEFAULT_FEED;
    if (f < 0) f = 0;
    if (f > 20) f = 20;
    feed = (uint8_t)f;
  } else {
    text = body;
  }

  if (text.length() == 0) return sendError(400, "text is empty");
  if (text.length() > MAX_TEXT_LEN) return sendError(413, "text too long");

  const String markup = applyInitialStyle(text, size, align, bold, underline);
  String error;
  if (!printerRichText(markup, feed, error)) {
    return sendError(503, error.length() ? error.c_str() : "printer not ready");
  }
  sendOk("printed");
}

void handleQr() {
  JsonDocument doc;
  String body;
  if (!parseJsonBody(doc, body)) return sendError(400, "invalid json");

  String data = doc["data"].is<const char*>() ? doc["data"].as<String>() : String();
  if (data.length() == 0) return sendError(400, "data is empty");
  if (data.length() > MAX_QR_LEN) return sendError(413, "data too long");

  int module = doc["module"] | 4;
  if (module < 1) module = 1;
  if (module > 8) module = 8;
  int feed = doc["feed"] | 2;
  if (feed < 0) feed = 0;
  if (feed > 20) feed = 20;

  if (!printerQr(data, (uint8_t)module, (uint8_t)feed)) {
    return sendError(503, "printer not ready");
  }
  sendOk("printed");
}

void handleBarcode() {
  JsonDocument doc;
  String body;
  if (!parseJsonBody(doc, body)) return sendError(400, "invalid json");

  String type = doc["type"] | "CODE128";
  String data = doc["data"].is<const char*>() ? doc["data"].as<String>() : String();
  if (!barcodeSupported(type)) return sendError(400, "unsupported barcode type");
  if (data.length() == 0) return sendError(400, "data is empty");
  if (data.length() > 255) return sendError(413, "data too long");

  int height = doc["height"] | 60;
  if (height < 1) height = 1;
  if (height > 255) height = 255;
  int width = doc["width"] | 2;
  if (width < 1) width = 1;
  if (width > 6) width = 6;
  int feed = doc["feed"] | 2;
  if (feed < 0) feed = 0;
  if (feed > 20) feed = 20;

  if (!printerBarcode(type, data, (uint8_t)height, (uint8_t)width, (uint8_t)feed)) {
    return sendError(503, "printer not ready");
  }
  sendOk("printed");
}

void handleFeed() {
  int lines = 3;
  String body = server.arg("plain");
  String trimmed = body;
  trimmed.trim();
  if (trimmed.startsWith("{")) {
    JsonDocument doc;
    if (deserializeJson(doc, trimmed) != DeserializationError::Ok) {
      return sendError(400, "invalid json");
    }
    lines = doc["lines"] | 3;
  } else if (server.hasArg("lines")) {
    lines = server.arg("lines").toInt();
  }
  if (lines < 1) lines = 1;
  if (lines > 255) lines = 255;
  if (!printerFeed((uint8_t)lines)) return sendError(503, "printer not ready");
  sendOk("fed");
}

void handleTest() {
  if (!printerTestPage()) return sendError(503, "printer not ready");
  sendOk("test page printed");
}

void handlePrintGet() {
  if (server.hasArg("qr")) {
    String data = server.arg("qr");
    if (data.length() == 0) return sendError(400, "qr is empty");
    int module = server.hasArg("module") ? server.arg("module").toInt() : 4;
    if (module < 1) module = 1;
    if (module > 8) module = 8;
    if (!printerQr(data, (uint8_t)module, DEFAULT_FEED)) {
      return sendError(503, "printer not ready");
    }
    return sendOk("printed");
  }

  String text = server.arg("text");
  if (text.length() == 0) return sendError(400, "text is empty");
  if (text.length() > MAX_TEXT_LEN) return sendError(413, "text too long");

  uint8_t size = sizeFromString(server.arg("size"), 1);
  char align = alignFromString(server.arg("align"), 'L');
  bool bold = boolFromString(server.arg("bold"));
  bool underline = boolFromString(server.arg("underline"));
  int feed = server.hasArg("feed") ? server.arg("feed").toInt() : DEFAULT_FEED;
  if (feed < 0) feed = 0;
  if (feed > 20) feed = 20;

  const String markup = applyInitialStyle(text, size, align, bold, underline);
  String error;
  if (!printerRichText(markup, (uint8_t)feed, error)) {
    return sendError(503, error.length() ? error.c_str() : "printer not ready");
  }
  sendOk("printed");
}

// ---------------------------------------------------------------------------
// HTTP: потоковая загрузка изображения
// ---------------------------------------------------------------------------

void handleBitmapUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    bmp = BitmapJob();
    bmp.active = true;
    return;
  }

  if (!bmp.active) return;

  if (upload.status == UPLOAD_FILE_WRITE) {
    size_t i = 0;
    while (bmp.headerCount < 4 && i < upload.currentSize) {
      bmp.header[bmp.headerCount++] = upload.buf[i++];
    }

    if (bmp.headerCount == 4 && !bmp.started && !bmp.error) {
      bmp.width = (uint16_t)bmp.header[0] | ((uint16_t)bmp.header[1] << 8);
      bmp.height = (uint16_t)bmp.header[2] | ((uint16_t)bmp.header[3] << 8);

      if (bmp.width == 0 || bmp.width > PRINTER_WIDTH) {
        bmp.error = true;
        snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "bad width %u", (unsigned)bmp.width);
      } else if (bmp.height == 0 || bmp.height > MAX_BITMAP_HEIGHT) {
        bmp.error = true;
        snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "bad height %u", (unsigned)bmp.height);
      } else {
        bmp.expected = (uint32_t)((bmp.width + 7) / 8) * bmp.height;
        if (!printerBeginJob()) {
          bmp.error = true;
          snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "printer not ready");
        } else {
          printerBitmapHeader(bmp.width, bmp.height);
          bmp.started = true;
        }
      }
    }

    if (bmp.started && !bmp.error && i < upload.currentSize) {
      uint32_t remaining = bmp.expected - bmp.received;
      size_t n = upload.currentSize - i;
      if (n > remaining) n = (size_t)remaining;
      if (n > 0) {
        if (!printerWrite(upload.buf + i, n)) {
          bmp.error = true;
          snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "printer write failed");
        }
        bmp.received += n;
      }
    }
    return;
  }

  if (upload.status == UPLOAD_FILE_END) {
    if (!bmp.error) {
      if (bmp.headerCount < 4) {
        bmp.error = true;
        snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "short header");
      } else if (bmp.received != bmp.expected) {
        bmp.error = true;
        snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "short data %u/%u",
                 (unsigned)bmp.received, (unsigned)bmp.expected);
      }
    }
    bmp.completed = true;
    bmp.active = false;
    return;
  }

  if (upload.status == UPLOAD_FILE_ABORTED) {
    bmp.error = true;
    snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "upload aborted");
    bmp.active = false;
  }
}

void handleBitmapDone() {
  const bool completed = bmp.completed;
  if (!completed && !bmp.error) {
    bmp.error = true;
    snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "no file uploaded");
  }
  bmp.completed = false;
  if (bmp.error) {
    if (bmp.errorMsg[0] == 0) snprintf(bmp.errorMsg, sizeof(bmp.errorMsg), "print failed");
    return sendError(400, bmp.errorMsg);
  }
  sendOk("printed");
}

// ---------------------------------------------------------------------------
// HTTP: сырой ESC/POS
// ---------------------------------------------------------------------------

void handleRawUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    rawJob = RawJob();
    rawJob.active = true;
    return;
  }

  if (!rawJob.active) return;

  if (upload.status == UPLOAD_FILE_WRITE) {
    if (!rawJob.error) {
      if (!printerWrite(upload.buf, upload.currentSize)) {
        rawJob.error = true;
        snprintf(rawJob.errorMsg, sizeof(rawJob.errorMsg), "printer write failed");
      }
    }
    rawJob.bytes += upload.currentSize;
    return;
  }

  if (upload.status == UPLOAD_FILE_END) {
    rawJob.completed = true;
    rawJob.active = false;
    return;
  }

  if (upload.status == UPLOAD_FILE_ABORTED) {
    rawJob.error = true;
    snprintf(rawJob.errorMsg, sizeof(rawJob.errorMsg), "upload aborted");
    rawJob.active = false;
  }
}

void handleRawDone() {
  const bool completed = rawJob.completed;
  if (!completed && !rawJob.error) {
    rawJob.error = true;
    snprintf(rawJob.errorMsg, sizeof(rawJob.errorMsg), "no file uploaded");
  }
  rawJob.completed = false;
  if (rawJob.error) {
    if (rawJob.errorMsg[0] == 0) snprintf(rawJob.errorMsg, sizeof(rawJob.errorMsg), "print failed");
    return sendError(400, rawJob.errorMsg);
  }
  JsonDocument doc;
  doc["ok"] = true;
  doc["bytes"] = rawJob.bytes;
  sendJson(200, doc);
}

// ---------------------------------------------------------------------------
// HTTP: SD-карта
// ---------------------------------------------------------------------------

void sdSetError(const char* message) {
  sdJob.error = true;
  snprintf(sdJob.errorMsg, sizeof(sdJob.errorMsg), "%s", message);
}

void handleSdList() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["mounted"] = sdMounted();
  doc["total"] = 0.0;
  doc["used"] = 0.0;
  doc["free"] = 0.0;
  doc["cardType"] = 0;
  JsonArray files = doc["files"].to<JsonArray>();

  if (sdMounted()) {
    const uint64_t total = sdTotalBytes();
    const uint64_t used = sdUsedBytes();
    doc["total"] = (double)total;
    doc["used"] = (double)used;
    doc["free"] = (double)(total > used ? total - used : 0);
    doc["cardType"] = sdCardType();

    File dir = SD.open("/");
    if (dir) {
      int count = 0;
      File entry = dir.openNextFile();
      while (entry && count < MAX_SD_FILES) {
        String name = entry.name();
        const int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);

        if (!entry.isDirectory() && sdIsAllowedName(name)) {
          const SdFileKind kind = sdFileKind(name);
          JsonObject item = files.add<JsonObject>();
          item["name"] = name;
          item["size"] = (uint32_t)entry.size();
          item["kind"] = sdKindName(kind);
          item["valid"] = true;

          if (kind == SdFileKind::Image) {
            uint16_t width = 0;
            uint16_t height = 0;
            uint32_t dataBytes = 0;
            const bool valid = sdReadBitmapHeader(name, width, height, dataBytes);
            item["valid"] = valid;
            if (valid) {
              item["width"] = width;
              item["height"] = height;
            }
          }
          count++;
        }
        entry.close();
        entry = dir.openNextFile();
      }
      dir.close();
    }
  }

  sendJson(200, doc);
}

void handleSdFile() {
  if (!sdMounted()) return sendError(503, "sd not mounted");

  const String name = server.arg("name");
  if (!sdIsAllowedName(name)) return sendError(400, "bad file name");

  File file = SD.open(sdPath(name), FILE_READ);
  if (!file) return sendError(404, "file not found");

  if (server.hasArg("download")) {
    server.sendHeader("Content-Disposition",
                      String("attachment; filename=\"") + name + "\"");
  }
  const bool text = sdFileKind(name) == SdFileKind::Text ||
                    sdFileKind(name) == SdFileKind::Plain;
  server.streamFile(file, text ? "text/plain; charset=utf-8"
                              : "application/octet-stream");
  file.close();
}

void handleSdUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    sdJob = SdUploadJob();
    sdJob.active = true;

    if (!sdMounted()) return sdSetError("sd not mounted");

    const String rawName = upload.filename;
    String name = sdSanitizeName(rawName);
    // Имя без расширения: если браузер сообщил text/*, сохраняем как .txt.
    if (rawName.lastIndexOf('.') < 0 && upload.type.startsWith("text/")) {
      const int dot = name.lastIndexOf('.');
      if (dot >= 0) name = name.substring(0, dot) + ".txt";
    }
    if (!sdIsAllowedName(name)) return sdSetError("bad file name");

    sdJob.name = name;
    sdJob.tmpName = name + ".tmp";
    sdJob.kind = sdFileKind(name);
    if (sdJob.kind == SdFileKind::Invalid) return sdSetError("bad file type");
    sdJob.file = SD.open(sdPath(sdJob.tmpName), FILE_WRITE);
    if (!sdJob.file) {
      sdSetError("cannot create file");
    } else {
      sdJob.opened = true;
    }
    return;
  }

  if (!sdJob.active) return;

  if (upload.status == UPLOAD_FILE_WRITE) {
    if (sdJob.error) return;

    if ((uint32_t)sdJob.bytes + upload.currentSize > MAX_SD_FILE_BYTES) {
      return sdSetError("file too large");
    }

    if (sdJob.kind == SdFileKind::Text || sdJob.kind == SdFileKind::Plain) {
      if ((uint32_t)sdJob.bytes + upload.currentSize > MAX_SD_TEXT_BYTES) {
        return sdSetError("text file too large");
      }
      for (size_t i = 0; i < upload.currentSize; i++) {
        if (upload.buf[i] == 0) return sdSetError("file is not text");
      }
    }

    if (sdJob.kind == SdFileKind::Image && !sdJob.headerChecked) {
      size_t i = 0;
      while (sdJob.headerCount < 4 && i < upload.currentSize) {
        sdJob.header[sdJob.headerCount++] = upload.buf[i++];
      }
      if (sdJob.headerCount == 4) {
        const uint16_t width =
            (uint16_t)sdJob.header[0] | ((uint16_t)sdJob.header[1] << 8);
        const uint16_t height =
            (uint16_t)sdJob.header[2] | ((uint16_t)sdJob.header[3] << 8);
        if (width == 0 || width > PRINTER_WIDTH || height == 0 ||
            height > MAX_BITMAP_HEIGHT) {
          return sdSetError("bad image size");
        }
        sdJob.expected = (uint32_t)((width + 7) / 8) * height + 4;
        sdJob.headerChecked = true;
      }
    }

    const size_t written = sdJob.file.write(upload.buf, upload.currentSize);
    if (written != upload.currentSize) return sdSetError("write failed");
    sdJob.bytes += upload.currentSize;
    return;
  }

  if (upload.status == UPLOAD_FILE_END) {
    if (sdJob.file) sdJob.file.close();

    if (!sdJob.error) {
      if (sdJob.bytes == 0) {
        sdSetError("empty file");
      } else if (sdJob.kind == SdFileKind::Image) {
        if (!sdJob.headerChecked) {
          sdSetError("short header");
        } else if (sdJob.bytes != sdJob.expected) {
          sdSetError("size mismatch");
        }
      }
    }

    if (sdJob.error) {
      if (sdJob.opened && sdJob.tmpName.length() > 0) {
        SD.remove(sdPath(sdJob.tmpName));
      }
    } else {
      // Записываем во временный файл, чтобы при ошибке не потерять старый.
      if (SD.exists(sdPath(sdJob.name))) SD.remove(sdPath(sdJob.name));
      if (!SD.rename(sdPath(sdJob.tmpName), sdPath(sdJob.name))) {
        sdSetError("rename failed");
        SD.remove(sdPath(sdJob.tmpName));
      }
    }
    sdJob.completed = true;
    sdJob.active = false;
    return;
  }

  if (upload.status == UPLOAD_FILE_ABORTED) {
    if (sdJob.file) sdJob.file.close();
    if (sdJob.opened && sdJob.tmpName.length() > 0) {
      SD.remove(sdPath(sdJob.tmpName));
    }
    sdSetError("upload aborted");
    sdJob.completed = true;
    sdJob.active = false;
  }
}

void handleSdUploadDone() {
  if (sdJob.error) {
    String message = sdJob.errorMsg[0] ? String(sdJob.errorMsg) : String("upload failed");
    sdJob.completed = false;
    return sendError(message == "sd not mounted" ? 503 : 400, message.c_str());
  }
  if (!sdJob.completed) return sendError(400, "no file uploaded");

  JsonDocument doc;
  doc["ok"] = true;
  doc["name"] = sdJob.name;
  doc["size"] = sdJob.bytes;
  doc["kind"] = sdKindName(sdJob.kind);
  sdJob.completed = false;
  sendJson(200, doc);
}

void handleSdDelete() {
  JsonDocument doc;
  String body;
  String name;

  if (parseJsonBody(doc, body)) {
    name = doc["name"].as<String>();
  } else if (server.hasArg("name")) {
    name = server.arg("name");
  } else {
    return sendError(400, "invalid json");
  }

  if (!sdMounted()) return sendError(503, "sd not mounted");
  if (!sdIsAllowedName(name)) return sendError(400, "bad file name");
  if (!sdExists(name)) return sendError(404, "file not found");
  if (!sdRemove(name)) return sendError(500, "delete failed");

  sendOk("deleted");
}

void handleSdPrint() {
  JsonDocument doc;
  String body;
  if (!parseJsonBody(doc, body)) return sendError(400, "invalid json");

  const String name = doc["name"].as<String>();
  if (!sdMounted()) return sendError(503, "sd not mounted");
  if (!sdIsAllowedName(name)) return sendError(400, "bad file name");
  if (!sdExists(name)) return sendError(404, "file not found");

  int feed = doc["feed"] | 2;
  if (feed < 0) feed = 0;
  if (feed > 20) feed = 20;

  String error;
  const SdFileKind kind = sdFileKind(name);
  bool ok = false;
  if (kind == SdFileKind::Raw) {
    ok = sdPrintRaw(name, error);
  } else if (kind == SdFileKind::Text) {
    ok = sdPrintText(name, (uint8_t)feed, error);
  } else if (kind == SdFileKind::Plain) {
    ok = sdPrintPlainText(name, (uint8_t)feed, error);
  } else {
    ok = sdPrintBitmap(name, (uint8_t)feed, error);
  }
  if (!ok) {
    const bool notReady = error == "printer not ready";
    return sendError(notReady ? 503 : 400, error.c_str());
  }

  JsonDocument out;
  out["ok"] = true;
  out["name"] = name;
  out["kind"] = sdKindName(kind);
  out["bytes"] = (uint32_t)sdFileSize(name);
  sendJson(200, out);
}

void handleSdRemount() {
  sdEnd();
  const bool ok = sdBegin();

  JsonDocument doc;
  doc["ok"] = ok;
  doc["mounted"] = ok;
  doc["total"] = (double)sdTotalBytes();
  doc["used"] = (double)sdUsedBytes();
  sendJson(ok ? 200 : 503, doc);
}

// ---------------------------------------------------------------------------
// HTTP: служебные
// ---------------------------------------------------------------------------

void handleRestart() {
  sendOk("restarting");
  delay(300);
  ESP.restart();
}

void handleWifiReset() {
  wm.resetSettings();
  sendOk("wifi settings cleared, restarting");
  delay(300);
  ESP.restart();
}

void handleNotFound() {
  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }
  sendError(404, "not found");
}

// ---------------------------------------------------------------------------
// Инициализация
// ---------------------------------------------------------------------------

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);

  wm.setDebugOutput(false);
  wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_S);
  wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT_S);
  wm.setHostname(HOSTNAME);
  wm.setTitle("Thermal58");

  if (!wm.autoConnect(WIFI_AP_NAME, WIFI_AP_PASS)) {
    Serial.println("[wifi] failed, restarting");
    delay(1000);
    ESP.restart();
  }

  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  Serial.print("[wifi] connected, IP: ");
  Serial.println(WiFi.localIP());
}

void setupMdns() {
  if (MDNS.begin(HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("[mdns] http://" HOSTNAME ".local");
  } else {
    Serial.println("[mdns] failed");
  }
}

void setupOta() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([]() { Serial.println("[ota] start"); });
  ArduinoOTA.onEnd([]() { Serial.println("[ota] end"); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[ota] error %u\n", (unsigned)error);
  });
  ArduinoOTA.begin();
}

void setupSD() {
  if (sdBegin()) {
    Serial.print("[sd] mounted, total bytes: ");
    Serial.println((unsigned long long)sdTotalBytes());
  } else {
    Serial.println("[sd] not mounted");
  }
}

void setupWeb() {
  server.enableCORS(true);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/info", HTTP_GET, handleInfo);
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on("/api/config", HTTP_POST, handlePostConfig);
  server.on("/api/printer/status", HTTP_GET, handlePrinterStatus);
  server.on("/api/text", HTTP_POST, handleText);
  server.on("/api/qr", HTTP_POST, handleQr);
  server.on("/api/barcode", HTTP_POST, handleBarcode);
  server.on("/api/feed", HTTP_POST, handleFeed);
  server.on("/api/test", HTTP_POST, handleTest);
  server.on("/api/print", HTTP_GET, handlePrintGet);
  server.on("/api/bitmap", HTTP_POST, handleBitmapDone, handleBitmapUpload);
  server.on("/api/raw", HTTP_POST, handleRawDone, handleRawUpload);
  server.on("/api/restart", HTTP_POST, handleRestart);
  server.on("/api/wifi/reset", HTTP_POST, handleWifiReset);
  server.on("/api/sd/list", HTTP_GET, handleSdList);
  server.on("/api/sd/file", HTTP_GET, handleSdFile);
  server.on("/api/sd/upload", HTTP_POST, handleSdUploadDone, handleSdUpload);
  server.on("/api/sd/delete", HTTP_POST, handleSdDelete);
  server.on("/api/sd/print", HTTP_POST, handleSdPrint);
  server.on("/api/sd/remount", HTTP_POST, handleSdRemount);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("[http] server started");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== " FW_NAME " " FW_VERSION " ===");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  printerBegin();
  setupSD();
  setupWiFi();
  setupMdns();
  setupOta();
  setupWeb();

  Serial.println("[main] ready");
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();

  // Переподключение, если Wi-Fi пропал.
  static uint32_t lastReconnect = 0;
  if (WiFi.status() != WL_CONNECTED && millis() - lastReconnect > 30000) {
    lastReconnect = millis();
    Serial.println("[wifi] reconnecting...");
    WiFi.reconnect();
  }

  // Индикация: медленное мигание при нормальной работе, частое — без Wi-Fi.
  static uint32_t lastLed = 0;
  static bool ledState = false;
  const uint32_t ledInterval = (WiFi.status() == WL_CONNECTED) ? 1000 : 250;
  if (millis() - lastLed > ledInterval) {
    lastLed = millis();
    ledState = !ledState;
    digitalWrite(PIN_LED, LED_ACTIVE_HIGH ? ledState : !ledState);
  }

  delay(1);
}
