#pragma once

#include <string>

namespace kanji_tools { /// \utils_group{String}
/// Type Aliases and functions for working with strings (like addBrackets(),
/// toHex(), isAllSingleByte(), etc.)

/// String and StringView type aliases may be changed to `std::u8` versions
/// later once they get wider standard library support (streams, regex, etc.)
using String = std::string;
using StringView = std::string_view; ///< \doc String

/// type alias for Unicode code points (use `char32_t` instead of `wchar_t`
/// since it's always 32 bits instead of platform dependent)
using Code = char32_t;
using CodeString = std::u32string; ///< \doc Code

// Global Enums

/// bracket type to use in functions like addBrackets(), toHex(), etc.
enum class BracketType {
  Curly,  ///< add curly braces: {}
  Round,  ///< add round brackets: ()
  Square, ///< add square brackets: []
  None    ///< don't add brackets
};

/// case for hex digits to use in string representations
enum class HexCase {
  Upper, ///< use upper-case (the standard for Unicode)
  Lower  ///< use lower-case (typical default when printing numbers)
};

// Global Constants

inline constexpr auto BinaryDigits{2}, ///< number of binary digits (0 and 1)
    DecimalDigits{10},                 ///< number of decimal digits (0-9)
    HexDigits{16},                     ///< number of hex digits (0-9 plus a-f)
    Bits{8},                           ///< number of bits in a byte
    SevenBitMax{128},                  ///< max value for a seven bit number
    UnicodeStringMinSize{4},           ///< min size for a Unicode hex string
    UnicodeStringMaxSize{5};           ///< max size for a Unicode hex string

// Global Functions

/// can be used by functions needing to return `const&` empty values
const String& emptyString();
const CodeString& emptyCodeString(); ///< \doc emptyString()

/// return a copy of `s` surrounded in brackets of the given type
[[nodiscard]] String addBrackets(const String& s, BracketType);

/// return a copy of `s` prepended with `minSize - s.size()` zeroes
[[nodiscard]] String addLeadingZeroes(const String& s, size_t minSize);
[[nodiscard]] CodeString addLeadingZeroes(
    const CodeString& s, size_t minSize); /// \doc addLeadingZeroes()

/// convert #Code into a Unicode code point (caps hex with minSize of 4)
[[nodiscard]] String toUnicode(Code, BracketType = BracketType::None);

/// convert a (UTF-8) #String into space-separated Unicode code points
/// \details brackets are put around the whole string (not each Unicode value)
[[nodiscard]] String toUnicode(const String&, BracketType = BracketType::None);

/// convert a (UTF-32) #CodeString into space-separated Unicode code points
/// \details brackets are put around the whole string (not each Unicode value)
[[nodiscard]] String toUnicode(
    const CodeString&, BracketType = BracketType::None);

/// safely converts `x` to a char
/// \param allowNegative if true (the default) then `x` can't be less than -128
///     otherwise `x` must be positive.
/// \param x the value to convert (must be < 256 regardless of `allowNegative`)
/// \return the char value
/// \throw RangeError if `x` is out of range for char type
[[nodiscard]] char toChar(int x, bool allowNegative = true);

/// toChar(int,bool) overload for `uint16_t`
/// \details type is `unsigned` so `allowNegative` param isn't needed
[[nodiscard]] char toChar(uint16_t);

/// `uint32_t` overload of toChar(int,bool)
[[nodiscard]] char toChar(uint32_t); ///< \copydetails toChar(uint16_t)

/// `size_t` overload of toChar(int,bool)
[[nodiscard]] char toChar(size_t); ///< \copydetails toChar(uint16_t)

/// #Code overload of toChar(int,bool)
[[nodiscard]] char toChar(Code); ///< \copydetails toChar(uint16_t)

/// convert `uint8_t` to `char`
/// \details doesn't throw since sizes are the same
[[nodiscard]] char toChar(uint8_t);

/// convert `char` to `uint8_t`
[[nodiscard]] uint8_t toUChar(char); ///< \copydetails toChar(uint8_t)

/// return a #String containing the binary representation of `x`
/// \tparam T must be an unsigned type
/// \param x the value to convert
/// \param brackets specify type of brackets (can be `None` for no brackets)
/// \param minSize `0` (the default) causes enough leading zeroes to be added to
///     make results the same size for a given type, i.e., if `T` is `char` then
///     the result will have a size of 8
/// \return #String with the binary representation of `x`
template <typename T>
[[nodiscard]] auto toBinary(T x, BracketType brackets, size_t minSize = 0) {
  static_assert(std::is_unsigned_v<T>);
  String result;
  for (; x > 0; x >>= 1U)
    result.insert(result.begin(), '0' + toChar(x % BinaryDigits));
  return addBrackets(
      addLeadingZeroes(result, minSize ? minSize : sizeof(T) * Bits), brackets);
}

/// overload of toBinary() that sets `brackets` to #BracketType::None
template <typename T> [[nodiscard]] auto toBinary(T x, size_t minSize = 0) {
  return toBinary(x, BracketType::None, minSize);
}

/// return a #String containing the hex representation of `x`
/// \tparam T must be an unsigned type
/// \param x the value to convert
/// \param brackets specify type of brackets (can be `None` for no brackets)
/// \param hexCase specify the case for hex digits
/// \param minSize `0` (the default) causes enough leading zeroes to be added to
///     make results the same size for a given type, i.e., if `T` is `char` then
///     the result will have a size of 2 (00 - FF)
/// \return #String with the binary representation of `x`
template <typename T>
[[nodiscard]] auto toHex(
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

/// overload of toHex() that sets `brackets` to #BracketType::None
template <typename T>
[[nodiscard]] auto toHex(T x, HexCase hexCase, size_t minSize = 0) {
  return toHex(x, BracketType::None, hexCase, minSize);
}

/// overload of toHex() that sets `hexCase` to #HexCase::Lower
template <typename T>
[[nodiscard]] auto toHex(T x, BracketType brackets, size_t minSize = 0) {
  return toHex(x, brackets, HexCase::Lower, minSize);
}

/// overload of toHex() that sets `brackets` to #BracketType::None and `hexCase`
/// to #HexCase::Lower
template <typename T> [[nodiscard]] auto toHex(T x, size_t minSize = 0) {
  return toHex(x, BracketType::None, HexCase::Lower, minSize);
}

/// specialization of toBinary() for `T = char` that casts to `uint8_t`
template <>
[[nodiscard]] inline auto toBinary(
    char x, BracketType brackets, size_t minSize) {
  return toBinary(toUChar(x), brackets, minSize);
}

/// specialization of toHex() for `T = char` that casts to `uint8_t`
template <>
[[nodiscard]] inline auto toHex(
    char x, BracketType brackets, HexCase hexCase, size_t minSize) {
  return toHex(toUChar(x), brackets, hexCase, minSize);
}

/// return true if `x` is regular Ascii, i.e., not a 'multi-byte' character
[[nodiscard]] constexpr auto isSingleByteChar(char x) noexcept {
  return x >= 0;
}

/// return true if `x` represents a single byte character (7-bit Ascii)
[[nodiscard]] constexpr auto isSingleByteChar(Code x) noexcept {
  return x < SevenBitMax;
}

/// checks if the first character of `s` is a single-byte character
/// \param s input string
/// \param sizeOne if false (the default) then `s` can be any non-empty size,
///     otherwise `s.size()` must have be exactly `1`
/// \return true if `sizeOne` criteria is met and the first character of `s` is
///     a single-byte character
[[nodiscard]] bool isSingleByte(const String& s, bool sizeOne = true) noexcept;
[[nodiscard]] bool isSingleByte(const CodeString& s,
    bool sizeOne = true) noexcept; ///< \doc isSingleByte()

/// return true if all characters are single-byte
[[nodiscard]] bool isAllSingleByte(const String&) noexcept;
[[nodiscard]] bool isAllSingleByte(
    const CodeString&) noexcept; ///< \doc isAllSingleByte()

/// return true if any characters are single-byte
[[nodiscard]] bool isAnySingleByte(const String&) noexcept;
[[nodiscard]] bool isAnySingleByte(
    const CodeString&) noexcept; ///< \doc isAnySingleByte()

/// convert first char of `s` (used by firstLower() and firstUpper())
/// \tparam P function that returns true if value should be converted
/// \tparam C function that returns converted value
/// \param s String to convert
/// \return converted copy of `s` or just `s` if `P` is false
template <auto P, auto C> [[nodiscard]] auto firstConvert(const String& s) {
  if (!s.empty() && P(s[0])) {
    String result{s};
    result[0] = toChar(C(result[0]));
    return result;
  }
  return s;
}

/// if first char in `s` is an Ascii upper case letter then return a copy with
/// the first letter converted to lower case, otherwise return `s`
[[nodiscard]] String firstLower(const String& s);

/// if first char in `s` is an Ascii lower case letter then return a copy with
/// the first letter converted to upper case, otherwise return `s`
[[nodiscard]] String firstUpper(const String& s);

/// return a copy of `s` with all Ascii letters converted to lower case
[[nodiscard]] String toLower(const String& s);

/// return a copy of `s` with all Ascii letters converted to upper case
[[nodiscard]] String toUpper(const String& s);

/// \end_group
} // namespace kanji_tools
