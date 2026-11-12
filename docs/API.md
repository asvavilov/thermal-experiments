# REST API

Базовый URL:

- `http://thermal.local` (mDNS, если поддерживается клиентом);
- `http://<IP>` — IP виден в Serial Monitor, в веб-интерфейсе и в
  `GET /api/status`.

Порт — 80. CORS включён: API можно вызывать с других устройств и из
браузерных приложений. Авторизации нет — устройство должно быть в
доверенной локальной сети.

## Общие правила

- Ответы — JSON `application/json; charset=utf-8`.
- Успех: HTTP 200 и `{"ok": true, ...}`.
- Ошибка: HTTP 400/404/413 и `{"ok": false, "error": "..."}`.
- `OPTIONS` на любой путь возвращает 204 (CORS preflight).
- Текстовые поля — UTF-8; прошивка сама конвертирует в CP1251.
- Теги форматирования разбираются только в `.txt` и в `/api/text`;
  `.md`, `.csv`, `.log` печатаются как обычный текст.
- Текстовые файлы на SD — UTF-8 без нулевых байтов.
- Размер изображения: ширина ровно 384 точки (48 байт на строку),
  высота до 2000 строк.

## GET /api/status

Состояние устройства и сети.

```json
{
  "ok": true,
  "name": "thermal",
  "version": "1.0.0",
  "ip": "192.168.1.50",
  "ssid": "HomeWiFi",
  "rssi": -57,
  "hostname": "thermal",
  "mdns": "thermal.local",
  "mac": "AA:BB:CC:DD:EE:FF",
  "uptime": 3725,
  "heap": 234567,
  "baud": 9600,
  "width": 384,
  "maxHeight": 2000,
  "heat": 120,
  "dtr": true
}
```

`dtr` — включён ли аппаратный flow control (если DTR не подключён,
прошивка отключает ожидание после таймаута).

## GET /api/info

Возможности и список эндпоинтов.

```json
{
  "ok": true,
  "name": "thermal",
  "version": "1.0.0",
  "width": 384,
  "maxHeight": 2000,
  "baud": 9600,
  "bitmapFormat": "uint16 width LE, uint16 height LE, packed 1-bit rows (MSB first), 1 = black",
  "endpoints": ["GET  /api/status", "..."]
}
```

## GET /api/config

```json
{"ok":true,"heat":120,"baud":9600,"codePage":6,"width":384,"maxHeight":2000,"dtr":true,"dtrEnabled":true}
```

## POST /api/config

```json
{"heat": 140, "codePage": 6, "baud": 9600, "dtr": true}
```

| Поле | Описание |
|---|---|
| `heat` | время нагрева ESC/POS (`ESC 7`), 1–255 |
| `codePage` | `ESC t`: 6 — CP1251, 7 — CP866 (нумерация Adafruit/Epson; 46 — Hebrew, 17 — Greek) |
| `baud` | 9600 / 19200 / 38400 / 57600 / 115200; должно совпадать с принтером |
| `dtr` | аппаратное управление потоком по DTR; `false` — печать с паузами без ожидания линии |

Все поля необязательны, сохраняются в NVS. Ответ — актуальные настройки.

## POST /api/test/codepage

Печатает по строке на каждую страницу `ESC t` 0…47: метка `NN:` и текст
«Привет, мир!», закодированный в CP1251, плюс контрольную строку
«07/866:» с CP866 на странице 7. На T7-US читаемая строка — `06:`.

```bash
curl.exe -X POST http://thermal.local/api/test/codepage
```

## GET /api/printer/status

Опрос принтера командами `DLE EOT 1` и `DLE EOT 2`.

```json
{
  "ok": true,
  "answered": true,
  "paper": true,
  "status": 0,
  "paperRaw": 0,
  "dtr": true
}
```

- `answered` — принтер ответил хотя бы на один запрос.
- `paper` — эвристика по биту 3 (`paperRaw`): `true` = бумага есть.
- `status`, `paperRaw` — сырые байты для диагностики.

Если принтер не отвечает, `answered` = `false`; это не ошибка HTTP.

## POST /api/test/bytes

Диагностика ESC/POS: печатает по строке на каждую команду, которую прошивка
отправляет в начале задания (`FF:`, `ESC8:`, `ESC@:`, …), затем варианты
полного старта (`JA:`…`JF:`) с паузой в секунду перед первым. Если после
метки появился лишний символ — виновата команда, которая в этом варианте
есть.

## GET /api/debug/tx

Hex последних байт, реально ушедших в UART принтера (до 512 байт).

