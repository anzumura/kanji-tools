#include <kanji_tools/utils/MBUtils.h>
#include <kanji_tools/utils/Utils.h>

#include <numeric>
#include <stdexcept>

namespace kanji_tools {

namespace {

template<typename T> void rangeError(const std::string& msg, T x) {
  throw std::out_of_range(msg + ": '" + std::to_string(x) + "' out of range");
}
template<> void rangeError<char32_t>(const std::string& msg, char32_t x) {
  throw std::out_of_range(msg + ": '" + toHex(x, 4) + "' out of range");
}

template<typename T> char toCharUnsigned(T x, const std::string& type) {
  if (x > std::numeric_limits<u_int8_t>::max())
    rangeError("toChar (" + type + ")", x);
  return static_cast<char>(x);
}

} // namespace

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

std::string toUnicode(Code s, BracketType brackets) {
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

char toChar(int x, bool allowNegative) {
  static const std::string Type{"int"};
  if (allowNegative) {
    if (x < std::numeric_limits<char>::min()) rangeError("toChar (int)", x);
  } else if (x < 0)
    rangeError("toChar (positive int)", x);
  return toCharUnsigned(x, Type);
}

char toChar(unsigned char x) { return toCharUnsigned(x, {}); }

char toChar(unsigned int x) {
  static const std::string Type{"unsigned int"};
  return toCharUnsigned(x, Type);
}

char toChar(u_int16_t x) {
  static const std::string Type{"u_int16_t"};
  return toCharUnsigned(x, Type);
}

char toChar(size_t x) {
  static const std::string Type{"size_t"};
  return toCharUnsigned(x, Type);
}

char toChar(char32_t x) {
  static const std::string Type{"char32_t"};
  return toCharUnsigned(x, Type);
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
