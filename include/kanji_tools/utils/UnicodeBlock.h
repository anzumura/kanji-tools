#ifndef KANJI_TOOLS_UTILS_UNICODE_BLOCK_H
#define KANJI_TOOLS_UTILS_UNICODE_BLOCK_H

#include <kanji_tools/utils/MBUtils.h>

#include <array>

namespace kanji_tools {

// Unicode version and release date
class UnicodeVersion {
public:
  constexpr UnicodeVersion(const char* v, uint8_t m, uint16_t y) noexcept
      : version(v), month(m), year(y) {}

  const char* const version;
  const uint8_t month;
  const uint16_t year;
};

// 'UnicodeBlock' holds a range which is used in the 'is' functions ('isKanji',
// 'isHiragana', etc.). Official blocks have a 'version' and a 'name' (just for
// reference). To keep it simple, 'version' represents the first version the
// block was introduced whereas in reality some characters in the block may be
// added in later versions. For example, the 'Katakana' block was introduced in
// version 1.1, but U+30A0 (゠) was added to the block in version 3.2.
class UnicodeBlock {
public:
  constexpr UnicodeBlock(char32_t s, char32_t e) noexcept
      : UnicodeBlock(s, e, nullptr) {}
  constexpr UnicodeBlock(char32_t s, char32_t e, const UnicodeVersion& v,
                         const char* const n) noexcept
      : UnicodeBlock(s, e, &v, n) {}

  // Official Unicode blocks start on a value having mod 16 = 0 (so ending in
  // hex '0') and end on a value having mod 16 = 15 (so ending in hex 'f'), but
  // some of the 'WideBlocks' used for determining if a character is narrow or
  // wide display can be a single entry.
  constexpr UnicodeBlock(char32_t s) noexcept : UnicodeBlock(s, s) {}

  // return number of code points in the block (inclusive of start and end)
  [[nodiscard]] constexpr auto range() const noexcept {
    return end - start + 1;
  }

  // 'opterator()' returns true if the given character is in this block
  [[nodiscard]] constexpr auto operator()(char32_t x) const noexcept {
    return x >= start && x <= end;
  }

  [[nodiscard]] constexpr auto
  operator<(const UnicodeBlock& rhs) const noexcept {
    return start < rhs.start;
  }
  [[nodiscard]] constexpr auto
  operator==(const UnicodeBlock& rhs) const noexcept {
    return start == rhs.start && end == rhs.end;
  }

  // 'wStart' and 'wEnd' are needed for wregex (may remove later)
  [[nodiscard]] constexpr wchar_t wStart() const noexcept {
    return static_cast<wchar_t>(start);
  }
  [[nodiscard]] constexpr wchar_t wEnd() const noexcept {
    return static_cast<wchar_t>(end);
  }

