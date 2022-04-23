#pragma once

#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

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

// 'BlockRange' takes one or more 'UnicodeBlock's and populates an array that
// can be used with 'wregex'. For each block passed to the ctor the '_range'
// array is populated with 'block.wStart()', '-', 'block.wEnd()'.
template<typename... Ts> class BlockRange : public BaseBlockRange {
public:
  explicit BlockRange(const UnicodeBlock& block, const Ts&... blocks) noexcept {
    fill(_range, block, blocks...);
  }

  [[nodiscard]] auto operator()() const noexcept { return _range; }

  [[nodiscard]] auto operator[](size_t i) const {
    return _range[checkIndex(i, size())];
  }

  [[nodiscard]] static constexpr auto size() noexcept { return Size; }
private:
  static constexpr auto Size{(sizeof...(Ts) + 1) * SizePerBlock};

  wchar_t _range[size() + 1]{}; // add 1 for final null
};

// 'KanjiRange' is for 'wregex' and contains the following blocks (in order):
// - CJK Extension A
// - CJK Unified Ideographs Kanji
// - CJK Compatibility Ideographs
// - CJK Extension B
// - Variation Selectors
// - CJK Radicals Supplement
// - CJK Extension C, D, E and F
// - CJK Compatibility Ideographs Supplement
// - CJK Extension G

inline const BlockRange KanjiRange{CommonKanjiBlocks[0], CommonKanjiBlocks[1],
    CommonKanjiBlocks[2], CommonKanjiBlocks[3], NonSpacingBlocks[0],
    RareKanjiBlocks[0], RareKanjiBlocks[1], RareKanjiBlocks[2],
    RareKanjiBlocks[3]};

inline const BlockRange WideLetterRange{LetterBlocks[6]};
inline const BlockRange HiraganaRange{HiraganaBlocks[0]};
inline const BlockRange KatakanaRange{KatakanaBlocks[0], KatakanaBlocks[1]};
inline const BlockRange KanaRange{CommonKanaBlock, KatakanaBlocks[1]};

} // namespace kanji_tools