```json
{"ok":true,"bytes":29,"hex":"1b401b3d011b21001d61201b7406…"}
```

`?reset=1` — очистить лог перед печатью задания.

## POST /api/text

Печать текста. В тексте можно использовать теги форматирования:
`[setSize,L]`, `[boldOn]`, `[boldOff]`, `[justify,R]`, `[inverseOn]` и
другие. Теги выполняются последовательно, закрывающие не нужны. Полный
список — в [`TEXT.md`](TEXT.md).

### JSON

```json
{
  "text": "[setSize,L]ЗАГОЛОВОК\n[setSize,S]Обычный текст, [boldOn]жирное слово[boldOff].",
  "feed": 2
}
```

| Поле | Тип | По умолчанию | Описание |
|---|---|---|---|
| `text` | string | — | Текст с тегами, до 4096 байт UTF-8 |
| `size` | `"S"`/`"M"`/`"L"` или 1–3 | 1 | Начальный размер до разбора тегов |
| `align` | `"L"`/`"C"`/`"R"` | `"L"` | Начальное выравнивание |
| `bold` | bool | false | Начальный жирный |
| `underline` | bool | false | Начальное подчёркивание |
| `feed` | int | 2 | Строк после текста, 0–20 |

Поля `size`, `align`, `bold`, `underline` — только начальное
форматирование. Теги в `text` выполняются после него и перекрывают его.

### Plain text

Можно отправить тело `text/plain` — оно будет напечатано как есть, теги
внутри тоже работают (до 4096 байт).

```bash
curl.exe -X POST http://thermal.local/api/text \
  -H "Content-Type: text/plain; charset=utf-8" \
  --data-binary "Привет, мир!"
```

> В Windows-консоли кириллица может искажаться. Надёжнее сохранить текст
> в UTF-8-файл и отправить `--data-binary @text.txt`.

Ответ: `{"ok":true,"message":"printed"}`.

## POST /api/bitmap

Печать 1-битного изображения. **Только `multipart/form-data`**, поле
`file`. Так файл передаётся потоком и не занимает RAM ESP32 целиком.

Формат файла:

```
uint16 width   (little-endian)   1..384
uint16 height  (little-endian)   1..2000
byte   rows[height][(width+7)/8] MSB first, 1 = чёрный
```

Ширина не обязана быть кратной 8: строки дополняются до границы байта.
Для ширины 384 это 48 байт на строку.

```bash
curl.exe -X POST http://thermal.local/api/bitmap \
  -F "file=@image.bin"
```

Ответ: `{"ok":true,"message":"printed"}`.

### Как подготовить image.bin на Python

```python
from PIL import Image

img = Image.open("input.jpg").convert("L")
img = img.resize((384, max(1, round(img.height * 384 / img.width))))
# 1 = печатать (чёрный); при необходимости добавьте дизеринг
img = img.point(lambda x: 255 if x < 128 else 0, "1")

with open("image.bin", "wb") as f:
    f.write(img.width.to_bytes(2, "little"))
    f.write(img.height.to_bytes(2, "little"))
    f.write(img.tobytes())

print(img.size)
```

Готовый пример — `convert.py` в корне проекта (формат совместим;
`convert.py` пишет `image.raw`, его можно отправлять как `file=@image.raw`).

## POST /api/qr

```json
{"data": "https://example.com", "module": 4, "feed": 2}
```

| Поле | Тип | По умолчанию | Описание |
|---|---|---|---|
| `data` | string | — | До 700 байт |
| `module` | int | 4 | Размер модуля 1–8 |
| `feed` | int | 2 | Строк после QR, 0–20 |

Команды ESC/POS `GS ( k` (модель 2, коррекция M).

## POST /api/barcode

```json
{"type": "CODE128", "data": "Thermal58", "height": 60, "width": 2, "feed": 2}
```

| Поле | Тип | По умолчанию | Описание |
|---|---|---|---|
| `type` | string | `"CODE128"` | CODE128, CODE93, EAN13, EAN8, CODE39, ITF, CODABAR, UPCA, UPCE |
| `data` | string | — | До 255 символов |
| `height` | int | 60 | Высота 1–255 |
| `width` | int | 2 | Ширина 1–6 |
| `feed` | int | 2 | Строк после, 0–20 |

ESC/POS `GS k` со штрихкодовыми `m`: `69/70/71` для CODE39/ITF/CODABAR,
`65/66/67/68` для UPC-A/UPC-E/EAN13/EAN8, `72` — CODE93, `73` — CODE128
(данные CODE128 автоматически оборачиваются в `{B`). Старые коды
`4/5/6` не используются: это формат без байта длины, а прошивка всегда
передаёт длину.

