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
.sd-list{display:flex;flex-direction:column;gap:8px}
.sd-item{display:flex;flex-wrap:wrap;gap:8px;align-items:center;justify-content:space-between;background:var(--card2);border:1px solid var(--line);border-radius:10px;padding:10px}
.sd-info{flex:1 1 100%;min-width:0}
.sd-name{font-weight:600;word-break:break-word}
.sd-meta{color:var(--mut);font-size:.75rem;margin-top:2px}
.sd-actions{display:flex;gap:6px;flex-wrap:wrap;width:100%}
.sd-actions button{padding:6px 9px;font-size:.8rem}
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
          <select id="codePage"><option value="46">CP1251 (46)</option><option value="17">CP866 (17)</option></select>
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
            <li><code>[reset]</code> — полный сброс задания и возврат к CP1251.</li>
            <li><code>[setCodePage,46|17]</code> — 46 = CP1251, 17 = CP866; другие значения только для латиницы.</li>
            <li><code>[setCharset,0..15]</code> — таблица символов ESC/POS (конвертацию кириллицы не меняет).</li>
            <li><code>[setBarcodeHeight,1..255]</code> — высота последующих штрихкодов.</li>
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
          <p>Штрихкоды, изображения, тестовая страница и проверка бумаги — отдельные вкладки/API, не текстовые теги.</p>
        </div>
      </details>
    </div>

    <div class="card">
      <h3>Шаблоны</h3>
      <div class="row">
        <input id="tplName" placeholder="Название шаблона">
        <button id="tplSave" disabled>Сохранить</button>
      </div>
      <div class="row">
        <select id="tplList" disabled><option value="">— выберите —</option></select>
        <button id="tplLoad" disabled>Загрузить</button>
        <button id="tplDel" disabled>Удалить</button>
        <button id="btnTplRefresh">Обновить</button>
      </div>
      <div class="hint" id="tplHint">Шаблоны хранятся на SD-карте (templates.json) вместе с тегами и доступны всем устройствам.</div>
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
          <option value="EAN13">EAN13</option>
          <option value="EAN8">EAN8</option>
          <option value="CODE39">CODE39</option>
          <option value="ITF">ITF</option>
          <option value="CODABAR">CODABAR</option>
        </select></label>
        <label>Данные<input id="bcData" placeholder="123456789012"></label>
        <label>Высота<input id="bcHeight" type="number" min="1" max="255" value="60"></label>
        <label>Ширина<input id="bcWidth" type="number" min="1" max="6" value="2"></label>
      </div>
      <div class="actions"><button class="primary" id="btnBc">Печать штрихкода</button></div>
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
        <label>Файл (.bin, .raw, .prn)<input id="sdUploadFile" type="file" accept=".bin,.raw,.prn,application/octet-stream"></label>
        <label>Имя на карте<input id="sdUploadName" placeholder="как в файле"></label>
      </div>
      <div class="actions"><button class="primary" id="btnSdUpload">Загрузить</button></div>
      <div class="hint">.bin/.raw — 1-битное изображение с заголовком; .prn — сырой ESC/POS.</div>
    </div>

    <div class="card">
      <h3>Файлы</h3>
      <div id="sdList" class="sd-list">—</div>
    </div>

    <div class="card">
      <h3>Просмотр</h3>
      <div class="preview-wrap"><canvas id="sdPreview" width="384" height="200"></canvas></div>
      <div class="hint" id="sdPreviewInfo">Нажмите «Просмотр» у файла .bin/.raw.</div>
    </div>
  </section>

  <section class="pane" id="pane-settings">
    <div class="card">
      <h2>Принтер</h2>
      <div class="grid">
        <label>Плотность (heat time) <span><input id="heat" type="range" min="1" max="255" value="120"> <span id="heatVal">120</span></span></label>
      </div>
      <div class="actions"><button id="btnHeat">Сохранить плотность</button></div>
      <div class="hint">Baud: <span id="cfgBaud">—</span> · ширина: <span id="cfgWidth">—</span> точек. Скорость задаётся в прошивке и должна совпадать с принтером.</div>
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
POST /api/config                 {"heat":120}
GET  /api/printer/status         опрос принтера (бумага/статус)
POST /api/text                   {"text":"...","size":"M","align":"C"}
POST /api/bitmap                 multipart file: заголовок + 1-битные строки
POST /api/qr                     {"data":"...","module":4}
POST /api/barcode                {"type":"CODE128","data":"..."}
POST /api/feed                   {"lines":3}
POST /api/raw                    multipart file: сырой ESC/POS
POST /api/test                   тестовая страница
GET  /api/print?text=...         печать текста из URL
POST /api/restart                перезагрузка
POST /api/wifi/reset             сброс Wi-Fi и перезагрузка
GET  /api/sd/list                список файлов на SD
GET  /api/sd/file?name=...       скачать/посмотреть файл
POST /api/sd/upload              multipart file: загрузка на SD
POST /api/sd/delete              {"name":"..."}
POST /api/sd/print               {"name":"...","feed":2}
POST /api/sd/remount             переподключить карту
GET  /api/templates              шаблоны с SD
POST /api/templates              {"name":"...","text":"..."}
POST /api/templates/delete       {"name":"..."}</pre>
    </div>
    <div class="card">
      <h3>Примеры</h3>
      <pre class="mono">curl.exe "http://thermal.local/api/print?text=Hello&size=L&align=C"
