#pragma once

#include <kanji_tools/utils/String.h>

namespace kanji_tools {

// UTF-8 conversion functions (between 'char' strings and 'wchar_t' wstrings)
// were originally implemented using 'codecvt', but this was changed to local
// implementations to remove the dependency and allow more flexibility. For
// example, the local implementations use 'U+FFFD' for errors instead of
// throwing a 'range_error'. Also, 'wstring_convert' was deprecated as of C++17
// (see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0618r0.html).

// 'maxSize' can be specified to control the number of characters converted
[[nodiscard]] CodeString fromUtf8(const char*, size_t maxSize = 0);
[[nodiscard]] CodeString fromUtf8(const String&, size_t maxSize = 0);

// functions that return a single UTF-8 'Code' (or 0 if input is empty)
[[nodiscard]] Code getCode(const char*) noexcept;
[[nodiscard]] Code getCode(const String&) noexcept;

[[nodiscard]] String toUtf8(Code);
[[nodiscard]] String toUtf8(int);
[[nodiscard]] String toUtf8(uint32_t x);
[[nodiscard]] String toUtf8(const CodeString&);

// safe conversions of Code to wchar_t
inline constexpr wchar_t toWChar(Code x) noexcept {
  static_assert(sizeof(wchar_t) == sizeof(Code));
  return static_cast<wchar_t>(x);
}

// keep wstring versions of conversion functions for now to work with wregex

[[nodiscard]] std::wstring fromUtf8ToWstring(const char*);
[[nodiscard]] std::wstring fromUtf8ToWstring(const String&);
[[nodiscard]] String toUtf8(const std::wstring&);

// 'MBUtf8Result' is the return value of 'validateMBUtf8' - see comments below
// for more details.
enum class MBUtf8Result {
  Valid,
  NotMultiByte, // returned when sequence is not a multi-byte character
  NotValid      // detailed conversion info will be in 'Utf8Result'
};

// 'Utf8Result' provides more information about errors during conversion and
// is the return value of 'validateUtf8'.
enum class Utf8Result {
  Valid,
  // error cases:
  CharTooLong,      // the first byte starts with more than 4 1's
  ContinuationByte, // the first byte is a continuation byte,
  InvalidCodePoint, // the bytes decode to an invalid Unicode code point
  MissingBytes,     // not enough continuation bytes
  // 'Overlong' is when a character is 'UTF-8' encoded with more bytes than the
  // minimum required, i.e., if a characer can be encoded in two bytes, but
  // instead is encoded using three or four bytes (with extra leading zero bits
  // - see https://en.wikipedia.org/wiki/UTF-8#Overlong_encodings).
  Overlong,
  StringTooLong // more than one UTF-8 character (see examples below)
};

// 'validateMBUtf8' returns 'Valid' if 's' starts with a valid 'Multi-Byte'
// UTF-8 sequence. Examples:
// - validateMBUtf8("") = NotMultiByte
// - validateMBUtf8("a") = NotMultiByte
// - validateMBUtf8("a猫") = NotMultiByte
// - validateMBUtf8("雪") = Valid
// - validateMBUtf8("雪s", true) = NotValid
// - validateMBUtf8("吹雪", true) = NotValid
// Note, the last two examples would be 'Valid' if 'sizeOne' was set to 'false'
// (the default). Use the first two overloads to get more info about errors.
MBUtf8Result validateMBUtf8(
    const char*, Utf8Result&, bool sizeOne = false) noexcept;

MBUtf8Result validateMBUtf8(
    const String&, Utf8Result& e, bool sizeOne = false) noexcept;

template<typename T>
[[nodiscard]] inline auto validateMBUtf8(
    const T& s, bool sizeOne = false) noexcept {
  auto e{Utf8Result::Valid};
  return validateMBUtf8(s, e, sizeOne);
}

// 'validateUtf8' returns 'Valid' if 's' starts a valid UTF-8 sequence
template<typename T>
[[nodiscard]] inline auto validateUtf8(
    const T& s, bool sizeOne = false) noexcept {
  auto e{Utf8Result::Valid};
  validateMBUtf8(s, e, sizeOne);
  return e;
}

[[nodiscard]] bool isValidMBUtf8(const String&, bool sizeOne = false) noexcept;

[[nodiscard]] bool isValidUtf8(const String&, bool sizeOne = false) noexcept;

// bit patterns used for processing UTF-8
enum BitPatterns : unsigned char {
  Bit5 = 0b00'00'10'00,
  Bit4 = 0b00'01'00'00,
  Bit3 = 0b00'10'00'00,
  Bit2 = 0b01'00'00'00,
  Bit1 = 0b10'00'00'00,      // continuation pattern
  TwoBits = 0b11'00'00'00,   // first two bits (starts multi-byte sequence)
  ThreeBits = 0b11'10'00'00, // start of a 3 byte multi-byte sequence
  FourBits = 0b11'11'00'00,  // start of a 4 byte multi-byte sequence
  FiveBits = 0b11'11'10'00   // illegal pattern for first byte (too long)
};

// This program supports Kanji variation selectors and Kana combining marks
// which are both '3 byte' UTF-8 sequences (and multi-byte UTF-8 sequences range
// from 2 to 4 bytes)
inline constexpr size_t VarSelectorSize{3}, MinMBSize{2}, MaxMBSize{4};

inline constexpr auto MaxAscii{U'\x7f'}, MaxUnicode{U'\x10ffff'},
    CombiningVoicedChar{U'\x3099'}, CombiningSemiVoicedChar{U'\x309a'};
inline static const String CombiningVoiced{"\xe3\x82\x99"}, // U+3099
    CombiningSemiVoiced{"\xe3\x82\x9a"};                    // U+309A

} // namespace kanji_tools