CODE39 и CODE93 кодируют только заглавные латинские буквы и цифры —
нижний регистр в них не пройдёт (`Thermal58` → `THERMAL58`). CODE128
принимает любой ASCII, включая строчные.

## POST /api/test/barcode

Печатает по одному штрихкоду каждого поддерживаемого типа (аналог
`testBars()` из исходного скетча плюс UPC-E): UPC-A, UPC-E, EAN-13,
EAN-8, CODE39, CODE93, ITF, CODABAR, CODE128.

## POST /api/feed

```json
{"lines": 3}
```

Или `POST /api/feed?lines=3`. Диапазон 1–255.

Подача выполняется переводами строки (`0x0A`), а не `ESC d`: на ESP32
команда `ESC d` доходила до принтера, но не исполнялась, пока буфер не
сбрасывался. Перед подачей выполняется сброс задания, поэтому подача
работает и после того, как принтер ушёл в сон.

## POST /api/raw

Сырой ESC/POS, **multipart/form-data**, поле `file`. Полезно для
диагностики и нестандартных команд.

```bash
curl.exe -X POST http://thermal.local/api/raw -F "file=@commands.bin"
```

Ответ: `{"ok":true,"bytes":123}`.

## POST /api/test

Печатает тестовую страницу: заголовок, русский текст, IP и QR.

## POST /api/test/self

Встроенный самотест принтера — `DC2 T` (`0x12 0x54`), тот же, что
`printer.testPage()` в Adafruit_Thermal. Плата печатает свою
конфигурацию: baudrate, heat density, версию прошивки, кодовую страницу.
Именно с такой страницы сняты параметры в `HARDWARE.md`, раздел 1.

```bash
curl.exe -X POST http://thermal.local/api/test
```

## GET /api/print

Быстрая печать текста из URL (удобно для скриптов, Home Assistant,
кнопок, ссылок).

```
GET /api/print?text=Hello&size=L&align=C&bold=1&feed=2
```

Параметры: `text`, `size`, `align`, `bold`, `underline`, `feed`.

`text` поддерживает теги форматирования, как и `POST /api/text`
(см. [`TEXT.md`](TEXT.md)); `size/align/bold/underline` — начальные
значения.

> Для кириллицы в GET-запросе используйте URL-кодирование или
> предпочитайте `POST /api/text` с JSON/UTF-8.

Также можно напечатать QR:

```
GET /api/print?qr=https://example.com&module=4
```

## POST /api/restart

Перезагрузка ESP32. Ответ может не дойти — устройство перезапускается.

## POST /api/wifi/reset

Сбрасывает сохранённые настройки Wi-Fi и перезагружает ESP32. После
перезагрузки поднимается точка доступа `Thermal58-setup` (пароль
`thermal58`).

## SD-карта

Карта монтируется при старте. Без карты остальные функции работают,
SD-эндпоинты возвращают `503 sd not mounted`.

Файлы: `.bin`/`.raw` — 1-битное изображение с заголовком, `.prn` — сырой
ESC/POS, `.txt` — текст UTF-8 с тегами форматирования, `.md`/`.markdown`/
`.csv`/`.log` — обычный текст UTF-8 без разбора тегов. Имена — ASCII, до
48 символов. Лимит загрузки — 512 КБ, текстового файла — 64 КБ. Карта —
FAT32, рекомендуется 4–32 ГБ.

> ESP32 не декодирует JPEG/PNG. Изображение конвертируется в браузере
> (вкладка «Фото» → «Сохранить на SD») или через `convert.py`, после чего
> на карту попадает `.bin`. Файлы других форматов в списке не появляются.

### GET /api/sd/list

```json
{
  "ok": true,
  "mounted": true,
  "total": 8388608,
  "used": 41584,
  "free": 8347024,
  "cardType": 3,
  "files": [
    {"name": "image.raw", "size": 36804, "kind": "image", "valid": true,
     "width": 363, "height": 800},
    {"name": "test.prn", "size": 16, "kind": "raw", "valid": true},
    {"name": "note.txt", "size": 342, "kind": "text", "valid": true},
    {"name": "note.md", "size": 1280, "kind": "plain", "valid": true}
  ]
}
```

`kind` — `image`, `raw`, `text` или `plain`. `text` — наш формат с
тегами, `plain` — обычный текст (`.md`, `.csv`, `.log`). Для изображений
`valid` = `false` означает неправильный заголовок или короткий файл;
печать такого файла будет отклонена.

