#pragma once

#include <Arduino.h>

struct PrinterStatus {
  bool ok = false;
  bool paper = true;
  uint8_t status = 0;
  uint8_t paperRaw = 0;
};

void printerBegin();
void printerLoadSettings();

bool printerBeginJob();
void printerReset();
bool printerFeed(uint8_t lines);
void printerSetHeat(uint8_t heat);
uint8_t printerHeat();
bool printerDtrActive();

bool printerText(const String& utf8, uint8_t size, bool bold, bool underline,
                 char align, uint8_t feed);
String printerEncodeText(const String& utf8, uint8_t codePage);

// Форматирование (ESC/POS, как в Adafruit_Thermal)
void printerSetSize(char size);
void printerSetJustify(char align);
void printerSetBold(bool on);
void printerSetUnderline(uint8_t weight);
void printerSetInverse(bool on);
void printerSetDoubleHeight(bool on);
void printerSetDoubleWidth(bool on);
void printerSetStrike(bool on);
void printerSetUpsideDown(bool on);
void printerSetFont(char font);
void printerSetLineHeight(uint8_t dots);
void printerSetCharSpacing(uint8_t dots);
void printerTab();
void printerFeedRows(uint8_t dots);
void printerSetCodePage(uint8_t page);
void printerSetCharset(uint8_t charset);
void printerSetBarcodeHeight(uint8_t height);
void printerSetPrintDensity(uint8_t density, uint8_t breakTime);
void printerSetHeatConfig(uint8_t dots, uint8_t time, uint8_t interval);
void printerSleepAfter(uint16_t seconds);
void printerWake();
void printerOnline(bool on);
void printerSetDefault();
void printerBitmapHeader(uint16_t width, uint16_t height);
bool printerWrite(const uint8_t* data, size_t len);
bool printerQr(const String& data, uint8_t moduleSize, uint8_t feed);
bool printerBarcode(const String& type, const String& data, uint8_t height,
                    uint8_t width, uint8_t feed);
bool printerTestPage();
PrinterStatus printerQueryStatus();
