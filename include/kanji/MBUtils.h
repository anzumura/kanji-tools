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
inline std::string toBinary(unsigned char x) {
  std::string result;
  for (; x > 0; x >>= 1)
    result.insert(result.begin(), '0' + x % 2);
  return result;
}

inline std::string toHex(unsigned char x) {
  std::string result;
  for (; x > 0; x >>= 4) {
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : 'a' + i - 10));
  }
  return result;
}

// 'UnicodeBlock' is used to hold a unicode block range which is used in the below 'is'
// functions (isKanji, isHiragana, etc.).
struct UnicodeBlock {
  const wchar_t start;
  const wchar_t end;
  // 'range' returns the number of code points in the block (inclusive of start and end)
  size_t range() const { return end - start + 1; }
  // 'opterator()' returns true if the given character is in this block
  bool operator()(wchar_t x) const { return x >= start && x <= end; }
  bool operator<(const UnicodeBlock& rhs) const { return start < rhs.start; }
  bool operator==(const UnicodeBlock& rhs) const { return start == rhs.start && end == rhs.end; }
};

constexpr std::array HiraganaBlocks = {UnicodeBlock{L'\u3040', L'\u309f'}};
// Second block is 'Katakana Extended' and contains things like ㇱ (small letter)
constexpr std::array KatakanaBlocks = {UnicodeBlock{L'\u30a0', L'\u30ff'}, UnicodeBlock{L'\u31f0', L'\u31ff'}};
// There are ~20K common kanji in one block and several more CJK extension blocks. For now just
// include 'Extension A' (which has ~6K kanji) in 'RareKanjiBlocks' and maybe add more extensions
// later if needed. Note: the test/sample-data files don't contain any 'rare' kanji so far, but
// they do contain more the 2600 unique kanji (out of over 75K total kanji).
constexpr std::array CommonKanjiBlocks = {UnicodeBlock{L'\u4e00', L'\u9ffc'}};
constexpr std::array RareKanjiBlocks = {UnicodeBlock{L'\u3400', L'\u4dbf'}};
constexpr std::array PunctuationBlocks = {
  UnicodeBlock{L'\u2000', L'\u206f'}, // General MB Punctuation: —, ‥, ”, “
  UnicodeBlock{L'\u3000', L'\u303f'}, // Wide Punctuation: 、, 。, （
  UnicodeBlock{L'\ufff0', L'\uffff'}  // Specials (like Object Replacement, etc.)
};
constexpr std::array SymbolBlocks = {
  UnicodeBlock{L'\u2100', L'\u2145'}, // Letterlike Symbols: ℃
  UnicodeBlock{L'\u2190', L'\u21ff'}, // Arrows: →
  UnicodeBlock{L'\u2200', L'\u22ff'}, // Math Symbols: ∀
  UnicodeBlock{L'\u2500', L'\u257f'}, // Box Drawing: ─
  UnicodeBlock{L'\u25A0', L'\u25ff'}, // Geometric Shapes: ○
  UnicodeBlock{L'\u2600', L'\u26ff'}  // Misc Symbols: ☆
};
constexpr std::array LetterBlocks = {
  UnicodeBlock{L'\u0080', L'\u00ff'}, // Latin Supplement: ·, ×
  UnicodeBlock{L'\u0100', L'\u017f'}, // Latin Extended
  UnicodeBlock{L'\u2150', L'\u2185'}, // Number Forms: Roman Numerals, etc.
  UnicodeBlock{L'\u2460', L'\u24ff'}, // Enclosed Alphanumeic: ⑦
  UnicodeBlock{L'\uff00', L'\uffef'}  // Wide Letters: full width Roman letters and half-width Katakana
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

template<typename... T> inline bool inWCharRange(const std::string& s, T... t) {
  if (s.length() > 1 && s.length() < 5) {
    auto w = fromUtf8(s);
    if (w.length() == 1) return inRange(w[0], t...);
  }
  return false;
}

// functions for classifying 'recognized' utf-8 encoded characters: 's' should contain one MB character (so 2-4 bytes)

// kana
inline bool isHiragana(const std::string& s) { return inWCharRange(s, HiraganaBlocks); }
inline bool isKatakana(const std::string& s) { return inWCharRange(s, KatakanaBlocks); }
inline bool isKana(const std::string& s) { return inWCharRange(s, HiraganaBlocks, KatakanaBlocks); }
// kanji
inline bool isCommonKanji(const std::string& s) { return inWCharRange(s, CommonKanjiBlocks); }
inline bool isRareKanji(const std::string& s) { return inWCharRange(s, RareKanjiBlocks); }
inline bool isKanji(const std::string& s) { return inWCharRange(s, CommonKanjiBlocks, RareKanjiBlocks); }
// 'isMBPunctuation' tests for wide space by default, but also allows not including spaces.
inline bool isMBPunctuation(const std::string& s, bool includeSpace = true) {
  return s == "　" ? includeSpace : inWCharRange(s, PunctuationBlocks);
}
inline bool isMBSymbol(const std::string& s) { return inWCharRange(s, SymbolBlocks); }
inline bool isMBLetter(const std::string& s) { return inWCharRange(s, LetterBlocks); }
// 'isRecognizedMB' returns true if 's' is in any UnicodeBlock defined in this header file (including wide space)
inline bool isRecognizedMB(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks, KatakanaBlocks, PunctuationBlocks,
                      SymbolBlocks, LetterBlocks);
}

// KanjiRange includes both the 'rare block' and the 'common block' defined above
constexpr wchar_t KanjiRange[] = L"\u3400-\u4dbf\u4e00-\u9ffc";
constexpr wchar_t HiraganaRange[] = L"\u3040-\u309f";

} // namespace kanji

#endif
