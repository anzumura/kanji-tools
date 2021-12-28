#ifndef KANJI_TOOLS_UTILS_MBUTILS_H
#define KANJI_TOOLS_UTILS_MBUTILS_H

#include <codecvt> // for codecvt_utf8
#include <locale>  // for wstring_convert
#include <string>

namespace kanji_tools {

// Helper functions to convert between 'utf8' strings and 'char32_t' wstrings
inline std::wstring fromUtf8(const std::string& s) {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return conv.from_bytes(s);
}

inline std::string toUtf8(char32_t c) {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return conv.to_bytes(c);
}

inline std::string toUtf8(const std::wstring& s) {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return conv.to_bytes(s);
}

// 'toBinary' and 'toHex' are helper functions to print binary or hex versions of 'x' (must be an integer
// type). 'padToSize' will cause leading zeroes to be added to make strings the same length for a given
// type, i.e., if 'x' is char then toHex with padToSize 'true' will always return a string of length 2.

template<typename T> inline std::string toBinary(T x, bool padToSize = true) {
  std::string result;
  for (; x > 0; x >>= 1)
    result.insert(result.begin(), '0' + x % 2);
  if (result.length() < sizeof(T) * 8) {
    if (padToSize)
      result = std::string(sizeof(T) * 8 - result.length(), '0') + result;
    else if (result.empty())
      result = "0";
  }
  return result;
}

template<typename T>
inline std::string toHex(T x, bool caps = false, bool squareBrackets = false, bool padToSize = true) {
  std::string result;
  for (; x > 0; x >>= 4) {
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : (caps ? 'A' : 'a') + i - 10));
  }
  if (result.length() < sizeof(T) * 2) {
    if (padToSize)
      result = std::string(sizeof(T) * 2 - result.length(), '0') + result;
    else if (result.empty())
      result = "0";
  }
  if (squareBrackets) result = '[' + result + ']';
  return result;
}

// provide specializations for 'char' that cast to 'unsigned char' (which is probably what is expected)
template<> inline std::string toBinary(char x, bool padToSize) {
  return toBinary(static_cast<unsigned char>(x), padToSize);
}
template<> inline std::string toHex(char x, bool caps, bool squareBrackets, bool padToSize) {
  return toHex(static_cast<unsigned char>(x), caps, squareBrackets, padToSize);
}

// 'toUnicode' converts a 'wchar_t' into a Unicode code point (so hex with caps and no initial padding)
inline std::string toUnicode(wchar_t s) { return toHex(s, true, false, false); }

// 'toUnicode' converts a UTF-8 string into space-separated Unicode code points
inline std::string toUnicode(const std::string& s) {
  std::string result;
  for (auto i : fromUtf8(s)) {
    if (!result.empty()) result += ' ';
    result += toUnicode(i);
  }
  return result;
}

// check if a given char or string is not a 'multi-byte char'
inline bool isSingleByteChar(char x) { return x >= 0; }
inline bool isSingleByteChar(wchar_t x) { return x >= 0 && x < 128; }
inline bool isSingleByteChar(char32_t x) { return x >= 0 && x < 128; }
inline bool isSingleByte(const std::string& s, bool checkLengthOne = true) {
  return (checkLengthOne ? s.length() == 1 : s.length() >= 1) && isSingleByteChar(s[0]);
}
inline bool isSingleByte(const std::wstring& s, bool checkLengthOne = true) {
  return (checkLengthOne ? s.length() == 1 : s.length() >= 1) && isSingleByteChar(s[0]);
}
inline bool isAllSingleByte(const std::string& s) {
  for (auto& i : s)
    if (!isSingleByteChar(i)) return false;
  return true;
}
inline bool isAllSingleByte(const std::wstring& s) {
  for (auto& i : s)
    if (!isSingleByteChar(i)) return false;
  return true;
}
inline bool isAnySingleByte(const std::string& s) {
  for (auto& i : s)
    if (isSingleByteChar(i)) return true;
  return false;
}
inline bool isAnySingleByte(const std::wstring& s) {
  for (auto& i : s)
    if (isSingleByteChar(i)) return true;
  return false;
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_MBUTILS_H