  const char32_t start;
  const char32_t end;
  const UnicodeVersion* const version;
  const char* const name;
private:
  constexpr UnicodeBlock(char32_t s, char32_t e, const UnicodeVersion* const v,
                         const char* const n = nullptr) noexcept
      : start(s), end(e), version(v), name(n) {}
};

// below are the Unicode versions referenced in this program, for the full list
// see: https://unicode.org/history/publicationdates.html
inline constexpr UnicodeVersion UVer1_0("1.0", 10, 1991),
  UVer1_1("1.1", 6, 1993), UVer2_0("2.0", 7, 1996), UVer3_0("3.0", 9, 1999),
  UVer3_1("3.1", 3, 2001), UVer3_2("3.2", 3, 2002), UVer4_1("4.1", 3, 2005),
  UVer5_0("5.0", 7, 2006), UVer5_2("5.2", 10, 2009), UVer13_0("13.0", 3, 2020);

inline constexpr std::array HiraganaBlocks = {
  UnicodeBlock(0x3040, 0x309f, UVer1_1, "Hiragana")};

// Second block contains small letters (for Ainu) like ㇱ
inline constexpr std::array KatakanaBlocks = {
  UnicodeBlock(0x30a0, 0x30ff, UVer1_1, "Katakana"),
  UnicodeBlock(0x31f0, 0x31ff, UVer3_2, "Katakana Phonetic Extension")};

// Almost all 'common' Japanese Kanji are in the original CJK Unified block.
// Extension A has one 'Kentei' and about 1000 'Ucd' Kanji. Extension B has an
// updated Jouyou Kanji '𠮟' (U+20B9F) which used to be '叱' (U+53F1)). The
// Compatibility block contains many 'single grapheme' versions of old/variant
// Japanese Kanji that used to require two graphemes, i.e., a base character
// followed by a variation selector.
inline constexpr std::array CommonKanjiBlocks = {
  UnicodeBlock(0x3400, 0x4dbf, UVer3_0, "CJK Extension A"), // ~6K kanji: 㵎
  UnicodeBlock(0x4e00, 0x9fff, UVer1_1, "CJK Unified Ideographs"), // ~20K
  UnicodeBlock(0xf900, 0xfaff, UVer1_1, "CJK Compat. Ideographs"), // 渚, 猪
  UnicodeBlock(0x20000, 0x2a6df, UVer3_1, "CJK Extension B")       // ~42K: 𠮟
};

// Note: Extensions C, D, E and F are contiguous so combine into one block (more
// efficient for 'isKanji' functions and wregex). Here are the actual ranges:
// - U+2A700 to U+2B73F : CJK Extension C, ver 5.2 Oct 2009, ~4K kanji
// - U+2B740 to U+2B81F : CJK Extension D, ver 6.0 Oct 2010, 222 kanji
// - U+2B820 to U+2CEAF : CJK Extension E, ver 8.0 Jun 2015, ~6K kanji
// - U+2CEB0 to U+2EBEF : CJK Extension F, ver 10.0 Jun 2016, ~7K kanji
inline constexpr std::array RareKanjiBlocks = {
  UnicodeBlock(0x2e80, 0x2eff, UVer3_0, "Radicals Supp."),      // 128
  UnicodeBlock(0x2a700, 0x2ebef, UVer5_2, "CJK Extension C-F"), // ~17K kanji
  UnicodeBlock(0x2f800, 0x2fa1f, UVer3_1, "CJK Compat. Supp."), // ~6K kanji
  UnicodeBlock(0x30000, 0x3134f, UVer13_0, "CJK Extension G")   // ~5K kanji
};

inline constexpr std::array PunctuationBlocks = {
  UnicodeBlock(0x2000, 0x206f, UVer1_1, "General Punctuation"), // —, ‥, ”, “
  UnicodeBlock(0x3000, 0x303f, UVer1_1, "CJK Symbols and Punctuation"), // 、,
  UnicodeBlock(0xfff0, 0xffff, UVer1_1, "Specials") // Object Replacement, etc.
};

// There are a lot more symbol and letter blocks, but they haven't come up in
// sample files so far
inline constexpr std::array SymbolBlocks = {
  UnicodeBlock(0x2100, 0x214f, UVer1_1, "Letterlike Symbols"),          // ℃
  UnicodeBlock(0x2190, 0x21ff, UVer1_1, "Arrows"),                      // →
  UnicodeBlock(0x2200, 0x22ff, UVer1_1, "Mathematical Operators"),      // ∀
  UnicodeBlock(0x2500, 0x257f, UVer1_1, "Box Drawing"),                 // ─
  UnicodeBlock(0x25a0, 0x25ff, UVer1_1, "Geometric Shapes"),            // ○
  UnicodeBlock(0x2600, 0x26ff, UVer1_1, "Miscellaneous Symbols"),       // ☆
  UnicodeBlock(0x2ff0, 0x2fff, UVer3_0, "CJK Ideographic Desc. Chars"), // ⿱
  UnicodeBlock(0x3190, 0x319f, UVer1_1, "Kanbun (Annotations)"),        // ㆑
  UnicodeBlock(0x31c0, 0x31ef, UVer4_1, "CJK Strokes")                  // ㇁
};

// the last block also includes 'halfwidth katakana'
inline constexpr std::array LetterBlocks = {
  UnicodeBlock(0x0080, 0x00ff, UVer1_1, "Latin-1 Supplement"),    // ·, ×
  UnicodeBlock(0x0100, 0x017f, UVer1_1, "Latin Extended-A"),      // Ā
  UnicodeBlock(0x0180, 0x024f, UVer1_1, "Latin Extended-B"),      // ƀ
  UnicodeBlock(0x2150, 0x218f, UVer1_1, "Number Forms"),          // Ⅳ
  UnicodeBlock(0x2460, 0x24ff, UVer1_1, "Enclosed Alphanumeics"), // ⑦
  UnicodeBlock(0x2c60, 0x2c7f, UVer5_0, "Latin Extended-C"),
  UnicodeBlock(0xff00, 0xffef, UVer1_1, "Halfwidth and Fullwidth Forms")};

// Skip codes in this range when reading in Kanji. See this link for more info:
// http://unicode.org/reports/tr28/tr28-3.html#13_7_variation_selectors
inline constexpr std::array NonSpacingBlocks = {
  UnicodeBlock(0xfe00, 0xfe0f, UVer3_2, "Variation Selectors")};

// 'inRange' checks if 'c' is contained in any of the UnicodeBocks in the array
// 't'. The blocks in 't' are assumed to be in order (order is checked by
// automated tests for all the arrays defined above).
template<size_t N>
[[nodiscard]] constexpr bool
inRange(char32_t c, const std::array<UnicodeBlock, N>& t) noexcept {
  for (auto& i : t) {
    if (c < i.start) break;
    if (i(c)) return true;
  }
  return false;
}

// 'inRange' with more than one 't' (block array) checks each array so there's
// no requirement for the arrays to be specified in a particular order (which
// wouldn't work anyway for overlapping ranges).
template<size_t N, typename... Ts>
[[nodiscard]] constexpr bool
inRange(char32_t c, const std::array<UnicodeBlock, N>& t, Ts... args) noexcept {
  return inRange(c, t) || inRange(c, args...);
}

inline constexpr auto CombiningMarkVoiced = U'\x3099',
                      CombiningMarkSemiVoiced = U'\x309a';

[[nodiscard]] constexpr auto isNonSpacing(char32_t c) noexcept {
  return inRange(c, NonSpacingBlocks) || c == CombiningMarkVoiced ||
         c == CombiningMarkSemiVoiced;
}

// Return true if the first 'MB character' is in the given blocks, empty string
// will return false and a string longer than one 'MB characer' will also return
// false unless 'sizeOne' is false.
template<typename... T>
[[nodiscard]] inline auto inWCharRange(const std::string& s, bool sizeOne,
                                       T... t) {
  if (s.size() > 1 && (!sizeOne || s.size() < 9))
    if (const auto w = fromUtf8(s);
        sizeOne ? w.size() == 1 || w.size() == 2 && isNonSpacing(w[1])
                : w.size() >= 1)
      return inRange(w[0], t...);
  return false;
}

// true if all characers are in the given blocks, empty is also considered true
template<typename... T>
[[nodiscard]] inline auto inWCharRange(const std::string& s, T... t) {
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

// functions for classifying 'recognized' utf-8 encoded characters: 's' should
// contain one MB character (so 2-4 bytes) by default, but 'sizeOne' can
// be set to 'false' to check just the first 'MB characer' in the string. There
// are alls 'isAll' functions that return 'true' only if all the characers in
// the string are the desired type.

// kana
[[nodiscard]] inline auto isHiragana(const std::string& s,
                                     bool sizeOne = true) {
  return inWCharRange(s, sizeOne, HiraganaBlocks);
}
[[nodiscard]] inline auto isAllHiragana(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks);
}
[[nodiscard]] inline auto isKatakana(const std::string& s,
                                     bool sizeOne = true) {
  return inWCharRange(s, sizeOne, KatakanaBlocks);
}
[[nodiscard]] inline auto isAllKatakana(const std::string& s) {
  return inWCharRange(s, KatakanaBlocks);
}
[[nodiscard]] inline auto isKana(const std::string& s, bool sizeOne = true) {
  return inWCharRange(s, sizeOne, HiraganaBlocks, KatakanaBlocks);
}
[[nodiscard]] inline auto isAllKana(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks, KatakanaBlocks);
}

