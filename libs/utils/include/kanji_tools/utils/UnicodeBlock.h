#pragma once

#include <kanji_tools/utils/Utf8.h>

#include <array>
#include <chrono>
#include <iostream>

namespace kanji_tools { /// \utils_group{UnicodeBlock}
/// UnicodeBlock class as well as 'is' and 'isAll' functions for checking which
/// block(s) a value belongs to

/// class represents a block (range) of Unicode Code points \utils{UnicodeBlock}
///
/// This class is used in `is` functions like isKanji(), isHiragana(), etc..
/// Official blocks have a Version and a `name` (for reference). To keep it
/// simple, `version` represents the first version the block was introduced
/// whereas in reality some characters may have been added in later versions.
/// For example, the 'Katakana' block was introduced in version 1.1, but U+30A0
/// (゠) was added to the block in version 3.2.
///
/// UnicodeBlock instances defined in DisplaySize.h (WideBlocks) can be a single
/// Code point and can also start or end on an 'unofficial' boundaries. GCC 11.2
/// didn't like default template (Code End = Start) in combination with friend
/// declarations so this was split into two makeBlock() functions. This also
/// allows better `static_assert` (using '<' instead of '<=').
class UnicodeBlock {
public:
  static constexpr auto Mod{16}, OfficialStartMod{0}, OfficialEndMod{15};

  /// Unicode version and release date \utils{UnicodeBlock}
  class Version {
  public:
    /// create a Version object
    /// \param v version name, like "1.1"
    /// \param m version month (1-12)
    /// \param y version year (should be valid year including century like 1991)
    consteval Version(StringView v, uint8_t m, uint16_t y)
        : _version{v}, _date{std::chrono::year{y}, std::chrono::month{m}} {}

    Version(const Version&) = delete; ///< deleted copy ctor

    /// return version name
    [[nodiscard]] constexpr auto version() const { return _version; }

    /// return version date as a `std::chrono::year_month`
    [[nodiscard]] constexpr auto date() const { return _date; }
  private:
    const StringView _version;
    const std::chrono::year_month _date;
  };

  UnicodeBlock(const UnicodeBlock&) = delete; ///< deleted copy ctor

  /// return number of Code points in the block (inclusive of start and end)
  [[nodiscard]] constexpr auto range() const noexcept {
    return _end - _start + 1;
  }

  /// return true if `x` is in this block
  [[nodiscard]] constexpr auto operator()(Code x) const noexcept {
    return x >= _start && x <= _end;
  }

  /// return first Code of the block range as a `wchar_t`
  /// \details wStart() and wEnd() are needed for wregex (may remove later)
  [[nodiscard]] constexpr auto wStart() const noexcept {
    return toWChar(_start);
  }

  /// return last Code of the block range as a `wchar_t`
  [[nodiscard]] constexpr auto wEnd() const noexcept {
    return toWChar(_end);
  } /// \copydetails wStart

  /// return first Code of the block range
  [[nodiscard]] constexpr auto start() const { return _start; }

  /// return last Code of the block range
  [[nodiscard]] constexpr auto end() const { return _end; }

  /// return pointer to Version (`nullptr` for unofficial blocks)
  [[nodiscard]] constexpr auto version() const { return _version; }

  /// return block name
  [[nodiscard]] constexpr auto name() const { return _name; }
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
      Code s, Code e, const Version* v = {}, StringView n = {})
      : _start{s}, _end{e}, _version{v}, _name{n} {}

  // grant access to 'makeBlock' functions
  template<Code Start> friend consteval auto makeBlock();
  template<Code Start, Code End> friend consteval auto makeBlock();
  template<Code Start, Code End>
  friend consteval auto makeBlock(const Version&, StringView);

  const Code _start;
  const Code _end;
  const Version* const _version;
  const StringView _name;
};

/// write Version name and date to stream
std::ostream& operator<<(std::ostream&, const UnicodeBlock::Version&);

/// write "name (version)" for official blocks, otherwise write "start=, end="
std::ostream& operator<<(std::ostream&, const UnicodeBlock&);

/// create a UnicodeBlock with a single Code point
template<Code Start> [[nodiscard]] consteval auto makeBlock() {
  UnicodeBlock::checkRange<Start>();
  return UnicodeBlock{Start, Start};
}

/// create an 'unofficial' UnicodeBlock: `Start` must be less than `End`
template<Code Start, Code End> [[nodiscard]] consteval auto makeBlock() {
  UnicodeBlock::checkLess<Start, End>();
  return UnicodeBlock{Start, End};
}

/// create an 'official' UnicodeBlock: `Start` must be less than `End` and they
/// also must end with hex `0` and hex `f` respectively (verified using mod)
template<Code Start, Code End>
[[nodiscard]] consteval auto makeBlock(
    const UnicodeBlock::Version& v, StringView n) {
  UnicodeBlock::checkLess<Start, End>();
  static_assert(Start % UnicodeBlock::Mod == UnicodeBlock::OfficialStartMod);
  static_assert(End % UnicodeBlock::Mod == UnicodeBlock::OfficialEndMod);
  return UnicodeBlock{Start, End, &v, n};
}

