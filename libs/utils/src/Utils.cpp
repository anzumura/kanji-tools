#include <kanji_tools/utils/MBUtils.h>
#include <kanji_tools/utils/Utils.h>

namespace kanji_tools {

std::string addBrackets(const std::string& s, BracketType t) {
  switch (t) {
  case BracketType::Curly: return '{' + s + '}';
  case BracketType::Round: return '(' + s + ')';
  case BracketType::Square: return '[' + s + ']';
  case BracketType::None: break;
  }
  return s;
}

std::string addLeadingZeroes(const std::string& s, size_t minSize) {
  static const std::string Zero{"0"};
  if (s.size() < minSize) return std::string(minSize - s.size(), '0') + s;
  if (s.empty()) return Zero;
  return s;
}

std::u32string addLeadingZeroes(const std::u32string& s, size_t minSize) {
  static const std::u32string Zero{U"0"};
  if (s.size() < minSize) return std::u32string(minSize - s.size(), U'0') + s;
  if (s.empty()) return Zero;
  return s;
}

std::string toUnicode(char32_t s, BracketType brackets) {
  return toHex(s, brackets, HexCase::Upper, 4);
}

std::string toUnicode(const std::string& s, BracketType brackets) {
  std::string result;
  for (const auto i : fromUtf8(s)) {
    if (!result.empty()) result += ' ';
    result += toUnicode(i);
  }
  return addBrackets(result, brackets);
}

std::string toUnicode(const std::u32string& s, BracketType brackets) {
  std::string result;
  for (const auto i : s) {
    if (!result.empty()) result += ' ';
    result += toUnicode(i);
  }
  return addBrackets(result, brackets);
}

bool isSingleByte(const std::string& s, bool sizeOne) noexcept {
  return (sizeOne ? s.size() == 1 : s.size() >= 1) && isSingleByteChar(s[0]);
}

bool isSingleByte(const std::u32string& s, bool sizeOne) noexcept {
  return (sizeOne ? s.size() == 1 : s.size() >= 1) && isSingleByteChar(s[0]);
}

bool isAllSingleByte(const std::string& s) noexcept {
  for (const auto i : s)
    if (!isSingleByteChar(i)) return false;
  return true;
}

bool isAllSingleByte(const std::u32string& s) noexcept {
  for (const auto i : s)
    if (!isSingleByteChar(i)) return false;
  return true;
}

bool isAnySingleByte(const std::string& s) noexcept {
  for (const auto i : s)
    if (isSingleByteChar(i)) return true;
  return false;
}

bool isAnySingleByte(const std::u32string& s) noexcept {
  for (const auto i : s)
    if (isSingleByteChar(i)) return true;
  return false;
}

} // namespace kanji_tools
