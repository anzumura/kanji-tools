#pragma once

#include <string>

namespace kanji_tools {

// helper functions for adding brackets and adding leading zeroes

enum class BracketType { Curly, Round, Square, None };

[[nodiscard]] std::string addBrackets(const std::string&, BracketType);

[[nodiscard]] std::string addLeadingZeroes(const std::string&, size_t minSize);

[[nodiscard]] std::u32string addLeadingZeroes(
    const std::u32string&, size_t minSize);

// convert a 'char32_t' into a Unicode code point (caps hex with minSize of 4)
[[nodiscard]] std::string toUnicode(char32_t, BracketType = BracketType::None);

// convert a UTF-8 string into space-separated Unicode code points. Note: non-
// None 'brackets' puts brackets around the whole string (not each entry).
[[nodiscard]] std::string toUnicode(
    const std::string&, BracketType = BracketType::None);

[[nodiscard]] std::string toUnicode(
    const std::u32string&, BracketType = BracketType::None);

// 'toChar' safely converts 'x' to a char. If 'x' is out of range then an
// exception is thrown. If 'allowNegative' is true (the default) then 'x' can't
// be less than -128, otherwise 'x' must be positive. 'x' must be less then 256
// regardless of the value of 'allowNegative'.
[[nodiscard]] char toChar(int x, bool allowNegative = true);

// 'toChar' overloads for common unsigned types (don't need 'allowNegative')
[[nodiscard]] char toChar(uint16_t);
[[nodiscard]] char toChar(unsigned int);
[[nodiscard]] char toChar(size_t);
[[nodiscard]] char toChar(char32_t);

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
  std::string result;
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
  std::string result;
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
[[nodiscard]] constexpr auto isSingleByteChar(char32_t x) noexcept {
  return x < SevenBitMax;
}

[[nodiscard]] bool isSingleByte(
    const std::string&, bool sizeOne = true) noexcept;
[[nodiscard]] bool isSingleByte(
    const std::u32string&, bool sizeOne = true) noexcept;

[[nodiscard]] bool isAllSingleByte(const std::string&) noexcept;
[[nodiscard]] bool isAllSingleByte(const std::u32string&) noexcept;

[[nodiscard]] bool isAnySingleByte(const std::string&) noexcept;
[[nodiscard]] bool isAnySingleByte(const std::u32string&) noexcept;

template<typename T>
[[nodiscard]] inline auto firstConvert(T pred, T conv, const std::string& s) {
  if (!s.empty() && pred(s[0])) {
    std::string result{s};
    result[0] = toChar(conv(result[0]));
    return result;
  }
  return s;
}

// if first letter of (ascii string) 's' is upper case then return a copy with
// the first character converted to lower case, otherwise return 's'
[[nodiscard]] inline auto firstLower(const std::string& s) {
  return firstConvert(::isupper, ::tolower, s);
}

// if first letter of (ascii string) 's' is lower case then return a copy with
// the first character converted to upper case, otherwise return 's'
[[nodiscard]] inline auto firstUpper(const std::string& s) {
  return firstConvert(::islower, ::toupper, s);
}

// return a copy of (ascii string) 's' converted to lower case
[[nodiscard]] inline auto toLower(const std::string& s) {
  std::string result{s};
  std::transform(
      s.begin(), s.end(), result.begin(), [](auto c) { return ::tolower(c); });
  return result;
}

// return a copy of (ascii string) 's' converted to upper case
[[nodiscard]] inline auto toUpper(const std::string& s) {
  std::string result{s};
  std::transform(
      s.begin(), s.end(), result.begin(), [](auto c) { return ::toupper(c); });
  return result;
}

inline const std::string EmptyString;
inline const std::u32string EmptyU32String;

} // namespace kanji_tools
