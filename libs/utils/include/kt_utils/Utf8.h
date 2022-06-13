#pragma once

#include <kt_utils/String.h>

namespace kanji_tools { /// \utils_group{Utf8}
/// UTF-8 conversion and validation functions and global UTF-8 related variables
/// \note conversion was originally implemented using `#include <codecvt>`, but
/// was changed to a local implementation to remove the dependency and increase
/// flexibility. For example, local implementations output 'U+FFFD' for errors
/// instead of throwing. Also, 'wstring_convert' was deprecated as of C++17
/// \sa http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0618r0.html).

/// convert from UTF-8 to UTF-32
/// \param s UTF-8 input to convert
/// \param maxSize can be used to control the number of characters converted
/// \return a UTF-32 #CodeString
[[nodiscard]] CodeString fromUtf8(const char* s, size_t maxSize = 0);
[[nodiscard]] CodeString fromUtf8(const String& s,
    size_t maxSize = 0); ///< \doc fromUtf8

/// convert from UTF-8 to a UTF-32 `wstring` (helpful for working with `wregex`)
/// \param s UTF-8 input to convert
/// \return a UTF-32 `wstring`
[[nodiscard]] std::wstring fromUtf8ToWstring(const char* s);
[[nodiscard]] std::wstring fromUtf8ToWstring(
    const String& s); ///< \doc fromUtf8ToWstring

/// convert from UTF-8 to one UTF-32 value
/// \param s UTF-8 input to convert
/// \return a UTF-32 #Code or `0` if input is empty
[[nodiscard]] Code getCode(const char* s) noexcept;
[[nodiscard]] Code getCode(const String& s) noexcept; ///< \doc getCode

/// convert from one UTF-32 value to UTF-8
/// \param x UTF-32 value
/// \return a UTF-8 #String containing on value
[[nodiscard]] String toUtf8(Code x);
[[nodiscard]] String toUtf8(int x);      ///< \doc toUtf8
[[nodiscard]] String toUtf8(uint32_t x); ///< \doc toUtf8

/// convert from UTF-32 to UTF-8
/// \param s UTF-32 input to convert
/// \return a UTF-8 #String
[[nodiscard]] String toUtf8(const CodeString& s);
[[nodiscard]] String toUtf8(
    const std::wstring& s); ///< \doc toUtf8(const CodeString&)

/// safe conversions of #Code to `wchar_t`
inline constexpr wchar_t toWChar(Code x) noexcept {
  static_assert(sizeof(wchar_t) == sizeof(Code));
  return static_cast<wchar_t>(x);
}

/// used as return value of validateMBUtf8()
enum class MBUtf8Result {
  Valid,        ///< valid multi-byte UTF-8
  NotMultiByte, ///< single-byte UTF-8 (Ascii)
  NotValid      ///< detailed conversion info will be in #Utf8Result
};

/// used as return value of validateUtf8(), provides more details about errors
enum class Utf8Result {
  Valid,            ///< valid UTF-8
  CharTooLong,      ///< first byte starts with more than 4 1's
  ContinuationByte, ///< first byte is a continuation byte
  InvalidCodePoint, ///< decodes to an invalid Unicode code point
  MissingBytes,     ///< not enough continuation bytes
  Overlong, ///< a character is 'UTF-8' encoded with more bytes than the minimum
            ///< required, i.e., it can be encoded in two bytes, but instead is
            ///< encoded using three or four bytes (with extra leading zero bits
            ///< \sa https://en.wikipedia.org/wiki/UTF-8#Overlong_encodings
  StringTooLong ///< more than one UTF-8 character (see examples below)
};

/// determine if input is valid 'multi-byte' UTF-8
/// \details Examples:
/// \code
///   validateMBUtf8(""); // returns NotMultiByte
///   validateMBUtf8("a"); // returns NotMultiByte
///   validateMBUtf8("a猫"); // returns NotMultiByte
///   validateMBUtf8("雪"); // returns Valid
///   validateMBUtf8("雪s", true); // returns NotValid
///   validateMBUtf8("吹雪", true); // returns NotValid
/// \endcode
/// Note, the last two would be 'Valid' if `sizeOne` was false (the default)
/// \param s UTF-8 input
/// \param[out] error set to detailed error type if `s` is not valid
/// \param sizeOne if true then `s` must consist of only one UTF-8 value
/// \return #MBUtf8Result (`Valid`, `NotMultiByte` or `NotValid`)
MBUtf8Result validateMBUtf8(
    const char* s, Utf8Result& error, bool sizeOne = false) noexcept;
MBUtf8Result validateMBUtf8(const String& s, Utf8Result& error,
    bool sizeOne = false) noexcept; ///< \doc validateMBUtf8

/// determine if input is valid 'multi-byte' UTF-8
/// \tparam T input type (should be able to convert to #String or `cons char*`)
/// \param s UTF-8 input
/// \param sizeOne if true then `s` must consist of only one UTF-8 value
/// \return #MBUtf8Result (`Valid`, `NotMultiByte` or `NotValid`)
template<typename T>
[[nodiscard]] inline auto validateMBUtf8(
    const T& s, bool sizeOne = false) noexcept {
  auto e{Utf8Result::Valid};
  return validateMBUtf8(s, e, sizeOne);
}

/// determine if input is valid UTF-8 (including single-byte Ascii)
/// \param s UTF-8 input
/// \param sizeOne if true then `s` must consist of only one UTF-8 value
/// \return #Utf8Result (`Valid` or 6 other values for invalid input)
template<typename T>
[[nodiscard]] inline auto validateUtf8(
    const T& s, bool sizeOne = false) noexcept {
  auto e{Utf8Result::Valid};
  validateMBUtf8(s, e, sizeOne);
  return e;
}

/// return true if input is valid 'multi-byte' UTF-8
/// \param s UTF-8 input
/// \param sizeOne if true then `s` must also consist of only one UTF-8 value
[[nodiscard]] bool isValidMBUtf8(
    const String& s, bool sizeOne = false) noexcept;

/// return true if input is valid UTF-8 (including single-byte Ascii)
/// \param s UTF-8 input
/// \param sizeOne if true then `s` must consist of only one UTF-8 value
[[nodiscard]] bool isValidUtf8(const String&, bool sizeOne = false) noexcept;

/// bit patterns used for processing UTF-8
enum BitPatterns : uint8_t {
  Bit5 = 0b00'00'10'00,      ///< only bit 5 is set
  Bit4 = 0b00'01'00'00,      ///< only bit 4 is set
  Bit3 = 0b00'10'00'00,      ///< only bit 3 is set
  Bit2 = 0b01'00'00'00,      ///< only bit 2 is set
  Bit1 = 0b10'00'00'00,      ///< only bit 1 is set (starts a continuation byte)
  TwoBits = 0b11'00'00'00,   ///< first two bits (starts multi-byte sequence)
  ThreeBits = 0b11'10'00'00, ///< start of a 3 byte multi-byte sequence
  FourBits = 0b11'11'00'00,  ///< start of a 4 byte multi-byte sequence
  FiveBits = 0b11'11'10'00   ///< illegal pattern for first byte (too long)
};

inline constexpr size_t VarSelectorSize{
    3},           ///< Kanji variation selectors and Kana combining marks
                  ///< are 3 bytes
    MinMBSize{2}, ///< multi-byte UTF-8 minimum size, i.e., 2 bytes
    MaxMBSize{4}; ///< multi-byte UTF-8 maximum size, i.e., 4 bytes

inline constexpr auto MaxAscii{U'\x7f'}, ///< maximum Ascii value
    MaxUnicode{U'\x10ffff'},             ///< maximum valid Unicode value
    CombiningVoicedChar{U'\x3099'},      ///< for dakuten (濁点) Kana
    CombiningSemiVoicedChar{U'\x309a'};  ///< for han-dakuten (半濁点) Kana
inline constexpr StringView CombiningVoiced{"\xe3\x82\x99"}, ///< UTF-8 'U+3099'
    CombiningSemiVoiced{"\xe3\x82\x9a"};                     ///< UTF-8 'U+309A'

/// \end_group
} // namespace kanji_tools
