#pragma once

// Веб-интерфейс хранится в flash. Никаких внешних CDN и сборки —
// страница работает в локальной сети без интернета.

const char INDEX_HTML[] PROGMEM = R"THERMAL(<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
<title>Thermal58</title>
<style>
:root{--bg:#0f1216;--card:#171c22;--card2:#1d242c;--line:#2a323c;--fg:#e6edf3;--mut:#8b98a5;--acc:#4da3ff;--ok:#3fb950;--err:#f85149;--warn:#d29922}
*{box-sizing:border-box}
html,body{margin:0;padding:0;background:var(--bg);color:var(--fg);font:16px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,Arial,sans-serif}
body{padding-bottom:40px}
header{position:sticky;top:0;z-index:5;display:flex;justify-content:space-between;align-items:center;gap:12px;padding:12px 16px;background:rgba(15,18,22,.92);border-bottom:1px solid var(--line);backdrop-filter:blur(8px)}
.brand{display:flex;align-items:center;gap:8px;font-weight:700;letter-spacing:.2px}
.brand .ver{font-weight:400;color:var(--mut);font-size:.8rem}
.dot{width:10px;height:10px;border-radius:50%;background:var(--mut);display:inline-block;box-shadow:0 0 0 3px rgba(139,152,165,.12)}
.dot.ok{background:var(--ok);box-shadow:0 0 0 3px rgba(63,185,80,.15)}
.dot.err{background:var(--err);box-shadow:0 0 0 3px rgba(248,81,73,.15)}
.net{display:flex;gap:12px;align-items:center;color:var(--mut);font-size:.82rem;text-align:right}
.tabs{display:flex;flex-wrap:wrap;gap:6px;padding:10px 12px;border-bottom:1px solid var(--line);background:var(--bg);position:sticky;top:57px;z-index:4}
.tab{flex:0 0 auto;border:1px solid var(--line);background:var(--card);color:var(--fg);border-radius:10px;padding:8px 12px;font:inherit;font-size:.9rem;cursor:pointer}
.tab:hover{border-color:#3a4653}
.tab.active{background:var(--acc);border-color:var(--acc);color:#08111c;font-weight:700}
main{max-width:920px;margin:0 auto;padding:16px}
.pane{display:none}
.pane.active{display:block}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px;margin:0 0 14px}
h2,h3{margin:0 0 12px;font-size:1.05rem}
h3{font-size:.98rem;color:#cfd8e3}
label{display:flex;flex-direction:column;gap:6px;font-size:.82rem;color:var(--mut)}
label.check{flex-direction:row;align-items:center;gap:8px;color:var(--fg);font-size:.9rem}
input,select,textarea{width:100%;background:var(--card2);color:var(--fg);border:1px solid var(--line);border-radius:9px;padding:9px 10px;font:inherit;outline:none}
input:focus,select:focus,textarea:focus{border-color:var(--acc)}
textarea{resize:vertical;min-height:90px}
input[type=checkbox]{width:auto;accent-color:var(--acc)}
input[type=range]{padding:0;accent-color:var(--acc);background:transparent;border:0}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(170px,1fr));gap:12px;margin-top:12px}
.fmt-group{display:flex;flex-wrap:wrap;gap:6px;align-items:center;margin-top:8px;padding-top:8px;border-top:1px solid var(--line)}
.fmt-group:first-of-type{border-top:0;padding-top:0}
.fmt-label{color:var(--mut);font-size:.78rem;font-weight:600;margin-right:2px}
.fmt-group button{padding:6px 9px;font-size:.78rem}
.fmt-group select{width:auto;min-width:9rem}
.fmt-inline{display:flex;flex-direction:row;align-items:center;gap:4px;font-size:.78rem;color:var(--mut);width:auto}
.fmt-inline input{width:4.5rem;padding:6px 8px}
.fmt-details{margin-top:12px;border-top:1px solid var(--line);padding-top:8px}
.fmt-details summary{cursor:pointer;color:var(--mut);font-size:.85rem;font-weight:600}
.tag-help{font-size:.82rem;color:#c8d3df;margin-top:8px}
.tag-help ul{padding-left:1.2rem;margin:8px 0}
.tag-help li{margin:4px 0}
.tag-help code{color:var(--fg);background:#0b0e12;border:1px solid var(--line);border-radius:5px;padding:1px 4px}
.tag-help pre{margin:8px 0}
.row{display:flex;gap:8px;align-items:center;margin-top:10px;flex-wrap:wrap}
.row input{flex:1}
.row select{flex:1 1 200px;min-width:0}
.actions{display:flex;gap:8px;flex-wrap:wrap;margin-top:14px;align-items:center}
button{border:1px solid var(--line);background:var(--card2);color:var(--fg);border-radius:10px;padding:9px 13px;font:inherit;font-size:.9rem;cursor:pointer}
button:hover{border-color:#3a4653}
button:active{transform:translateY(1px)}
button:disabled{opacity:.55;cursor:wait}
button.primary{background:var(--acc);border-color:var(--acc);color:#08111c;font-weight:700}
button.danger{background:#2a1618;border-color:#5a2428;color:#ffb3b0}
.hint{color:var(--mut);font-size:.78rem;margin-top:8px}
.mono,pre{font-family:ui-monospace,SFMono-Regular,Consolas,monospace;font-size:.78rem;line-height:1.5;white-space:pre-wrap;word-break:break-word;background:#0b0e12;border:1px solid var(--line);border-radius:10px;padding:10px;overflow:auto;color:#c8d3df}
.preview-wrap{margin-top:14px;display:flex;justify-content:center}
canvas#preview,canvas#sdPreview{width:100%;max-width:384px;height:auto;image-rendering:pixelated;background:#fff;border:1px solid var(--line);border-radius:10px}
.sd-text{max-height:18rem;overflow:auto;white-space:pre-wrap;word-break:break-word;background:#0b0e12;border:1px solid var(--line);border-radius:10px;padding:10px;font-size:.8rem;color:#c8d3df;margin:0}
.sd-list{display:flex;flex-direction:column;gap:8px}
.sd-item{display:flex;flex-wrap:wrap;gap:8px;align-items:center;justify-content:space-between;background:var(--card2);border:1px solid var(--line);border-radius:10px;padding:10px}
.sd-info{flex:1 1 100%;min-width:0}
.sd-name{font-weight:600;word-break:break-word}
.sd-meta{color:var(--mut);font-size:.75rem;margin-top:2px}
.sd-actions{display:flex;gap:6px;flex-wrap:wrap;width:100%}
.sd-actions button{padding:6px 9px;font-size:.8rem}
.md-preview{width:384px;max-width:100%;max-height:70vh;margin:0 auto;background:#fff;color:#000;border:1px solid var(--line);border-radius:10px;padding:12px 10px;font-family:ui-monospace,SFMono-Regular,Consolas,monospace;font-size:12px;line-height:1.28;overflow:auto}
.md-preview h1,.md-preview h2,.md-preview h3,.md-preview h4{margin:.55em 0 .3em;line-height:1.1}
.md-preview h1{font-size:2em}
.md-preview h2{font-size:1.55em}
.md-preview h3{font-size:1.18em;text-decoration:underline}
.md-preview h4{font-size:1em}
.md-preview p{margin:.35em 0}
.md-preview ul,.md-preview ol{margin:.35em 0;padding-left:1.35em}
.md-preview blockquote{margin:.4em 0;padding-left:.6em;border-left:3px solid #999;color:#333}
.md-preview pre{background:#eee;color:#000;border:1px solid #ccc;padding:6px;border-radius:4px;font-size:11px;white-space:pre-wrap}
.md-preview code{background:#eee;color:#000;padding:0 2px;border-radius:3px}
.md-preview table{border-collapse:collapse;margin:.45em 0;font-size:11px}
.md-preview th,.md-preview td{border:1px solid #999;padding:2px 4px}
.md-preview img{max-width:100%;image-rendering:pixelated;display:block;margin:.35em 0}
.md-preview hr{border:0;border-top:1px dashed #777;margin:.6em 0}
.md-preview .muted{color:#777;font-size:11px}
.md-img-row{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
.md-img-row img{width:64px;height:64px;object-fit:contain;image-rendering:pixelated;background:#fff;border:1px solid var(--line);border-radius:6px}
.md-img-row input[type=file]{max-width:210px}
#toast{position:fixed;left:50%;bottom:20px;transform:translateX(-50%) translateY(20px);background:#1f2933;color:#fff;border:1px solid var(--line);border-radius:12px;padding:11px 16px;font-size:.9rem;opacity:0;pointer-events:none;transition:.2s;max-width:92vw;z-index:20;box-shadow:0 10px 30px rgba(0,0,0,.4)}
#toast.show{opacity:1;transform:translateX(-50%) translateY(0)}
#toast.ok{border-color:#1f6f34}
#toast.err{border-color:#7a2529}
#heatVal{display:inline-block;min-width:3rem;color:var(--fg);font-variant-numeric:tabular-nums}
@media(max-width:560px){main{padding:12px}header{padding:10px 12px}.net{font-size:.75rem}.tabs{top:53px}}
</style>
</head>
<body>
<header>
  <div class="brand"><span class="dot" id="stDot"></span><strong>Thermal58</strong><span class="ver" id="stVersion">—</span></div>
  <div class="net"><span id="stIp">—</span><span id="stSsid"></span></div>
</header>

<nav class="tabs">
  <button class="tab active" data-tab="text">Текст</button>
  <button class="tab" data-tab="markdown">Markdown</button>
  <button class="tab" data-tab="image">Фото</button>
  <button class="tab" data-tab="code">QR / Штрихкод</button>
  <button class="tab" data-tab="sd">SD-карта</button>
  <button class="tab" data-tab="settings">Настройки</button>
  <button class="tab" data-tab="api">API</button>
</nav>

<main>
  <section class="pane active" id="pane-text">
    <div class="card">
      <h2>Текст</h2>
      <label for="text">Содержимое</label>
      <textarea id="text" rows="8" placeholder="[setSize,L]ЗАГОЛОВОК&#10;[setSize,M]Обычный текст, [boldOn]жирное слово[boldOff]."></textarea>
      <div class="hint"><span id="charCount">0</span> символов · теги в квадратных скобках выполняются по порядку, закрывающие не нужны.</div>
      <div class="grid">
        <label>Подача в конце, строк<input id="feed" type="number" min="0" max="20" value="2"></label>
      </div>
      <div class="actions">
        <button class="primary" id="btnText">Печать</button>
        <button id="btnTest">Тестовая страница</button>
        <button id="btnFeedTop">Подать 3 строки</button>
      </div>
    </div>

    <div class="card">
      <h3>Форматирование</h3>
      <div class="hint">Кнопка вставляет тег в позицию курсора. Действие тега продолжается до следующего тега. Пример: <code>[setSize,L]</code>Заголовок<code>[setSize,S]</code>обычный текст.</div>

      <div class="fmt-group">
        <span class="fmt-label">Размер</span>
        <button class="tagbtn" data-tag="[setSize,S]">S</button>
        <button class="tagbtn" data-tag="[setSize,M]">M</button>
        <button class="tagbtn" data-tag="[setSize,L]">L</button>
        <span class="fmt-label">Шрифт</span>
        <button class="tagbtn" data-tag="[setFont,A]">A</button>
        <button class="tagbtn" data-tag="[setFont,B]">B</button>
      </div>

      <div class="fmt-group">
        <span class="fmt-label">Выравнивание</span>
        <button class="tagbtn" data-tag="[justify,L]">Слева</button>
        <button class="tagbtn" data-tag="[justify,C]">Центр</button>
        <button class="tagbtn" data-tag="[justify,R]">Справа</button>
      </div>

      <div class="fmt-group">
        <span class="fmt-label">Стиль</span>
        <button class="tagbtn" data-tag="[boldOn]">boldOn</button>
        <button class="tagbtn" data-tag="[boldOff]">boldOff</button>
        <button class="tagbtn" data-tag="[underlineOn]">underlineOn</button>
        <button class="tagbtn" data-tag="[underlineOn,2]">underlineOn,2</button>
        <button class="tagbtn" data-tag="[underlineOff]">underlineOff</button>
        <button class="tagbtn" data-tag="[inverseOn]">inverseOn</button>
        <button class="tagbtn" data-tag="[inverseOff]">inverseOff</button>
        <button class="tagbtn" data-tag="[strikeOn]">strikeOn</button>
        <button class="tagbtn" data-tag="[strikeOff]">strikeOff</button>
      </div>

      <div class="fmt-group">
        <span class="fmt-label">Ширина / высота</span>
        <button class="tagbtn" data-tag="[doubleWidthOn]">doubleWidthOn</button>
        <button class="tagbtn" data-tag="[doubleWidthOff]">doubleWidthOff</button>
        <button class="tagbtn" data-tag="[doubleHeightOn]">doubleHeightOn</button>
        <button class="tagbtn" data-tag="[doubleHeightOff]">doubleHeightOff</button>
        <button class="tagbtn" data-tag="[upsideDownOn]">upsideDownOn</button>
        <button class="tagbtn" data-tag="[upsideDownOff]">upsideDownOff</button>
      </div>

      <div class="fmt-group">
        <span class="fmt-label">Интервалы</span>
        <label class="fmt-inline">Строка <input id="lineHeight" type="number" min="24" max="255" value="30"></label>
        <button id="btnLineHeight">setLineHeight</button>
        <label class="fmt-inline">Символ <input id="charSpacing" type="number" min="0" max="255" value="0"></label>
        <button id="btnCharSpacing">setCharSpacing</button>
      </div>

      <div class="fmt-group">
        <span class="fmt-label">Подача</span>
        <label class="fmt-inline">Строк <input id="feedTag" type="number" min="0" max="255" value="1"></label>
        <button id="btnFeedTag">feed</button>
        <label class="fmt-inline">Точек <input id="feedRowsTag" type="number" min="0" max="255" value="20"></label>
        <button id="btnFeedRows">feedRows</button>
        <button class="tagbtn" data-tag="[tab]">tab</button>
      </div>

      <div class="fmt-group">
        <span class="fmt-label">Сброс</span>
        <button class="tagbtn" data-tag="[setDefault]">setDefault</button>
        <button class="tagbtn" data-tag="[reset]">reset</button>
      </div>

      <details class="fmt-details">
        <summary>Дополнительно (кодовая страница, плотность, сон, online)</summary>
        <div class="fmt-group">
          <span class="fmt-label">Кодовая страница</span>
          <select id="codePage"><option value="6">CP1251 (6)</option><option value="7">CP866 (7)</option></select>
          <button id="btnCodePage">setCodePage</button>
          <label class="fmt-inline">Charset <input id="charset" type="number" min="0" max="15" value="0"></label>
          <button id="btnCharset">setCharset</button>
        </div>
        <div class="fmt-group">
          <span class="fmt-label">Штрихкод</span>
          <label class="fmt-inline">Высота <input id="barcodeHeight" type="number" min="1" max="255" value="50"></label>
          <button id="btnBarcodeHeight">setBarcodeHeight</button>
          <span class="hint">Высота действует на последующие штрихкоды (сейчас они в отдельной вкладке).</span>
        </div>
        <div class="fmt-group">
          <span class="fmt-label">Плотность</span>
          <label class="fmt-inline">Density <input id="printDensity" type="number" min="0" max="31" value="10"></label>
          <label class="fmt-inline">Break <input id="printBreak" type="number" min="0" max="7" value="2"></label>
          <button id="btnPrintDensity">setPrintDensity</button>
        </div>
        <div class="fmt-group">
          <span class="fmt-label">Нагрев</span>
          <label class="fmt-inline">Dots <input id="heatDots" type="number" min="0" max="255" value="11"></label>
          <label class="fmt-inline">Time <input id="heatTime" type="number" min="0" max="255" value="120"></label>
          <label class="fmt-inline">Interval <input id="heatInterval" type="number" min="0" max="255" value="40"></label>
          <button id="btnHeatConfig">setHeatConfig</button>
        </div>
        <div class="fmt-group">
          <span class="fmt-label">Питание</span>
          <label class="fmt-inline">Секунд <input id="sleepSeconds" type="number" min="0" max="65535" value="30"></label>
          <button id="btnSleepAfter">sleepAfter</button>
          <button class="tagbtn" data-tag="[sleep]">sleep</button>
          <button class="tagbtn" data-tag="[wake]">wake</button>
          <button class="tagbtn" data-tag="[online]">online</button>
          <button class="tagbtn" data-tag="[offline]">offline</button>
        </div>
      </details>

      <details class="fmt-details">
        <summary>Справка по тегам</summary>
        <div class="tag-help">
          <p>Теги выполняются последовательно, как вызовы методов библиотеки. Закрывающие теги не нужны. Регистр команд не важен.</p>
          <ul>
            <li><code>[setSize,S|M|L]</code> — размер: S обычный, M двойная высота, L двойная ширина и высота.</li>
            <li><code>[setFont,A|B]</code> — шрифт A или B.</li>
            <li><code>[justify,L|C|R]</code> — выравнивание слева/по центру/справа.</li>
            <li><code>[boldOn]</code>, <code>[boldOff]</code> — жирный.</li>
            <li><code>[underlineOn]</code>, <code>[underlineOn,2]</code>, <code>[underlineOff]</code> — подчёркивание 1/2 px.</li>
            <li><code>[inverseOn]</code>, <code>[inverseOff]</code> — инверсия.</li>
            <li><code>[doubleWidthOn]</code>, <code>[doubleWidthOff]</code> — двойная ширина.</li>
            <li><code>[doubleHeightOn]</code>, <code>[doubleHeightOff]</code> — двойная высота.</li>
            <li><code>[strikeOn]</code>, <code>[strikeOff]</code> — зачёркивание.</li>
            <li><code>[upsideDownOn]</code>, <code>[upsideDownOff]</code> — перевёрнутый текст.</li>
            <li><code>[setLineHeight,24..255]</code> — межстрочный интервал в точках (по умолчанию 30).</li>
            <li><code>[setCharSpacing,0..255]</code> — дополнительный интервал между символами.</li>
            <li><code>[tab]</code> — табуляция.</li>
            <li><code>[feed,0..255]</code> — подача строк; <code>[feedRows,0..255]</code> — подача в точках.</li>
            <li><code>[setDefault]</code> — сброс форматирования (кодовую страницу не меняет).</li>
            <li><code>[reset]</code> — полный сброс задания и возврат к кодовой странице из настроек.</li>
            <li><code>[setCodePage,6|7]</code> — 6 = CP1251, 7 = CP866; другие значения только для латиницы.</li>
            <li><code>[setCharset,0..15]</code> — таблица символов ESC/POS (конвертацию кириллицы не меняет).</li>
            <li><code>[setBarcodeHeight,1..255]</code> — высота последующих штрихкодов.</li>
            <li><code>[qr,URL]</code> — вставить QR с данными (модуль 2–4 автоматически, до 690 байт).</li>
            <li><code>[image,photo.bin]</code> — вставить изображение .bin/.raw с SD в поток текста.</li>
            <li><code>[setPrintDensity,0..31,0..7]</code> — плотность и пауза (ESC/POS DC2 #).</li>
            <li><code>[setHeatConfig,dots,time,interval]</code> — параметры нагрева ESC 7.</li>
            <li><code>[sleepAfter,N]</code>, <code>[sleep]</code>, <code>[wake]</code> — сон/пробуждение.</li>
            <li><code>[online]</code>, <code>[offline]</code> — перевод принтера online/offline.</li>
          </ul>
          <p>Неизвестный тег печатается как обычный текст. Чтобы напечатать литерал <code>[</code>, используйте <code>[[</code>.</p>
          <pre>[setSize,L]ЗАГОЛОВОК
[setSize,M]Обычный текст, [boldOn]жирное слово[boldOff] и [underlineOn]подчёркнутое[underlineOff].
[justify,R]Итого: 1000 ₽
[justify,L][inverseOn]Инверсия[inverseOff]</pre>
          <p>Теги работают только в файлах <code>.txt</code> и в поле «Текст». Файлы <code>.md</code>, <code>.csv</code>, <code>.log</code> печатаются как обычный текст, без разбора тегов.</p>
          <p>Штрихкоды, тестовая страница и проверка бумаги — отдельные вкладки/API. QR и картинки можно вставлять тегами <code>[qr,...]</code> и <code>[image,...]</code>.</p>
        </div>
      </details>
    </div>
  </section>

  <section class="pane" id="pane-markdown">
    <div class="card">
      <h2>Markdown → текст с тегами</h2>
      <div class="grid">
        <label>Файл .md с устройства<input id="mdFile" type="file" accept=".md,.markdown,text/markdown,text/plain"></label>
        <label>Или .md с SD<select id="mdSdSelect"><option value="">— выберите —</option></select></label>
      </div>
      <div class="actions">
        <button id="btnMdLoadSd">Загрузить с SD</button>
        <button class="primary" id="btnMdConvert">Конвертировать</button>
      </div>
      <div class="hint" id="mdStatus">Загрузите .md и нажмите «Конвертировать». Конвертация выполняется в браузере, ESP32 Markdown не разбирает.</div>
    </div>

    <div class="card">
      <h3>Предпросмотр</h3>
      <div class="fmt-group">
        <label class="check"><input id="mdShowTags" type="checkbox"> Показать теги</label>
        <label class="check"><input id="mdQrLinks" type="checkbox" checked> QR для ссылок</label>
        <label class="check"><input id="mdImgSd" type="checkbox" checked> Вставлять картинки с SD</label>
      </div>
      <div id="mdPreview" class="md-preview"><span class="muted">Здесь появится предпросмотр.</span></div>
      <pre id="mdTags" class="sd-text" hidden></pre>
      <div class="hint">Предпросмотр приблизительный: реальные шрифты принтера отличаются. Точные переносы и теги — в режиме «Показать теги».</div>
      <div class="hint">Дизеринг, яркость и контраст для картинок берутся из вкладки «Фото».</div>
    </div>

    <div class="card">
      <h3>Картинки</h3>
      <div id="mdImages" class="sd-list">—</div>
      <div class="hint">Ссылки http(s) браузер пытается скачать сам. Если сайт запрещает CORS — выберите файл вручную. Локальные .bin/.raw берутся с SD.</div>
    </div>

    <div class="card">
      <h3>Результат</h3>
      <div class="grid">
        <label>Имя .txt на SD<input id="mdSaveName" placeholder="например note.txt"></label>
      </div>
      <div class="actions">
        <button id="btnMdSaveSd" disabled>Сохранить .txt на SD</button>
        <button id="btnMdToText" disabled>В текстовое поле</button>
      </div>
      <div class="hint">Теги и картинки работают только в .txt. Сам .md с карты печатается как обычный текст.</div>
    </div>
  </section>

  <section class="pane" id="pane-image">
    <div class="card">
      <h2>Фото</h2>
      <label for="file">Изображение</label>
      <input id="file" type="file" accept="image/*">
      <div class="grid">
        <label>Дизеринг<select id="dither"><option value="floyd">Флойд–Стейнберг</option><option value="ordered">Упорядоченный 4×4</option><option value="threshold">Порог</option></select></label>
        <label>Яркость<input id="brightness" type="range" min="-100" max="100" value="0"></label>
        <label>Контраст<input id="contrast" type="range" min="-100" max="100" value="0"></label>
        <label class="check"><input id="invert" type="checkbox"> Инверсия</label>
      </div>
      <div class="preview-wrap"><canvas id="preview" width="384" height="200"></canvas></div>
      <div class="hint" id="imgInfo">Выберите изображение. Ширина всегда 384 точки (48 мм).</div>
      <div class="actions">
        <button class="primary" id="btnImg">Печать фото</button>
        <button id="btnImgRefresh">Обновить предпросмотр</button>
      </div>
      <div class="row">
        <input id="sdSaveName" placeholder="Имя файла на SD (необязательно)">
        <button id="btnImgSave">Сохранить на SD</button>
      </div>
      <div class="hint">Файл сохраняется в формате 1-битного изображения 384 точки и потом печатается с SD.</div>
    </div>
  </section>

  <section class="pane" id="pane-code">
    <div class="card">
      <h2>QR-код</h2>
      <label for="qrData">Данные</label>
      <textarea id="qrData" rows="3" placeholder="https://example.com или любой текст"></textarea>
      <div class="grid">
        <label>Размер модуля<input id="qrSize" type="number" min="1" max="8" value="4"></label>
      </div>
      <div class="actions"><button class="primary" id="btnQr">Печать QR</button></div>
      <div class="hint">QR печатается средствами принтера (ESC/POS GS ( k).</div>
    </div>

    <div class="card">
      <h3>Штрихкод</h3>
      <div class="grid">
        <label>Тип<select id="bcType">
          <option value="CODE128">CODE128</option>
          <option value="CODE93">CODE93</option>
          <option value="EAN13">EAN13</option>
          <option value="EAN8">EAN8</option>
          <option value="CODE39">CODE39</option>
          <option value="ITF">ITF</option>
          <option value="CODABAR">CODABAR</option>
          <option value="UPCA">UPC-A</option>
          <option value="UPCE">UPC-E</option>
        </select></label>
        <label>Данные<input id="bcData" placeholder="123456789012"></label>
        <label>Высота<input id="bcHeight" type="number" min="1" max="255" value="60"></label>
        <label>Ширина<input id="bcWidth" type="number" min="1" max="6" value="2"></label>
      </div>
      <div class="actions"><button class="primary" id="btnBc">Печать штрихкода</button>
        <button id="btnBcTest">Тест всех типов</button></div>
    </div>
  </section>

  <section class="pane" id="pane-sd">
    <div class="card">
      <h2>SD-карта</h2>
      <div class="grid">
        <div>Состояние: <b id="sdState">—</b></div>
        <div>Всего: <b id="sdTotal">—</b></div>
        <div>Занято: <b id="sdUsed">—</b></div>
        <div>Свободно: <b id="sdFree">—</b></div>
      </div>
      <div class="actions">
        <button id="btnSdRefresh">Обновить</button>
        <button id="btnSdRemount">Переподключить карту</button>
      </div>
    </div>

    <div class="card">
      <h3>Загрузить файл</h3>
      <div class="grid">
        <label>Файл (.bin, .raw, .prn, .txt, .md, .markdown, .csv, .log)<input id="sdUploadFile" type="file" accept=".bin,.raw,.prn,.txt,.md,.markdown,.csv,.log,application/octet-stream,text/plain"></label>
        <label>Имя на карте<input id="sdUploadName" placeholder="как в файле"></label>
      </div>
      <div class="actions"><button class="primary" id="btnSdUpload">Загрузить</button></div>
      <div class="hint">.bin/.raw — 1-битное изображение с заголовком; .prn — сырой ESC/POS; .txt — текст с тегами; .md/.markdown/.csv/.log — обычный текст без разбора тегов.</div>
    </div>

    <div class="card">
      <h3>Файлы</h3>
      <div id="sdList" class="sd-list">—</div>
    </div>

    <div class="card">
      <h3>Просмотр</h3>
      <div class="preview-wrap"><canvas id="sdPreview" width="384" height="200"></canvas></div>
      <pre id="sdTextPreview" class="sd-text" hidden></pre>
      <div class="hint" id="sdPreviewInfo">Нажмите «Просмотр» у файла изображения или текста.</div>
    </div>
  </section>

  <section class="pane" id="pane-settings">
    <div class="card">
      <h2>Принтер</h2>
      <div class="grid">
        <label>Плотность (heat time) <span><input id="heat" type="range" min="1" max="255" value="120"> <span id="heatVal">120</span></span></label>
        <label>Кодовая страница<select id="cfgCodePage">
          <option value="6">CP1251 (ESC t 6)</option>
          <option value="7">CP866 (ESC t 7)</option>
        </select></label>
        <label>Скорость UART<select id="cfgBaudSel">
          <option value="9600">9600</option>
          <option value="19200">19200</option>
          <option value="38400">38400</option>
          <option value="57600">57600</option>
          <option value="115200">115200</option>
        </select></label>
        <label><span>DTR (аппаратный поток)</span><input id="cfgDtr" type="checkbox" checked></label>
      </div>
      <div class="actions">
        <button class="primary" id="btnSaveCfg">Сохранить настройки</button>
        <button id="btnCodePageTest">Тест кодовых страниц</button>
        <button id="btnCommandTest">Тест команд</button>
        <button id="btnSelfTest">Самотест принтера</button>
      </div>
      <div class="hint">Baud: <span id="cfgBaud">—</span> · ширина: <span id="cfgWidth">—</span> точек · кодовая страница: <span id="cfgCpLabel">—</span>. Кириллица: 6 = CP1251, 7 = CP866 (46 — это Hebrew, 17 — Greek, на них будут «кракозябры»). Скорость должна совпадать с принтером — обычно 9600.</div>
      <div class="actions">
        <button id="btnPrinterStatus">Проверить принтер</button>
        <button id="btnFeed">Подать бумагу</button>
        <input id="feedLines" type="number" min="1" max="20" value="3" style="width:5rem" aria-label="Строк бумаги">
      </div>
      <pre id="printerStatus" class="mono">—</pre>
    </div>

    <div class="card">
      <h3>Сеть и устройство</h3>
      <div class="grid">
        <div>IP: <b id="stIp2">—</b></div>
        <div>SSID: <b id="stSsid2">—</b></div>
        <div>RSSI: <b id="stRssi">—</b></div>
        <div>Heap: <b id="stHeap">—</b></div>
        <div>Uptime: <b id="stUptime">—</b></div>
      </div>
      <div class="actions">
        <button id="btnRestart">Перезагрузить</button>
        <button class="danger" id="btnWifiReset">Сбросить Wi-Fi</button>
      </div>
      <div class="hint">После сброса ESP32 поднимет точку доступа <code>Thermal58-setup</code> (пароль <code>thermal58</code>).</div>
    </div>
  </section>

  <section class="pane" id="pane-api">
    <div class="card">
      <h2>API</h2>
      <h3>Базовый URL</h3>
      <pre class="mono" id="baseUrl">—</pre>
      <div class="hint">API доступен с любого устройства в сети, CORS включён. Если <code>thermal.local</code> не открывается — используйте этот IP.</div>
    </div>
    <div class="card">
      <h3>Эндпоинты</h3>
      <pre class="mono">GET  /api/status                 состояние устройства
GET  /api/info                   возможности и список API
GET  /api/config                 настройки
POST /api/config                 {"heat":120,"codePage":6,"baud":9600,"dtr":true}
GET  /api/printer/status         опрос принтера (бумага/статус)
POST /api/text                   {"text":"...","size":"M","align":"C"}
POST /api/bitmap                 multipart file: заголовок + 1-битные строки
POST /api/qr                     {"data":"...","module":4}
POST /api/barcode                {"type":"CODE128","data":"..."}
POST /api/feed                   {"lines":3}
POST /api/raw                    multipart file: сырой ESC/POS
POST /api/test                   тестовая страница
POST /api/test/self              самотест принтера (DC2 T)
POST /api/test/codepage          тест кодовых страниц (6/7)
GET  /api/print?text=...         печать текста из URL
POST /api/restart                перезагрузка
POST /api/wifi/reset             сброс Wi-Fi и перезагрузка
GET  /api/sd/list                список файлов на SD
GET  /api/sd/file?name=...       скачать/посмотреть файл
POST /api/sd/upload              multipart file: загрузка на SD
POST /api/sd/delete              {"name":"..."}
POST /api/sd/print               {"name":"...","feed":2} — изображение, .txt с тегами, .md/.csv/.log как текст, .prn
POST /api/sd/remount             переподключить карту</pre>
    </div>
    <div class="card">
      <h3>Примеры</h3>
      <pre class="mono">curl.exe "http://thermal.local/api/print?text=Hello&size=L&align=C"
curl.exe -X POST http://thermal.local/api/text -H "Content-Type: application/json" -d "{\"text\":\"Hello\",\"size\":\"M\"}"
curl.exe -X POST http://thermal.local/api/qr -H "Content-Type: application/json" -d "{\"data\":\"https://example.com\",\"module\":4}"
curl.exe -X POST http://thermal.local/api/bitmap -F "file=@image.bin"
curl.exe -X POST http://thermal.local/api/sd/upload -F "file=@image.raw"
curl.exe -X POST http://thermal.local/api/sd/upload -F "file=@note.txt"
curl.exe -X POST http://thermal.local/api/sd/print -H "Content-Type: application/json" -d "{\"name\":\"note.txt\"}"</pre>
      <div class="hint">Подробное описание — в <code>docs/API.md</code>.</div>
    </div>
  </section>
</main>

<div id="toast"></div>

<script>
const $ = (s) => document.querySelector(s);
const $$ = (s) => Array.from(document.querySelectorAll(s));

async function api(path, opts) {
  const res = await fetch(path, opts || {});
  const text = await res.text();
  let data = null;
  try { data = text ? JSON.parse(text) : {}; } catch (e) { data = { ok: res.ok, raw: text }; }
  if (!res.ok || data.ok === false) throw new Error(data.error || ('HTTP ' + res.status));
  return data;
}
const jsonPost = (path, obj) => api(path, {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify(obj)
});

let toastTimer = 0;
function toast(msg, ok = true) {
  const el = $('#toast');
  el.textContent = msg;
  el.className = 'show ' + (ok ? 'ok' : 'err');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { el.className = ''; }, 4000);
}

function fmtUptime(sec) {
  sec = Math.max(0, Math.floor(sec || 0));
  const d = Math.floor(sec / 86400); sec %= 86400;
  const h = Math.floor(sec / 3600); sec %= 3600;
  const m = Math.floor(sec / 60); sec %= 60;
  if (d) return d + 'д ' + h + 'ч';
  if (h) return h + 'ч ' + m + 'м';
  return m + 'м ' + sec + 'с';
}

function fmtBytes(n) {
  n = Number(n) || 0;
  if (n < 1024) return n + ' Б';
  if (n < 1048576) return (n / 1024).toFixed(1) + ' КБ';
  if (n < 1073741824) return (n / 1048576).toFixed(1) + ' МБ';
  return (n / 1073741824).toFixed(2) + ' ГБ';
}

function mkBtn(text, fn, cls) {
  const b = document.createElement('button');
  b.textContent = text;
  if (cls) b.className = cls;
  b.addEventListener('click', fn);
  return b;
}

/* ---------- вкладки ---------- */
$$('.tab').forEach((btn) => btn.addEventListener('click', () => {
  $$('.tab').forEach((b) => b.classList.toggle('active', b === btn));
  const name = btn.dataset.tab;
  $$('.pane').forEach((p) => p.classList.toggle('active', p.id === 'pane-' + name));
  if (name === 'sd') refreshSd();
  if (name === 'markdown') refreshMdSdList();
}));

/* ---------- статус ---------- */
async function refreshStatus() {
  try {
    const d = await api('/api/status');
    $('#stDot').className = 'dot ok';
    $('#stIp').textContent = d.ip || '—';
    $('#stIp2').textContent = d.ip || '—';
    $('#stSsid').textContent = d.ssid || '';
    $('#stSsid2').textContent = d.ssid || '—';
    $('#stRssi').textContent = (d.rssi || 0) + ' dBm';
    $('#stHeap').textContent = Math.round((d.heap || 0) / 1024) + ' КБ';
    $('#stUptime').textContent = fmtUptime(d.uptime || 0);
    $('#stVersion').textContent = d.version || '';
    $('#cfgBaud').textContent = d.baud || '—';
    $('#cfgWidth').textContent = d.width || '—';
  } catch (e) {
    $('#stDot').className = 'dot err';
  }
}

/* ---------- текст ---------- */
function updateCharCount() { $('#charCount').textContent = $('#text').value.length; }
$('#text').addEventListener('input', updateCharCount);

function insertTag(tag) {
  const ta = $('#text');
  const start = (ta.selectionStart !== null && ta.selectionStart !== undefined)
    ? ta.selectionStart : ta.value.length;
  ta.value = ta.value.slice(0, start) + tag + ta.value.slice(start);
  const pos = start + tag.length;
  ta.selectionStart = ta.selectionEnd = pos;
  ta.focus();
  updateCharCount();
}

function intVal(id, min, max, def) {
  let n = parseInt($(id).value, 10);
  if (!isFinite(n)) n = def;
  if (n < min) n = min;
  if (n > max) n = max;
  return n;
}

$$('.tagbtn[data-tag]').forEach((b) => {
  b.addEventListener('click', () => insertTag(b.dataset.tag));
});

$('#btnLineHeight').addEventListener('click', () => {
  insertTag('[setLineHeight,' + intVal('#lineHeight', 24, 255, 30) + ']');
});
$('#btnCharSpacing').addEventListener('click', () => {
  insertTag('[setCharSpacing,' + intVal('#charSpacing', 0, 255, 0) + ']');
});
$('#btnFeedTag').addEventListener('click', () => {
  insertTag('[feed,' + intVal('#feedTag', 0, 255, 1) + ']');
});
$('#btnFeedRows').addEventListener('click', () => {
  insertTag('[feedRows,' + intVal('#feedRowsTag', 0, 255, 20) + ']');
});
$('#btnCodePage').addEventListener('click', () => {
  insertTag('[setCodePage,' + $('#codePage').value + ']');
});
$('#btnCharset').addEventListener('click', () => {
  insertTag('[setCharset,' + intVal('#charset', 0, 15, 0) + ']');
});
$('#btnBarcodeHeight').addEventListener('click', () => {
  insertTag('[setBarcodeHeight,' + intVal('#barcodeHeight', 1, 255, 50) + ']');
});
$('#btnPrintDensity').addEventListener('click', () => {
  insertTag('[setPrintDensity,' + intVal('#printDensity', 0, 31, 10) + ',' +
    intVal('#printBreak', 0, 7, 2) + ']');
});
$('#btnHeatConfig').addEventListener('click', () => {
  insertTag('[setHeatConfig,' + intVal('#heatDots', 0, 255, 11) + ',' +
    intVal('#heatTime', 0, 255, 120) + ',' +
    intVal('#heatInterval', 0, 255, 40) + ']');
});
$('#btnSleepAfter').addEventListener('click', () => {
  insertTag('[sleepAfter,' + intVal('#sleepSeconds', 0, 65535, 30) + ']');
});

async function printText() {
  const text = $('#text').value;
  if (!text.trim()) return toast('Введите текст', false);
  const payload = {
    text,
    feed: Number($('#feed').value) || 0
  };
  const btn = $('#btnText');
  btn.disabled = true;
  try { await jsonPost('/api/text', payload); toast('Отправлено на печать'); }
  catch (e) { toast(e.message, false); }
  finally { btn.disabled = false; }
}
$('#btnText').addEventListener('click', printText);

async function doTest() {
  try { await api('/api/test', { method: 'POST' }); toast('Тестовая страница отправлена'); }
  catch (e) { toast(e.message, false); }
}
$('#btnTest').addEventListener('click', doTest);

async function doFeed(lines) {
  try { await jsonPost('/api/feed', { lines }); toast('Бумага подана'); }
  catch (e) { toast(e.message, false); }
}
$('#btnFeedTop').addEventListener('click', () => doFeed(3));
$('#btnFeed').addEventListener('click', () => doFeed(Number($('#feedLines').value) || 3));

/* ---------- Markdown: конвертация в теги ---------- */
let md = {
  sourceName: '',
  sourceText: '',
  model: null,
  tags: '',
  images: [],
  seq: 0,
  converted: false
};

function mdEscHtml(s) {
  return String(s).replace(/[&<>"']/g, (c) => ({
    '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
  })[c]);
}

function mdEscTag(s) {
  return String(s).replace(/\[/g, '[[');
}

function mdCharWidth(ch) {
  return ch === '₽' ? 4 : 1;
}

function mdPlain(nodes) {
  let out = '';
  for (const n of nodes || []) {
    if (n.type === 'text') out += n.text;
    else if (n.type === 'br') out += '\n';
    else if (n.type === 'link') out += mdPlain(n.children);
    else if (n.type === 'image') out += n.alt || '';
  }
  return out;
}

function mdFindClose(s, start, marker) {
  let p = start;
  while (p <= s.length) {
    const q = s.indexOf(marker, p);
    if (q < 0) return -1;
    if (marker.length === 1) {
      if (s[q + 1] === marker) { p = q + 2; continue; }
      if (marker === '_' && q > 0 && q + 1 < s.length &&
          /[\wА-Яа-яЁё]/.test(s[q - 1]) && /[\wА-Яа-яЁё]/.test(s[q + 1])) {
        p = q + 1;
        continue;
      }
    }
    return q;
  }
  return -1;
}

function mdMatchLink(s, start) {
  if (s[start] !== '[') return null;
  const close = s.indexOf(']', start + 1);
  if (close < 0 || s[close + 1] !== '(') return null;
  let depth = 1;
  let j = close + 2;
  for (; j < s.length; j++) {
    if (s[j] === '(') depth++;
    else if (s[j] === ')') {
      depth--;
      if (depth === 0) break;
    }
  }
  if (depth !== 0) return null;
  const inner = s.slice(close + 2, j);
  const m = inner.match(/^\s*(\S+)(?:\s+["'].*)?\s*$/);
  if (!m) return null;
  return { text: s.slice(start + 1, close), url: m[1], end: j + 1 };
}

function mdParseInline(s, style) {
  style = style || {};
  const out = [];
  let buf = '';
  const flush = () => {
    if (buf) {
      out.push({ type: 'text', text: buf, style: Object.assign({}, style) });
      buf = '';
    }
  };
  let i = 0;
  while (i < s.length) {
    const c = s[i];
    if (c === '\\' && i + 1 < s.length) {
      buf += s[i + 1];
      i += 2;
      continue;
    }
    if (c === '\n') {
      flush();
      out.push({ type: 'br' });
      i++;
      continue;
    }
    if (c === '`') {
      const m = s.slice(i).match(/^(`+)([\s\S]*?)\1/);
      if (m) {
        flush();
        out.push({ type: 'text', text: m[2], style: Object.assign({}, style, { code: true }) });
        i += m[0].length;
        continue;
      }
    }
    if (c === '!' && s[i + 1] === '[') {
      const m = mdMatchLink(s, i + 1);
      if (m) {
        flush();
        out.push({ type: 'image', alt: m.text, src: m.url });
        i = m.end;
        continue;
      }
    }
    if (c === '[') {
      const m = mdMatchLink(s, i);
      if (m) {
        flush();
        out.push({ type: 'link', children: mdParseInline(m.text, style), url: m.url });
        i = m.end;
        continue;
      }
    }
    if (c === '<') {
      const m = s.slice(i).match(/^<((?:https?:\/\/|mailto:)[^>]+)>/);
      if (m) {
        flush();
        out.push({
          type: 'link',
          children: [{ type: 'text', text: m[1], style: Object.assign({}, style) }],
          url: m[1]
        });
        i += m[0].length;
        continue;
      }
    }
    let matched = false;
    const marks = ['***', '___', '**', '__', '~~', '*', '_'];
    for (const mark of marks) {
      if (!s.startsWith(mark, i)) continue;
      if (mark[0] === '_' && i > 0 && i + 1 < s.length &&
          /[\wА-Яа-яЁё]/.test(s[i - 1]) && /[\wА-Яа-яЁё]/.test(s[i + 1])) continue;
      const end = mdFindClose(s, i + mark.length, mark);
      if (end < 0) continue;
      const inner = s.slice(i + mark.length, end);
      if (!inner) continue;
      const st = Object.assign({}, style);
      if (mark === '***' || mark === '___') { st.bold = true; st.italic = true; }
      else if (mark === '**' || mark === '__') st.bold = true;
      else if (mark === '~~') st.strike = true;
      else st.italic = true;
      flush();
      out.push(...mdParseInline(inner, st));
      i = end + mark.length;
      matched = true;
      break;
    }
    if (matched) continue;
    buf += c;
    i++;
  }
  flush();
  return out;
}

function mdSplitRow(line) {
  let s = line.trim();
  if (s.startsWith('|')) s = s.slice(1);
  if (s.endsWith('|')) s = s.slice(0, -1);
  const cells = [];
  let cur = '';
  for (let i = 0; i < s.length; i++) {
    const c = s[i];
    if (c === '\\' && s[i + 1] === '|') { cur += '|'; i++; }
    else if (c === '|') { cells.push(cur.trim()); cur = ''; }
    else cur += c;
  }
  cells.push(cur.trim());
  return cells;
}

function mdIsTableDelim(line) {
  const cells = mdSplitRow(line);
  return cells.length > 0 && cells.every((c) => /^:?-{3,}:?$/.test(c));
}

function mdIsHr(line) {
  return /^ {0,3}((\*\s*){3,}|(-\s*){3,}|(_\s*){3,})$/.test(line);
}

function mdIsBlockStart(line, next) {
  if (/^\s*$/.test(line)) return true;
  if (/^ {0,3}(```+|~~~+)/.test(line)) return true;
  if (/^ {0,3}(#{1,6})\s+/.test(line)) return true;
  if (mdIsHr(line)) return true;
  if (/^ {0,3}>/.test(line)) return true;
  if (/^(\s*)([-+*]|\d+[.)])\s+/.test(line)) return true;
  if (/^ {0,3}!\[([^\]]*)\]\(([^)\s]+)(?:\s+["'][^"']*["'])?\)\s*$/.test(line)) return true;
  if (line.includes('|') && next && mdIsTableDelim(next)) return true;
  return false;
}

function mdJoinParagraph(lines) {
  let out = '';
  for (let i = 0; i < lines.length; i++) {
    const raw = lines[i];
    const hard = / {2,}$/.test(raw) || /\\\s*$/.test(raw);
    let line = raw.replace(/\s+$/, '').replace(/^\s+/, '');
    if (hard) line = line.replace(/\\$/, '');
    out += line;
    if (i < lines.length - 1) out += hard ? '\n' : ' ';
  }
  return out;
}

function mdParseBlocks(src) {
  const lines = String(src).replace(/\r\n?/g, '\n').split('\n');
  const blocks = [];
  let i = 0;
  while (i < lines.length) {
    const line = lines[i];
    if (/^\s*$/.test(line)) { i++; continue; }

    let m = line.match(/^ {0,3}(```+|~~~+)(.*)$/);
    if (m) {
      const fence = m[1][0];
      const len = m[1].length;
      const code = [];
      i++;
      while (i < lines.length) {
        const close = lines[i].match(/^ {0,3}(`{3,}|~{3,})\s*$/);
        if (close && close[1][0] === fence && close[1].length >= len) { i++; break; }
        code.push(lines[i]);
        i++;
      }
      blocks.push({ type: 'code', text: code.join('\n') });
      continue;
    }

    m = line.match(/^ {0,3}(#{1,6})\s+(.*?)\s*#*\s*$/);
    if (m) {
      blocks.push({ type: 'heading', level: m[1].length, text: m[2] });
      i++;
      continue;
    }

    if (i + 1 < lines.length && /^ {0,3}(=+|-+)\s*$/.test(lines[i + 1]) &&
        !/^\s*$/.test(line)) {
      const ch = lines[i + 1].trim()[0];
      blocks.push({ type: 'heading', level: ch === '=' ? 1 : 2, text: line.trim() });
      i += 2;
      continue;
    }

    if (mdIsHr(line)) { blocks.push({ type: 'hr' }); i++; continue; }

    if (/^ {0,3}>/.test(line)) {
      const q = [];
      while (i < lines.length && /^ {0,3}>/.test(lines[i])) {
        q.push(lines[i].replace(/^ {0,3}>\s?/, ''));
        i++;
      }
      blocks.push({ type: 'quote', text: q.join('\n') });
      continue;
    }

    if (line.includes('|') && i + 1 < lines.length && mdIsTableDelim(lines[i + 1])) {
      const header = mdSplitRow(line);
      const delim = mdSplitRow(lines[i + 1]);
      const aligns = delim.map((c) => {
        const l = c.startsWith(':');
        const r = c.endsWith(':');
        return l && r ? 'C' : r ? 'R' : 'L';
      });
      i += 2;
      const rows = [];
      while (i < lines.length && !/^\s*$/.test(lines[i]) && lines[i].includes('|')) {
        const cells = mdSplitRow(lines[i]);
        while (cells.length < header.length) cells.push('');
        rows.push(cells.slice(0, header.length));
        i++;
      }
      blocks.push({ type: 'table', header, aligns, rows });
      continue;
    }

    m = line.match(/^(\s*)([-+*]|\d+[.)])\s+(.*)$/);
    if (m) {
      const items = [];
      while (i < lines.length) {
        const lm = lines[i].match(/^(\s*)([-+*]|\d+[.)])\s+(.*)$/);
        if (!lm) {
          if (/^\s+/.test(lines[i]) && !/^\s*$/.test(lines[i]) && items.length) {
            items[items.length - 1].text += ' ' + lines[i].trim();
            i++;
            continue;
          }
          break;
        }
        const indent = lm[1].replace(/\t/g, '  ').length;
        const level = Math.floor(indent / 2);
        const num = lm[2].match(/^\d+/);
        const marker = num ? (num[0] + '.') : '•';
        items.push({ level, marker, text: lm[3] });
        i++;
        while (i < lines.length && /^\s+/.test(lines[i]) && !/^\s*$/.test(lines[i]) &&
               !/^(\s*)([-+*]|\d+[.)])\s+/.test(lines[i])) {
          items[items.length - 1].text += ' ' + lines[i].trim();
          i++;
        }
      }
      blocks.push({ type: 'list', items });
      continue;
    }

    m = line.match(/^ {0,3}!\[([^\]]*)\]\(([^)\s]+)(?:\s+["'][^"']*["'])?\)\s*$/);
    if (m) {
      blocks.push({ type: 'image', alt: m[1], src: m[2] });
      i++;
      continue;
    }

    if (/^ {4}/.test(line)) {
      const code = [];
      while (i < lines.length) {
        if (/^\s*$/.test(lines[i])) { code.push(''); i++; continue; }
        if (!/^ {4}/.test(lines[i])) break;
        code.push(lines[i].slice(4));
        i++;
      }
      while (code.length && code[code.length - 1] === '') code.pop();
      blocks.push({ type: 'code', text: code.join('\n') });
      continue;
    }

    const p = [];
    while (i < lines.length && !/^\s*$/.test(lines[i])) {
      if (p.length && mdIsBlockStart(lines[i], lines[i + 1])) break;
      p.push(lines[i]);
      i++;
    }
    blocks.push({ type: 'paragraph', text: mdJoinParagraph(p) });
  }
  return blocks;
}

function mdCollectImages(model) {
  const found = [];
  const add = (alt, src) => {
    if (!src || found.some((x) => x.src === src)) return;
    found.push({ alt: alt || '', src });
  };
  const walkInline = (nodes) => {
    for (const n of nodes || []) {
      if (n.type === 'image') add(n.alt, n.src);
      else if (n.type === 'link') walkInline(n.children);
    }
  };
  for (const b of model) {
    if (b.type === 'image') add(b.alt, b.src);
    else if (b.type === 'paragraph' || b.type === 'quote' || b.type === 'heading') {
      walkInline(mdParseInline(b.text));
    } else if (b.type === 'list') {
      for (const it of b.items) walkInline(mdParseInline(it.text));
    } else if (b.type === 'table') {
      for (const cell of b.header) walkInline(mdParseInline(cell));
      for (const row of b.rows) for (const cell of row) walkInline(mdParseInline(cell));
    }
  }
  return found;
}

/* ----- разбор тегов в символы и переносы ----- */
function mdNodesToChars(nodes, ctx, opts) {
  const chars = [];
  const pushText = (text, style) => {
    for (const ch of String(text)) {
      if (ch === '\n') chars.push({ br: true });
      else chars.push({ ch, style: style || {} });
    }
  };
  for (const n of nodes || []) {
    if (n.type === 'text') {
      pushText(n.text, n.style || {});
    } else if (n.type === 'br') {
      chars.push({ br: true });
    } else if (n.type === 'link') {
      const before = chars.length;
      chars.push(...mdNodesToChars(n.children, ctx, opts));
      const label = mdPlain(n.children);
      const empty = chars.length === before;
      if (empty) {
        pushText(n.url, { code: true });
      } else if (n.url && n.url !== label) {
        pushText(' (' + n.url + ')', { code: true });
      }
      if (opts.qrLinks && /^https?:|^mailto:/i.test(n.url)) ctx.links.push(n.url);
    } else if (n.type === 'image') {
      const im = md.images.find((x) => x.src === n.src);
      if (opts.images && im && im.fileName &&
          (im.status === 'local' || im.status === 'uploaded' ||
           (opts.includeReady && im.status === 'ready'))) {
        ctx.inlineImages.push(im.fileName);
        pushText(n.alt || im.fileName, {});
      } else {
        pushText(n.alt || n.src, {});
        if (n.src && n.src !== n.alt && !/^data:/i.test(n.src)) {
          pushText(' (' + n.src + ')', { code: true });
        }
        if (opts.qrLinks && /^https?:/i.test(n.src)) ctx.links.push(n.src);
      }
    }
  }
  return chars;
}

function mdWrapChars(chars, width) {
  const lines = [];
  let line = [];
  let w = 0;
  const trimEnd = (arr) => {
    while (arr.length && arr[arr.length - 1].ch === ' ') arr.pop();
    return arr;
  };
  const pushLine = () => {
    const out = trimEnd(line);
    if (out.length) lines.push(out);
    line = [];
    w = 0;
  };
  for (const item of chars) {
    if (item.br) { pushLine(); continue; }
    if (line.length === 0 && item.ch === ' ') continue;
    line.push(item);
    w += mdCharWidth(item.ch);
    if (w <= width) continue;

    let sp = -1;
    for (let k = line.length - 2; k > 0; k--) {
      if (line[k].ch === ' ') { sp = k; break; }
    }
    if (sp > 0) {
      const out = trimEnd(line.slice(0, sp));
      if (out.length) lines.push(out);
      line = line.slice(sp + 1);
    } else {
      let cut = 0;
      let cw = 0;
      while (cut < line.length) {
        const cwid = mdCharWidth(line[cut].ch);
        if (cw + cwid > width) break;
        cw += cwid;
        cut++;
      }
      if (cut === 0) cut = 1;
      lines.push(line.slice(0, cut));
      line = line.slice(cut);
    }
    w = 0;
    for (const c of line) w += mdCharWidth(c.ch);
    while (line.length && line[0].ch === ' ') { line.shift(); w--; }
  }
  pushLine();
  return lines;
}

function mdLineTags(line) {
  let out = '';
  let i = 0;
  while (i < line.length) {
    const st = line[i].style || {};
    const stUl = !!st.underline || !!st.italic;
    let j = i;
    while (j < line.length) {
      const a = line[j].style || {};
      const aUl = !!a.underline || !!a.italic;
      if (!!a.bold !== !!st.bold || aUl !== stUl ||
          !!a.strike !== !!st.strike || !!a.code !== !!st.code) break;
      j++;
    }
    let text = '';
    for (let k = i; k < j; k++) text += line[k].ch;
    if (st.code) out += '[setFont,B]';
    if (st.bold) out += '[boldOn]';
    if (stUl) out += '[underlineOn]';
    if (st.strike) out += '[strikeOn]';
    out += mdEscTag(text);
    if (st.strike) out += '[strikeOff]';
    if (stUl) out += '[underlineOff]';
    if (st.bold) out += '[boldOff]';
    if (st.code) out += '[setFont,A]';
    i = j;
  }
  return out;
}

function mdRenderInlineLines(nodes, width, ctx, opts, prefixFirst, prefixRest) {
  const chars = mdNodesToChars(nodes, ctx, opts);
  const lines = mdWrapChars(chars, width);
  let out = '';
  for (let i = 0; i < lines.length; i++) {
    out += (i === 0 ? prefixFirst : prefixRest) + mdLineTags(lines[i]) + '\n';
  }
  return out;
}

function mdQrSafe(url) {
  return String(url).replace(/\[/g, '%5B').replace(/\]/g, '%5D');
}

function mdRenderImageBlock(alt, src, ctx, opts) {
  const im = md.images.find((x) => x.src === src);
  if (opts.images && im && im.fileName &&
      (im.status === 'local' || im.status === 'uploaded' ||
       (opts.includeReady && im.status === 'ready'))) {
    return '[image,' + im.fileName + ']\n';
  }
  let text = alt || src;
  let out = mdEscTag(text);
  if (src && src !== text && !/^data:/i.test(src)) out += ' (' + mdEscTag(src) + ')';
  out += '\n';
  if (opts.qrLinks && /^https?:/i.test(src)) ctx.links.push(src);
  return out;
}

function mdRenderTags(model, opts) {
  opts = opts || {};
  let out = '[setDefault]';
  const ctx = { links: [], inlineImages: [] };
  const qrSeen = new Set();
  const addQr = () => {
    if (!opts.qrLinks) { ctx.links = []; return; }
    for (const url of ctx.links) {
      if (!/^https?:|^mailto:/i.test(url) || url.length > 690) continue;
      const key = url.toLowerCase();
      if (qrSeen.has(key)) continue;
      qrSeen.add(key);
      out += '[qr,' + mdQrSafe(url) + ']';
    }
    ctx.links = [];
  };

  for (const b of model) {
    if (b.type === 'heading') {
      const plain = mdPlain(mdParseInline(b.text)).trim();
      if (!plain) continue;
      const width = b.level === 1 ? 16 : 32;
      const size = b.level === 1 ? 'L' : b.level === 2 ? 'M' : 'S';
      const chars = [];
      for (const ch of plain) chars.push({ ch, style: {} });
      const lines = mdWrapChars(chars, width);
      out += '[setSize,' + size + '][boldOn]';
      if (b.level === 3) out += '[underlineOn]';
      out += mdLineTags(lines[0] || []);
      for (let i = 1; i < lines.length; i++) out += '\n' + mdLineTags(lines[i]);
      if (b.level === 3) out += '[underlineOff]';
      out += '[boldOff][setSize,S]\n\n';
      addQr();
    } else if (b.type === 'paragraph') {
      const nodes = mdParseInline(b.text);
      out += mdRenderInlineLines(nodes, 32, ctx, opts, '', '');
      out += '\n';
      addQr();
    } else if (b.type === 'list') {
      for (const it of b.items) {
        const indent = '  '.repeat(Math.min(it.level, 4));
        const marker = it.marker + ' ';
        const width = Math.max(8, 32 - indent.length - marker.length);
        const nodes = mdParseInline(it.text);
        out += mdRenderInlineLines(nodes, width, ctx, opts,
          indent + marker, indent + ' '.repeat(marker.length));
      }
      out += '\n';
      addQr();
    } else if (b.type === 'quote') {
      const nodes = mdParseInline(b.text);
      out += mdRenderInlineLines(nodes, 30, ctx, opts, '> ', '> ');
      out += '\n';
      addQr();
    } else if (b.type === 'code') {
      out += '[setFont,B][setLineHeight,24]';
      for (const line of String(b.text).split('\n')) out += mdEscTag(line) + '\n';
      out += '[setLineHeight,30][setFont,A]\n\n';
    } else if (b.type === 'hr') {
      out += '-'.repeat(32) + '\n\n';
    } else if (b.type === 'table') {
      out += mdRenderTable(b, opts) + '\n';
      addQr();
    } else if (b.type === 'image') {
      const imgText = mdRenderImageBlock(b.alt, b.src, ctx, opts);
      out += imgText;
      if (!imgText.startsWith('[image,')) out += '\n';
      addQr();
    }
  }
  return out;
}

function mdStrWidth(s) {
  let w = 0;
  for (const ch of String(s)) w += mdCharWidth(ch);
  return w;
}

function mdTruncateWidth(s, max) {
  s = String(s || '');
  if (mdStrWidth(s) <= max) return s;
  let out = '';
  let w = 0;
  const limit = Math.max(0, max - 2);
  for (const ch of s) {
    const cw = mdCharWidth(ch);
    if (w + cw > limit) break;
    out += ch;
    w += cw;
  }
  return out + '..';
}

function mdPadWidth(s, w, align) {
  s = mdTruncateWidth(s, w);
  const pad = Math.max(0, w - mdStrWidth(s));
  if (align === 'R') return ' '.repeat(pad) + s;
  if (align === 'C') {
    const left = Math.floor(pad / 2);
    return ' '.repeat(left) + s + ' '.repeat(pad - left);
  }
  return s + ' '.repeat(pad);
}

function mdRenderTable(b, opts) {
  const cols = b.header.length;
  const widths = new Array(cols).fill(3);
  const cellPlain = (c) => mdPlain(mdParseInline(c)).replace(/\s+/g, ' ').trim();
  const header = b.header.map(cellPlain);
  const rows = b.rows.map((r) => r.map(cellPlain));
  for (let c = 0; c < cols; c++) {
    widths[c] = Math.max(widths[c], Math.min(20, mdStrWidth(header[c])));
    for (const r of rows) widths[c] = Math.max(widths[c], Math.min(20, mdStrWidth(r[c] || '')));
  }
  const sep = 3 * (cols - 1);
  let total = widths.reduce((a, v) => a + v, 0) + sep;
  while (total > 42) {
    let max = 0;
    for (let c = 1; c < cols; c++) if (widths[c] > widths[max]) max = c;
    if (widths[max] <= 3) break;
    widths[max]--;
    total--;
  }
  const pad = (s, w, align) => mdPadWidth(s, w, align);
  const line = (cells) => cells.map((c, i) =>
    mdEscTag(pad(c, widths[i], b.aligns[i] || 'L'))).join(' | ');
  let out = '[setFont,B]' + line(header) + '\n';
  out += widths.map((w) => '-'.repeat(w)).join('-+-') + '\n';
  for (const r of rows) out += line(r) + '\n';
  out += '[setFont,A]';
  return out;
}

/* ----- предпросмотр ----- */
function mdInlineHtml(nodes) {
  let out = '';
  for (const n of nodes || []) {
    if (n.type === 'text') {
      let t = mdEscHtml(n.text);
      if (n.style && n.style.code) t = '<code>' + t + '</code>';
      if (n.style && n.style.bold) t = '<strong>' + t + '</strong>';
      if (n.style && n.style.italic) t = '<u>' + t + '</u>';
      if (n.style && n.style.strike) t = '<s>' + t + '</s>';
      out += t;
    } else if (n.type === 'br') {
      out += '<br>';
    } else if (n.type === 'link') {
      out += '<a href="' + mdEscHtml(n.url) + '" target="_blank" rel="noopener">' +
        mdInlineHtml(n.children) + '</a>';
      if (mdPlain(n.children) !== n.url) out += ' <span class="muted">(' + mdEscHtml(n.url) + ')</span>';
    } else if (n.type === 'image') {
      out += mdEscHtml(n.alt || n.src);
    }
  }
  return out;
}

function mdImagePreviewHtml(src, alt) {
  const im = md.images.find((x) => x.src === src);
  if (im && im.preview) {
    return '<figure><img src="' + im.preview + '" alt=""><figcaption class="muted">' +
      mdEscHtml(alt || src) + '</figcaption></figure>';
  }
  return '<div class="muted">[картинка] ' + mdEscHtml(alt || '') + ' — ' +
    mdEscHtml(src.length > 90 ? src.slice(0, 87) + '...' : src) + '</div>';
}

function mdRenderPreview(model) {
  let out = '';
  for (const b of model) {
    if (b.type === 'heading') {
      const plain = mdPlain(mdParseInline(b.text)).trim();
      out += '<h' + Math.min(4, b.level) + '>' + mdEscHtml(plain) + '</h' + Math.min(4, b.level) + '>';
    } else if (b.type === 'paragraph') {
      out += '<p>' + mdInlineHtml(mdParseInline(b.text)) + '</p>';
    } else if (b.type === 'list') {
      for (const it of b.items) {
        out += '<div style="padding-left:' + (it.level * 1.2) + 'em">' +
          mdEscHtml(it.marker) + ' ' + mdInlineHtml(mdParseInline(it.text)) + '</div>';
      }
    } else if (b.type === 'quote') {
      out += '<blockquote>' + mdInlineHtml(mdParseInline(b.text)) + '</blockquote>';
    } else if (b.type === 'code') {
      out += '<pre>' + mdEscHtml(b.text) + '</pre>';
    } else if (b.type === 'hr') {
      out += '<hr>';
    } else if (b.type === 'table') {
      out += '<table><thead><tr>';
      for (const c of b.header) out += '<th>' + mdEscHtml(mdPlain(mdParseInline(c))) + '</th>';
      out += '</tr></thead><tbody>';
      for (const r of b.rows) {
        out += '<tr>';
        for (const c of r) out += '<td>' + mdEscHtml(mdPlain(mdParseInline(c))) + '</td>';
        out += '</tr>';
      }
      out += '</tbody></table>';
    } else if (b.type === 'image') {
      out += mdImagePreviewHtml(b.src, b.alt);
    }
  }
  return out || '<span class="muted">Пустой документ.</span>';
}

/* ----- картинки Markdown ----- */
function mdBaseName(src) {
  const s = String(src).split(/[?#]/)[0];
  const p = s.split('/');
  return p[p.length - 1] || s;
}

function mdImageName() {
  md.seq++;
  return 'mdimg_' + Date.now() + '_' + md.seq + '.bin';
}

function mdBitmapBuffer(bmp) {
  const buf = new Uint8Array(4 + bmp.bits.length);
  const dv = new DataView(buf.buffer);
  dv.setUint16(0, bmp.w, true);
  dv.setUint16(2, bmp.h, true);
  buf.set(bmp.bits, 4);
  return buf;
}

function mdLoadImage(blob) {
  return new Promise((resolve, reject) => {
    const url = URL.createObjectURL(blob);
    const im = new Image();
    im.onload = () => { URL.revokeObjectURL(url); resolve(im); };
    im.onerror = () => { URL.revokeObjectURL(url); reject(new Error('не удалось открыть изображение')); };
    im.src = url;
  });
}

async function mdBitmapPreview(fileName) {
  try {
    const res = await fetch('/api/sd/file?name=' + encodeURIComponent(fileName));
    if (!res.ok) return '';
    return mdBitmapDataUrl(new Uint8Array(await res.arrayBuffer()));
  } catch (e) {
    return '';
  }
}

function mdBitmapDataUrl(buf) {
  try {
    if (buf.length < 4) return '';
    const dv = new DataView(buf.buffer, buf.byteOffset, buf.byteLength);
    const w = dv.getUint16(0, true);
    const h = dv.getUint16(2, true);
    if (!w || !h || w > 384 || h > 2000) return '';
    const stride = Math.ceil(w / 8);
    const data = buf.subarray(4);
    if (data.length < stride * h) return '';
    const c = document.createElement('canvas');
    c.width = w; c.height = h;
    const ctx = c.getContext('2d');
    const id = ctx.createImageData(w, h);
    for (let y = 0; y < h; y++) {
      for (let x = 0; x < w; x++) {
        const bit = (data[y * stride + (x >> 3)] >> (7 - (x & 7))) & 1;
        const p = (y * w + x) * 4;
        const v = bit ? 0 : 255;
        id.data[p] = v; id.data[p + 1] = v; id.data[p + 2] = v; id.data[p + 3] = 255;
      }
    }
    ctx.putImageData(id, 0, 0);
    return c.toDataURL('image/png');
  } catch (e) {
    return '';
  }
}

async function mdConvertImageFile(im, blob) {
  if (/\.(bin|raw)$/i.test(blob.name || '')) {
    const buf = new Uint8Array(await blob.arrayBuffer());
    if (buf.length < 4) throw new Error('короткий .bin');
    const dv = new DataView(buf.buffer);
    const w = dv.getUint16(0, true);
    const h = dv.getUint16(2, true);
    if (!w || !h || w > 384 || h > 2000) throw new Error('неверный .bin');
    im.blob = buf;
    im.fileName = mdBaseName(blob.name);
    im.status = 'ready';
    im.preview = mdBitmapDataUrl(buf);
    return;
  }
  const image = await mdLoadImage(blob);
  const bmp = imageToBitmap(
    image,
    Number($('#brightness').value),
    Number($('#contrast').value),
    $('#dither').value,
    $('#invert').checked
  );
  im.blob = mdBitmapBuffer(bmp);
  im.fileName = mdImageName();
  im.status = 'ready';
  const url = URL.createObjectURL(blob);
  im.preview = url;
}

async function mdPrepareImages() {
  for (const im of md.images) {
    if (im.status === 'local' || im.status === 'uploaded' || im.status === 'ready') continue;
    if (/^https?:/i.test(im.src) || /^data:/i.test(im.src)) {
      try {
        const res = await fetch(im.src, { mode: 'cors' });
        if (!res.ok) throw new Error('HTTP ' + res.status);
        const blob = await res.blob();
        await mdConvertImageFile(im, blob);
      } catch (e) {
        im.status = 'error';
        im.error = e.message || 'ошибка загрузки';
      }
    } else {
      const name = mdBaseName(im.src);
      const f = sdFiles.find((x) => x.name.toLowerCase() === name.toLowerCase() &&
        x.kind === 'image' && x.valid !== false);
      if (f) {
        im.status = 'local';
        im.fileName = f.name;
        im.preview = await mdBitmapPreview(f.name);
      } else {
        im.status = 'missing';
        im.error = 'нет на SD';
      }
    }
  }
  renderMdImages();
  renderMdOutput();
}

function renderMdImages() {
  const box = $('#mdImages');
  box.innerHTML = '';
  if (!md.images.length) {
    box.textContent = 'В документе нет картинок.';
    return;
  }
  md.images.forEach((im) => {
    const item = document.createElement('div');
    item.className = 'sd-item';
    const info = document.createElement('div');
    info.className = 'sd-info md-img-row';
    if (im.preview) {
      const img = document.createElement('img');
      img.src = im.preview;
      img.alt = '';
      info.append(img);
    }
    const txt = document.createElement('div');
    txt.style.flex = '1';
    const name = document.createElement('div');
    name.className = 'sd-name';
    name.textContent = im.alt || im.fileName || im.src;
    const meta = document.createElement('div');
    meta.className = 'sd-meta';
    const status = im.status === 'local' ? 'на SD'
      : im.status === 'ready' ? 'готова к загрузке'
      : im.status === 'uploaded' ? 'загружена на SD'
      : im.status === 'error' ? 'ошибка: ' + (im.error || '')
      : im.status === 'missing' ? 'нет на SD'
      : 'ожидает';
    const dispSrc = im.src.length > 90 ? im.src.slice(0, 87) + '...' : im.src;
    meta.textContent = status + ' · ' + dispSrc;
    txt.append(name, meta);
    info.append(txt);
    item.append(info);

    if (im.status !== 'local' && im.status !== 'uploaded') {
      const row = document.createElement('div');
      row.className = 'sd-actions';
      const input = document.createElement('input');
      input.type = 'file';
      input.accept = 'image/*,.bin,.raw';
      input.addEventListener('change', async (e) => {
        const f = e.target.files && e.target.files[0];
        if (!f) return;
        try {
          im.status = 'ready';
          im.error = '';
          await mdConvertImageFile(im, f);
          renderMdImages();
          renderMdOutput();
        } catch (err) {
          im.status = 'error';
          im.error = err.message;
          renderMdImages();
          renderMdOutput();
        }
      });
      row.append(input);
      item.append(row);
    }
    box.append(item);
  });
}

/* ----- состояние и кнопки Markdown ----- */
function mdSetStatus(text, ok) {
  const el = $('#mdStatus');
  el.textContent = text;
  el.style.color = ok === false ? 'var(--err)' : '';
}

function mdReleasePreviews() {
  for (const im of md.images) {
    if (im.preview && im.preview.startsWith('blob:')) {
      try { URL.revokeObjectURL(im.preview); } catch (e) {}
    }
  }
}

function mdSourceLoaded(name, text) {
  md.sourceName = name || 'document.md';
  md.sourceText = text;
  md.converted = false;
  mdReleasePreviews();
  md.model = null;
  md.tags = '';
  md.images = [];
  md.preparePromise = null;
  $('#btnMdSaveSd').disabled = true;
  $('#btnMdToText').disabled = true;
  $('#mdTags').textContent = '';
  $('#mdPreview').innerHTML = '<span class="muted">Нажмите «Конвертировать».</span>';
  renderMdImages();
  const base = md.sourceName.replace(/\.[^.]+$/, '') || 'document';
  $('#mdSaveName').value = base + '.txt';
  mdSetStatus('Загружено: ' + md.sourceName + ' · нажмите «Конвертировать»');
}

function mdConvert() {
  if (!md.sourceText.trim()) return toast('Сначала загрузите .md', false);
  md.model = mdParseBlocks(md.sourceText);
  const found = mdCollectImages(md.model);
  mdReleasePreviews();
  md.images = found.map((x) => ({
    src: x.src, alt: x.alt, status: 'pending', fileName: '', blob: null,
    preview: '', error: '', uploaded: false
  }));
  md.converted = true;
  $('#btnMdSaveSd').disabled = false;
  $('#btnMdToText').disabled = false;
  mdSetStatus('Конвертация выполнена. Картинок: ' + md.images.length);
  renderMdImages();
  renderMdOutput();
  md.preparePromise = mdPrepareImages();
}

function renderMdOutput() {
  if (!md.model) return;
  const opts = {
    qrLinks: $('#mdQrLinks').checked,
    images: $('#mdImgSd').checked,
    includeReady: true
  };
  md.tags = mdRenderTags(md.model, opts);
  $('#mdTags').textContent = md.tags;
  $('#mdPreview').innerHTML = mdRenderPreview(md.model);
  const showTags = $('#mdShowTags').checked;
  $('#mdTags').hidden = !showTags;
  $('#mdPreview').hidden = showTags;
}

function mdFileLoaded(file) {
  if (!file) return;
  const reader = new FileReader();
  reader.onload = () => mdSourceLoaded(file.name, String(reader.result || ''));
  reader.onerror = () => toast('Не удалось прочитать файл', false);
  reader.readAsText(file, 'utf-8');
}

async function mdLoadFromSd(name) {
  if (!name) return toast('Выберите .md с SD', false);
  try {
    const res = await fetch('/api/sd/file?name=' + encodeURIComponent(name));
    if (!res.ok) throw new Error('HTTP ' + res.status);
    mdSourceLoaded(name, await res.text());
    document.querySelector('[data-tab=markdown]').click();
  } catch (e) { toast(e.message, false); }
}

$('#mdFile').addEventListener('change', (e) => {
  const f = e.target.files && e.target.files[0];
  mdFileLoaded(f);
});

$('#btnMdLoadSd').addEventListener('click', () => mdLoadFromSd($('#mdSdSelect').value));
$('#btnMdConvert').addEventListener('click', mdConvert);
$('#mdQrLinks').addEventListener('change', renderMdOutput);
$('#mdImgSd').addEventListener('change', renderMdOutput);
$('#mdShowTags').addEventListener('change', renderMdOutput);

function refreshMdSdList() {
  const sel = $('#mdSdSelect');
  if (!sel) return;
  const cur = sel.value;
  sel.innerHTML = '<option value="">— выберите —</option>';
  sdFiles.filter((f) => /\.(md|markdown)$/i.test(f.name)).sort((a, b) => a.name.localeCompare(b.name))
    .forEach((f) => {
      const o = document.createElement('option');
      o.value = f.name;
      o.textContent = f.name;
      sel.appendChild(o);
    });
  if (cur) sel.value = cur;
}

async function mdUploadIfNeeded() {
  for (const im of md.images) {
    if (im.status === 'local' || im.status === 'uploaded' || !im.fileName) continue;
    if (!im.blob) continue;
    const fd = new FormData();
    fd.append('file', new Blob([im.blob], { type: 'application/octet-stream' }), im.fileName);
    const r = await api('/api/sd/upload', { method: 'POST', body: fd });
    im.fileName = r.name || im.fileName;
    im.status = 'uploaded';
    im.uploaded = true;
  }
  renderMdImages();
  renderMdOutput();
}

$('#btnMdSaveSd').addEventListener('click', async () => {
  if (!md.converted) return toast('Сначала сконвертируйте документ', false);
  const btn = $('#btnMdSaveSd');
  btn.disabled = true;
  try {
    if (md.preparePromise) await md.preparePromise;
    await mdUploadIfNeeded();
    const bad = md.images.filter((x) => x.status !== 'local' && x.status !== 'uploaded');
    if (bad.length && !confirm('Не все картинки готовы. Они будут заменены alt/URL. Продолжить?')) return;
    const opts = { qrLinks: $('#mdQrLinks').checked, images: $('#mdImgSd').checked };
    const tags = mdRenderTags(md.model, opts);
    const bytes = new TextEncoder().encode(tags).length;
    if (bytes > 65536) return toast('Текст больше 64 КБ — сократите документ', false);
    let name = $('#mdSaveName').value.trim() || (md.sourceName.replace(/\.[^.]+$/, '') + '.txt');
    if (!/\.txt$/i.test(name)) name += '.txt';
    if (sdFiles.some((f) => f.name.toLowerCase() === name.toLowerCase()) &&
        !confirm('Файл ' + name + ' уже есть. Перезаписать?')) return;
    const fd = new FormData();
    fd.append('file', new Blob([tags], { type: 'text/plain;charset=utf-8' }), name);
    const r = await api('/api/sd/upload', { method: 'POST', body: fd });
    toast('Сохранено на SD: ' + r.name);
    await refreshSd();
  } catch (e) {
    toast(e.message, false);
  } finally {
    btn.disabled = false;
  }
});

$('#btnMdToText').addEventListener('click', async () => {
  if (!md.converted) return toast('Сначала сконвертируйте документ', false);
  try {
    if (md.preparePromise) await md.preparePromise;
  } catch (e) { /* ignore */ }
  const tags = mdRenderTags(md.model, { qrLinks: $('#mdQrLinks').checked, images: true });
  $('#text').value = tags;
  updateCharCount();
  document.querySelector('[data-tab=text]').click();
  const bytes = new TextEncoder().encode(tags).length;
  const hasUnprinted = md.images.some((x) => x.status !== 'local' && x.status !== 'uploaded');
  if (bytes > 4096) {
    toast('Текст в поле, но он длиннее 4096 байт — для печати сохраните на SD', false);
  } else if (hasUnprinted) {
    toast('Текст перенесён; картинки без SD заменены на alt/URL', false);
  } else {
    toast('Текст перенесён в поле');
  }
});

/* ---------- изображение ---------- */
let img = null;
let currentBitmap = null;
let sdFiles = [];
let previewTimer = 0;

function schedulePreview() {
  clearTimeout(previewTimer);
  previewTimer = setTimeout(renderPreview, 120);
}

function getGray(imageData, w, h, brightness, contrast) {
  const src = imageData.data;
  const out = new Float32Array(w * h);
  const b = brightness * 1.27;
  const c = (contrast + 100) / 100;
  for (let i = 0, p = 0; i < out.length; i++, p += 4) {
    let v = 0.299 * src[p] + 0.587 * src[p + 1] + 0.114 * src[p + 2];
    v = (v - 128) * c + 128 + b;
    out[i] = Math.max(0, Math.min(255, v));
  }
  return out;
}

function ditherFloyd(gray, w, h) {
  const buf = new Float32Array(gray);
  const bits = new Uint8Array((w * h) >> 3);
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const i = y * w + x;
      const old = buf[i];
      const nv = old < 128 ? 0 : 255;
      if (nv === 0) bits[i >> 3] |= 0x80 >> (x & 7);
      const err = old - nv;
      if (x + 1 < w) buf[i + 1] += err * 7 / 16;
      if (y + 1 < h) {
        if (x > 0) buf[i + w - 1] += err * 3 / 16;
        buf[i + w] += err * 5 / 16;
        if (x + 1 < w) buf[i + w + 1] += err * 1 / 16;
      }
    }
  }
  return bits;
}

function ditherOrdered(gray, w, h) {
  const m = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]];
  const bits = new Uint8Array((w * h) >> 3);
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const i = y * w + x;
      const t = (m[y & 3][x & 3] + 0.5) / 16 * 255;
      if (gray[i] < t) bits[i >> 3] |= 0x80 >> (x & 7);
    }
  }
  return bits;
}

function ditherThreshold(gray, w, h) {
  const bits = new Uint8Array((w * h) >> 3);
  for (let i = 0; i < gray.length; i++) {
    if (gray[i] < 128) bits[i >> 3] |= 0x80 >> (i & 7);
  }
  return bits;
}

function drawPreview(bits, w, h) {
  const c = $('#preview');
  c.width = w; c.height = h;
  const ctx = c.getContext('2d');
  const id = ctx.createImageData(w, h);
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const i = y * w + x;
      const bit = (bits[i >> 3] >> (7 - (x & 7))) & 1;
      const p = i * 4;
      const v = bit ? 0 : 255;
      id.data[p] = v; id.data[p + 1] = v; id.data[p + 2] = v; id.data[p + 3] = 255;
    }
  }
  ctx.putImageData(id, 0, 0);
}

function estimateSeconds(h, bytes) {
  const data = bytes * 10 / 9600;       // 9600 бод, 8N1
  const head = h / 720;                  // ~90 мм/с = 720 строк/с
  return Math.ceil(data + head);
}

function imageToBitmap(im, brightness, contrast, method, invert) {
  const W = 384;
  const MAXH = 2000;
  let drawW = W;
  let h = Math.round(im.height * W / im.width);
  if (h > MAXH) {
    h = MAXH;
    drawW = Math.max(1, Math.round(im.width * MAXH / im.height));
  }
  h = Math.max(1, h);

  const c = document.createElement('canvas');
  c.width = W; c.height = h;
  const ctx = c.getContext('2d');
  ctx.fillStyle = '#fff';
  ctx.fillRect(0, 0, W, h);
  ctx.drawImage(im, Math.floor((W - drawW) / 2), 0, drawW, h);

  const data = ctx.getImageData(0, 0, W, h);
  const gray = getGray(data, W, h, brightness, contrast);
  let bits = method === 'ordered' ? ditherOrdered(gray, W, h)
           : method === 'threshold' ? ditherThreshold(gray, W, h)
           : ditherFloyd(gray, W, h);
  if (invert) {
    for (let i = 0; i < bits.length; i++) bits[i] ^= 0xFF;
  }
  return { w: W, h, bits };
}

function renderPreview() {
  if (!img) return;
  currentBitmap = imageToBitmap(
    img,
    Number($('#brightness').value),
    Number($('#contrast').value),
    $('#dither').value,
    $('#invert').checked
  );
  const { w, h, bits } = currentBitmap;
  drawPreview(bits, w, h);
  const bytes = 4 + bits.length;
  const secs = estimateSeconds(h, bytes);
  $('#imgInfo').textContent = w + '×' + h + ' px · ' + bytes + ' байт · ~' +
    secs + ' с при 9600 бод' + (secs > 60 ? ' · долго' : '');
}

$('#file').addEventListener('change', (e) => {
  const f = e.target.files && e.target.files[0];
  if (!f) return;
  const url = URL.createObjectURL(f);
  const im = new Image();
  im.onload = () => { img = im; URL.revokeObjectURL(url); renderPreview(); };
  im.onerror = () => { URL.revokeObjectURL(url); toast('Не удалось открыть изображение', false); };
  im.src = url;
});
$('#dither').addEventListener('change', schedulePreview);
$('#brightness').addEventListener('input', schedulePreview);
$('#contrast').addEventListener('input', schedulePreview);
$('#invert').addEventListener('change', schedulePreview);
$('#btnImgRefresh').addEventListener('click', renderPreview);

function bitmapBuffer() {
  const { w, h, bits } = currentBitmap;
  const buf = new Uint8Array(4 + bits.length);
  const dv = new DataView(buf.buffer);
  dv.setUint16(0, w, true);
  dv.setUint16(2, h, true);
  buf.set(bits, 4);
  return buf;
}

$('#btnImg').addEventListener('click', async () => {
  if (!currentBitmap) return toast('Сначала выберите изображение', false);
  const { w, h } = currentBitmap;
  const buf = bitmapBuffer();

  const fd = new FormData();
  fd.append('file', new Blob([buf], { type: 'application/octet-stream' }), 'image.bin');
  const btn = $('#btnImg');
  btn.disabled = true;
  btn.textContent = 'Печать...';
  try {
    await api('/api/bitmap', { method: 'POST', body: fd });
    toast('Изображение напечатано');
  } catch (e) {
    toast(e.message, false);
  } finally {
    btn.disabled = false;
    btn.textContent = 'Печать фото';
  }
});

$('#btnImgSave').addEventListener('click', async () => {
  if (!currentBitmap) return toast('Сначала выберите изображение', false);

  let name = $('#sdSaveName').value.trim();
  if (!name) {
    const d = new Date();
    const pad = (x) => String(x).padStart(2, '0');
    name = 'img_' + d.getFullYear() + pad(d.getMonth() + 1) + pad(d.getDate()) +
      '_' + pad(d.getHours()) + pad(d.getMinutes()) + pad(d.getSeconds()) + '.bin';
  }
  if (!/\.(bin|raw)$/i.test(name)) name += '.bin';

  if (sdFiles.some((f) => f.name.toLowerCase() === name.toLowerCase())) {
    if (!confirm('Файл ' + name + ' уже есть. Перезаписать?')) return;
  }

  const fd = new FormData();
  fd.append('file', new Blob([bitmapBuffer()], { type: 'application/octet-stream' }), name);
  const btn = $('#btnImgSave');
  btn.disabled = true;
  try {
    const r = await api('/api/sd/upload', { method: 'POST', body: fd });
    toast('Сохранено на SD: ' + r.name);
    $('#sdSaveName').value = '';
    refreshSd();
  } catch (e) {
    toast(e.message, false);
  } finally {
    btn.disabled = false;
  }
});

/* ---------- QR и штрихкод ---------- */
$('#btnQr').addEventListener('click', async () => {
  const data = $('#qrData').value.trim();
  if (!data) return toast('Введите данные для QR', false);
  try {
    await jsonPost('/api/qr', { data, module: Number($('#qrSize').value), feed: 2 });
    toast('QR отправлен на печать');
  } catch (e) { toast(e.message, false); }
});

$('#btnBc').addEventListener('click', async () => {
  const data = $('#bcData').value.trim();
  if (!data) return toast('Введите данные штрихкода', false);
  try {
    await jsonPost('/api/barcode', {
      type: $('#bcType').value,
      data,
      height: Number($('#bcHeight').value),
      width: Number($('#bcWidth').value),
      feed: 2
    });
    toast('Штрихкод отправлен на печать');
  } catch (e) { toast(e.message, false); }
});

$('#btnBcTest').addEventListener('click', async () => {
  try { await api('/api/test/barcode', { method: 'POST' }); toast('Печать всех типов штрихкодов'); }
  catch (e) { toast(e.message, false); }
});

/* ---------- SD-карта ---------- */
function renderSdList() {
  const box = $('#sdList');
  box.innerHTML = '';
  if (!sdFiles.length) {
    box.textContent = 'Файлов нет';
    return;
  }

  sdFiles.slice().sort((a, b) => a.name.localeCompare(b.name)).forEach((f) => {
    const item = document.createElement('div');
    item.className = 'sd-item';

    const kind = f.kind || (f.raw ? 'raw' : 'image');

    const info = document.createElement('div');
    info.className = 'sd-info';
    const name = document.createElement('div');
    name.className = 'sd-name';
    name.textContent = f.name;
    const meta = document.createElement('div');
    meta.className = 'sd-meta';
    const dims = kind === 'raw' ? 'raw ESC/POS'
      : kind === 'text' ? 'текст с тегами'
      : kind === 'plain' ? 'текст без тегов'
      : f.valid ? (f.width + '×' + f.height) : 'неверный формат';
    meta.textContent = fmtBytes(f.size) + ' · ' + dims;
    info.append(name, meta);

    const actions = document.createElement('div');
    actions.className = 'sd-actions';

    const printBtn = mkBtn('Печать', () => printSd(f.name), 'primary');
    if (kind === 'image' && f.valid === false) printBtn.disabled = true;
    actions.append(printBtn);
    if (kind !== 'raw') actions.append(mkBtn('Просмотр', () => previewSd(f.name)));
    if (kind === 'plain' && /\.(md|markdown)$/i.test(f.name)) {
      actions.append(mkBtn('В Markdown', () => mdLoadFromSd(f.name)));
    }
    actions.append(mkBtn('Скачать', () => downloadSd(f.name)));
    actions.append(mkBtn('Удалить', () => deleteSd(f.name), 'danger'));

    item.append(info, actions);
    box.append(item);
  });
}

async function refreshSd() {
  try {
    const d = await api('/api/sd/list');
    sdFiles = d.files || [];
    $('#sdState').textContent = d.mounted ? 'OK' : 'нет карты';
    $('#sdTotal').textContent = d.mounted ? fmtBytes(d.total) : '—';
    $('#sdUsed').textContent = d.mounted ? fmtBytes(d.used) : '—';
    $('#sdFree').textContent = d.mounted ? fmtBytes(d.free) : '—';
    renderSdList();
    refreshMdSdList();
    if (!d.mounted) $('#sdList').textContent = 'Карта не подключена';
  } catch (e) {
    $('#sdState').textContent = 'ошибка';
    $('#sdList').textContent = e.message;
  }
}

async function printSd(name) {
  try {
    await jsonPost('/api/sd/print', { name, feed: 2 });
    toast('Печать с SD: ' + name);
  } catch (e) { toast(e.message, false); }
}

async function previewSd(name) {
  const file = sdFiles.find((x) => x.name === name);
  if (file && (file.kind === 'text' || file.kind === 'plain')) return previewSdText(name);

  try {
    $('#sdTextPreview').hidden = true;
    $('#sdPreview').hidden = false;

    const res = await fetch('/api/sd/file?name=' + encodeURIComponent(name));
    if (!res.ok) throw new Error('HTTP ' + res.status);
    const buf = new Uint8Array(await res.arrayBuffer());
    if (buf.length < 4) throw new Error('короткий файл');

    const dv = new DataView(buf.buffer);
    const w = dv.getUint16(0, true);
    const h = dv.getUint16(2, true);
    if (!w || !h || w > 384 || h > 2000) throw new Error('неверный размер');

    const stride = Math.ceil(w / 8);
    const need = stride * h;
    const data = buf.subarray(4);
    if (data.length < need) throw new Error('короткий файл');

    const c = $('#sdPreview');
    c.width = w; c.height = h;
    const ctx = c.getContext('2d');
    const id = ctx.createImageData(w, h);
    for (let y = 0; y < h; y++) {
      for (let x = 0; x < w; x++) {
        const bit = (data[y * stride + (x >> 3)] >> (7 - (x & 7))) & 1;
        const p = (y * w + x) * 4;
        const v = bit ? 0 : 255;
        id.data[p] = v; id.data[p + 1] = v; id.data[p + 2] = v; id.data[p + 3] = 255;
      }
    }
    ctx.putImageData(id, 0, 0);
    $('#sdPreviewInfo').textContent = name + ' · ' + w + '×' + h + ' · ' + fmtBytes(buf.length);
  } catch (e) {
    toast(e.message, false);
  }
}

async function previewSdText(name) {
  try {
    const res = await fetch('/api/sd/file?name=' + encodeURIComponent(name));
    if (!res.ok) throw new Error('HTTP ' + res.status);
    const text = await res.text();
    const shown = text.slice(0, 8000);
    const pre = $('#sdTextPreview');
    pre.hidden = false;
    pre.textContent = shown;
    $('#sdPreview').hidden = true;
    const bytes = new TextEncoder().encode(text).length;
    $('#sdPreviewInfo').textContent = name + ' · ' + fmtBytes(bytes) +
      (text.length > shown.length ? ' · показаны первые 8000 символов' : '');
  } catch (e) {
    toast(e.message, false);
  }
}

function downloadSd(name) {
  const a = document.createElement('a');
  a.href = '/api/sd/file?name=' + encodeURIComponent(name) + '&download=1';
  a.download = name;
  document.body.appendChild(a);
  a.click();
  a.remove();
}

async function deleteSd(name) {
  if (!confirm('Удалить ' + name + '?')) return;
  try {
    await jsonPost('/api/sd/delete', { name });
    toast('Удалено: ' + name);
    refreshSd();
  } catch (e) { toast(e.message, false); }
}

async function uploadSd() {
  const f = $('#sdUploadFile').files && $('#sdUploadFile').files[0];
  if (!f) return toast('Выберите файл', false);

  let name = $('#sdUploadName').value.trim() || f.name;
  if (!/\.(bin|raw|prn|txt|md|markdown|csv|log)$/i.test(name)) {
    name += /^text\//i.test(f.type || '') ? '.txt' : '.bin';
  }

  if (sdFiles.some((x) => x.name.toLowerCase() === name.toLowerCase())) {
    if (!confirm('Файл ' + name + ' уже есть. Перезаписать?')) return;
  }

  const fd = new FormData();
  fd.append('file', f, name);
  const btn = $('#btnSdUpload');
  btn.disabled = true;
  try {
    const d = await api('/api/sd/upload', { method: 'POST', body: fd });
    toast('Загружено на SD: ' + d.name);
    $('#sdUploadName').value = '';
    refreshSd();
  } catch (e) {
    toast(e.message, false);
  } finally {
    btn.disabled = false;
  }
}

$('#sdUploadFile').addEventListener('change', (e) => {
  const f = e.target.files && e.target.files[0];
  if (f && !$('#sdUploadName').value.trim()) $('#sdUploadName').value = f.name;
});
$('#btnSdRefresh').addEventListener('click', refreshSd);
$('#btnSdUpload').addEventListener('click', uploadSd);
$('#btnSdRemount').addEventListener('click', async () => {
  try {
    const d = await api('/api/sd/remount', { method: 'POST' });
    toast(d.mounted ? 'Карта подключена' : 'Карта не найдена', !!d.mounted);
  } catch (e) {
    toast(e.message, false);
  }
  refreshSd();
});

/* ---------- настройки ---------- */
$('#heat').addEventListener('input', () => { $('#heatVal').textContent = $('#heat').value; });

async function loadConfig() {
  try {
    const d = await api('/api/config');
    const cp = d.codePage === 7 ? 7 : 6;
    $('#heat').value = d.heat;
    $('#heatVal').textContent = d.heat;
    $('#cfgBaud').textContent = d.baud;
    $('#cfgWidth').textContent = d.width;
    $('#cfgCodePage').value = String(cp);
    $('#cfgCpLabel').textContent = cp === 7 ? 'CP866 (7)' : 'CP1251 (6)';
    $('#cfgBaudSel').value = String(d.baud);
    $('#cfgDtr').checked = !!d.dtrEnabled;
  } catch (e) { /* ignore */ }
}

$('#btnSaveCfg').addEventListener('click', async () => {
  try {
    const d = await jsonPost('/api/config', {
      heat: Number($('#heat').value),
      codePage: Number($('#cfgCodePage').value) || 6,
      baud: Number($('#cfgBaudSel').value),
      dtr: $('#cfgDtr').checked
    });
    $('#heat').value = d.heat;
    $('#heatVal').textContent = d.heat;
    $('#cfgBaud').textContent = d.baud;
    $('#cfgCpLabel').textContent = d.codePage === 7 ? 'CP866 (7)' : 'CP1251 (6)';
    toast('Настройки сохранены');
  } catch (e) { toast(e.message, false); }
});

$('#btnCodePageTest').addEventListener('click', async () => {
  try { await api('/api/test/codepage', { method: 'POST' }); toast('Тест кодовых страниц напечатан'); }
  catch (e) { toast(e.message, false); }
});

$('#btnCommandTest').addEventListener('click', async () => {
  try { await api('/api/test/bytes', { method: 'POST' }); toast('Тест команд напечатан'); }
  catch (e) { toast(e.message, false); }
});

$('#btnSelfTest').addEventListener('click', async () => {
  if (!confirm('Принтер напечатает свою страницу самотеста (baudrate, heat, версия). Продолжить?')) return;
  try { await api('/api/test/self', { method: 'POST' }); toast('Самотест запущен'); }
  catch (e) { toast(e.message, false); }
});

$('#btnPrinterStatus').addEventListener('click', async () => {
  try {
    const d = await api('/api/printer/status');
    $('#printerStatus').textContent = JSON.stringify(d, null, 2);
  } catch (e) {
    $('#printerStatus').textContent = 'Ошибка: ' + e.message;
  }
});

$('#btnRestart').addEventListener('click', async () => {
  if (!confirm('Перезагрузить ESP32?')) return;
  try { await api('/api/restart', { method: 'POST' }); } catch (e) { /* ignore */ }
  toast('Перезагрузка...');
});

$('#btnWifiReset').addEventListener('click', async () => {
  if (!confirm('Сбросить настройки Wi-Fi? После перезагрузки нужно будет подключиться к Thermal58-setup.')) return;
  try { await api('/api/wifi/reset', { method: 'POST' }); } catch (e) { /* ignore */ }
  toast('Сброс Wi-Fi, перезагрузка...');
});

/* ---------- запуск ---------- */
$('#baseUrl').textContent = location.origin;
refreshStatus();
setInterval(refreshStatus, 10000);
loadConfig();
refreshSd();
updateCharCount();
</script>
</body>
</html>
)THERMAL";