// kanji
[[nodiscard]] inline auto isCommonKanji(const std::string& s,
                                        bool sizeOne = true) {
  return inWCharRange(s, sizeOne, CommonKanjiBlocks);
}
[[nodiscard]] inline auto isAllCommonKanji(const std::string& s) {
  return inWCharRange(s, CommonKanjiBlocks);
}
[[nodiscard]] inline auto isRareKanji(const std::string& s,
                                      bool sizeOne = true) {
  return inWCharRange(s, sizeOne, RareKanjiBlocks);
}
[[nodiscard]] inline auto isAllRareKanji(const std::string& s) {
  return inWCharRange(s, RareKanjiBlocks);
}
[[nodiscard]] inline auto isKanji(const std::string& s, bool sizeOne = true) {
  return inWCharRange(s, sizeOne, CommonKanjiBlocks, RareKanjiBlocks);
}
[[nodiscard]] inline auto isAllKanji(const std::string& s) {
  return inWCharRange(s, CommonKanjiBlocks, RareKanjiBlocks);
}

// 'isMBPunctuation' tests for wide space by default, but also allows not
// including spaces.
[[nodiscard]] inline auto isMBPunctuation(const std::string& s,
                                          bool includeSpace = false,
                                          bool sizeOne = true) {
  return s.starts_with("　") ? (includeSpace && (s.size() < 4 || !sizeOne))
                             : inWCharRange(s, sizeOne, PunctuationBlocks);
}
[[nodiscard]] inline auto isAllMBPunctuation(const std::string& s) {
  return inWCharRange(s, PunctuationBlocks);
}
[[nodiscard]] inline auto isMBSymbol(const std::string& s,
                                     bool sizeOne = true) {
  return inWCharRange(s, sizeOne, SymbolBlocks);
}
[[nodiscard]] inline auto isAllMBSymbol(const std::string& s) {
  return inWCharRange(s, SymbolBlocks);
}
[[nodiscard]] inline auto isMBLetter(const std::string& s,
                                     bool sizeOne = true) {
  return inWCharRange(s, sizeOne, LetterBlocks);
}
[[nodiscard]] inline auto isAllMBLetter(const std::string& s) {
  return inWCharRange(s, LetterBlocks);
}

