#include <kanji_tools/utils/MBUtils.h>

#ifdef USE_CODECVT_FOR_UTF_8
#include <codecvt> // for codecvt_utf8
#include <locale>  // for wstring_convert
#endif

namespace kanji_tools {

namespace {

#ifdef USE_CODECVT_FOR_UTF_8
inline auto utf8Converter() {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return &conv;
}
#else
constexpr wchar_t Six = 0b11'11'11, FirstFive = 0b1'11'11 << 6, FirstFour = 0b11'11 << 12, FirstThree = 0b1'11 << 18;
constexpr wchar_t SecondSix = Six << 6, ThirdSix = Six << 12;

inline void convertToUtf8(wchar_t c, std::string& s) {
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
  } else if (c <= MaxUnicode) {
    s += static_cast<char>(((FirstThree & c) >> 18) + FourBits);
    s += static_cast<char>(((ThirdSix & c) >> 12) + Bit1);
    s += static_cast<char>(((SecondSix & c) >> 6) + Bit1);
    s += static_cast<char>((Six & c) + Bit1);
  } else
    s += ReplacementCharacter;
}
#endif

} // namespace

std::wstring fromUtf8(const char* s) {
#ifdef USE_CODECVT_FOR_UTF_8
  return utf8Converter()->from_bytes(s);
#else
  std::wstring result;
  if (!s) return result;
  auto* u = reinterpret_cast<const unsigned char*>(s);
  do {
    if (*u <= 0x7fU) // one byte case
      result += static_cast<wchar_t>(*u++);
    else if ((*u & TwoBits) == Bit1 || (*u & FiveBits) == FiveBits) {
      result += ErrorReplacement; // first byte was '10' or started with more than four '1's
      ++u;
    } else {
      const unsigned byte1 = *u;
      if ((*++u & TwoBits) != Bit1)
        result += ErrorReplacement; // second byte didn't start with '10'
      else {
        const unsigned byte2 = *u ^ Bit1; // last 6 bits of the second byte
        if (byte1 & Bit3) {
          if ((*++u & TwoBits) != Bit1)
            result += ErrorReplacement; // third byte didn't start with '10'
          else if (byte1 & Bit4) {
            const unsigned byte3 = *u ^ Bit1; // last 6 bits of the third byte
            if ((*++u & TwoBits) != Bit1)
              result += ErrorReplacement; // fourth byte didn't start with '10'
            else {
              // four byte case - check for overlong and max unicode
              const wchar_t c = ((byte1 ^ FourBits) << 18) + (byte2 << 12) + (byte3 << 6) + (*u ^ Bit1);
              result += c > 0xffff && c <= MaxUnicode ? c : ErrorReplacement;
              ++u;
            }
          } else {
            // three byte case - check for overlong and surrogate range
            const wchar_t c = ((byte1 ^ ThreeBits) << 12) + (byte2 << 6) + (*u ^ Bit1);
            result += c > 0x7ff && (c < MinSurrogate || c > MaxSurrogate) ? c : ErrorReplacement;
            ++u;
          }
        } else {
          // two byte case - check for overlong
          result += (byte1 ^ TwoBits) > 1 ? ((byte1 ^ TwoBits) << 6) + byte2 : ErrorReplacement;
          ++u;
        }
      }
    }
  } while (*u);
  return result;
#endif
}

std::string toUtf8(wchar_t c) {
#ifdef USE_CODECVT_FOR_UTF_8
  return utf8Converter()->to_bytes(c);
#else
  std::string result;
  convertToUtf8(c, result);
  return result;
#endif
}

std::string toUtf8(const std::wstring& s) {
#ifdef USE_CODECVT_FOR_UTF_8
  return utf8Converter()->to_bytes(s);
#else
  std::string result;
  result.reserve(s.length()); // result will be bigger than s if there are any multibyte chars
  for (auto c : s) convertToUtf8(c, result);
  return result;
#endif
}

} // namespace kanji_tools
