#ifndef KANJI_TOOLS_UTILS_UNICODE_BLOCK_H
#define KANJI_TOOLS_UTILS_UNICODE_BLOCK_H

#include <kanji_tools/utils/MBUtils.h>

#include <array>

namespace kanji_tools {

// 'UnicodeBlock' is used to hold a unicode block range which is used in the below 'is'
// functions (isKanji, isHiragana, etc.). 'wide' should be set to true if the block has
// all wide chars (so display on a terminal is 2 columns instead of 1). This is
class UnicodeBlock {
public:
  constexpr UnicodeBlock(char32_t s, char32_t e) noexcept : start(s), end(e) {}

  // Official Unicode blocks start on a value having mod 16 = 0 (so ending in hex '0') and
  // end on a value having mod 16 = 15 (so ending in hex 'f'), but some of the 'WideBlocks'
  // used for determining if a character is narrow or wide display can be a single entry.
  constexpr UnicodeBlock(char32_t s) noexcept : start(s), end(s) {}

  // 'range' returns the number of code points in the block (inclusive of start and end)
  [[nodiscard]] constexpr auto range() const noexcept { return end - start + 1; }

  // 'opterator()' returns true if the given character is in this block
  [[nodiscard]] constexpr auto operator()(char32_t x) const noexcept { return x >= start && x <= end; }

  [[nodiscard]] constexpr auto operator<(const UnicodeBlock& rhs) const noexcept { return start < rhs.start; }
  [[nodiscard]] constexpr auto operator==(const UnicodeBlock& rhs) const noexcept {
    return start == rhs.start && end == rhs.end;
  }

  // 'wStart' and 'wEnd' are needed for wregex (may remove later)
  [[nodiscard]] constexpr wchar_t wStart() const noexcept { return static_cast<wchar_t>(start); }
  [[nodiscard]] constexpr wchar_t wEnd() const noexcept { return static_cast<wchar_t>(end); }