curl.exe -X POST http://thermal.local/api/text -H "Content-Type: application/json" -d "{\"text\":\"Hello\",\"size\":\"M\"}"
curl.exe -X POST http://thermal.local/api/qr -H "Content-Type: application/json" -d "{\"data\":\"https://example.com\",\"module\":4}"
curl.exe -X POST http://thermal.local/api/bitmap -F "file=@image.bin"
curl.exe -X POST http://thermal.local/api/sd/upload -F "file=@image.raw"
curl.exe -X POST http://thermal.local/api/sd/print -H "Content-Type: application/json" -d "{\"name\":\"image.raw\"}"
curl.exe -X POST http://thermal.local/api/templates -H "Content-Type: application/json" -d "{\"name\":\"Note\",\"text\":\"Hello\"}"</pre>
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
  if (name === 'text') refreshTemplates();
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

/* ---------- шаблоны на SD ---------- */
let templates = {};

function setTemplateUi(ok) {
  ['tplSave', 'tplLoad', 'tplDel'].forEach((id) => {
    const b = document.getElementById(id);
    if (b) b.disabled = !ok;
  });
  const list = $('#tplList');
  if (list) list.disabled = !ok;
  const hint = $('#tplHint');
  if (hint) {
    hint.textContent = ok
      ? 'Шаблоны хранятся на SD-карте (templates.json) вместе с тегами и доступны всем устройствам.'
      : 'SD-карта не подключена — шаблоны недоступны.';
  }
}

function renderTemplateList() {
  const sel = $('#tplList');
  sel.innerHTML = '<option value="">— выберите —</option>';
  Object.keys(templates).sort().forEach((k) => {
    const opt = document.createElement('option');
    opt.value = k; opt.textContent = k;
    sel.appendChild(opt);
  });
}

async function refreshTemplates() {
  try {
    const d = await api('/api/templates');
    templates = d.templates || {};
    setTemplateUi(true);
    renderTemplateList();
  } catch (e) {
    templates = {};
    setTemplateUi(false);
    renderTemplateList();
    const hint = $('#tplHint');
    if (hint) hint.textContent = 'Шаблоны недоступны: ' + e.message;
  }
}

$('#tplSave').addEventListener('click', async () => {
  const name = $('#tplName').value.trim();
  if (!name) return toast('Введите название шаблона', false);
  if (templates[name] !== undefined &&
      !confirm('Шаблон «' + name + '» уже есть. Перезаписать?')) return;
  try {
    await jsonPost('/api/templates', { name, text: $('#text').value });
    toast('Шаблон сохранён на SD');
    await refreshTemplates();
    $('#tplList').value = name;
  } catch (e) { toast(e.message, false); }
});

