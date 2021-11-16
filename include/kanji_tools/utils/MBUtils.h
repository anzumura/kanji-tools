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

// Helper functions to print binary or hex versions of an unsigned char
template<typename T> inline std::string toBinary(T x) {
  std::string result;
  for (; x > 0; x >>= 1)
    result.insert(result.begin(), '0' + x % 2);
  return result;
}

template<typename T> inline std::string toHex(T x, bool caps = false, bool squareBrackets = false) {
  std::string result;
  for (; x > 0; x >>= 4) {
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : (caps ? 'A' : 'a') + i - 10));
  }
  if (squareBrackets) result = '[' + result + ']';
  return result;
}

// provide specializations for 'char' that cast to 'unsigned char' (which is probably what is expected)
template<> inline std::string toBinary(char x) { return toBinary(static_cast<unsigned char>(x)); }
template<> inline std::string toHex(char x, bool caps, bool squareBrackets) {
  return toHex(static_cast<unsigned char>(x), caps, squareBrackets);
}

// 'toUnicode' converts a UTF-8 string into space-separated Unicode code point values
inline std::string toUnicode(const std::string& s, bool caps = true) {
  std::string result;
  auto w = fromUtf8(s);
  for (auto i : w) {
    if (!result.empty()) result += ' ';
    result += toHex(i, caps);
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
