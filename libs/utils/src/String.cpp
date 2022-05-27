#include <kanji_tools/utils/Exception.h>
#include <kanji_tools/utils/Utf8.h>

#include <numeric>

namespace kanji_tools {

namespace {

template<typename T> void rangeError(const String& msg, T x) {
  throw RangeError(msg + ": '" + std::to_string(x) + "' out of range");
}
template<> void rangeError<Code>(const String& msg, Code x) {
  throw RangeError(msg + ": '" + toHex(x, 4) + "' out of range");
}

template<typename T, typename U> auto cast(U u) { return static_cast<T>(u); }

template<typename T> char toCharUnsigned(T x, const String& type) {
  if (x > std::numeric_limits<uint8_t>::max())
    rangeError("toChar (" + type + ")", x);
  return cast<char>(x);
}

} // namespace

String addBrackets(const String& s, BracketType t) {
  switch (t) {
  case BracketType::Curly: return '{' + s + '}';
  case BracketType::Round: return '(' + s + ')';
  case BracketType::Square: return '[' + s + ']';
  case BracketType::None: break;
  }
  return s;
}

String addLeadingZeroes(const String& s, size_t minSize) {
  static const String Zero{"0"};
  if (s.size() < minSize) return String(minSize - s.size(), '0') + s;
  if (s.empty()) return Zero;
  return s;
}

CodeString addLeadingZeroes(const CodeString& s, size_t minSize) {
  static const CodeString Zero{U"0"};
  if (s.size() < minSize) return CodeString(minSize - s.size(), U'0') + s;
  if (s.empty()) return Zero;
  return s;
}

String toUnicode(Code s, BracketType brackets) {
  return toHex(s, brackets, HexCase::Upper, 4);
}

String toUnicode(const String& s, BracketType brackets) {
  String result;
  for (const auto i : fromUtf8(s)) {
    if (!result.empty()) result += ' ';
    result += toUnicode(i);
  }
  return addBrackets(result, brackets);
}

String toUnicode(const CodeString& s, BracketType brackets) {
  String result;
  for (const auto i : s) {
    if (!result.empty()) result += ' ';
    result += toUnicode(i);
  }
  return addBrackets(result, brackets);
}

// conversion functions

char toChar(int x, bool allowNegative) {
  static const String Type{"int"};
  if (allowNegative) {
    if (x < std::numeric_limits<char>::min()) rangeError("toChar (int)", x);
  } else if (x < 0)
    rangeError("toChar (positive int)", x);
  return toCharUnsigned(x, Type);
}

char toChar(unsigned int x) {
  static const String Type{"unsigned int"};
  return toCharUnsigned(x, Type);
}

char toChar(uint16_t x) {
  static const String Type{"uint16_t"};
  return toCharUnsigned(x, Type);
}

char toChar(size_t x) {
  static const String Type{"size_t"};
  return toCharUnsigned(x, Type);
}

char toChar(Code x) {
  static const String Type{"Code"};
  return toCharUnsigned(x, Type);
}

char toChar(uint8_t x) { return toCharUnsigned(x, {}); }

uint8_t toUChar(char x) { return cast<uint8_t>(x); }

// 'is' functions for testing single bytes

bool isSingleByte(const String& s, bool sizeOne) noexcept {
  return (sizeOne ? s.size() == 1 : !s.empty()) && isSingleByteChar(s[0]);
}

bool isSingleByte(const CodeString& s, bool sizeOne) noexcept {
  return (sizeOne ? s.size() == 1 : !s.empty()) && isSingleByteChar(s[0]);
}

bool isAllSingleByte(const String& s) noexcept {
  return std::all_of(
      s.begin(), s.end(), [](auto i) { return isSingleByteChar(i); });
}

bool isAllSingleByte(const CodeString& s) noexcept {
  return std::all_of(
      s.begin(), s.end(), [](auto i) { return isSingleByteChar(i); });
}

bool isAnySingleByte(const String& s) noexcept {
  return std::any_of(
      s.begin(), s.end(), [](auto i) { return isSingleByteChar(i); });
}

bool isAnySingleByte(const CodeString& s) noexcept {
  return std::any_of(
      s.begin(), s.end(), [](auto i) { return isSingleByteChar(i); });
}

} // namespace kanji_tools
