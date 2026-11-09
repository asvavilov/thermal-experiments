#pragma once

#include <Arduino.h>

// Печатает текст с последовательными тегами форматирования:
// [setSize,L]...[boldOn]...[boldOff], закрывающие теги не нужны.
bool printerRichText(const String& markup, uint8_t feed, String& error);
