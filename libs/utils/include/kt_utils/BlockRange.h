#pragma once

#include <kt_utils/UnicodeBlock.h>

namespace kanji_tools { /// \utils_group{BlockRange}
/// BlockRange class and global constants for working with Unicode in `wregex`

/// non-templated bass class for BlockRange \utils{BlockRange}
class BaseBlockRange {
protected:
  static constexpr size_t SizePerBlock{3};

  static constexpr void fill(wchar_t* i, const UnicodeBlock& block) noexcept {
    *i++ = block.wStart();
    *i++ = L'-';
    *i = block.wEnd();
  }

  template <typename... Ts>
  static constexpr void fill(
      wchar_t* i, const UnicodeBlock& block, const Ts&... blocks) noexcept {
    fill(i, block);
    fill(i + SizePerBlock, blocks...);
  }

  static size_t checkIndex(size_t i, size_t size);
};

/// makes an array from `UnicodeBlock`s to use in `wregex()` \utils{BlockRange}
///
/// For each block passed to the ctor, `block.wStart()`, `-`, `block.wEnd()` is
/// added to the internal array.
///
/// The following code should print "all Kana" (see #KanaRange below)
/// \code
///   const std::wregex allKana{std::wstring{L"^["} + KanaRange() + L"]+$"};
///   const auto s{L"ひらがな"};
///   if (std::regex_search(s, allKana)) std::cout << "all Kana\n";
/// \endcode
template <typename... Ts> class BlockRange final : public BaseBlockRange {
public:
  /// ctor takes one or more `UnicodeBlock`s and populated internal array
  explicit constexpr BlockRange(
      const UnicodeBlock& block, const Ts&... blocks) noexcept {
    fill(_range, block, blocks...);
  }

  /// return null terminated `wchar_t` array to be used in `wregex()`
  [[nodiscard]] constexpr auto operator()() const noexcept { return _range; }

  /// return character at position `i`
  /// \throw RangeError if `i` is out of range
  [[nodiscard]] auto operator[](size_t i) const {
    return _range[checkIndex(i, size())];
  }

  /// return size of the internal array (not including the final null)
  [[nodiscard]] static constexpr auto size() noexcept { return Size; }

private:
  static constexpr auto Size{(sizeof...(Ts) + 1) * SizePerBlock};

  wchar_t _range[size() + 1]{}; // add 1 for final null
};

/// KanjiRange is for `wregex()` and contains the following blocks (in order):
/// - CJK Extension A
/// - CJK Unified Ideographs Kanji
/// - CJK Compatibility Ideographs
/// - CJK Extension B
/// - Variation Selectors
/// - CJK Radicals Supplement
/// - CJK Extension C, D, E and F
/// - CJK Compatibility Ideographs Supplement
/// - CJK Extension G
inline constexpr BlockRange KanjiRange{CommonKanjiBlocks[0],
    CommonKanjiBlocks[1], CommonKanjiBlocks[2], CommonKanjiBlocks[3],
    NonSpacingBlocks[0], RareKanjiBlocks[0], RareKanjiBlocks[1],
    RareKanjiBlocks[2], RareKanjiBlocks[3]};

/// range for wide letters including halfwidth Katakana (U+FF00 - U+FFEF)
inline constexpr BlockRange WideLetterRange{LetterBlocks[6]};

/// range for standard Hiragana
inline constexpr BlockRange HiraganaRange{HiraganaBlocks[0]};

/// range for Katakana (both official blocks)
inline constexpr BlockRange KatakanaRange{KatakanaBlocks[0], KatakanaBlocks[1]};

/// range that includes both Hiragana and Katakana
inline constexpr BlockRange KanaRange{CommonKanaBlock, KatakanaBlocks[1]};

/// \end_group
} // namespace kanji_tools
