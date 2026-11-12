#pragma once

// ---------------------------------------------------------------------------
// Прошивка Wi-Fi принтера на ESP32.
// Принтер: GZQIANJI T7-US, 58 мм, 384 точки, ESC/POS, UART TTL.
// ---------------------------------------------------------------------------

#define FW_NAME "thermal"
#define FW_VERSION "1.4.6"

// --- Принтер ---------------------------------------------------------------
#define PRINTER_BAUD 9600
// Кодовая страница ESC t по таблице Adafruit/Epson (её же понимает T7-US):
// 6 = WPC1251, 7 = CP866. Внимание: 46 — это Hebrew CP856, а 17 — Greek
// WCP1253, поэтому кириллица на них получается «кракозябрами».
// Переключается в настройках (POST /api/config {"codePage":7}).
#define DEFAULT_CODEPAGE 6
// Аппаратное управление потоком по DTR. Можно выключить через
// POST /api/config {"dtr":false}, если принтер дёргает линию и печать
// идёт рывками.
#define DEFAULT_USE_DTR 1
#define PIN_PRINTER_RX 16   // ESP32 RX2  <- TX принтера (через делитель)
#define PIN_PRINTER_TX 17   // ESP32 TX2  -> RX принтера
#define PIN_PRINTER_DTR 4   // ESP32 GPIO <- DTR принтера (через делитель)

#define USE_DTR 1
// DTR принтера — активный низкий уровень: LOW = готов принимать данные.
// Так же это делает библиотека Adafruit_Thermal (INPUT_PULLUP + ожидание LOW).
#define DTR_READY_LEVEL LOW
#define DTR_BOOT_PROBE_MS 3000
#define DTR_WAIT_TIMEOUT_MS 30000

#define PRINTER_WIDTH 384
#define PRINTER_BYTES_PER_ROW (PRINTER_WIDTH / 8)
#define MAX_BITMAP_HEIGHT 2000
#define MAX_BITMAP_BYTES (4 + PRINTER_BYTES_PER_ROW * MAX_BITMAP_HEIGHT)

// --- SD-карта (SPI / VSPI) -------------------------------------------------
#define PIN_SD_CS 21
#define PIN_SD_SCK 18
#define PIN_SD_MISO 19
#define PIN_SD_MOSI 23
#define SD_SPI_HZ 4000000
#define SD_MOUNT "/sd"
#define MAX_SD_FILE_BYTES (512 * 1024)
#define MAX_SD_FILES 64
// Исходные картинки (.png/.jpg/...) хранятся на SD как есть и могут быть
// заметно больше готового 1-битного .bin.
#define MAX_SD_SOURCE_BYTES (8 * 1024 * 1024)

// --- Текстовые файлы на SD -------------------------------------------------
#define MAX_SD_TEXT_BYTES (64 * 1024)
#define MAX_RICH_TAG_LEN 64

#define DEFAULT_HEAT 120
#define DEFAULT_FEED 2

// --- Сеть ------------------------------------------------------------------
#define HOSTNAME "thermal"
#define WIFI_AP_NAME "Thermal58-setup"
#define WIFI_AP_PASS "thermal58"
#define WIFI_CONNECT_TIMEOUT_S 20
#define WIFI_PORTAL_TIMEOUT_S 180

// --- Служебные пины ---------------------------------------------------------
#define PIN_LED 2
#define LED_ACTIVE_HIGH 1

// --- Ограничения API --------------------------------------------------------
#define MAX_TEXT_LEN 4096
#define MAX_QR_LEN 700
#define MAX_JSON_BODY 16384
