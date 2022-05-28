#pragma once

#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools { /// \utils_group{BlockRange}

/// non-templated bass class for BlockRange \utils{BlockRange}
class BaseBlockRange {
protected:
  static constexpr size_t SizePerBlock{3};

  static void fill(wchar_t*, const UnicodeBlock&) noexcept;

  template<typename... Ts>
  static void fill(
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
template<typename... Ts> class BlockRange : public BaseBlockRange {
public:
  explicit BlockRange(const UnicodeBlock& block, const Ts&... blocks) noexcept {
    fill(_range, block, blocks...);
  }

  /// return null terminated `wchar_t` array to be used in `wregex()`
  [[nodiscard]] auto operator()() const noexcept { return _range; }

  /// return character at position `i`, will throw if `i` is out of range
  [[nodiscard]] auto operator[](size_t i) const {
    return _range[checkIndex(i, size())];
  }

  /// return size of the internal array (not including the final null)
  [[nodiscard]] static constexpr auto size() noexcept { return Size; }
private:
  static constexpr auto Size{(sizeof...(Ts) + 1) * SizePerBlock};

  wchar_t _range[size() + 1]{}; // add 1 for final null
};

/// KanjiRange is for 'wregex' and contains the following blocks (in order):
/// - CJK Extension A
/// - CJK Unified Ideographs Kanji
/// - CJK Compatibility Ideographs
/// - CJK Extension B
/// - Variation Selectors
/// - CJK Radicals Supplement
/// - CJK Extension C, D, E and F
/// - CJK Compatibility Ideographs Supplement
/// - CJK Extension G
inline const BlockRange KanjiRange{CommonKanjiBlocks[0], CommonKanjiBlocks[1],
    CommonKanjiBlocks[2], CommonKanjiBlocks[3], NonSpacingBlocks[0],
    RareKanjiBlocks[0], RareKanjiBlocks[1], RareKanjiBlocks[2],
    RareKanjiBlocks[3]};

inline const BlockRange WideLetterRange{LetterBlocks[6]};
inline const BlockRange HiraganaRange{HiraganaBlocks[0]};
inline const BlockRange KatakanaRange{KatakanaBlocks[0], KatakanaBlocks[1]};
inline const BlockRange KanaRange{CommonKanaBlock, KatakanaBlocks[1]};

/// \end_group
} // namespace kanji_tools