/// Unicode versions referenced in this program \details for the full list
/// see: https://unicode.org/history/publicationdates.html @{
inline constexpr UnicodeBlock::Version UVer1_0{"1.0", 10, 1991},
    UVer1_1{"1.1", 6, 1993}, UVer2_0{"2.0", 7, 1996}, UVer3_0{"3.0", 9, 1999},
    UVer3_1{"3.1", 3, 2001}, UVer3_2{"3.2", 3, 2002}, UVer4_1{"4.1", 3, 2005},
    UVer5_0{"5.0", 7, 2006}, UVer5_2{"5.2", 10, 2009},
    UVer13_0{"13.0", 3, 2020}; ///@}

/// official Hiragana block
inline constexpr std::array HiraganaBlocks{
    makeBlock<0x3040, 0x309f>(UVer1_1, "Hiragana")};

/// official Katakana blocks, second one contains small letters for Ainu like ㇱ
inline constexpr std::array KatakanaBlocks{
    makeBlock<0x30a0, 0x30ff>(UVer1_1, "Katakana"),
    makeBlock<0x31f0, 0x31ff>(UVer3_2, "Katakana Phonetic Extension")};

/// first Katakana block immediately follows Hiragana block so create a single
/// merged block for 'Common Kana' (to use in #KanaRange)
inline constexpr auto CommonKanaBlock{
    makeBlock<HiraganaBlocks[0].start(), KatakanaBlocks[0].end()>(
        UVer1_1, "Kana")};

/// Almost all 'common' Japanese Kanji are in the original CJK Unified block.
/// Extension A has one 'Kentei' and about 1000 'Ucd' Kanji. Extension B has an
/// updated Jouyou Kanji '𠮟' (U+20B9F) which used to be '叱' (U+53F1)). The
/// Compatibility block contains many 'single grapheme' versions of old/variant
/// Japanese Kanji that used to require two graphemes, i.e., a base character
/// followed by a variation selector.
inline constexpr std::array CommonKanjiBlocks{
    makeBlock<0x3400, 0x4dbf>(UVer3_0, "CJK Extension A"), // ~6K kanji: 㵎
    makeBlock<0x4e00, 0x9fff>(UVer1_1, "CJK Unified Ideographs"), // ~20K
    makeBlock<0xf900, 0xfaff>(UVer1_1, "CJK Compat. Ideographs"), // 渚, 猪
    makeBlock<0x20000, 0x2a6df>(UVer3_1, "CJK Extension B")       // ~42K: 𠮟
};

/// Extensions C, D, E and F are contiguous so combine into one block (more
/// efficient for 'isKanji' functions and wregex). Here are the actual ranges:
/// - U+2A700 to U+2B73F : CJK Extension C, ver 5.2 Oct 2009, ~4K kanji
/// - U+2B740 to U+2B81F : CJK Extension D, ver 6.0 Oct 2010, 222 kanji
/// - U+2B820 to U+2CEAF : CJK Extension E, ver 8.0 Jun 2015, ~6K kanji
/// - U+2CEB0 to U+2EBEF : CJK Extension F, ver 10.0 Jun 2016, ~7K kanji
inline constexpr std::array RareKanjiBlocks{
    makeBlock<0x2e80, 0x2eff>(UVer3_0, "Radicals Supp."),      // 128
    makeBlock<0x2a700, 0x2ebef>(UVer5_2, "CJK Extension C-F"), // ~17K kanji
    makeBlock<0x2f800, 0x2fa1f>(UVer3_1, "CJK Compat. Supp."), // ~6K kanji
    makeBlock<0x30000, 0x3134f>(UVer13_0, "CJK Extension G")   // ~5K kanji
};

/// punctuation commonly used in Japanese text
inline constexpr std::array PunctuationBlocks{
    makeBlock<0x2000, 0x206f>(UVer1_1, "General Punctuation"), // —, ‥, ”, “
    makeBlock<0x3000, 0x303f>(UVer1_1, "CJK Symbols and Punctuation"), // 、,
    makeBlock<0xfff0, 0xffff>(UVer1_1, "Specials") // Object Replacement, etc.
};

/// symbols commonly used in Japanese text (there are a lot more symbol blocks,
/// but they haven't come up so far in sample data)
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

/// non-Ascii letters commonly used in Japanese text (the last block also
/// includes 'halfwidth Katakana')
inline constexpr std::array LetterBlocks{
    makeBlock<0x0080, 0x00ff>(UVer1_1, "Latin-1 Supplement"),     // ·, ×
    makeBlock<0x0100, 0x017f>(UVer1_1, "Latin Extended-A"),       // Ā
    makeBlock<0x0180, 0x024f>(UVer1_1, "Latin Extended-B"),       // ƀ
    makeBlock<0x2150, 0x218f>(UVer1_1, "Number Forms"),           // Ⅳ
    makeBlock<0x2460, 0x24ff>(UVer1_1, "Enclosed Alphanumerics"), // ⑦
    makeBlock<0x2c60, 0x2c7f>(UVer5_0, "Latin Extended-C"),
    makeBlock<0xff00, 0xffef>(UVer1_1, "Halfwidth and Fullwidth Forms")};