$('#tplLoad').addEventListener('click', () => {
  const name = $('#tplList').value;
  if (!name) return toast('Выберите шаблон', false);
  if (templates[name] === undefined) return toast('Шаблон не найден', false);
  $('#text').value = templates[name];
  updateCharCount();
  toast('Шаблон загружен');
});

$('#tplDel').addEventListener('click', async () => {
  const name = $('#tplList').value;
  if (!name) return toast('Выберите шаблон', false);
  if (!confirm('Удалить шаблон «' + name + '» с SD?')) return;
  try {
    await jsonPost('/api/templates/delete', { name });
    toast('Шаблон удалён');
    await refreshTemplates();
  } catch (e) { toast(e.message, false); }
});

$('#btnTplRefresh').addEventListener('click', refreshTemplates);

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

function renderPreview() {
  if (!img) return;
  const W = 384;
  const MAXH = 2000;
  let drawW = W;
  let h = Math.round(img.height * W / img.width);
  if (h > MAXH) {
    h = MAXH;
    drawW = Math.max(1, Math.round(img.width * MAXH / img.height));
  }
  h = Math.max(1, h);

  const c = document.createElement('canvas');
  c.width = W; c.height = h;
  const ctx = c.getContext('2d');
  ctx.fillStyle = '#fff';
  ctx.fillRect(0, 0, W, h);
  ctx.drawImage(img, Math.floor((W - drawW) / 2), 0, drawW, h);

  const data = ctx.getImageData(0, 0, W, h);
  const gray = getGray(data, W, h, Number($('#brightness').value), Number($('#contrast').value));
  const method = $('#dither').value;
  let bits = method === 'ordered' ? ditherOrdered(gray, W, h)
           : method === 'threshold' ? ditherThreshold(gray, W, h)
           : ditherFloyd(gray, W, h);
  if ($('#invert').checked) {
    for (let i = 0; i < bits.length; i++) bits[i] ^= 0xFF;
  }
  currentBitmap = { w: W, h, bits };
  drawPreview(bits, W, h);
  const bytes = 4 + bits.length;
  const secs = estimateSeconds(h, bytes);
  $('#imgInfo').textContent = W + '×' + h + ' px · ' + bytes + ' байт · ~' +
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

    const info = document.createElement('div');
    info.className = 'sd-info';
    const name = document.createElement('div');
    name.className = 'sd-name';
    name.textContent = f.name;
    const meta = document.createElement('div');
    meta.className = 'sd-meta';
    const dims = f.raw ? 'raw ESC/POS'
      : f.valid ? (f.width + '×' + f.height) : 'неверный формат';
    meta.textContent = fmtBytes(f.size) + ' · ' + dims;
    info.append(name, meta);

    const actions = document.createElement('div');
    actions.className = 'sd-actions';

    const printBtn = mkBtn('Печать', () => printSd(f.name), 'primary');
    if (f.valid === false) printBtn.disabled = true;
    actions.append(printBtn);
    if (!f.raw) actions.append(mkBtn('Просмотр', () => previewSd(f.name)));
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
  try {
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
  if (!/\.(bin|raw|prn)$/i.test(name)) name += '.bin';

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
    $('#heat').value = d.heat;
    $('#heatVal').textContent = d.heat;
    $('#cfgBaud').textContent = d.baud;
    $('#cfgWidth').textContent = d.width;
  } catch (e) { /* ignore */ }
}

$('#btnHeat').addEventListener('click', async () => {
  try {
    const d = await jsonPost('/api/config', { heat: Number($('#heat').value) });
    $('#heat').value = d.heat;
    $('#heatVal').textContent = d.heat;
    toast('Плотность сохранена');
  } catch (e) { toast(e.message, false); }
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
refreshTemplates();
loadConfig();
refreshSd();
updateCharCount();
</script>
</body>
</html>
)THERMAL";
