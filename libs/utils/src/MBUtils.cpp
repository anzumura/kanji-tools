#include <kanji_tools/utils/MBUtils.h>
#include <kanji_tools/utils/Utils.h>

#ifdef USE_CODECVT_FOR_UTF_8
#include <codecvt> // for codecvt_utf8
#include <locale>  // for wstring_convert
#else
#include <array>
#endif

namespace kanji_tools {

using uInt = const unsigned int;

namespace {

// Values for determining invalid Unicde code points when doing UTF-8
// conversion (in addition to 'MaxUnicode' in MBUtils.h). Here's a quote from
// https://en.wikipedia.org/wiki/UTF-8#Invalid_sequences_and_error_handling:
//   Since RFC 3629 (November 2003), the high and low surrogate halves used by
//   UTF-16 (U+D800 through U+DFFF) and code points not encodable by UTF-16
//   (those after U+10FFFF) are not legal Unicode values, and their UTF-8
//   encoding must be treated as an invalid byte sequence.
constexpr Code MinSurrogate{0xd800}, MaxSurrogate{0xdfff}, Max2Uni{0x7ff},
    Max3Uni{0xffff};

// constants and functions for shifting to help simplify the code and get rid
// of 'magic' numbers

constexpr uInt Shift6{6};
constexpr uInt Shift12{Shift6 * 2}, Shift18{Shift6 * 3};

constexpr auto left6(uInt x) noexcept { return x << Shift6; }
constexpr auto left12(uInt x) noexcept { return x << Shift12; }
constexpr auto left18(uInt x) noexcept { return x << Shift18; }

constexpr auto left6(uInt x, uInt y) noexcept { return left6(x) + y; }
constexpr auto left12(uInt x, uInt y) noexcept { return left12(x) + y; }
constexpr auto left18(uInt x, uInt y) noexcept { return left18(x) + y; }

constexpr auto right6(uInt x, uInt y) noexcept { return (x >> Shift6) + y; }
constexpr auto right12(uInt x, uInt y) noexcept { return (x >> Shift12) + y; }
constexpr auto right18(uInt x, uInt y) noexcept { return (x >> Shift18) + y; }

// allow casting 'uInt' to 'char32_t' or 'wchar_t' (used by 'convertFromUtf8')
template<typename T>
constexpr std::enable_if_t<
    std::is_same_v<T, Code> || std::is_same_v<T, wchar_t>, T>
cast(uInt x) noexcept {
  return static_cast<T>(x);
}

template<typename T>
constexpr auto threeByteUtf8(const unsigned char* u, uInt b1, uInt b2) {
  return cast<T>(left12(b1 ^ ThreeBits, left6(b2, *u ^ Bit1)));
}

template<typename T>
constexpr auto fourByteUtf8(const unsigned char* u, uInt b1, uInt b2, uInt b3) {
  return cast<T>(left18(b1 ^ FourBits, left12(b2, left6(b3, *u ^ Bit1))));
}

#ifdef USE_CODECVT_FOR_UTF_8
inline auto utf8Converter() {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return &conv;
}
#else

constexpr Code ErrorReplacement{0xfffd};

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

// 'R' is the sequence (so u32string or wstring) and 'T' is char32_t or wchar_t
template<typename R, typename T = typename R::value_type>
[[nodiscard]] R convertFromUtf8(const char* s, const Consts<T>& v) {
  R result;
  if (!s || !*s) return result;
  auto u{reinterpret_cast<const unsigned char*>(s)};

  // return a 'T' that represents a 2-byte UTF-8 character (U+0080 to U+07FF)
  const auto twoByte{[&u, &v](uInt b1, uInt b2) {
    ++u;
    return (b1 ^ TwoBits) > 1 ? cast<T>(left6(b1 ^ TwoBits, b2)) : v[Err];
  }};

  // return a 'T' that represents a 3-byte UTF-8 character (U+0800 to U+FFFF)
  const auto threeByte{[&u, &v](uInt b1, uInt b2) {
    const auto t{threeByteUtf8<T>(u, b1, b2)};
    ++u;
    // return Error if 't' is 'overlong' or in the Surrogate range
    return t > v[MaxTwo] && (t < v[MinSur] || t > v[MaxSur]) ? t : v[Err];
  }};

  // return a 'T' that represents a 4-byte UTF-8 character (U+10080 to U+10FFFF)
  const auto fourByte{[&u, &v](uInt b1, uInt b2, uInt b3) {
    if ((*++u & TwoBits) != Bit1) return v[Err]; // 4th byte not '10...'
    const auto t{fourByteUtf8<T>(u, b1, b2, b3)};
    ++u;
    // return Error if 't' is 'overlong' or beyond max Unicode range
    return t > v[MaxThree] && t <= v[MaxUni] ? t : v[Err];
  }};

  do {
    if (*u <= MaxAscii) {
      const T t{*u++};
      result += t; // single byte UTF-8 case (so regular Ascii)
    } else if ((*u & TwoBits) == Bit1 || (*u & FiveBits) == FiveBits) {
      // LCOV_EXCL_START: gcov-11 bug
      ++u;
      result += v[Err]; // 1st byte was '10...' or more than four '1's
      // LCOV_EXCL_STOP
    } else if (uInt b1{*u}; (*++u & TwoBits) != Bit1)
      result += v[Err]; // 2nd byte not '10...'
    else if (uInt b2 = *u ^ Bit1; b1 & Bit3)
      result += (*++u & TwoBits) != Bit1 ? v[Err] // 3rd not '10...'
                : (b1 & Bit4)            ? fourByte(b1, b2, *u ^ Bit1)
                                         : threeByte(b1, b2);
    else
      result += twoByte(b1, b2); // LCOV_EXCL_LINE: gcov-11 bug
  } while (*u);
  return result;
}

void convertToUtf8(Code c, std::string& s) {
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
  } else if (c <= MaxUnicode) { // LCOV_EXCL_LINE: gcov-11 bug
    s += toChar(right18(FirstThree & c, FourBits));
    s += toChar(right12(ThirdSix & c, Bit1));
    s += toChar(right6(SecondSix & c, Bit1));
    s += toChar((Six & c) + Bit1);
  } else
    s += ReplacementCharacter; // LCOV_EXCL_LINE: gcov-11 bug
}
#endif

} // namespace

