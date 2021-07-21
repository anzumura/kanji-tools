#ifndef KANJI_MBUTILS_H
#define KANJI_MBUTILS_H

#include <array>
#include <codecvt> // for codecvt_utf8
#include <locale>  // for wstring_convert
#include <string>

namespace kanji {

// Helper functions to convert between 'utf8' strings and 'wchar_t' wstrings
inline std::wstring fromUtf8(const std::string& s) {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return conv.from_bytes(s);
}

inline std::string toUtf8(wchar_t c) {
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

template<typename T> inline std::string toHex(T x) {
  std::string result;
  for (; x > 0; x >>= 4) {
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : 'a' + i - 10));
  }
  return result;
}

// provide specializations for 'char' that cast to 'unsigned char' (which is probably what is expected)
template<> inline std::string toBinary(char x) { return toBinary(static_cast<unsigned char>(x)); }
template<> inline std::string toHex(char x) { return toHex(static_cast<unsigned char>(x)); }

// 'UnicodeBlock' is used to hold a unicode block range which is used in the below 'is'
// functions (isKanji, isHiragana, etc.).
class UnicodeBlock {
public:
  constexpr UnicodeBlock(wchar_t s, wchar_t e) : start(s), end(e) {
    assert(start % 16 == 0); // Unicode blocks must start with a value having MOD 16 of zero
    assert(end % 16 == 15);  // Unicode blocks must end with a value having MOD 16 of 15 (hex f)
  }
  const wchar_t start;
  const wchar_t end;
  // 'range' returns the number of code points in the block (inclusive of start and end)
  size_t range() const { return end - start + 1; }
  // 'opterator()' returns true if the given character is in this block
  bool operator()(wchar_t x) const { return x >= start && x <= end; }
  bool operator<(const UnicodeBlock& rhs) const { return start < rhs.start; }
  bool operator==(const UnicodeBlock& rhs) const { return start == rhs.start && end == rhs.end; }
};

constexpr std::array HiraganaBlocks = {UnicodeBlock{0x3040, 0x309f}};
// Second block is 'Katakana Phonetic Extensions' which contains small letters (for Ainu) like ㇱ
constexpr std::array KatakanaBlocks = {UnicodeBlock{0x30a0, 0x30ff}, UnicodeBlock{0x31f0, 0x31ff}};
// There are ~20K common kanji and several more CJK extension blocks. For now just include
// 'Extension A' (~6K kanji) and 'Radicals Supplement' (added in 1999 - version 3.0) in
// 'RareKanjiBlocks' and maybe add more extensions later if needed - the rest are outside
// the BMP (Basic Multilingual Plane). Note: the test/sample-data files don't contain any
// 'rare' kanji so far, but they do contain more than 2600 unique kanji (out of almost
// 100K total kanji).
constexpr std::array CommonKanjiBlocks = {UnicodeBlock{0x4e00, 0x9fff}};
constexpr std::array RareKanjiBlocks = {UnicodeBlock{0x2e80, 0x2eff}, UnicodeBlock{0x3400, 0x4dbf}};
constexpr std::array PunctuationBlocks = {
  UnicodeBlock{0x2000, 0x206f}, // General MB Punctuation: —, ‥, ”, “
  UnicodeBlock{0x3000, 0x303f}, // Wide Punctuation: 、, 。, （
  UnicodeBlock{0xfff0, 0xffff}  // Specials (like Object Replacement, etc.)
};
// There are a lot more symbol and letter blocks, but they haven't come up in sample files so far
constexpr std::array SymbolBlocks = {
  UnicodeBlock{0x2100, 0x214f}, // Letterlike Symbols: ℃
  UnicodeBlock{0x2190, 0x21ff}, // Arrows: →
  UnicodeBlock{0x2200, 0x22ff}, // Math Symbols: ∀
  UnicodeBlock{0x2500, 0x257f}, // Box Drawing: ─
  UnicodeBlock{0x25A0, 0x25ff}, // Geometric Shapes: ○
  UnicodeBlock{0x2600, 0x26ff}, // Misc Symbols: ☆
  UnicodeBlock{0x2ff0, 0x2fff}, // CJK Ideographic Description Characters: ⿱
  UnicodeBlock{0x3190, 0x319f}, // Kanbun (Ideographic Annotations): ㆑
  UnicodeBlock{0x31c0, 0x31ef}  // CJK Strokes: ㇁
};
constexpr std::array LetterBlocks = {
  UnicodeBlock{0x0080, 0x00ff}, // Latin Supplement: ·, ×
  UnicodeBlock{0x0100, 0x017f}, // Latin Extension A
  UnicodeBlock{0x0180, 0x024f}, // Latin Extension B
  UnicodeBlock{0x2150, 0x218f}, // Number Forms: Roman Numerals, etc.
  UnicodeBlock{0x2460, 0x24ff}, // Enclosed Alphanumeic: ⑦
  UnicodeBlock{0x2c60, 0x2c7f}, // Latin Extension C
  UnicodeBlock{0xff00, 0xffef}  // Wide Letters: full width Roman letters and half-width Katakana
};

