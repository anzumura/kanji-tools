#pragma once

#include <kanji_tools/utils/MBUtils.h>

#include <array>

namespace kanji_tools {

// Unicode version and release date
class UnicodeVersion {
public:
  consteval UnicodeVersion(const char* v, uint8_t m, uint16_t y)
      : _version{v}, _month{m}, _year{y} {}

  UnicodeVersion(const UnicodeVersion&) = delete;

  constexpr auto version() const { return _version; }
  constexpr auto month() const { return _month; }
  constexpr auto year() const { return _year; }
private:
  const char* const _version;
  const uint8_t _month;
  const uint16_t _year;
};

// 'UnicodeBlock' holds a range which is used in the 'is' functions ('isKanji',
// 'isHiragana', etc.). Official blocks have a 'version' and a 'name' (just for
// reference). To keep it simple, 'version' represents the first version the
// block was introduced whereas in reality some characters in the block may be
// added in later versions. For example, the 'Katakana' block was introduced in
// version 1.1, but U+30A0 (゠) was added to the block in version 3.2.
class UnicodeBlock {
public:
  static constexpr auto Mod{16}, OfficialStartMod{0}, OfficialEndMod{15};

  UnicodeBlock(const UnicodeBlock&) = delete;

  // return number of code points in the block (inclusive of start and end)
  [[nodiscard]] constexpr auto range() const noexcept {
    return end - start + 1;
  }

  // 'opterator()' returns true if 'x' is in this block
  [[nodiscard]] constexpr auto operator()(Code x) const noexcept {
    return x >= start && x <= end;
  }

  // 'wStart' and 'wEnd' are needed for wregex (may remove later)
  [[nodiscard]] constexpr auto wStart() const noexcept {
    return toWChar(start);
  }
  [[nodiscard]] constexpr auto wEnd() const noexcept { return toWChar(end); }

  const Code start;
  const Code end;
  const UnicodeVersion* const version;
  const char* const name;
private:
  template<Code Start, Code End = Start> static consteval void checkRange() {
    static_assert(Start > MaxAscii);
    static_assert(End <= MaxUnicode);
  }

  template<Code Start, Code End> static consteval void checkLess() {
    checkRange<Start, End>();
    static_assert(Start < End);
  }

  consteval UnicodeBlock(
      Code s, Code e, const UnicodeVersion* v = {}, const char* n = {})
      : start{s}, end{e}, version{v}, name{n} {}

