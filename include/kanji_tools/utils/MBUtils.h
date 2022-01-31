#ifndef KANJI_TOOLS_UTILS_MBUTILS_H
#define KANJI_TOOLS_UTILS_MBUTILS_H

#include <string>

namespace kanji_tools {

// Bit patterns used for processing UTF-8
enum Values : unsigned char {
  Bit5 = 0b00'00'10'00,
  Bit4 = 0b00'01'00'00,
  Bit3 = 0b00'10'00'00,
  Bit2 = 0b01'00'00'00,
  Bit1 = 0b10'00'00'00,      // continuation pattern
  TwoBits = 0b11'00'00'00,   // mask for first two bits (starts a multi-byte sequence)
  ThreeBits = 0b11'10'00'00, // start of a 3 byte multi-byte sequence
  FourBits = 0b11'11'00'00,  // start of a 4 byte multi-byte sequence
  FiveBits = 0b11'11'10'00   // illegal pattern for first byte (too long)
};

// Helper functions to convert between 'utf8' strings and 'wchar_t' wstrings were originally
// implemented using code like the following, but are now coded here to avoid dependencies:
//
// #include <codecvt> // for codecvt_utf8
// #include <locale>  // for wstring_convert
//
// static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
//
// inline auto fromUtf8(const std::string& s) { return conv.from_bytes(s); }
// inline auto toUtf8(wchar_t c) { return conv.to_bytes(c); }
// inline auto toUtf8(const std::wstring& s) {return conv.to_bytes(s); }

std::wstring fromUtf8(const char*);
inline auto fromUtf8(const std::string& s) { return fromUtf8(s.c_str()); }

std::string toUtf8(wchar_t);
std::string toUtf8(const std::wstring&);

// Helper functions for adding brackets and adding leading zeroes

enum class BracketType { Curly, Round, Square, None };

inline auto addBrackets(const std::string& s, BracketType t) {
  switch (t) {
  case BracketType::Curly: return '{' + s + '}';
  case BracketType::Round: return '(' + s + ')';
  case BracketType::Square: return '[' + s + ']';
  default: break;
  }
  return s;
}

inline auto addLeadingZeroes(const std::string& result, size_t minSize) {
  static const std::string Zero("0");
  if (result.length() < minSize) return std::string(minSize - result.length(), '0') + result;
  if (result.empty()) return Zero;
  return result;
}

// 'toBinary' and 'toHex' are helper functions to print binary or hex versions of 'x' ('x' must be
// integer type). 'minSize' of '0' (the default) causes leading zeroes to be added to make strings
// the same length for a given type, i.e., if 'x' is char then toHex returns a string of length 2
// and toBinary returns a string of length 8. 'minSize' is ignored if it's less than 'result' size.

template<typename T> inline auto toBinary(T x, BracketType brackets, size_t minSize = 0) {
  static_assert(std::is_integral_v<T>);
  std::string result;
  for (; x > 0; x >>= 1) result.insert(result.begin(), '0' + x % 2);
  return addBrackets(addLeadingZeroes(result, minSize ? minSize : sizeof(T) * 8), brackets);
}
template<typename T> inline auto toBinary(T x, int minSize = 0) { return toBinary(x, BracketType::None, minSize); }

enum class HexCase { Upper, Lower };

template<typename T> inline auto toHex(T x, BracketType brackets, HexCase hexCase, size_t minSize = 0) {
  static_assert(std::is_integral_v<T>);
  std::string result;
  for (; x > 0; x >>= 4) {
    const char i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : (hexCase == HexCase::Upper ? 'A' : 'a') + i - 10));
  }
  return addBrackets(addLeadingZeroes(result, minSize ? minSize : sizeof(T) * 2), brackets);
}
template<typename T> inline auto toHex(T x, HexCase hexCase, size_t minSize = 0) {
  return toHex(x, BracketType::None, hexCase, minSize);
}
template<typename T> inline auto toHex(T x, BracketType brackets, size_t minSize = 0) {
  return toHex(x, brackets, HexCase::Lower, minSize);
}
template<typename T> inline auto toHex(T x, size_t minSize = 0) {
  return toHex(x, BracketType::None, HexCase::Lower, minSize);
}

// provide specializations for 'char' that cast to 'unsigned char' (which is probably what is expected)
template<> inline auto toBinary(char x, BracketType brackets, size_t minSize) {
  return toBinary(static_cast<unsigned char>(x), brackets, minSize);
}
template<> inline auto toHex(char x, BracketType brackets, HexCase hexCase, size_t minSize) {
  return toHex(static_cast<unsigned char>(x), brackets, hexCase, minSize);
}

// 'toUnicode' converts a 'wchar_t' into a Unicode code point (so hex with caps and minSize of 4)
inline auto toUnicode(wchar_t s, BracketType b = BracketType::None) { return toHex(s, b, HexCase::Upper, 4); }

// 'toUnicode' converts a UTF-8 string into space-separated Unicode code points. Note: setting
// 'squareBrackets' to true puts brackets around the whole string instead of each entry.
inline auto toUnicode(const std::string& s, BracketType brackets = BracketType::None) {
  std::string result;
  for (const auto i : fromUtf8(s)) {
    if (!result.empty()) result += ' ';
    result += toUnicode(i);
  }
  return addBrackets(result, brackets);
}

// check if a given char or string is not a 'multi-byte char'
constexpr auto isSingleByteChar(char x) noexcept { return x >= 0; }
constexpr auto isSingleByteChar(wchar_t x) noexcept { return x >= 0 && x < 128; }
inline auto isSingleByte(const std::string& s, bool checkLengthOne = true) noexcept {
  return (checkLengthOne ? s.length() == 1 : s.length() >= 1) && isSingleByteChar(s[0]);
}
inline auto isSingleByte(const std::wstring& s, bool checkLengthOne = true) noexcept {
  return (checkLengthOne ? s.length() == 1 : s.length() >= 1) && isSingleByteChar(s[0]);
}
inline auto isAllSingleByte(const std::string& s) noexcept {
  for (const auto i : s)
    if (!isSingleByteChar(i)) return false;
  return true;
}
inline auto isAllSingleByte(const std::wstring& s) noexcept {
  for (const auto i : s)
    if (!isSingleByteChar(i)) return false;
  return true;
}
inline auto isAnySingleByte(const std::string& s) noexcept {
  for (const auto i : s)
    if (isSingleByteChar(i)) return true;
  return false;
}
inline auto isAnySingleByte(const std::wstring& s) noexcept {
  for (const auto i : s)
    if (isSingleByteChar(i)) return true;
  return false;
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_MBUTILS_H