template<typename T> inline bool inRange(wchar_t c, const T& t) {
  for (auto& i : t)
    if (i(c)) return true;
  return false;
}

template<typename T, typename... Ts> inline bool inRange(wchar_t c, const T& t, Ts... args) {
  for (auto& i : t)
    if (i(c)) return true;
  return inRange(c, args...);
}

// Return true if the first 'MB character' is in the given blocks, empty string will return false and
// a string longer than one 'MB characer' will also return false unless 'checkLengthOne' is false.
template<typename... T> inline bool inWCharRange(const std::string& s, bool checkLengthOne, T... t) {
  if (s.length() > 1 && (!checkLengthOne || s.length() < 5)) {
    auto w = fromUtf8(s);
    if (checkLengthOne ? w.length() == 1 : w.length() >= 1) return inRange(w[0], t...);
  }
  return false;
}

// Return true if all characers are in the given blocks, empty string will also return true
template<typename... T> inline bool inWCharRange(const std::string& s, T... t) {
  auto w = fromUtf8(s);
  for (auto i : w)
    if (!inRange(i, t...)) return false;
  return true;
}

// functions for classifying 'recognized' utf-8 encoded characters: 's' should contain one MB
// character (so 2-4 bytes) by default, but 'checkLengthOne' can be set to 'false' to check
// just the first 'MB characer' in the string. There are alls 'isAll' functions that return
// 'true' only if all the characers in the string are the desired type.

// kana
inline bool isHiragana(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, HiraganaBlocks);
}
inline bool isAllHiragana(const std::string& s) { return inWCharRange(s, HiraganaBlocks); }
inline bool isKatakana(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, KatakanaBlocks);
}
inline bool isAllKatakana(const std::string& s) { return inWCharRange(s, KatakanaBlocks); }
inline bool isKana(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, HiraganaBlocks, KatakanaBlocks);
}
inline bool isAllKana(const std::string& s) { return inWCharRange(s, HiraganaBlocks, KatakanaBlocks); }
// kanji
inline bool isCommonKanji(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, CommonKanjiBlocks);
}
inline bool isAllCommonKanji(const std::string& s) { return inWCharRange(s, CommonKanjiBlocks); }
inline bool isRareKanji(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, RareKanjiBlocks);
}
inline bool isAllRareKanji(const std::string& s) { return inWCharRange(s, RareKanjiBlocks); }
inline bool isKanji(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, CommonKanjiBlocks, RareKanjiBlocks);
}
inline bool isAllKanji(const std::string& s) { return inWCharRange(s, CommonKanjiBlocks, RareKanjiBlocks); }
// 'isMBPunctuation' tests for wide space by default, but also allows not including spaces.
inline bool isMBPunctuation(const std::string& s, bool includeSpace = true, bool checkLengthOne = true) {
  return s.starts_with("　") ? (includeSpace && (s.length() < 4 || !checkLengthOne))
                             : inWCharRange(s, checkLengthOne, PunctuationBlocks);
}
inline bool isAllMBPunctuation(const std::string& s) { return inWCharRange(s, PunctuationBlocks); }
inline bool isMBSymbol(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, SymbolBlocks);
}
inline bool isAllMBSymbol(const std::string& s) { return inWCharRange(s, SymbolBlocks); }
inline bool isMBLetter(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, LetterBlocks);
}
inline bool isAllMBLetter(const std::string& s) { return inWCharRange(s, LetterBlocks); }
// 'isRecognizedMB' returns true if 's' is in any UnicodeBlock defined in this header file (including wide space)
inline bool isRecognizedMB(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks, KatakanaBlocks,
                      PunctuationBlocks, SymbolBlocks, LetterBlocks);
}
inline bool isAllRecognizedMB(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks, KatakanaBlocks, PunctuationBlocks,
                      SymbolBlocks, LetterBlocks);
}
// check if a given char or string is not a 'multi-byte char'
inline bool isSingleByteChar(char x) { return x >= 0; }
inline bool isSingleByteChar(wchar_t x) { return x >= 0 && x < 128; }
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

// KanjiRange includes both the 'rare block' and the 'common block' defined above
constexpr wchar_t KanjiRange[] = L"\u2e80-\u2eff\u3400-\u4dbf\u4e00-\u9fff";
constexpr wchar_t HiraganaRange[] = L"\u3040-\u309f";

} // namespace kanji

#endif // KANJI_MBUTILS_H