#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// Загружает /templates.json с SD в объект JSON.
// true — файл прочитан или отсутствует (пустой объект).
bool templatesLoad(JsonDocument& doc);

// Атомарно сохраняет объект в /templates.json через /templates.tmp.
bool templatesSave(JsonDocument& doc);