// 'isRecognizedCharacter' returns true if 's' is in any UnicodeBlock defined in
// this header file (including wide space)
[[nodiscard]] inline auto isRecognizedCharacter(const std::string& s,
                                                bool sizeOne = true) {
  return inWCharRange(s, sizeOne, HiraganaBlocks, CommonKanjiBlocks,
                      RareKanjiBlocks, KatakanaBlocks, PunctuationBlocks,
                      SymbolBlocks, LetterBlocks);
}
[[nodiscard]] inline auto isAllRecognizedCharacters(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks,
                      KatakanaBlocks, PunctuationBlocks, SymbolBlocks,
                      LetterBlocks);
}

// KanjiRange is for wregex and includes the common and rare kanji as well as
// variation selectors.
inline constexpr wchar_t WideDash = L'-';

// 'KanjiRange' contains the following blocks (in order):
// - CJK Extension A
// - CJK Unified Ideographs Kanji
// - CJK Compatibility Ideographs
// - CJK Extension B
// - Variation Selectors
// - CJK Radicals Supplement
// - CJK Extension C, D, E and F
// - CJK Compatibility Ideographs Supplement
// - CJK Extension G

// clang-format off
inline constexpr wchar_t KanjiRange[] = {
  CommonKanjiBlocks[0].wStart(), WideDash, CommonKanjiBlocks[0].wEnd(),
  CommonKanjiBlocks[1].wStart(), WideDash, CommonKanjiBlocks[1].wEnd(),
  CommonKanjiBlocks[2].wStart(), WideDash, CommonKanjiBlocks[2].wEnd(),
  CommonKanjiBlocks[3].wStart(), WideDash, CommonKanjiBlocks[3].wEnd(),
  NonSpacingBlocks[0].wStart(), WideDash, NonSpacingBlocks[0].wEnd(),
  RareKanjiBlocks[0].wStart(), WideDash, RareKanjiBlocks[0].wEnd(),
  RareKanjiBlocks[1].wStart(), WideDash, RareKanjiBlocks[1].wEnd(),
  RareKanjiBlocks[2].wStart(), WideDash, RareKanjiBlocks[2].wEnd(),
  RareKanjiBlocks[3].wStart(), WideDash, RareKanjiBlocks[3].wEnd(),
  L'\0' // null
};
// clang-format on

inline constexpr wchar_t HiraganaRange[] = L"\u3040-\u309f";
inline constexpr wchar_t KatakanaRange[] = L"\u30a0-\u30ff\u31f0-\u31ff";
inline constexpr wchar_t KanaRange[] = L"\u3040-\u30ff\u31f0-\u31ff";

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_UNICODE_BLOCK_H