std::u32string fromUtf8(const char* s) {
#ifdef USE_CODECVT_FOR_UTF_8
  const auto r{utf8Converter()->from_bytes(s)};
  return std::u32string(reinterpret_cast<const Code*>(r.c_str()));
#else
  return convertFromUtf8<std::u32string>(s, Char32Vals);
#endif
}

std::u32string fromUtf8(const std::string& s) { return fromUtf8(s.c_str()); }

std::string toUtf8(Code c) {
#ifdef USE_CODECVT_FOR_UTF_8
  return utf8Converter()->to_bytes(toWChar(c));
#else
  std::string result;
  convertToUtf8(c, result);
  return result;
#endif
}

std::string toUtf8(int x) { return toUtf8(static_cast<Code>(x)); }

std::string toUtf8(long x) { return toUtf8(static_cast<Code>(x)); }

std::string toUtf8(const std::u32string& s) {
#ifdef USE_CODECVT_FOR_UTF_8
  return utf8Converter()->to_bytes(reinterpret_cast<const wchar_t*>(s.c_str()));
#else
  std::string result;
  // result will be bigger than 's' if there are any multibyte chars
  result.reserve(s.size());
  for (auto c : s) convertToUtf8(c, result);
  return result;
#endif
}

// wstring versions of conversion functions

std::wstring fromUtf8ToWstring(const char* s) {
#ifdef USE_CODECVT_FOR_UTF_8
  return utf8Converter()->from_bytes(s);
#else
  return convertFromUtf8<std::wstring>(s, WCharVals);
#endif
}

std::wstring fromUtf8ToWstring(const std::string& s) {
  return fromUtf8ToWstring(s.c_str());
}

std::string toUtf8(const std::wstring& s) {
#ifdef USE_CODECVT_FOR_UTF_8
  return utf8Converter()->to_bytes(s);
#else
  std::string result;
  // result will be bigger than 's' if there are any multibyte chars
  result.reserve(s.size());
  for (auto c : s) convertToUtf8(static_cast<Code>(c), result);
  return result;
#endif
}

// validateMBUtf8

MBUtf8Result validateMBUtf8(
    const char* s, Utf8Result& error, bool sizeOne) noexcept {
  const auto err{[&error](auto e) {
    error = e;
    return MBUtf8Result::NotValid;
  }};
  if (!s || !(*s & Bit1)) return MBUtf8Result::NotMultiByte;
  if ((*s & TwoBits) == Bit1) return err(Utf8Result::ContinuationByte);
  auto u{reinterpret_cast<const unsigned char*>(s)};
  uInt b1{*u};
  if ((*++u & TwoBits) != Bit1) return err(Utf8Result::MissingBytes);
  if (b1 & Bit3) {
    uInt b2 = *u ^ Bit1; // last 6 bits of the second byte
    if ((*++u & TwoBits) != Bit1) return err(Utf8Result::MissingBytes);
    if (b1 & Bit4) {
      if (b1 & Bit5) return err(Utf8Result::CharTooLong);
      uInt b3 = *u ^ Bit1; // last 6 bits of the third byte
      if ((*++u & TwoBits) != Bit1) return err(Utf8Result::MissingBytes);
      uInt c{fourByteUtf8<Code>(u, b1, b2, b3)};
      if (c <= Max3Uni) return err(Utf8Result::Overlong); // overlong 4 byte
      if (c > MaxUnicode) return err(Utf8Result::InvalidCodePoint);
    } else if (uInt c{threeByteUtf8<Code>(u, b1, b2)}; c <= Max2Uni)
      return err(Utf8Result::Overlong); // overlong 3 byte
    else if (c >= MinSurrogate && c <= MaxSurrogate)
      return err(Utf8Result::InvalidCodePoint);
  } else if ((b1 ^ TwoBits) < 2)      // LCOV_EXCL_LINE: gcov-11 bug
    return err(Utf8Result::Overlong); // overlong 2 byte
  return !sizeOne || !*++u ? MBUtf8Result::Valid
                           : err(Utf8Result::StringTooLong);
}

MBUtf8Result validateMBUtf8(
    const std::string& s, Utf8Result& e, bool sizeOne) noexcept {
  return validateMBUtf8(s.c_str(), e, sizeOne);
}

bool isValidMBUtf8(const std::string& s, bool sizeOne) noexcept {
  return validateMBUtf8(s, sizeOne) == MBUtf8Result::Valid;
}

bool isValidUtf8(const std::string& s, bool sizeOne) noexcept {
  return validateUtf8(s, sizeOne) == Utf8Result::Valid;
}

} // namespace kanji_tools
