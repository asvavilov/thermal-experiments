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

bool printerRichText(const String& markup, uint8_t feed, String& error) {
  error = "";
  if (!printerBeginJob()) {
    error = "printer not ready";
    return false;
  }

  uint8_t codePage = 46;
  String segment;
  segment.reserve(128);
  bool printed = false;
  bool lastWasFeed = false;

  auto flush = [&]() -> bool {
    if (segment.length() == 0) return true;
    const String encoded = printerEncodeText(segment, codePage);
    if (!printerWrite((const uint8_t*)encoded.c_str(), encoded.length())) {
      return false;
    }
    printed = true;
    segment = "";
    return true;
  };

  size_t i = 0;
  while (i < markup.length()) {
    const char c = markup[i];
    if (c == '\r') {
      i++;
      continue;
    }

    if (c != '[') {
      segment += c;
      lastWasFeed = false;
      i++;
      continue;
    }

    // [[ — литерал [
    if (i + 1 < markup.length() && markup[i + 1] == '[') {
      segment += '[';
      i += 2;
      continue;
    }

    const int end = markup.indexOf(']', i + 1);
    if (end < 0) {
      segment += '[';
      i++;
      continue;
    }

    if (!flush()) {
      error = "printer not ready";
      return false;
    }

    String tag = markup.substring(i + 1, end);
    tag.trim();
    String cmd = argAt(tag, 0);
    cmd.toLowerCase();
    const TagResult result = execTag(tag, codePage);
    if (result == TagResult::Error) {
      error = "printer not ready";
      return false;
    }
    if (result == TagResult::Unknown) {
      segment += markup.substring(i, end + 1);
      lastWasFeed = false;
    } else {
      lastWasFeed = (cmd == "feed" || cmd == "feedrows");
    }
    i = (size_t)end + 1;
  }

  if (!flush()) {
    error = "printer not ready";
    return false;
  }

  if (printed && !lastWasFeed && markup.length() > 0 &&
      markup[markup.length() - 1] != '\n') {
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
