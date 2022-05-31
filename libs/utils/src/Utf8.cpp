#include <kanji_tools/utils/Utf8.h>

#include <array>
#include <bit>

namespace kanji_tools {

using uInt = const uint32_t;

namespace {

// Values for determining invalid Unicode code points when doing UTF-8
// conversion (in addition to 'MaxUnicode' in Utf8.h). Here's a quote from
// https://en.wikipedia.org/wiki/UTF-8#Invalid_sequences_and_error_handling:
//   Since RFC 3629 (November 2003), the high and low surrogate halves used by
//   UTF-16 (U+D800 through U+DFFF) and code points not encodable by UTF-16
//   (those after U+10FFFF) are not legal Unicode values, and their UTF-8
//   encoding must be treated as an invalid byte sequence.
constexpr Code MinSurrogate{0xd800}, MaxSurrogate{0xdfff}, Max2Uni{0x7ff},
    Max3Uni{0xffff}, ErrorReplacement{0xfffd};

// constants and functions for shifting to help simplify the code and get rid
// of 'magic' numbers

constexpr uInt Shift6{6};
constexpr uInt Shift12{Shift6 * 2}, Shift18{Shift6 * 3};

[[nodiscard]] constexpr auto left6(uInt x) noexcept { return x << Shift6; }
[[nodiscard]] constexpr auto left12(uInt x) noexcept { return x << Shift12; }
[[nodiscard]] constexpr auto left18(uInt x) noexcept { return x << Shift18; }

[[nodiscard]] constexpr auto left6(uInt x, uInt y) noexcept {
  return left6(x) + y;
}
[[nodiscard]] constexpr auto left12(uInt x, uInt y) noexcept {
  return left12(x) + y;
}
[[nodiscard]] constexpr auto left18(uInt x, uInt y) noexcept {
  return left18(x) + y;
}

[[nodiscard]] constexpr auto right6(uInt x, uInt y) noexcept {
  return (x >> Shift6) + y;
}
[[nodiscard]] constexpr auto right12(uInt x, uInt y) noexcept {
  return (x >> Shift12) + y;
}
[[nodiscard]] constexpr auto right18(uInt x, uInt y) noexcept {
  return (x >> Shift18) + y;
}

// 'getNextByte' advances 'u' to the next byte and returns true it it starts
// with '10', i.e., a continuation byte
[[nodiscard]] constexpr auto getNextByte(const uint8_t*& u) noexcept {
  return std::countl_one(*++u) == 1;
}

// allow casting 'uInt' to 'Code' or 'wchar_t' (used by 'convertFromUtf8')
template<typename T>
[[nodiscard]] constexpr std::enable_if_t<
    std::is_same_v<T, Code> || std::is_same_v<T, wchar_t>, T>
cast(uInt x) noexcept {
  return static_cast<T>(x);
}

// 'threeByteUtf8' returns a value of type 'T' (Code or wchar_t) that
// represents a 3 byte UTF-8 character. The values passed in are:
//   'b1': first byte (strip leading '1's to get 'aaaa' part of '1110aaaa')
//   'b2': second byte without leading '1' (the 'bbbbbb' part of '10bbbbbb')
//   'u':  pointer to third byte (get the 'cccccc' part of '10cccccc')
// The result is made from the 16 bits: 'aaaa bbbbbb cccccc'
template<typename T>
[[nodiscard]] constexpr auto threeByteUtf8(
    uInt b1, uInt b2, const uint8_t* u) noexcept {
  return cast<T>(left12(b1 ^ ThreeBits, left6(b2, *u ^ Bit1)));
}

// 'fourByteUtf8' returns a value of type 'T' (Code or wchar_t) that
// represents a 4 byte UTF-8 character. The values passed in are:
//   'b1': first byte (strip leading '1's to get 'aaa' part of '11110aaa')
//   'b2': second byte without leading '1' (the 'bbbbbb' part of '10bbbbbb')
//   'b3': third byte without leading '1' (the 'cccccc' part of '10cccccc')
//   'u':  pointer to fourth byte (get the 'dddddd' part of '10dddddd')
// The result is made from the 21 bits: 'aaa bbbbbb cccccc dddddd'
template<typename T>
[[nodiscard]] constexpr auto fourByteUtf8(
    uInt b1, uInt b2, uInt b3, const uint8_t* u) noexcept {
  return cast<T>(left18(b1 ^ FourBits, left12(b2, left6(b3, *u ^ Bit1))));
}

// UTF-8 sequence for U+FFFD (�) - used by the local 'toUtf8' functions for
// invalid code points
constexpr auto ReplacementCharacter{"\xEF\xBF\xBD"};

// 'convertFromUtf8' is templated so use arrays of constants of the templated
// type in order to avoid casting and type warnings

constexpr auto Err{0}, MinSur{1}, MaxSur{2}, MaxTwo{3}, MaxThree{4}, MaxUni{5};
constexpr std::array Char32Vals{
    ErrorReplacement, MinSurrogate, MaxSurrogate, Max2Uni, Max3Uni, MaxUnicode};
constexpr std::array WCharVals{toWChar(ErrorReplacement), toWChar(MinSurrogate),
    toWChar(MaxSurrogate), toWChar(Max2Uni), toWChar(Max3Uni),
    toWChar(MaxUnicode)};

template<typename T> using Consts = std::array<T, Char32Vals.size()>;

template<typename T>
[[nodiscard]] T convertOneUtf8(const uint8_t*& u, const Consts<T>& v) noexcept {
  const auto utfLen{std::countl_one(*u)};
  if (!utfLen) return {*u++}; // single byte UTF-8 case (Ascii)
  if (utfLen == 1 || utfLen > 4) {
    ++u;           // GCOV_EXCL_START
    return v[Err]; // 1st byte was '10...' or more than four '1's
  }                // GCOV_EXCL_STOP
  uInt byte1{*u};
  if (!getNextByte(u)) return v[Err]; // 2nd byte not '10...'
  uInt byte2 = *u ^ Bit1;
  if (utfLen > 2) {
    if (!getNextByte(u)) return v[Err]; // 3rd not '10...'
    if (utfLen == 4) {
      uInt byte3 = *u ^ Bit1;
      if (!getNextByte(u)) return v[Err]; // 4th byte not '10...'
      const auto t{fourByteUtf8<T>(byte1, byte2, byte3, u++)};
      // return Error if 't' is 'overlong' or beyond max Unicode range
      return t > v[MaxThree] && t <= v[MaxUni] ? t : v[Err];
    }
    const auto t{threeByteUtf8<T>(byte1, byte2, u++)};
    // return Error if 't' is 'overlong' or in the Surrogate range
    return t > v[MaxTwo] && (t < v[MinSur] || t > v[MaxSur]) ? t : v[Err];
  }
  ++u;
  return (byte1 ^ TwoBits) > 1 ? cast<T>(left6(byte1 ^ TwoBits, byte2))
                               : v[Err];
}

// 'R' is a sequence (so u32string or wstring) and 'T' is Code or wchar_t
template<typename R, typename T = typename R::value_type>
[[nodiscard]] R convertFromUtf8(
    const char* s, size_t maxSize, const Consts<T>& v) {
  R result;
  if (!s || !*s) return result;
  auto u{reinterpret_cast<const uint8_t*>(s)};
  do {
    result += convertOneUtf8(u, v);
  } while (*u && (!maxSize || result.size() < maxSize));
  return result;
}

void convertToUtf8(Code c, String& s) {
  static constexpr Code FirstThree{left18(0b111)}, FirstFour{left12(0b11'11)},
      FirstFive{left6(0b1'11'11)}, Six{0b11'11'11};
  static constexpr Code SecondSix{left6(Six)}, ThirdSix{left12(Six)};
  if (c <= MaxAscii)
    s += toChar(c);
  else if (c <= Max2Uni) {
    s += toChar(right6(FirstFive & c, TwoBits));
    s += toChar((Six & c) + Bit1);
  } else if (c <= Max3Uni) {
    if (c < MinSurrogate || c > MaxSurrogate) {
      s += toChar(right12(FirstFour & c, ThreeBits));
      s += toChar(right6(SecondSix & c, Bit1));
      s += toChar((Six & c) + Bit1);
    } else
      s += ReplacementCharacter;
  } else if (c <= MaxUnicode) {
    s += toChar(right18(FirstThree & c, FourBits));
    s += toChar(right12(ThirdSix & c, Bit1));
    s += toChar(right6(SecondSix & c, Bit1));
    s += toChar((Six & c) + Bit1);
  } else
    s += ReplacementCharacter; // GCOV_EXCL_LINE
}

// 'validateMB' is called by 'validateMBUtf8' ('u' has already been verified to
// point to the first byte of a UTF-8 sequence by the calling function)
template<typename T>
[[nodiscard]] auto validateMB(const T& err, const uint8_t* u, bool sizeOne) {
  uInt byte1{*u};
  if (!getNextByte(u)) return err(Utf8Result::MissingBytes);
  if (byte1 & Bit3) {
    uInt byte2 = *u ^ Bit1; // last 6 bits of the second byte
    if (!getNextByte(u)) return err(Utf8Result::MissingBytes);
    if (byte1 & Bit4) {
      if (byte1 & Bit5) return err(Utf8Result::CharTooLong);
      uInt byte3 = *u ^ Bit1; // last 6 bits of the third byte
      if (!getNextByte(u)) return err(Utf8Result::MissingBytes);
      const auto code4{fourByteUtf8<Code>(byte1, byte2, byte3, u)};
      if (code4 <= Max3Uni) return err(Utf8Result::Overlong); // overlong 4 byte
      if (code4 > MaxUnicode) return err(Utf8Result::InvalidCodePoint);
    } else if (const auto code3{threeByteUtf8<Code>(byte1, byte2, u)};
               code3 <= Max2Uni)
      return err(Utf8Result::Overlong); // GCOV_EXCL_LINE
    else if (code3 >= MinSurrogate && code3 <= MaxSurrogate)
      return err(Utf8Result::InvalidCodePoint);
  } else if ((byte1 ^ TwoBits) < 2)
    return err(Utf8Result::Overlong); // overlong 2 byte
  return !sizeOne || !*++u ? MBUtf8Result::Valid
                           : err(Utf8Result::StringTooLong);
}

} // namespace

CodeString fromUtf8(const char* s, size_t maxSize) {
  return convertFromUtf8<CodeString>(s, maxSize, Char32Vals);
}

CodeString fromUtf8(const String& s, size_t maxSize) {
  return fromUtf8(s.c_str(), maxSize);
}

std::wstring fromUtf8ToWstring(const char* s) {
  return convertFromUtf8<std::wstring>(s, 0, WCharVals);
}

std::wstring fromUtf8ToWstring(const String& s) {
  return fromUtf8ToWstring(s.c_str());
}

Code getCode(const char* s) noexcept {
  if (!s || !*s) return {};
  auto u{reinterpret_cast<const uint8_t*>(s)};
  return convertOneUtf8(u, Char32Vals);
}

Code getCode(const String& s) noexcept { return getCode(s.c_str()); }

String toUtf8(Code x) {
  String result;
  convertToUtf8(x, result);
  return result;
}

String toUtf8(int x) { return toUtf8(static_cast<Code>(x)); }

String toUtf8(uint32_t x) { return toUtf8(static_cast<Code>(x)); }

String toUtf8(const CodeString& s) {
  String result;
  // result will be bigger than 's' if there are any multibyte chars
  result.reserve(s.size());
  for (auto c : s) convertToUtf8(c, result);
  return result;
}

String toUtf8(const std::wstring& s) {
  String result;
  // result will be bigger than 's' if there are any multibyte chars
  result.reserve(s.size());
  for (auto c : s) convertToUtf8(static_cast<Code>(c), result);
  return result;
}

// validateMBUtf8

MBUtf8Result validateMBUtf8(
    const char* s, Utf8Result& error, bool sizeOne) noexcept {
  if (!s) return MBUtf8Result::NotMultiByte;
  auto u{reinterpret_cast<const uint8_t*>(s)};
  if (!(*u & Bit1)) return MBUtf8Result::NotMultiByte;
  const auto err{[&error](auto e) {
    error = e;
    return MBUtf8Result::NotValid;
  }};
  if ((*u & TwoBits) == Bit1) return err(Utf8Result::ContinuationByte);
  return validateMB(err, u, sizeOne);
}

MBUtf8Result validateMBUtf8(
    const String& s, Utf8Result& error, bool sizeOne) noexcept {
  return validateMBUtf8(s.c_str(), error, sizeOne);
}

bool isValidMBUtf8(const String& s, bool sizeOne) noexcept {
  return validateMBUtf8(s, sizeOne) == MBUtf8Result::Valid;
}

bool isValidUtf8(const String& s, bool sizeOne) noexcept {
  return validateUtf8(s, sizeOne) == Utf8Result::Valid;
}

} // namespace kanji_tools