  const char32_t start;
  const char32_t end;
};

constexpr std::array HiraganaBlocks = {UnicodeBlock{0x3040, 0x309f}};

// Second block is 'Katakana Phonetic Extensions' which contains small letters (for Ainu) like ㇱ
constexpr std::array KatakanaBlocks = {UnicodeBlock{0x30a0, 0x30ff}, UnicodeBlock{0x31f0, 0x31ff}};

// Almost all 'common' Japanese Kanji are in the original CJK Unified block. Extension A has one 'Kentei'
// and about 1000 'Ucd' Kanji. Extension B has an updated Jouyou Kanji '𠮟' (U+20B9F) which used to be
// '叱' (U+53F1)). The Compatibility block contains many 'single grapheme' versions of old/variant Japanese
// Kanji that used to require two graphemes, i.e., a base character followed by a variation selector.
constexpr std::array CommonKanjiBlocks = {
  UnicodeBlock{0x3400, 0x4dbf},  // CJK Extension A (ver 3.0 Sep 1999, ~6K kanji): 㵎
  UnicodeBlock{0x4e00, 0x9fff},  // CJK Unified Ideographs Kanji (ver 1.1 Jun 1992, ~20K)
  UnicodeBlock(0xf900, 0xfaff),  // CJK Compatibility Ideographs (ver 3.2 Mar 2002, 512): 渚, 猪
  UnicodeBlock(0x20000, 0x2a6df) // CJK Extension B (ver 3.1 March 2001, ~42K): 𠮟
};

// Note: Extensions C, D, E and F are contiguous so combine into one block (more efficient for isKanji)
// functions and wregex). Here are the actual block ranges:
// - U+2A700 to U+2B73F : CJK Extension C (ver 5.2 Oct 2009, ~4K kanji)
// - U+2B740 to U+2B81F : CJK Extension D (ver 6.0 Oct 2010, 222 kanji)
// - U+2B820 to U+2CEAF : CJK Extension E (ver 8.0 Jun 2015, ~6K kanji)
// - U+2CEB0 to U+2EBEF : CJK Extension F (ver 10.0 Jun 2016, ~7K kanji)
constexpr std::array RareKanjiBlocks = {
  UnicodeBlock{0x2e80, 0x2eff},   // CJK Radicals Supplement (ver 3.0 Sep 1999, 128)
  UnicodeBlock{0x2a700, 0x2ebef}, // CJK Extension C, D, E and F (~17K kanji)
  UnicodeBlock{0x2f800, 0x2fa1f}, // CJK Compatibility Ideographs Supplement (ver 3.1 Mar 2001, ~6K kanji)
  UnicodeBlock{0x30000, 0x3134f}  // CJK Extension G (ver 13.0 Mar 2020, ~5K kanji)
};

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

// Skip codes in this range when reading in Kanji. They can inform see the following link for more info:
// http://unicode.org/reports/tr28/tr28-3.html#13_7_variation_selectors
constexpr std::array NonSpacingBlocks = {
  UnicodeBlock(0xfe00, 0xfe0f) // Variation Selection (comes after some Kanji in jinmei file)
};

// 'inRange' checks if 'c' is contained in any of the UnicodeBocks in the array 't'. The blocks in 't'
// are assumed to be in order (order is checked by automated tests for all the arrays defined above).
template<size_t N> [[nodiscard]] constexpr bool inRange(char32_t c, const std::array<UnicodeBlock, N>& t) noexcept {
  for (auto& i : t) {
    if (c < i.start) break;
    if (i(c)) return true;
  }
  return false;
}

// 'inRange' with more than one 't' (block array) checks each array so there's no requirement for the
// arrays to be specified in a particular order (which wouldn't work anyway for overlapping ranges).
template<size_t N, typename... Ts>
[[nodiscard]] constexpr bool inRange(char32_t c, const std::array<UnicodeBlock, N>& t, Ts... args) noexcept {
  return inRange(c, t) || inRange(c, args...);
}

inline constexpr auto CombiningMarkVoiced = U'\x3099', CombiningMarkSemiVoiced = U'\x309a';

[[nodiscard]] constexpr auto isNonSpacing(char32_t c) noexcept {
  return inRange(c, NonSpacingBlocks) || c == CombiningMarkVoiced || c == CombiningMarkSemiVoiced;
}

// Return true if the first 'MB character' is in the given blocks, empty string will return false and
// a string longer than one 'MB characer' will also return false unless 'checkLengthOne' is false.
template<typename... T> [[nodiscard]] inline auto inWCharRange(const std::string& s, bool checkLengthOne, T... t) {
  if (s.length() > 1 && (!checkLengthOne || s.length() < 9))
    if (const auto w = fromUtf8(s);
        checkLengthOne ? w.length() == 1 || w.length() == 2 && isNonSpacing(w[1]) : w.length() >= 1)
      return inRange(w[0], t...);
  return false;
}

// Return true if all characers are in the given blocks, empty string will also return true
template<typename... T> [[nodiscard]] inline auto inWCharRange(const std::string& s, T... t) {
  // an 'inRange' character can be followed by a 'variation selector'
  for (auto allowNonSpacing = false; const auto i : fromUtf8(s))
    if (allowNonSpacing && isNonSpacing(i))
      allowNonSpacing = false;
    else if (inRange(i, t...))
      allowNonSpacing = true;
    else
      return false;
  return true;
}

// functions for classifying 'recognized' utf-8 encoded characters: 's' should contain one MB
// character (so 2-4 bytes) by default, but 'checkLengthOne' can be set to 'false' to check
// just the first 'MB characer' in the string. There are alls 'isAll' functions that return
// 'true' only if all the characers in the string are the desired type.

// kana
[[nodiscard]] inline auto isHiragana(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, HiraganaBlocks);
}
[[nodiscard]] inline auto isAllHiragana(const std::string& s) { return inWCharRange(s, HiraganaBlocks); }
[[nodiscard]] inline auto isKatakana(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, KatakanaBlocks);
}
[[nodiscard]] inline auto isAllKatakana(const std::string& s) { return inWCharRange(s, KatakanaBlocks); }
[[nodiscard]] inline auto isKana(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, HiraganaBlocks, KatakanaBlocks);
}
[[nodiscard]] inline auto isAllKana(const std::string& s) { return inWCharRange(s, HiraganaBlocks, KatakanaBlocks); }