  // grant access to 'makeBlock' functions
  template<Code Start> friend consteval auto makeBlock();
  template<Code Start, Code End> friend consteval auto makeBlock();
  template<Code Start, Code End, typename T>
  friend consteval auto makeBlock(T&, const char*);
};

// UnicodeBlocks defined in DisplaySize.h (WideBlocks) are used for determining
// if a character is narrow or wide display can be a single entry and also not
// start or end on an 'official' boundary. gcc 11.2 didn't like using a default
// template (Code End = Start) in combination with the friend declaration
// inside UnicodeBlock so split into two 'makeBlock' functions. This also allows
// better static_assert (using '<' instead of '<=').

template<Code Start> [[nodiscard]] consteval auto makeBlock() {
  UnicodeBlock::checkRange<Start>();
  return UnicodeBlock{Start, Start};
}

template<Code Start, Code End> [[nodiscard]] consteval auto makeBlock() {
  UnicodeBlock::checkLess<Start, End>();
  return UnicodeBlock{Start, End};
}

// Official Unicode blocks start on a value having mod 16 = 0 (so ending in hex
// '0') and end on a value having mod 16 = 15 (so ending in hex 'f').

template<Code Start, Code End, typename T>
[[nodiscard]] consteval auto makeBlock(T& v, const char* n) {
  UnicodeBlock::checkLess<Start, End>();
  static_assert(Start % UnicodeBlock::Mod == UnicodeBlock::OfficialStartMod);
  static_assert(End % UnicodeBlock::Mod == UnicodeBlock::OfficialEndMod);
  return UnicodeBlock{Start, End, &v, n};
}

// below are the Unicode versions referenced in this program, for the full list
// see: https://unicode.org/history/publicationdates.html
inline constexpr UnicodeVersion UVer1_0{"1.0", 10, 1991},
    UVer1_1{"1.1", 6, 1993}, UVer2_0{"2.0", 7, 1996}, UVer3_0{"3.0", 9, 1999},
    UVer3_1{"3.1", 3, 2001}, UVer3_2{"3.2", 3, 2002}, UVer4_1{"4.1", 3, 2005},
    UVer5_0{"5.0", 7, 2006}, UVer5_2{"5.2", 10, 2009},
    UVer13_0{"13.0", 3, 2020};

inline constexpr std::array HiraganaBlocks{
    makeBlock<0x3040, 0x309f>(UVer1_1, "Hiragana")};

// Second block contains small letters (for Ainu) like ㇱ
inline constexpr std::array KatakanaBlocks{
    makeBlock<0x30a0, 0x30ff>(UVer1_1, "Katakana"),
    makeBlock<0x31f0, 0x31ff>(UVer3_2, "Katakana Phonetic Extension")};

// first Katakana block immediately follows Hiragana block so create a merged
// block (to use in 'KanaRange')
inline constexpr auto CommonKanaBlock{
    makeBlock<HiraganaBlocks[0].start, KatakanaBlocks[0].end>(UVer1_1, "Kana")};

// Almost all 'common' Japanese Kanji are in the original CJK Unified block.
// Extension A has one 'Kentei' and about 1000 'Ucd' Kanji. Extension B has an
// updated Jouyou Kanji '𠮟' (U+20B9F) which used to be '叱' (U+53F1)). The
// Compatibility block contains many 'single grapheme' versions of old/variant
// Japanese Kanji that used to require two graphemes, i.e., a base character
// followed by a variation selector.
inline constexpr std::array CommonKanjiBlocks{
    makeBlock<0x3400, 0x4dbf>(UVer3_0, "CJK Extension A"), // ~6K kanji: 㵎
    makeBlock<0x4e00, 0x9fff>(UVer1_1, "CJK Unified Ideographs"), // ~20K
    makeBlock<0xf900, 0xfaff>(UVer1_1, "CJK Compat. Ideographs"), // 渚, 猪
    makeBlock<0x20000, 0x2a6df>(UVer3_1, "CJK Extension B")       // ~42K: 𠮟
};

// Note: Extensions C, D, E and F are contiguous so combine into one block (more
// efficient for 'isKanji' functions and wregex). Here are the actual ranges:
// - U+2A700 to U+2B73F : CJK Extension C, ver 5.2 Oct 2009, ~4K kanji
// - U+2B740 to U+2B81F : CJK Extension D, ver 6.0 Oct 2010, 222 kanji
// - U+2B820 to U+2CEAF : CJK Extension E, ver 8.0 Jun 2015, ~6K kanji
// - U+2CEB0 to U+2EBEF : CJK Extension F, ver 10.0 Jun 2016, ~7K kanji
inline constexpr std::array RareKanjiBlocks{
    makeBlock<0x2e80, 0x2eff>(UVer3_0, "Radicals Supp."),      // 128
    makeBlock<0x2a700, 0x2ebef>(UVer5_2, "CJK Extension C-F"), // ~17K kanji
    makeBlock<0x2f800, 0x2fa1f>(UVer3_1, "CJK Compat. Supp."), // ~6K kanji
    makeBlock<0x30000, 0x3134f>(UVer13_0, "CJK Extension G")   // ~5K kanji
};

inline constexpr std::array PunctuationBlocks{
    makeBlock<0x2000, 0x206f>(UVer1_1, "General Punctuation"), // —, ‥, ”, “
    makeBlock<0x3000, 0x303f>(UVer1_1, "CJK Symbols and Punctuation"), // 、,
    makeBlock<0xfff0, 0xffff>(UVer1_1, "Specials") // Object Replacement, etc.
};

// There are a lot more symbol and letter blocks, but they haven't come up in
// sample files so far
inline constexpr std::array SymbolBlocks{
    makeBlock<0x2100, 0x214f>(UVer1_1, "Letterlike Symbols"),          // ℃
    makeBlock<0x2190, 0x21ff>(UVer1_1, "Arrows"),                      // →
    makeBlock<0x2200, 0x22ff>(UVer1_1, "Mathematical Operators"),      // ∀
    makeBlock<0x2500, 0x257f>(UVer1_1, "Box Drawing"),                 // ─
    makeBlock<0x25a0, 0x25ff>(UVer1_1, "Geometric Shapes"),            // ○
    makeBlock<0x2600, 0x26ff>(UVer1_1, "Miscellaneous Symbols"),       // ☆
    makeBlock<0x2ff0, 0x2fff>(UVer3_0, "CJK Ideographic Desc. Chars"), // ⿱
    makeBlock<0x3190, 0x319f>(UVer1_1, "Kanbun (Annotations)"),        // ㆑
    makeBlock<0x31c0, 0x31ef>(UVer4_1, "CJK Strokes")                  // ㇁
};

// the last block also includes 'halfwidth katakana'
inline constexpr std::array LetterBlocks{
    makeBlock<0x0080, 0x00ff>(UVer1_1, "Latin-1 Supplement"),     // ·, ×
    makeBlock<0x0100, 0x017f>(UVer1_1, "Latin Extended-A"),       // Ā
    makeBlock<0x0180, 0x024f>(UVer1_1, "Latin Extended-B"),       // ƀ
    makeBlock<0x2150, 0x218f>(UVer1_1, "Number Forms"),           // Ⅳ
    makeBlock<0x2460, 0x24ff>(UVer1_1, "Enclosed Alphanumerics"), // ⑦
    makeBlock<0x2c60, 0x2c7f>(UVer5_0, "Latin Extended-C"),
    makeBlock<0xff00, 0xffef>(UVer1_1, "Halfwidth and Fullwidth Forms")};

// Skip codes in this range when reading in Kanji. See this link for more info:
// http://unicode.org/reports/tr28/tr28-3.html#13_7_variation_selectors
inline constexpr std::array NonSpacingBlocks{
    makeBlock<0xfe00, 0xfe0f>(UVer3_2, "Variation Selectors")};

// 'inRange' checks if 'c' is contained in any of the UnicodeBocks in the array
// 't'. The blocks in 't' are assumed to be in order (order is checked by
// automated tests for all the arrays defined above).
template<size_t N>
[[nodiscard]] constexpr auto inRange(
    Code c, const std::array<UnicodeBlock, N>& t) noexcept {
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
[[nodiscard]] constexpr bool inRange(
    Code c, const std::array<UnicodeBlock, N>& t, Ts&... args) noexcept {
  return inRange(c, t) || inRange(c, args...);
}

[[nodiscard]] constexpr auto isNonSpacing(Code c) noexcept {
  return inRange(c, NonSpacingBlocks) || c == CombiningVoicedChar ||
         c == CombiningSemiVoicedChar;
}

// Return true if the first 'MB character' is in the given blocks, empty string
// will return false and a string longer than one 'MB characer' will also return
// false unless 'sizeOne' is false.
template<typename... T>
[[nodiscard]] inline auto inWCharRange(
    const std::string& s, bool sizeOne, T&... t) {
  if (s.size() > 1 && (!sizeOne || s.size() <= MaxMBSize * 2U))
    if (const auto w{fromUtf8(s, sizeOne ? 3 : 1)};
        !sizeOne || (w.size() == 1 || w.size() == 2 && isNonSpacing(w[1])))
      return inRange(w[0], t...);
  return false;
}

// true if all characers are in the given blocks, empty is also considered true
template<typename... T>
[[nodiscard]] inline auto inWCharRange(const std::string& s, T&... t) {
  // an 'inRange' character can be followed by a 'variation selector'
  for (auto allowNonSpacing{false}; const auto i : fromUtf8(s))
    if (allowNonSpacing && isNonSpacing(i))
      allowNonSpacing = false;
    else if (inRange(i, t...))
      allowNonSpacing = true;
    else
      return false;
  return true;
}

// functions for classifying 'recognized' utf-8 encoded characters. The string
// parameter should contain one MB character (so 2-4 bytes) by default, but
// 'sizeOne' can be set to 'false' to check just the first 'MB characer' in the
// string. There are alls 'isAll' functions that return 'true' only if all the
// characers in the string are the desired type.

// Kana

[[nodiscard]] bool isHiragana(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllHiragana(const std::string&);
[[nodiscard]] bool isKatakana(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllKatakana(const std::string&);
[[nodiscard]] bool isKana(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllKana(const std::string&);

// Kanji

[[nodiscard]] bool isCommonKanji(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllCommonKanji(const std::string&);
[[nodiscard]] bool isRareKanji(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllRareKanji(const std::string&);
[[nodiscard]] bool isKanji(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllKanji(const std::string&);

// other multi-byte characters

[[nodiscard]] bool isMBPunctuation(
    const std::string&, bool includeSpace = false, bool sizeOne = true);
[[nodiscard]] bool isAllMBPunctuation(const std::string&);
[[nodiscard]] bool isMBSymbol(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllMBSymbol(const std::string&);
[[nodiscard]] bool isMBLetter(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllMBLetter(const std::string&);

// 'isRecognizedMBChar' returns true if the string is in any UnicodeBlock
// defined in this header file (including wide space)
[[nodiscard]] bool isRecognizedMBChar(const std::string&, bool sizeOne = true);
[[nodiscard]] bool isAllRecognizedCharacters(const std::string&);

} // namespace kanji_tools
