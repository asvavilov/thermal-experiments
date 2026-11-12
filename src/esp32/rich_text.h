#pragma once

#include <Arduino.h>

// Потоковый печатающий разборщик текста с последовательными тегами:
// [setSize,L]...[boldOn]...[boldOff], закрывающие теги не нужны.
// Позволяет печатать файлы с SD по частям, не загружая их целиком в RAM.
class RichTextPrinter {
 public:
  bool begin(String& error);
  bool beginPlain(String& error);
  bool write(const char* data, size_t len, String& error);
  bool end(uint8_t feed, String& error);

 private:
  bool process(String& error);
  bool flushSegment(bool force, String& error);

  String pending_;
  String segment_;
  uint8_t codePage_ = 46;
  bool printed_ = false;
  bool lastWasFeed_ = false;
  bool parseTags_ = true;
  char lastChar_ = 0;
};

// Печатает текст с тегами за один вызов.
bool printerRichText(const String& markup, uint8_t feed, String& error);

// Печатает текст как есть: квадратные скобки не разбираются.
bool printerPlainText(const String& text, uint8_t feed, String& error);
