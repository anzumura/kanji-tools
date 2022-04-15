#include <kanji_tools/utils/MBUtils.h>

#ifdef USE_CODECVT_FOR_UTF_8
#include <codecvt> // for codecvt_utf8
#include <locale>  // for wstring_convert
#endif

namespace kanji_tools {

namespace {

// Values for determining invalid Unicde code points when doing UTF-8
// conversion (in addition to 'MaxUnicode' in MBUtils.h). Here's a quote from
// https://en.wikipedia.org/wiki/UTF-8#Invalid_sequences_and_error_handling:
//   Since RFC 3629 (November 2003), the high and low surrogate halves used by
//   UTF-16 (U+D800 through U+DFFF) and code points not encodable by UTF-16
//   (those after U+10FFFF) are not legal Unicode values, and their UTF-8
//   encoding must be treated as an invalid byte sequence.
constexpr char32_t MinSurrogate{0xd800}, MaxSurrogate{0xdfff};

#ifdef USE_CODECVT_FOR_UTF_8
inline auto utf8Converter() {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return &conv;
}
#else
constexpr char32_t ErrorReplacement{0xfffd};

// UTF-8 sequence for U+FFFD (�) - used by the local 'toUtf8' functions for
// invalid code points
constexpr auto ReplacementCharacter{"\xEF\xBF\xBD"};

template<typename R, typename T = typename R::value_type>
[[nodiscard]] R convertFromUtf8(const char* s) {
  static_assert(std::is_integral_v<T>);
  static_assert(sizeof(T) == 4);
  static constexpr T TError{static_cast<T>(ErrorReplacement)},
      TMinSur{static_cast<T>(MinSurrogate)},
      TMaxSur{static_cast<T>(MaxSurrogate)},
      TMaxUni{static_cast<T>(MaxUnicode)};
  R result;
  if (!s || !*s) return result;
  auto u{reinterpret_cast<const unsigned char*>(s)};
  do {
    if (*u <= 0x7fU) // one byte case
      result += static_cast<T>(*u++);
    else if ((*u & TwoBits) == Bit1 || (*u & FiveBits) == FiveBits) {
      // first byte was '10' or started with more than four '1's
      result += TError; // LCOV_EXCL_LINE: gcov-11 bug
      ++u;
    } else {
      const unsigned byte1{*u};
      if ((*++u & TwoBits) != Bit1)
        result += TError; // second byte didn't start with '10'
      else {
        const unsigned byte2 = *u ^ Bit1; // last 6 bits of the second byte
        if (byte1 & Bit3) {
          if ((*++u & TwoBits) != Bit1)
            result += TError; // third byte didn't start with '10'
          else if (byte1 & Bit4) {
            const unsigned byte3 = *u ^ Bit1; // last 6 bits of the third byte
            if ((*++u & TwoBits) != Bit1)
              result += TError; // fourth byte didn't start with '10'
            else {
              // four byte case - check for overlong and max unicode
              const auto c{
                  static_cast<T>(((byte1 ^ FourBits) << 18) + (byte2 << 12) +
                                 (byte3 << 6) + (*u ^ Bit1))};
              result += c > 0xffff && c <= TMaxUni ? c : TError;
              ++u;
            }
          } else {
            // three byte case - check for overlong and surrogate range
            const auto c{static_cast<T>(
                ((byte1 ^ ThreeBits) << 12) + (byte2 << 6) + (*u ^ Bit1))};
            result += c > 0x7ff && (c < TMinSur || c > TMaxSur) ? c : TError;
            ++u;
          }
        } else {
          // two byte case - check for overlong
          result += (byte1 ^ TwoBits) > 1
                        ? static_cast<T>(((byte1 ^ TwoBits) << 6) + byte2)
                        : TError;
          ++u;
        }
      }
    }
  } while (*u);
  return result;
}

void convertToUtf8(char32_t c, std::string& s) {
  static constexpr char32_t FirstThree{0b111 << 18}, FirstFour{0b11'11 << 12},
      FirstFive{0b1'11'11 << 6}, Six{0b11'11'11};
  static constexpr char32_t SecondSix{Six << 6}, ThirdSix{Six << 12};
  if (c <= 0x7f)
    s += static_cast<char>(c);
  else if (c <= 0x7ff) {
    s += static_cast<char>(((FirstFive & c) >> 6) + TwoBits);
    s += static_cast<char>((Six & c) + Bit1);
  } else if (c <= 0xffff) {
    if (c < MinSurrogate || c > MaxSurrogate) {
      s += static_cast<char>(((FirstFour & c) >> 12) + ThreeBits);
      s += static_cast<char>(((SecondSix & c) >> 6) + Bit1);
      s += static_cast<char>((Six & c) + Bit1);
    } else
      s += ReplacementCharacter;
  } else if (c <= MaxUnicode) { // LCOV_EXCL_LINE: gcov-11 bug
    s += static_cast<char>(((FirstThree & c) >> 18) + FourBits);
    s += static_cast<char>(((ThirdSix & c) >> 12) + Bit1);
    s += static_cast<char>(((SecondSix & c) >> 6) + Bit1);
    s += static_cast<char>((Six & c) + Bit1);
  } else
    s += ReplacementCharacter; // LCOV_EXCL_LINE: gcov-11 bug
}
#endif

} // namespace

std::u32string fromUtf8(const char* s) {
#ifdef USE_CODECVT_FOR_UTF_8
  const auto r{utf8Converter()->from_bytes(s)};
  return std::u32string(reinterpret_cast<const char32_t*>(r.c_str()));
#else
  return convertFromUtf8<std::u32string>(s);
#endif
}

std::u32string fromUtf8(const std::string& s) { return fromUtf8(s.c_str()); }

std::string toUtf8(char32_t c) {
#ifdef USE_CODECVT_FOR_UTF_8
  return utf8Converter()->to_bytes(static_cast<wchar_t>(c));
#else
  std::string result;
  convertToUtf8(c, result);
  return result;
#endif
}

std::string toUtf8(int x) { return toUtf8(static_cast<char32_t>(x)); }

std::string toUtf8(long x) { return toUtf8(static_cast<char32_t>(x)); }

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
  return convertFromUtf8<std::wstring>(s);
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
  for (auto c : s) convertToUtf8(static_cast<char32_t>(c), result);
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
  const unsigned byte1{*u};
  if ((*++u & TwoBits) != Bit1)
    return err(Utf8Result::MissingBytes); // didn't start with '10'
  if (byte1 & Bit3) {
    const unsigned byte2 = *u ^ Bit1; // last 6 bits of the second byte
    if ((*++u & TwoBits) != Bit1)
      return err(Utf8Result::MissingBytes); // didn't start with '10'
    if (byte1 & Bit4) {
      if (byte1 & Bit5)
        return err(Utf8Result::CharTooLong); // UTF-8 more than 4 bytes
      const unsigned byte3 = *u ^ Bit1;      // last 6 bits of the third byte
      if ((*++u & TwoBits) != Bit1)
        return err(Utf8Result::MissingBytes); // didn't start with '10'
      const unsigned c{((byte1 ^ FourBits) << 18) + (byte2 << 12) +
                       (byte3 << 6) + (*u ^ Bit1)};
      if (c <= 0xffffU)
        return err(Utf8Result::Overlong); // overlong 4 byte encoding
      if (c > MaxUnicode) return err(Utf8Result::InvalidCodePoint);
    } else if (const unsigned c{
                   ((byte1 ^ ThreeBits) << 12) + (byte2 << 6) + (*u ^ Bit1)};
               c <= 0x7ffU)
      return err(Utf8Result::Overlong); // overlong 3 byte encoding
    else if (c >= MinSurrogate && c <= MaxSurrogate)
      return err(Utf8Result::InvalidCodePoint);
  } else if ((byte1 ^ TwoBits) < 2)   // LCOV_EXCL_LINE: gcov-11 bug
    return err(Utf8Result::Overlong); // overlong 2 byte encoding
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
