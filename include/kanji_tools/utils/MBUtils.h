#ifndef KANJI_TOOLS_UTILS_MBUTILS_H
#define KANJI_TOOLS_UTILS_MBUTILS_H

#include <string>

namespace kanji_tools {

// bit patterns used for processing UTF-8
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

// 'Min' and 'Max' Values for determining invalid Unicde code points when doing UTF-8 conversion.
// Here's a quote from https://en.wikipedia.org/wiki/UTF-8#Invalid_sequences_and_error_handling:
//   Since RFC 3629 (November 2003), the high and low surrogate halves used by UTF-16 (U+D800
//   through U+DFFF) and code points not encodable by UTF-16 (those after U+10FFFF) are not legal
//   Unicode values, and their UTF-8 encoding must be treated as an invalid byte sequence.
constexpr char32_t MinSurrogate = 0xd800, MaxSurrogate = 0xdfff, MaxUnicode = 0x10ffff, ErrorReplacement = 0xfffd;

// UTF-8 sequence for U+FFFD (�) - used by the local 'toUtf8' functions for invalid code points
constexpr auto ReplacementCharacter = "\xEF\xBF\xBD";

// 'MBUtf8Result' is used for the return value of 'validateMBUtf8' - see comments below for more details.
enum class MBUtf8Result {
  Valid,
  ContinuationByte,   // returned when the first byte is a continuation byte, i.e., starts with '10'
  InvalidCodePoint,   // returned when the bytes decode to an invalid Unicode code point (see MBUtils.h)
  MBCharTooLong,      // returned when the first byte starts with more than 4 1's (so too long for UTF-8)
  MBCharMissingBytes, // returned when there are not enough continuation bytes
  NotMBUtf8,          // returned when sequence is not a multi-byte character, i.e, it's regular ASCII
  // 'Overlong' is when a character is 'UTF-8' encoded with more bytes than the minimum required, i.e.,
  // if a characer can be encoded in two bytes, but instead is encoded using three or four bytes (with
  // extra leading zero bits - see https://en.wikipedia.org/wiki/UTF-8#Overlong_encodings).
  Overlong,
  StringTooLong,
};

// 'validateMBUtf8' returns 'Valid' if string contains one proper multi-byte sequence, i.e., a single
// well-formed 'multi-byte symbol'. Examples:
// - validateMBUtf8("") = NotMBUtf8
// - validateMBUtf8("a") = NotMBUtf8
// - validateMBUtf8("a猫") = NotMBUtf8
// - validateMBUtf8("雪") = Valid
// - validateMBUtf8("雪s") = StringTooLong
// - validateMBUtf8("吹雪") = StringTooLong
// Note, the last two cases can be considered 'valid' if checkLengthOne is set to false
[[nodiscard]] MBUtf8Result validateMBUtf8(const char* s, bool checkLengthOne = true) noexcept;
[[nodiscard]] inline auto validateMBUtf8(const std::string& s, bool checkLengthOne = true) noexcept {
  return validateMBUtf8(s.c_str(), checkLengthOne);
}

[[nodiscard]] inline auto isValidMBUtf8(const std::string& s, bool checkLengthOne = true) noexcept {
  return validateMBUtf8(s, checkLengthOne) == MBUtf8Result::Valid;
}

// UTF-8 conversion functions (between 'char' strings and 'char32_t' wstrings) were originally
// implemented using 'codecvt', but this was changed to local implementations to remove the
// dependency and allow more flexibility. For example, the local implementaions use 'U+FFFD'
// for errors instead of throwing a 'range_error'. Also, 'wstring_convert' was depecated as of
// C++17 (see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0618r0.html).

// uncomment the following line to revert to use 'codecvt' (may remove this soon):
//#define USE_CODECVT_FOR_UTF_8

[[nodiscard]] std::u32string fromUtf8(const char*);
[[nodiscard]] inline auto fromUtf8(const std::string& s) { return fromUtf8(s.c_str()); }

[[nodiscard]] std::string toUtf8(char32_t);
[[nodiscard]] std::string toUtf8(const std::u32string&);

// keep wstring versions of conversion functions for now to work with wregex

[[nodiscard]] std::wstring fromUtf8ToWstring(const char*);
[[nodiscard]] inline auto fromUtf8ToWstring(const std::string& s) { return fromUtf8ToWstring(s.c_str()); }
[[nodiscard]] std::string toUtf8(const std::wstring&);

// helper functions for adding brackets and adding leading zeroes

enum class BracketType { Curly, Round, Square, None };

[[nodiscard]] inline auto addBrackets(const std::string& s, BracketType t) {
  switch (t) {
  case BracketType::Curly: return '{' + s + '}';
  case BracketType::Round: return '(' + s + ')';
  case BracketType::Square: return '[' + s + ']';
  default: break;
  }
  return s;
}

[[nodiscard]] inline auto addLeadingZeroes(const std::string& result, size_t minSize) {
  static const std::string Zero("0");
  if (result.length() < minSize) return std::string(minSize - result.length(), '0') + result;
  if (result.empty()) return Zero;
  return result;
}

// 'toBinary' and 'toHex' are helper functions to print binary or hex versions of 'x' ('x' must be
// integer type). 'minSize' of '0' (the default) causes leading zeroes to be added to make strings
// the same length for a given type, i.e., if 'x' is char then toHex returns a string of length 2
// and toBinary returns a string of length 8. 'minSize' is ignored if it's less than 'result' size.

template<typename T> [[nodiscard]] inline auto toBinary(T x, BracketType brackets, size_t minSize = 0) {
  static_assert(std::is_integral_v<T>);
  std::string result;
  for (; x > 0; x >>= 1) result.insert(result.begin(), '0' + x % 2);
  return addBrackets(addLeadingZeroes(result, minSize ? minSize : sizeof(T) * 8), brackets);
}
template<typename T> [[nodiscard]] inline auto toBinary(T x, int minSize = 0) {
  return toBinary(x, BracketType::None, minSize);
}

enum class HexCase { Upper, Lower };

template<typename T> [[nodiscard]] inline auto toHex(T x, BracketType brackets, HexCase hexCase, size_t minSize = 0) {
  static_assert(std::is_integral_v<T>);
  std::string result;
  for (; x > 0; x >>= 4) {
    const char i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : (hexCase == HexCase::Upper ? 'A' : 'a') + i - 10));
  }
  return addBrackets(addLeadingZeroes(result, minSize ? minSize : sizeof(T) * 2), brackets);
}
template<typename T> [[nodiscard]] inline auto toHex(T x, HexCase hexCase, size_t minSize = 0) {
  return toHex(x, BracketType::None, hexCase, minSize);
}
template<typename T> [[nodiscard]] inline auto toHex(T x, BracketType brackets, size_t minSize = 0) {
  return toHex(x, brackets, HexCase::Lower, minSize);
}
template<typename T> [[nodiscard]] inline auto toHex(T x, size_t minSize = 0) {
  return toHex(x, BracketType::None, HexCase::Lower, minSize);
}

