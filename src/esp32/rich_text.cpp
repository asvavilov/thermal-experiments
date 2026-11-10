#include "rich_text.h"

#include "config.h"
#include "printer.h"

namespace {

enum class TagResult { Unknown, Ok, Error };

String argAt(const String& tag, int index) {
  int start = 0;
  int part = 0;
  while (start <= (int)tag.length()) {
    const int comma = tag.indexOf(',', start);
    String value = (comma < 0) ? tag.substring(start) : tag.substring(start, comma);
    if (part == index) {
      value.trim();
      return value;
    }
    part++;
    if (comma < 0) break;
    start = comma + 1;
  }
  return String();
}

int argInt(const String& tag, int index, int fallback) {
  const String value = argAt(tag, index);
  if (value.length() == 0) return fallback;
  return value.toInt();
}

TagResult execTag(const String& tag, uint8_t& codePage) {
  const String command = argAt(tag, 0);
  String cmd = command;
  cmd.toLowerCase();

  if (cmd == "setsize") {
    const String arg = argAt(tag, 1);
    const char size = arg.length() ? (char)toupper(arg[0]) : 'S';
    printerSetSize(size);
    return TagResult::Ok;
  }
  if (cmd == "justify") {
    const String arg = argAt(tag, 1);
    printerSetJustify(arg.length() ? (char)toupper(arg[0]) : 'L');
    return TagResult::Ok;
  }
  if (cmd == "boldon") {
    printerSetBold(true);
    return TagResult::Ok;
  }
  if (cmd == "boldoff") {
    printerSetBold(false);
    return TagResult::Ok;
  }
  if (cmd == "underlineon") {
    int weight = argInt(tag, 1, 1);
    if (weight < 1) weight = 1;
    if (weight > 2) weight = 2;
    printerSetUnderline((uint8_t)weight);
    return TagResult::Ok;
  }
  if (cmd == "underlineoff") {
    printerSetUnderline(0);
    return TagResult::Ok;
  }
  if (cmd == "inverseon") {
    printerSetInverse(true);
    return TagResult::Ok;
  }
  if (cmd == "inverseoff") {
    printerSetInverse(false);
    return TagResult::Ok;
  }
  if (cmd == "doubleheighton") {
    printerSetDoubleHeight(true);
    return TagResult::Ok;
  }
  if (cmd == "doubleheightoff") {
    printerSetDoubleHeight(false);
    return TagResult::Ok;
  }
  if (cmd == "doublewidthon") {
    printerSetDoubleWidth(true);
    return TagResult::Ok;
  }
  if (cmd == "doublewidthoff") {
    printerSetDoubleWidth(false);
    return TagResult::Ok;
  }
  if (cmd == "strikeon") {
    printerSetStrike(true);
    return TagResult::Ok;
  }
  if (cmd == "strikeoff") {
    printerSetStrike(false);
    return TagResult::Ok;
  }
  if (cmd == "upsidedownon") {
    printerSetUpsideDown(true);
    return TagResult::Ok;
  }
  if (cmd == "upsidedownoff") {
    printerSetUpsideDown(false);
    return TagResult::Ok;
  }
  if (cmd == "setfont") {
    const String arg = argAt(tag, 1);
    printerSetFont(arg.length() ? (char)toupper(arg[0]) : 'A');
    return TagResult::Ok;
  }
  if (cmd == "setlineheight") {
    printerSetLineHeight((uint8_t)constrain(argInt(tag, 1, 30), 24, 255));
    return TagResult::Ok;
  }
  if (cmd == "setcharspacing") {
    printerSetCharSpacing((uint8_t)constrain(argInt(tag, 1, 0), 0, 255));
    return TagResult::Ok;
  }
  if (cmd == "tab") {
    printerTab();
    return TagResult::Ok;
  }
  if (cmd == "feed") {
    const int lines = constrain(argInt(tag, 1, 1), 0, 255);
    if (lines > 0) printerFeed((uint8_t)lines);
    return TagResult::Ok;
  }
  if (cmd == "feedrows") {
    printerFeedRows((uint8_t)constrain(argInt(tag, 1, 1), 0, 255));
    return TagResult::Ok;
  }
  if (cmd == "setcodepage") {
    const int page = constrain(argInt(tag, 1, 46), 0, 47);
    printerSetCodePage((uint8_t)page);
    codePage = (uint8_t)page;
    return TagResult::Ok;
  }
  if (cmd == "setcharset") {
    printerSetCharset((uint8_t)constrain(argInt(tag, 1, 0), 0, 15));
    return TagResult::Ok;
  }
  if (cmd == "setbarcodeheight") {
    printerSetBarcodeHeight((uint8_t)constrain(argInt(tag, 1, 50), 1, 255));
    return TagResult::Ok;
  }
  if (cmd == "setprintdensity") {
    printerSetPrintDensity(
        (uint8_t)constrain(argInt(tag, 1, 10), 0, 31),
        (uint8_t)constrain(argInt(tag, 2, 2), 0, 7));
    return TagResult::Ok;
  }
  if (cmd == "setheatconfig") {
    printerSetHeatConfig(
        (uint8_t)constrain(argInt(tag, 1, 11), 0, 255),
        (uint8_t)constrain(argInt(tag, 2, 120), 0, 255),
        (uint8_t)constrain(argInt(tag, 3, 40), 0, 255));
    return TagResult::Ok;
  }
  if (cmd == "sleepafter") {
    const long seconds = constrain(argInt(tag, 1, 1), 0, 65535);
    printerSleepAfter((uint16_t)seconds);
    return TagResult::Ok;
  }
  if (cmd == "sleep") {
    printerSleepAfter(1);
    return TagResult::Ok;
  }
  if (cmd == "wake") {
    printerWake();
    return TagResult::Ok;
  }
  if (cmd == "online") {
    printerOnline(true);
    return TagResult::Ok;
  }
  if (cmd == "offline") {
    printerOnline(false);
    return TagResult::Ok;
  }
  if (cmd == "setdefault") {
    printerSetDefault();
    return TagResult::Ok;
  }
  if (cmd == "reset") {
    if (!printerBeginJob()) return TagResult::Error;
    codePage = 46;
    return TagResult::Ok;
  }

  return TagResult::Unknown;
}

}  // namespace