### GET /api/sd/file?name=...

Отдаёт файл для предпросмотра в браузере или скачивания. Текстовые файлы
отдаются как `text/plain; charset=utf-8`, остальные — как
`application/octet-stream`.

```
GET /api/sd/file?name=image.raw
GET /api/sd/file?name=note.txt
GET /api/sd/file?name=note.txt&download=1
```

### POST /api/sd/upload

`multipart/form-data`, поле `file`. Тип определяется по расширению из
`filename`. Имя приводится к ASCII: кириллица транслитерируется,
недопустимые символы заменяются на `_`, известное расширение
сохраняется.

```bash
curl.exe -X POST http://thermal.local/api/sd/upload -F "file=@image.raw"
curl.exe -X POST http://thermal.local/api/sd/upload -F "file=@note.txt"
```

Для `.bin`/`.raw` проверяется заголовок и точный размер. Для текстовых
файлов проверяется отсутствие нулевых байтов и лимит 64 КБ. При ошибке
временный файл удаляется, старый не страдает. Ответ:

```json
{"ok": true, "name": "note.txt", "size": 342, "kind": "text"}
```

### POST /api/sd/delete

```json
{"name": "note.txt"}
```

Удаляет файл. Ответ: `{"ok":true,"message":"deleted"}`.

### POST /api/sd/print

```json
{"name": "note.txt", "feed": 2}
```

- `.bin`/`.raw` — печатается как изображение, `feed` — строк после;
- `.prn` — сырой ESC/POS, `feed` игнорируется (файл сам управляет);
- `.txt` — текст с тегами, `feed` — строк после;
- `.md`/`.markdown`/`.csv`/`.log` — обычный текст без разбора тегов,
  `feed` — строк после.

Текст читается с карты потоково, поэтому файл не занимает всю RAM и
может быть больше лимита `/api/text` (до 64 КБ). В `.txt` теги
выполняются последовательно, как в `POST /api/text`; см.
[`TEXT.md`](TEXT.md).

Ответ: `{"ok":true,"name":"note.txt","kind":"text","bytes":342}`.

### POST /api/sd/remount

Переподключает карту без перезагрузки (после вставки/замены).

```json
{"ok": true, "mounted": true, "total": 8388608, "used": 0}
```

## Примеры

### PowerShell

```powershell
Invoke-RestMethod -Uri "http://thermal.local/api/print?text=Hello&size=L"
Invoke-RestMethod -Method Post -Uri "http://thermal.local/api/text" `
  -ContentType "application/json; charset=utf-8" `
  -Body '{"text":"Hello","size":"M","align":"C"}'
```

### JavaScript (браузер)

```js
await fetch("http://thermal.local/api/text", {
  method: "POST",
  headers: { "Content-Type": "application/json" },
  body: JSON.stringify({ text: "Hello", size: "M", align: "C" })
});
```

### Python

```python
import requests

requests.post("http://thermal.local/api/text", json={
    "text": "Привет из Python",
    "size": "M",
    "align": "C",
})

with open("image.bin", "rb") as f:
    requests.post("http://thermal.local/api/bitmap",
                  files={"file": ("image.bin", f, "application/octet-stream")})
```

### curl.exe (Windows)

```bat
curl.exe "http://thermal.local/api/print?text=Hello&size=L&align=C"
curl.exe -X POST http://thermal.local/api/text -H "Content-Type: application/json" -d "{\"text\":\"Hello\"}"
curl.exe -X POST http://thermal.local/api/bitmap -F "file=@image.bin"
curl.exe -X POST http://thermal.local/api/sd/upload -F "file=@image.raw"
curl.exe -X POST http://thermal.local/api/sd/upload -F "file=@note.txt"
curl.exe -X POST http://thermal.local/api/sd/print -H "Content-Type: application/json" -d "{\"name\":\"note.txt\",\"feed\":2}"
curl.exe "http://thermal.local/api/sd/file?name=note.txt&download=1" -o note.txt
```

## Коды ошибок

| Код | Когда |
|---|---|
| 400 | Неверный JSON, пустой текст, неподдерживаемый тип штрихкода, плохие размеры изображения, нет файла, неподдерживаемое расширение, файл не является текстом |
| 404 | Неизвестный путь / файл не найден |
| 413 | Текст/данные/файл больше лимита |
| 500 | Не удалось удалить файл с SD или записать его |
| 503 | Принтер не готов (DTR не отпустил линию за 30 с) или SD-карта не смонтирована |
| 200 | Успех (в том числе `answered:false` у статуса принтера) |