// kanji
[[nodiscard]] inline auto isCommonKanji(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, CommonKanjiBlocks);
}
[[nodiscard]] inline auto isAllCommonKanji(const std::string& s) { return inWCharRange(s, CommonKanjiBlocks); }
[[nodiscard]] inline auto isRareKanji(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, RareKanjiBlocks);
}
[[nodiscard]] inline auto isAllRareKanji(const std::string& s) { return inWCharRange(s, RareKanjiBlocks); }
[[nodiscard]] inline auto isKanji(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, CommonKanjiBlocks, RareKanjiBlocks);
}
[[nodiscard]] inline auto isAllKanji(const std::string& s) {
  return inWCharRange(s, CommonKanjiBlocks, RareKanjiBlocks);
}

// 'isMBPunctuation' tests for wide space by default, but also allows not including spaces.
[[nodiscard]] inline auto isMBPunctuation(const std::string& s, bool includeSpace = true, bool checkLengthOne = true) {
  return s.starts_with("　") ? (includeSpace && (s.length() < 4 || !checkLengthOne))
                             : inWCharRange(s, checkLengthOne, PunctuationBlocks);
}
[[nodiscard]] inline auto isAllMBPunctuation(const std::string& s) { return inWCharRange(s, PunctuationBlocks); }
[[nodiscard]] inline auto isMBSymbol(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, SymbolBlocks);
}
[[nodiscard]] inline auto isAllMBSymbol(const std::string& s) { return inWCharRange(s, SymbolBlocks); }
[[nodiscard]] inline auto isMBLetter(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, LetterBlocks);
}
[[nodiscard]] inline auto isAllMBLetter(const std::string& s) { return inWCharRange(s, LetterBlocks); }

// 'isRecognizedCharacter' returns true if 's' is in any UnicodeBlock defined in this header file (including wide space)
[[nodiscard]] inline auto isRecognizedCharacter(const std::string& s, bool checkLengthOne = true) {
  return inWCharRange(s, checkLengthOne, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks, KatakanaBlocks,
                      PunctuationBlocks, SymbolBlocks, LetterBlocks);
}
[[nodiscard]] inline auto isAllRecognizedCharacters(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks, KatakanaBlocks, PunctuationBlocks,
                      SymbolBlocks, LetterBlocks);
}

// KanjiRange is for wregex and includes the common and rare kanji as well as variation selectors.
constexpr wchar_t WideDash = L'-';

// clang-format off
constexpr wchar_t KanjiRange[] = {
  CommonKanjiBlocks[0].wStart(), WideDash, CommonKanjiBlocks[0].wEnd(), // CJK Extension A
  CommonKanjiBlocks[1].wStart(), WideDash, CommonKanjiBlocks[1].wEnd(), // CJK Unified Ideographs Kanji
  CommonKanjiBlocks[2].wStart(), WideDash, CommonKanjiBlocks[2].wEnd(), // CJK Compatibility Ideographs
  CommonKanjiBlocks[3].wStart(), WideDash, CommonKanjiBlocks[3].wEnd(), // CJK Extension B
  NonSpacingBlocks[0].wStart(), WideDash, NonSpacingBlocks[0].wEnd(),   // Variation Selectors
  RareKanjiBlocks[0].wStart(), WideDash, RareKanjiBlocks[0].wEnd(),     // CJK Radicals Supplement
  RareKanjiBlocks[1].wStart(), WideDash, RareKanjiBlocks[1].wEnd(),     // CJK Extension C, D, E and F
  RareKanjiBlocks[2].wStart(), WideDash, RareKanjiBlocks[2].wEnd(),     // CJK Compatibility Ideographs Supplement
  RareKanjiBlocks[3].wStart(), WideDash, RareKanjiBlocks[3].wEnd(),     // CJK Extension G
  L'\0' // null
};
// clang-format on

constexpr wchar_t HiraganaRange[] = L"\u3040-\u309f";
constexpr wchar_t KatakanaRange[] = L"\u30a0-\u30ff\u31f0-\u31ff";
constexpr wchar_t KanaRange[] = L"\u3040-\u30ff\u31f0-\u31ff";

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_UNICODE_BLOCK_H
