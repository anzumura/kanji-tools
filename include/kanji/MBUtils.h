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

template<typename T> inline std::string toHex(T x, bool caps = false) {
  std::string result;
  for (; x > 0; x >>= 4) {
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : (caps ? 'A' : 'a') + i - 10));
  }
  return result;
}

// provide specializations for 'char' that cast to 'unsigned char' (which is probably what is expected)
template<> inline std::string toBinary(char x) { return toBinary(static_cast<unsigned char>(x)); }
template<> inline std::string toHex(char x, bool caps) { return toHex(static_cast<unsigned char>(x), caps); }

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

// 'UnicodeBlock' is used to hold a unicode block range which is used in the below 'is'
// functions (isKanji, isHiragana, etc.). 'wide' should be set to true if the block has
// all wide chars (so display on a terminal is 2 columns instead of 1). This is
class UnicodeBlock {
public:
  constexpr UnicodeBlock(wchar_t s, wchar_t e) : start(s), end(e) {}
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
constexpr wchar_t KatakanaRange[] = L"\u30a0-\u30ff\u31f0-\u31ff";
constexpr wchar_t KanaRange[] = L"\u3040-\u30ff\u31f0-\u31ff";

// --- begin generated code from 'parseEastAsiaWide.sh' ---
constexpr std::array WideBlocks = {
  UnicodeBlock{0x1100, 0x115F}, UnicodeBlock{0x231A, 0x231B}, UnicodeBlock{0x2329, 0x232A},
  UnicodeBlock{0x23E9, 0x23EC}, UnicodeBlock{0x23F0, 0x23F0}, UnicodeBlock{0x23F3, 0x23F3},
  UnicodeBlock{0x25FD, 0x25FE}, UnicodeBlock{0x2614, 0x2615}, UnicodeBlock{0x2648, 0x2653},
  UnicodeBlock{0x267F, 0x267F}, UnicodeBlock{0x2693, 0x2693}, UnicodeBlock{0x26A1, 0x26A1},
  UnicodeBlock{0x26AA, 0x26AB}, UnicodeBlock{0x26BD, 0x26BE}, UnicodeBlock{0x26C4, 0x26C5},
  UnicodeBlock{0x26CE, 0x26CE}, UnicodeBlock{0x26D4, 0x26D4}, UnicodeBlock{0x26EA, 0x26EA},
  UnicodeBlock{0x26F2, 0x26F3}, UnicodeBlock{0x26F5, 0x26F5}, UnicodeBlock{0x26FA, 0x26FA},
  UnicodeBlock{0x26FD, 0x26FD}, UnicodeBlock{0x2705, 0x2705}, UnicodeBlock{0x270A, 0x270B},
  UnicodeBlock{0x2728, 0x2728}, UnicodeBlock{0x274C, 0x274C}, UnicodeBlock{0x274E, 0x274E},
  UnicodeBlock{0x2753, 0x2755}, UnicodeBlock{0x2757, 0x2757}, UnicodeBlock{0x2795, 0x2797},
  UnicodeBlock{0x27B0, 0x27B0}, UnicodeBlock{0x27BF, 0x27BF}, UnicodeBlock{0x2B1B, 0x2B1C},
  UnicodeBlock{0x2B50, 0x2B50}, UnicodeBlock{0x2B55, 0x2B55}, UnicodeBlock{0x2E80, 0x2E99},
  UnicodeBlock{0x2E9B, 0x2EF3}, UnicodeBlock{0x2F00, 0x2FD5}, UnicodeBlock{0x2FF0, 0x2FFB},
  UnicodeBlock{0x3000, 0x303E}, UnicodeBlock{0x3041, 0x3096}, UnicodeBlock{0x3099, 0x30FF},
  UnicodeBlock{0x3105, 0x312F}, UnicodeBlock{0x3131, 0x318E}, UnicodeBlock{0x3190, 0x31E3},
  UnicodeBlock{0x31F0, 0x321E}, UnicodeBlock{0x3220, 0x3247}, UnicodeBlock{0x3250, 0x4DBF},
  UnicodeBlock{0x4E00, 0xA48C}, UnicodeBlock{0xA490, 0xA4C6}, UnicodeBlock{0xA960, 0xA97C},
  UnicodeBlock{0xAC00, 0xD7A3}, UnicodeBlock{0xF900, 0xFAFF}, UnicodeBlock{0xFE10, 0xFE19},
  UnicodeBlock{0xFE30, 0xFE52}, UnicodeBlock{0xFE54, 0xFE66}, UnicodeBlock{0xFE68, 0xFE6B},
  UnicodeBlock{0xFF01, 0xFF60}, UnicodeBlock{0xFFE0, 0xFFE6}};
// --- end generated code from 'parseEastAsiaWide.sh' ---

// 'displayLength' returns the length of 's' in terms of how many columns would be required for
// display on a terminal, i.e, 1 column for a normal sized character and 2 for a wide character.
inline size_t displayLength(const std::string& s) {
  auto w = fromUtf8(s);
  size_t result = w.length();
  for (auto i : w)
    if (inRange(i, WideBlocks)) ++result;
  return result;
}

} // namespace kanji

#endif // KANJI_MBUTILS_H
