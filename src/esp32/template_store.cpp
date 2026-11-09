#include "template_store.h"

#include <SD.h>

#include "config.h"
#include "sd_store.h"

bool templatesLoad(JsonDocument& doc) {
  doc.clear();
  doc.to<JsonObject>();

  if (!sdMounted()) return false;
  if (!SD.exists(TEMPLATES_PATH)) return true;

  File f = SD.open(TEMPLATES_PATH, FILE_READ);
  if (!f) return false;
  if (f.size() > MAX_TEMPLATES_FILE) {
    f.close();
    return false;
  }

  const DeserializationError err = deserializeJson(doc, f);
  f.close();

  if (err || !doc.is<JsonObject>()) {
    doc.clear();
    doc.to<JsonObject>();
    return false;
  }
  return true;
}

bool templatesSave(JsonDocument& doc) {
  if (!sdMounted()) return false;
  const size_t expected = measureJson(doc);
  if (expected > MAX_TEMPLATES_FILE) return false;

  File f = SD.open(TEMPLATES_TMP_PATH, FILE_WRITE);
  if (!f) return false;

  const size_t written = serializeJson(doc, f);
  f.close();

  if (written != expected || expected == 0) {
    SD.remove(TEMPLATES_TMP_PATH);
    return false;
  }

  if (SD.exists(TEMPLATES_PATH)) SD.remove(TEMPLATES_PATH);
  if (!SD.rename(TEMPLATES_TMP_PATH, TEMPLATES_PATH)) {
    SD.remove(TEMPLATES_TMP_PATH);
    return false;
  }
  return true;
}
