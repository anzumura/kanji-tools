#include <kanji_tools/kana/DisplaySize.h>

namespace kanji_tools {

size_t displaySize(const CodeString& s) {
  size_t result{};
  for (const auto i : s)
    if (i && !isNonSpacing(i)) result += inRange(i, WideBlocks) ? 2 : 1;
  return result;
}

size_t displaySize(const char* s) { return displaySize(fromUtf8(s)); }

size_t displaySize(const String& s) { return displaySize(s.c_str()); }

int wideSetw(const String& s, size_t setwLen) {
  // cast to int since std::setw takes an int
  return static_cast<int>(setwLen + s.size() - displaySize(s));
}

} // namespace kanji_tools
