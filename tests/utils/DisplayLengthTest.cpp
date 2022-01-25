#include <gtest/gtest.h>
#include <kanji_tools/utils/DisplayLength.h>

namespace kanji_tools {

TEST(DisplayLengthTest, WideBlocksRange) {
  // WideBlocks.size() may change after parsing newer Unicode files
  EXPECT_EQ(WideBlocks.size(), 121);
  // make sure 'WideBlocks' (from generated code) has no overlaps
  for (int oldEnd = 0; auto& i : WideBlocks) {
    EXPECT_LT(oldEnd, i.start);
    oldEnd = i.end;
    EXPECT_LE(i.start, i.end);
  }
}

TEST(DisplayLengthTest, DisplayLength) {
  EXPECT_EQ(displayLength("abc"), 3);
  EXPECT_EQ(displayLength("abクcカ"), 7); // 3 narrow + 2 wide (wide count as len 2)
  EXPECT_EQ(displayLength("。、Ｈ"), 6);  // 2 wide punctuation + 1 wide letter
  // rare kanji, common kanji, 4 narrow numbers and a wide space = 10
  EXPECT_EQ(displayLength("㐀中1234　"), 10);
  // don't include non-spacing characters
  std::string s = "逸︁";
  EXPECT_EQ(s.length(), 6);             // two 3-byte sequences
  EXPECT_EQ(toUnicode(s), "9038 FE01"); // 'FE01' is a variation selector
  EXPECT_EQ(displayLength(s), 2);       // should be 2 for the single displayable wide char
  // try a character beyond BMP
  EXPECT_EQ(displayLength("𠮟"), 2);
}

TEST(DisplayLengthTest, WideSetw) {
  EXPECT_EQ(wideSetw("abc", 5), 5);      // no change for all narrow
  EXPECT_EQ(wideSetw("abクcカ", 8), 10); // 3 narrow + 2 wide (each 3 bytes)
  EXPECT_EQ(wideSetw("。、Ｈ", 8), 11);  // 2 wide punctuation + 1 wide letter
  // rare kanji, common kanji, 4 narrow numbers and a wide space = 10
  EXPECT_EQ(wideSetw("㐀中1234　", 11), 14);
  // don't include non-spacing characters
  std::string s = "逸︁";
  EXPECT_EQ(s.length(), 6);             // two 3-byte sequences
  EXPECT_EQ(toUnicode(s), "9038 FE01"); // 'FE01' is a variation selector
  EXPECT_EQ(wideSetw(s, 4), 8);         // need to add 2 spaces for setw so result is '6 + 2'
  // try a character beyond BMP
  EXPECT_EQ(wideSetw("𠮟", 3), 5); // character is 4 bytes so return '4 + 1'
  EXPECT_EQ(wideSetw("𠮟", 2), 4); // return '4 + 0'
  EXPECT_EQ(wideSetw("𠮟", 1), 3); // request is shorted than wide char (return '4 + -1')
}

} // namespace kanji_tools