/// skip codes in this range when reading in Kanji - link for more info:
/// http://unicode.org/reports/tr28/tr28-3.html#13_7_variation_selectors
inline constexpr std::array NonSpacingBlocks{
    makeBlock<0xfe00, 0xfe0f>(UVer3_2, "Variation Selectors")};

/// check if `c` is contained in any of the blocks in array `t`. The blocks in
/// `t` are assumed to be in order (based on 'start' values) and non-overlapping
/// \details arrays defined in UnicodeBlock.h should all be in the correct order
/// for usage in this function (this is also checked by automated tests)
template<size_t N>
[[nodiscard]] constexpr auto inRange(
    Code c, const std::array<UnicodeBlock, N>& t) noexcept {
  for (auto& i : t) {
    if (c < i.start()) break;
    if (i(c)) return true;
  }
  return false;
}

/// check if `c` is contained in any of the block arrays. There's no requirement
/// for the arrays to be specified in a particular order (which wouldn't work
/// anyway because of overlapping ranges).
template<size_t N, typename... Ts>
[[nodiscard]] constexpr bool inRange(
    Code c, const std::array<UnicodeBlock, N>& t, Ts&... args) noexcept {
  return inRange(c, t) || inRange(c, args...);
}

/// return true if `c` is a non-spacing Code
[[nodiscard]] constexpr auto isNonSpacing(Code c) noexcept {
  return inRange(c, NonSpacingBlocks) || c == CombiningVoicedChar ||
         c == CombiningSemiVoicedChar;
}

/// return true if the first 'MB character' is in the given blocks. Empty string
/// will return false and a string longer than one 'MB character' also returns
/// false unless `sizeOne` is false.
template<typename... T>
[[nodiscard]] inline auto inWCharRange(const String& s, bool sizeOne, T&... t) {
  // a string with only one byte can't hold an MB char so don't need to check it
  if (s.size() > 1) {
    if (!sizeOne) {
      // only check the first 'wide' character when 'sizeOne' is false
      const auto w{fromUtf8(s, 1)};
      return w.size() == 1 && inRange(w[0], t...);
    }
    if (s.size() <= MaxMBSize * 2U) {
      // when 'sizeOne' is true then need to convert (up to) three characters so
      // that the second position can be tested for 'non-spacing'
      const auto w{fromUtf8(s, 3)};
      return (w.size() == 1 || w.size() == 2 && isNonSpacing(w[1])) &&
             inRange(w[0], t...);
    }
  }
  return false;
}

/// true if `s` is empty or every char in `s` is in the given blocks
template<typename... T>
[[nodiscard]] inline auto inWCharRange(const String& s, T&... t) {
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

// 'is' functions

/// return true if `s` is empty or is one UTF-8 char of the expected type (set
/// `sizeOne` to false to only check first char and ignore size restrictions)
[[nodiscard]] bool isKana(const String& s, bool sizeOne = true);
[[nodiscard]] bool isHiragana(const String& s, bool = true);    ///< \doc isKana
[[nodiscard]] bool isKatakana(const String& s, bool = true);    ///< \doc isKana
[[nodiscard]] bool isKanji(const String& s, bool = true);       ///< \doc isKana
[[nodiscard]] bool isCommonKanji(const String& s, bool = true); ///< \doc isKana
[[nodiscard]] bool isRareKanji(const String& s, bool = true);   ///< \doc isKana
[[nodiscard]] bool isMBSymbol(const String& s, bool = true);    ///< \doc isKana
[[nodiscard]] bool isMBLetter(const String& s, bool = true);    ///< \doc isKana
[[nodiscard]] bool isMBPunctuation(const String& s, bool includeSpace = false,
    bool sizeOne =
        true); ///< \doc isKana (don't include wide spaces by default)
[[nodiscard]] bool isRecognizedUtf8(const String& s,
    bool sizeOne = true); ///< \doc isKana (includes wide spaces)

// 'isAll' functions

/// return true if `s` is empty or only contains expected type chars
[[nodiscard]] bool isAllKana(const String&);
[[nodiscard]] bool isAllHiragana(const String& s);       ///< \doc isAllKana
[[nodiscard]] bool isAllKatakana(const String& s);       ///< \doc isAllKana
[[nodiscard]] bool isAllKanji(const String& s);          ///< \doc isAllKana
[[nodiscard]] bool isAllCommonKanji(const String& s);    ///< \doc isAllKana
[[nodiscard]] bool isAllRareKanji(const String& s);      ///< \doc isAllKana
[[nodiscard]] bool isAllMBSymbol(const String& s);       ///< \doc isAllKana
[[nodiscard]] bool isAllMBLetter(const String& s);       ///< \doc isAllKana
[[nodiscard]] bool isAllMBPunctuation(const String& s);  ///< \doc isAllKana
[[nodiscard]] bool isAllRecognizedUtf8(const String& s); ///< \doc isAllKana

/// \end_group
} // namespace kanji_tools
