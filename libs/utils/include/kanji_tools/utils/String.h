#pragma once

#include <string>

namespace kanji_tools { //! \utils_group{String}

//!@{
//! String and StringView type aliases may be changed to **std::u8** versions
//! later once they get wider support (streams, regex, etc.)
using String = std::string;
using StringView = std::string_view;
//!@}

//!@{
//! type alias for Unicode code points (use **char32_t** instead of **wchar_t**
//! since it's always 32 bits instead of platform dependent)
using Code = char32_t;
using CodeString = std::u32string;
//!@}

// helper functions for adding brackets and adding leading zeroes

enum class BracketType { Curly, Round, Square, None };

[[nodiscard]] String addBrackets(const String&, BracketType);

[[nodiscard]] String addLeadingZeroes(const String&, size_t minSize);

[[nodiscard]] CodeString addLeadingZeroes(const CodeString&, size_t minSize);

//! convert a #Code into a Unicode code point (caps hex with minSize of 4)
[[nodiscard]] String toUnicode(Code, BracketType = BracketType::None);

//!@{
//! convert a UTF-8 string into space-separated Unicode code points, brackets
//! are put around the whole string (not each Unicode value).
[[nodiscard]] String toUnicode(const String&, BracketType = BracketType::None);
[[nodiscard]] String toUnicode(
    const CodeString&, BracketType = BracketType::None);
//!@}

//! safely converts `x` to a char
//! \param allowNegative if true (the default) then `x` can't be less than -128
//!                      otherwise `x` must be positive.
//! \param x the value to convert (must be < 256 regardless of `allowNegative`)
//! \return the char value
//! \throw RangeError if `x` is out of range for char type
[[nodiscard]] char toChar(int x, bool allowNegative = true);

//!@{
//! overloads or toChar(int,bool) for common `unsigned` types
[[nodiscard]] char toChar(uint16_t);
[[nodiscard]] char toChar(unsigned int);
[[nodiscard]] char toChar(size_t);
[[nodiscard]] char toChar(Code);
//!@}

// conversion functions that don't throw since size is the same
[[nodiscard]] char toChar(unsigned char);
[[nodiscard]] unsigned char toUChar(char);

// 'toBinary' and 'toHex' are helper functions to print binary or hex versions
// of 'x' ('x' must be integral). 'minSize' 0 (the default) causes leading
// zeroes to be added to make strings the same size for a given type, i.e., if
// 'x' is char then toHex returns a string of size 2 and toBinary returns a
// string of size 8. 'minSize' is ignored if it's less than 'result' size.

constexpr auto BinaryDigits{2}, DecimalDigits{10}, HexDigits{16}, Bits{8},
    SevenBitMax{128}, UnicodeStringMinSize{4}, UnicodeStringMaxSize{5};

template<typename T>
[[nodiscard]] inline auto toBinary(
    T x, BracketType brackets, size_t minSize = 0) {
  static_assert(std::is_unsigned_v<T>);
  String result;
  for (; x > 0; x >>= 1U)
    result.insert(result.begin(), '0' + toChar(x % BinaryDigits));
  return addBrackets(
      addLeadingZeroes(result, minSize ? minSize : sizeof(T) * Bits), brackets);
}

template<typename T>
[[nodiscard]] inline auto toBinary(T x, size_t minSize = 0) {
  return toBinary(x, BracketType::None, minSize);
}

enum class HexCase { Upper, Lower };

template<typename T>
[[nodiscard]] inline auto toHex(
    T x, BracketType brackets, HexCase hexCase, size_t minSize = 0) {
  static_assert(std::is_unsigned_v<T>);
  String result;
  for (; x > 0; x >>= 4U) {
    const char i = x % HexDigits;
    result.insert(result.begin(),
        (i < DecimalDigits
                ? '0' + i
                : (hexCase == HexCase::Upper ? 'A' : 'a') + i - DecimalDigits));
  }
  return addBrackets(
      addLeadingZeroes(result, minSize ? minSize : sizeof(T) * 2), brackets);
}

template<typename T>
[[nodiscard]] inline auto toHex(T x, HexCase hexCase, size_t minSize = 0) {
  return toHex(x, BracketType::None, hexCase, minSize);
}

template<typename T>
[[nodiscard]] inline auto toHex(T x, BracketType brackets, size_t minSize = 0) {
  return toHex(x, brackets, HexCase::Lower, minSize);
}

template<typename T> [[nodiscard]] inline auto toHex(T x, size_t minSize = 0) {
  return toHex(x, BracketType::None, HexCase::Lower, minSize);
}

// provide specializations for 'char' that cast to 'unsigned char' (which is
// probably what is expected)

template<>
[[nodiscard]] inline auto toBinary(
    char x, BracketType brackets, size_t minSize) {
  return toBinary(toUChar(x), brackets, minSize);
}

template<>
[[nodiscard]] inline auto toHex(
    char x, BracketType brackets, HexCase hexCase, size_t minSize) {
  return toHex(toUChar(x), brackets, hexCase, minSize);
}

// 'isSingle..' functions return false if a given char or string is 'multi-byte'

[[nodiscard]] constexpr auto isSingleByteChar(char x) noexcept {
  return x >= 0;
}
[[nodiscard]] constexpr auto isSingleByteChar(Code x) noexcept {
  return x < SevenBitMax;
}

[[nodiscard]] bool isSingleByte(const String&, bool sizeOne = true) noexcept;
[[nodiscard]] bool isSingleByte(
    const CodeString&, bool sizeOne = true) noexcept;

[[nodiscard]] bool isAllSingleByte(const String&) noexcept;
[[nodiscard]] bool isAllSingleByte(const CodeString&) noexcept;

[[nodiscard]] bool isAnySingleByte(const String&) noexcept;
[[nodiscard]] bool isAnySingleByte(const CodeString&) noexcept;

template<typename T>
[[nodiscard]] inline auto firstConvert(T pred, T conv, const String& s) {
  if (!s.empty() && pred(s[0])) {
    String result{s};
    result[0] = toChar(conv(result[0]));
    return result;
  }
  return s;
}

// if first letter of (ascii string) 's' is upper case then return a copy with
// the first character converted to lower case, otherwise return 's'
[[nodiscard]] inline auto firstLower(const String& s) {
  return firstConvert(::isupper, ::tolower, s);
}

// if first letter of (ascii string) 's' is lower case then return a copy with
// the first character converted to upper case, otherwise return 's'
[[nodiscard]] inline auto firstUpper(const String& s) {
  return firstConvert(::islower, ::toupper, s);
}

// return a copy of (ascii string) 's' converted to lower case
[[nodiscard]] inline auto toLower(const String& s) {
  String result{s};
  std::transform(
      s.begin(), s.end(), result.begin(), [](auto c) { return ::tolower(c); });
  return result;
}

// return a copy of (ascii string) 's' converted to upper case
[[nodiscard]] inline auto toUpper(const String& s) {
  String result{s};
  std::transform(
      s.begin(), s.end(), result.begin(), [](auto c) { return ::toupper(c); });
  return result;
}

inline const String EmptyString;
inline const CodeString EmptyCodeString;

//! \end_group
} // namespace kanji_tools
