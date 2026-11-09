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
{"ok":true,"heat":120,"baud":9600,"width":384,"maxHeight":2000,"dtr":true}
```

## POST /api/config

```json
{"heat": 140}
```

`heat` — время нагрева ESC/POS (`ESC 7`), 1–255. Сохраняется в NVS.
Ответ — актуальные настройки.

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

## POST /api/text

Печать текста.

### JSON

```json
{
  "text": "Привет, мир!\nВторая строка",
  "size": "M",
  "align": "C",
  "bold": false,
  "underline": false,
  "feed": 2
}
```

| Поле | Тип | По умолчанию | Описание |
|---|---|---|---|
| `text` | string | — | Текст, до 4096 байт UTF-8 |
| `size` | `"S"`/`"M"`/`"L"` или 1–3 | 1 | Размер шрифта |
| `align` | `"L"`/`"C"`/`"R"` | `"L"` | Выравнивание |
| `bold` | bool | false | Жирный |
| `underline` | bool | false | Подчёркивание |
| `feed` | int | 2 | Строк после текста, 0–20 |

### Plain text

Можно отправить тело `text/plain` — оно будет напечатано как есть
(до 4096 байт).

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
{"type": "CODE128", "data": "ABC-123", "height": 60, "width": 2, "feed": 2}
```

| Поле | Тип | По умолчанию | Описание |
|---|---|---|---|
| `type` | string | `"CODE128"` | CODE128, EAN13, EAN8, CODE39, ITF, CODABAR, UPCA, UPCE |
| `data` | string | — | До 255 символов |
| `height` | int | 60 | Высота 1–255 |
| `width` | int | 2 | Ширина 1–6 |
| `feed` | int | 2 | Строк после, 0–20 |

## POST /api/feed

```json
{"lines": 3}
```

Или `POST /api/feed?lines=3`. Диапазон 1–255.

## POST /api/raw

Сырой ESC/POS, **multipart/form-data**, поле `file`. Полезно для
диагностики и нестандартных команд.

```bash
curl.exe -X POST http://thermal.local/api/raw -F "file=@commands.bin"
```

Ответ: `{"ok":true,"bytes":123}`.

## POST /api/test

Печатает тестовую страницу: заголовок, русский текст, IP и QR.

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
ESC/POS. Имена — ASCII, до 48 символов. Лимит загрузки — 512 КБ.
Карта — FAT32, рекомендуется 4–32 ГБ.

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
    {"name": "image.raw", "size": 36804, "raw": false, "valid": true,
     "width": 363, "height": 800},
    {"name": "test.prn", "size": 16, "raw": true, "valid": true}
  ]
}
```

`valid` = `false` — файл с неправильным заголовком или короткий; печать
такого файла будет отклонена.

### GET /api/sd/file?name=...

Отдаёт файл как `application/octet-stream` — для предпросмотра в браузере
или скачивания.

```
GET /api/sd/file?name=image.raw
GET /api/sd/file?name=image.raw&download=1
```

### POST /api/sd/upload

`multipart/form-data`, поле `file`. Имя берётся из `filename` части.

```bash
curl.exe -X POST http://thermal.local/api/sd/upload -F "file=@image.raw"
```

Для `.bin`/`.raw` проверяется заголовок и точный размер; при ошибке файл
удаляется. Ответ:

```json
{"ok": true, "name": "image.raw", "size": 36804, "raw": false}
```

### POST /api/sd/delete

```json
{"name": "image.raw"}
```

Удаляет файл. Ответ: `{"ok":true,"message":"deleted"}`.

### POST /api/sd/print

```json
{"name": "image.raw", "feed": 2}
```

- `.bin`/`.raw` — печатается как изображение, `feed` — строк после;
- `.prn` — сырой ESC/POS, `feed` игнорируется (файл сам управляет).

Ответ: `{"ok":true,"name":"image.raw","bytes":36804}`.

### POST /api/sd/remount

Переподключает карту без перезагрузки (после вставки/замены).

```json
{"ok": true, "mounted": true, "total": 8388608, "used": 0}
```

## Шаблоны текста на SD

Шаблоны из вкладки «Текст» хранятся в `/templates.json` на SD. Без карты
эндпоинты возвращают `503 sd not mounted`.

Лимиты: до 50 шаблонов, имя 1–48 байт (без управляющих символов), текст
до 4096 байт, файл до 48 КБ. Имя — ключ JSON, поэтому кириллица
допустима. Запись атомарная через `/templates.tmp`.

### GET /api/templates

```json
{
  "ok": true,
  "mounted": true,
  "templates": {
    "Заметка": "Привет с SD",
    "Список": "молоко\nхлеб"
  }
}
```

### POST /api/templates

Сохраняет или перезаписывает шаблон.

```json
{"name": "Заметка", "text": "Привет с SD"}
```

Ответ: `{"ok":true,"name":"Заметка","count":2}`.

```bash
curl.exe -X POST http://thermal.local/api/templates \
  -H "Content-Type: application/json" \
  -d "{\"name\":\"Note\",\"text\":\"Hello\"}"
```

### POST /api/templates/delete

```json
{"name": "Заметка"}
```

Ответ: `{"ok":true,"message":"deleted"}`.

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
curl.exe -X POST http://thermal.local/api/sd/print -H "Content-Type: application/json" -d "{\"name\":\"image.raw\",\"feed\":2}"
curl.exe "http://thermal.local/api/sd/file?name=image.raw&download=1" -o image.raw
```

## Коды ошибок

| Код | Когда |
|---|---|
| 400 | Неверный JSON, пустой текст, неподдерживаемый тип штрихкода, плохие размеры изображения, нет файла, плохое имя шаблона |
| 404 | Неизвестный путь / файл или шаблон не найден |
| 413 | Текст/данные/шаблоны больше лимита |
| 500 | Не удалось удалить файл с SD, прочитать или записать `templates.json` |
| 503 | Принтер не готов (DTR не отпустил линию за 30 с) или SD-карта не смонтирована |
| 200 | Успех (в том числе `answered:false` у статуса принтера) |