bool RichTextPrinter::begin(String& error) {
  error = "";
  pending_ = "";
  segment_ = "";
  codePage_ = 46;
  printed_ = false;
  lastWasFeed_ = false;
  lastChar_ = 0;
  pending_.reserve(MAX_RICH_TAG_LEN + 8);
  segment_.reserve(512);

  if (!printerBeginJob()) {
    error = "printer not ready";
    return false;
  }
  return true;
}

bool RichTextPrinter::flushSegment(bool force, String& error) {
  if (segment_.length() == 0) return true;

  size_t cut = segment_.length();
  if (!force && cut > 0) {
    // Не разрезаем многобайтовый UTF-8 символ между частями.
    size_t lead = cut;
    while (lead > 0 && ((uint8_t)segment_[lead - 1] & 0xC0) == 0x80) lead--;
    if (lead > 0) {
      const uint8_t b = (uint8_t)segment_[lead - 1];
      size_t need = 1;
      if ((b & 0xE0) == 0xC0) need = 2;
      else if ((b & 0xF0) == 0xE0) need = 3;
      else if ((b & 0xF8) == 0xF0) need = 4;
      if (cut - (lead - 1) < need) cut = lead - 1;
    }
  }
  if (cut == 0) return true;

  const String part = segment_.substring(0, cut);
  const String encoded = printerEncodeText(part, codePage_);
  if (!printerWrite((const uint8_t*)encoded.c_str(), encoded.length())) {
    error = "printer not ready";
    return false;
  }
  printed_ = true;

  if (cut == segment_.length()) {
    segment_ = "";
  } else {
    segment_.remove(0, cut);
  }
  return true;
}

bool RichTextPrinter::process(String& error) {
  size_t i = 0;
  while (i < pending_.length()) {
    if (segment_.length() >= 512 && !flushSegment(false, error)) return false;

    const char c = pending_[i];
    if (c == '\r') {
      i++;
      continue;
    }

    if (c != '[') {
      segment_ += c;
      lastChar_ = c;
      lastWasFeed_ = false;
      i++;
      continue;
    }

    // [[ — литерал [
    if (i + 1 >= pending_.length()) break;
    if (pending_[i + 1] == '[') {
      segment_ += '[';
      lastChar_ = '[';
      lastWasFeed_ = false;
      i += 2;
      continue;
    }

    const int end = pending_.indexOf(']', (unsigned int)(i + 1));
    if (end < 0) {
      // Слишком длинная незакрытая скобка — считаем её обычным текстом.
      if (pending_.length() - i > MAX_RICH_TAG_LEN) {
        segment_ += '[';
        lastChar_ = '[';
        lastWasFeed_ = false;
        i++;
        continue;
      }
      break;
    }

    if ((size_t)(end - i - 1) > MAX_RICH_TAG_LEN) {
      segment_ += '[';
      lastChar_ = '[';
      lastWasFeed_ = false;
      i++;
      continue;
    }

    if (!flushSegment(true, error)) return false;

    String tag = pending_.substring(i + 1, end);
    tag.trim();
    String cmd = argAt(tag, 0);
    cmd.toLowerCase();
    const TagResult result = execTag(tag, codePage_);
    if (result == TagResult::Error) {
      error = "printer not ready";
      return false;
    }
    if (result == TagResult::Unknown) {
      segment_ += pending_.substring(i, end + 1);
      lastChar_ = ']';
      lastWasFeed_ = false;
    } else {
      lastWasFeed_ = (cmd == "feed" || cmd == "feedrows");
    }
    i = (size_t)end + 1;
  }

  if (i > 0) pending_.remove(0, (unsigned int)i);
  return true;
}

bool RichTextPrinter::write(const char* data, size_t len, String& error) {
  if (!data || len == 0) return true;
  pending_.reserve(pending_.length() + len + 1);
  if (!pending_.concat(data, (unsigned int)len)) {
    error = "out of memory";
    return false;
  }
  return process(error);
}

bool RichTextPrinter::end(uint8_t feed, String& error) {
  // Незакрытый тег в конце печатаем как обычный текст.
  for (size_t i = 0; i < pending_.length(); i++) {
    const char c = pending_[i];
    if (c == '\r') continue;
    segment_ += c;
    lastChar_ = c;
    lastWasFeed_ = false;
  }
  pending_ = "";

  if (!flushSegment(true, error)) return false;

  if (printed_ && !lastWasFeed_ && lastChar_ != '\n') {
    const uint8_t nl = '\n';
    if (!printerWrite(&nl, 1)) {
      error = "printer not ready";
      return false;
    }
  }

  if (feed > 0 && !printerFeed(feed)) {
    error = "printer not ready";
    return false;
  }
  return true;
}

bool printerRichText(const String& markup, uint8_t feed, String& error) {
  RichTextPrinter printer;
  if (!printer.begin(error)) return false;
  if (!printer.write(markup.c_str(), markup.length(), error)) return false;
  return printer.end(feed, error);
}