// provide specializations for 'char' that cast to 'unsigned char' (which is probably what is expected)
template<> [[nodiscard]] inline auto toBinary(char x, BracketType brackets, size_t minSize) {
  return toBinary(static_cast<unsigned char>(x), brackets, minSize);
}
template<> [[nodiscard]] inline auto toHex(char x, BracketType brackets, HexCase hexCase, size_t minSize) {
  return toHex(static_cast<unsigned char>(x), brackets, hexCase, minSize);
}

// 'toUnicode' converts a 'char32_t' into a Unicode code point (so hex with caps and minSize of 4)
[[nodiscard]] inline auto toUnicode(char32_t s, BracketType b = BracketType::None) {
  return toHex(s, b, HexCase::Upper, 4);
}

// 'toUnicode' converts a UTF-8 string into space-separated Unicode code points. Note: setting
// 'squareBrackets' to true puts brackets around the whole string instead of each entry.
[[nodiscard]] inline auto toUnicode(const std::string& s, BracketType brackets = BracketType::None) {
  std::string result;
  for (const auto i : fromUtf8(s)) {
    if (!result.empty()) result += ' ';
    result += toUnicode(i);
  }
  return addBrackets(result, brackets);
}

// check if a given char or string is not a 'multi-byte char'
[[nodiscard]] constexpr auto isSingleByteChar(char x) noexcept { return x >= 0; }
[[nodiscard]] constexpr auto isSingleByteChar(char32_t x) noexcept { return x >= 0 && x < 128; }
[[nodiscard]] inline auto isSingleByte(const std::string& s, bool checkLengthOne = true) noexcept {
  return (checkLengthOne ? s.length() == 1 : s.length() >= 1) && isSingleByteChar(s[0]);
}
[[nodiscard]] inline auto isSingleByte(const std::u32string& s, bool checkLengthOne = true) noexcept {
  return (checkLengthOne ? s.length() == 1 : s.length() >= 1) && isSingleByteChar(s[0]);
}
[[nodiscard]] inline auto isAllSingleByte(const std::string& s) noexcept {
  for (const auto i : s)
    if (!isSingleByteChar(i)) return false;
  return true;
}
[[nodiscard]] inline auto isAllSingleByte(const std::u32string& s) noexcept {
  for (const auto i : s)
    if (!isSingleByteChar(i)) return false;
  return true;
}
[[nodiscard]] inline auto isAnySingleByte(const std::string& s) noexcept {
  for (const auto i : s)
    if (isSingleByteChar(i)) return true;
  return false;
}
[[nodiscard]] inline auto isAnySingleByte(const std::u32string& s) noexcept {
  for (const auto i : s)
    if (isSingleByteChar(i)) return true;
  return false;
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_MBUTILS_H
