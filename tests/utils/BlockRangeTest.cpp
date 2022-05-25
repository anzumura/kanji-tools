#include <gtest/gtest.h>
#include <kanji_tools/utils/BlockRange.h>
#include <kanji_tools/utils/Exception.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

TEST(BlockRangeTest, KanjiRange) {
  // KanjiRange should include all the common and rare kanji + variant selectors
  // and a null terminator
  ASSERT_EQ(std::size(KanjiRange),
      (CommonKanjiBlocks.size() + RareKanjiBlocks.size() + 1) * 3);
  size_t pos{};
  auto checkKanjiRange{[&pos](auto& blocks) {
    for (auto& i : blocks) {
      EXPECT_EQ(KanjiRange[pos++], i.start()) << pos;
      EXPECT_EQ(KanjiRange[pos++], U'-') << pos;
      EXPECT_EQ(KanjiRange[pos++], i.end()) << pos;
    }
  }};
  checkKanjiRange(CommonKanjiBlocks);
  checkKanjiRange(NonSpacingBlocks);
  checkKanjiRange(RareKanjiBlocks);
  EXPECT_EQ(KanjiRange[pos], U'\0');
}

TEST(BlockRangeTest, CheckOtherBlocks) {
  ASSERT_EQ(std::size(WideLetterRange), 3);
  ASSERT_EQ(std::size(HiraganaRange), 3);
  ASSERT_EQ(HiraganaBlocks.size(), 1);
  EXPECT_EQ(HiraganaRange[0], HiraganaBlocks[0].start());
  EXPECT_EQ(HiraganaRange[2], HiraganaBlocks[0].end());
  ASSERT_EQ(std::size(KatakanaRange), 6);
  ASSERT_EQ(KatakanaBlocks.size(), 2);
  EXPECT_EQ(KatakanaRange[0], KatakanaBlocks[0].start());
  EXPECT_EQ(KatakanaRange[2], KatakanaBlocks[0].end());
  EXPECT_EQ(KatakanaRange[3], KatakanaBlocks[1].start());
  EXPECT_EQ(KatakanaRange[5], KatakanaBlocks[1].end());
  ASSERT_EQ(std::size(KanaRange), 6);
  EXPECT_EQ(KanaRange[0], HiraganaBlocks[0].start());
  // first katakana block immediately follows hiragana block so can use a bigger
  // range but check the assumption by comparing 'end + 1' to 'start'
  EXPECT_EQ(HiraganaBlocks[0].end() + 1, KatakanaBlocks[0].start());
  EXPECT_EQ(KanaRange[2], KatakanaBlocks[0].end());
  EXPECT_EQ(KanaRange[3], KatakanaBlocks[1].start());
  EXPECT_EQ(KanaRange[5], KatakanaBlocks[1].end());
}

TEST(BlockRangeTest, BlockRangeError) {
  EXPECT_THROW(call([] { return KanaRange[7]; },
                   "index '7' is out of range for BlockRange with size '6'"),
      RangeError);
  EXPECT_THROW(call([] { return HiraganaRange[6]; },
                   "index '6' is out of range for BlockRange with size '3'"),
      RangeError);
}

TEST(BlockRangeTest, CreateBlockRange) {
  const BlockRange r{CommonKanaBlock, NonSpacingBlocks[0]};
  ASSERT_EQ(r.size(), 6); // 'size' doesn't include the final null
  ASSERT_EQ(r[6], L'\0');
  const std::wstring w{r()};
  EXPECT_EQ(w, L"\x3040-\x30ff\xfe00-\xfe0f");
}

} // namespace kanji_tools
