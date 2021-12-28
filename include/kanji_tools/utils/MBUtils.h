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

// 'toBinary' and 'toHex' are helper functions to print binary or hex versions of 'x' (must be
// integer type). 'minSize' of '-1' (the default) causes leading zeroes to be added to make strings
// the same length for a given type, i.e., if 'x' is char then toHex returns a string of length 2
// and toBinary returns a string of length 8. 'minSize' is ignored if it's less than 'result' size.

inline void addLeadingZeroes(std::string& result, int minSize) {
  if (result.length() < minSize)
    result = std::string(minSize - result.length(), '0') + result;
  else if (result.empty())
    result = "0";
}

template<typename T> inline std::string toBinary(T x, int minSize = -1) {
  std::string result;
  for (; x > 0; x >>= 1)
    result.insert(result.begin(), '0' + x % 2);
  addLeadingZeroes(result, minSize == -1 ? sizeof(T) * 8 : minSize);
  return result;
}

template<typename T> inline std::string toHex(T x, bool caps = false, bool squareBrackets = false, int minSize = -1) {
  std::string result;
  for (; x > 0; x >>= 4) {
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : (caps ? 'A' : 'a') + i - 10));
  }
  addLeadingZeroes(result, minSize == -1 ? sizeof(T) * 2 : minSize);
  if (squareBrackets) result = '[' + result + ']';
  return result;
}

// provide specializations for 'char' that cast to 'unsigned char' (which is probably what is expected)

template<> inline std::string toBinary(char x, int minSize) { return toBinary(static_cast<unsigned char>(x), minSize); }
template<> inline std::string toHex(char x, bool caps, bool squareBrackets, int minSize) {
  return toHex(static_cast<unsigned char>(x), caps, squareBrackets, minSize);
}

// 'toUnicode' converts a 'wchar_t' into a Unicode code point (so hex with caps and minSize of 4)
inline std::string toUnicode(wchar_t s, bool squareBrackets = false) { return toHex(s, true, squareBrackets, 4); }

// 'toUnicode' converts a UTF-8 string into space-separated Unicode code points. Note: setting
// 'squareBrackets' to true puts brackets around the whole string instead of each entry.
inline std::string toUnicode(const std::string& s, bool squareBrackets = false) {
  std::string result;
  for (auto i : fromUtf8(s)) {
    if (!result.empty()) result += ' ';
    result += toUnicode(i);
  }
  if (squareBrackets) result = '[' + result + ']';
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
