#pragma once

#include <Arduino.h>

enum class SdFileKind : uint8_t { Invalid, Image, Raw, Text, Plain };

bool sdBegin();
void sdEnd();
bool sdMounted();
uint64_t sdTotalBytes();
uint64_t sdUsedBytes();
uint8_t sdCardType();

String sdSanitizeName(const String& input);
bool sdIsAllowedName(const String& name);
SdFileKind sdFileKind(const String& name);
const char* sdKindName(SdFileKind kind);
bool sdIsRawFile(const String& name);
bool sdIsTextFile(const String& name);
String sdPath(const String& name);
bool sdExists(const String& name);
bool sdRemove(const String& name);
size_t sdFileSize(const String& name);

// Читает заголовок 1-битного файла: width, height, размер данных.
bool sdReadBitmapHeader(const String& name, uint16_t& width, uint16_t& height,
                        uint32_t& dataBytes);

bool sdPrintBitmap(const String& name, uint8_t feed, String& error);
bool sdPrintBitmapInline(const String& name, String& error);
bool sdPrintRaw(const String& name, String& error);
bool sdPrintText(const String& name, uint8_t feed, String& error);
bool sdPrintPlainText(const String& name, uint8_t feed, String& error);
