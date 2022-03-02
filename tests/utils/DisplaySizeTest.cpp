#include <gtest/gtest.h>
#include <kanji_tools/utils/DisplaySize.h>

namespace kanji_tools {

TEST(DisplaySizeTest, WideBlocksRange) {
  // WideBlocks.size() may change after parsing newer Unicode files
  EXPECT_EQ(WideBlocks.size(), 121);
  // make sure 'WideBlocks' (from generated code) has no overlaps
  for (char32_t oldEnd = 0; auto& i : WideBlocks) {
    EXPECT_LT(oldEnd, i.start);
    oldEnd = i.end;
    EXPECT_LE(i.start, i.end);
  }
}

TEST(DisplaySizeTest, DisplaySize) {
  EXPECT_EQ(displaySize(""), 0);
  EXPECT_EQ(displaySize("abc ."), 5);
  std::string empty;
  EXPECT_EQ(displaySize(empty), 0);
  EXPECT_EQ(displaySize(std::string("abc .")), 5);
  EXPECT_EQ(displaySize("abクcカ"), 7); // 3 narrow + 2 wide
  EXPECT_EQ(displaySize("。、Ｈ"), 6);  // 2 wide punctuation + 1 wide letter
  // rare kanji, common kanji, 4 narrow numbers and a wide space = 10
  EXPECT_EQ(displaySize("𫠜中1234　"), 10);
  // don't include non-spacing characters
  std::string s = "逸︁";
  EXPECT_EQ(s.size(), 6);               // two 3-byte sequences
  EXPECT_EQ(toUnicode(s), "9038 FE01"); // 'FE01' is a variation selector
  EXPECT_EQ(displaySize(s), 2);
  // don't include combining marks
  s = "と\xe3\x82\x99ヒ\xe3\x82\x9a";
  EXPECT_EQ(displaySize(s), 4);
  // try a character beyond BMP
  EXPECT_EQ(displaySize("𠮟"), 2);
}

TEST(DisplaySizeTest, U32DisplaySize) {
  EXPECT_EQ(displaySize(U""), 0);
  EXPECT_EQ(displaySize(U"abc ."), 5);
  std::u32string empty;
  EXPECT_EQ(displaySize(empty), 0);
  EXPECT_EQ(displaySize(std::u32string(U"abc .")), 5);
  EXPECT_EQ(displaySize(U"abクcカ"), 7); // 3 narrow + 2 wide
  EXPECT_EQ(displaySize(U"。、Ｈ"), 6);  // 2 wide punctuation + 1 wide letter
  // rare kanji, common kanji, 4 narrow numbers and a wide space = 10
  EXPECT_EQ(displaySize(U"𫠜中1234　"), 10);
  // don't include non-spacing characters
  std::u32string s = U"逸︁";
  EXPECT_EQ(s.size(), 2);               // two unicode chars
  EXPECT_EQ(toUnicode(s), "9038 FE01"); // 'FE01' is a variation selector
  EXPECT_EQ(displaySize(s), 2);
  // don't include combining marks
  s = U"と\x3099ヒ\x309a";
  EXPECT_EQ(displaySize(s), 4);
  // try a character beyond BMP
  EXPECT_EQ(displaySize(U"𠮟"), 2);
}

TEST(DisplaySizeTest, WideSetw) {
  EXPECT_EQ(wideSetw("abc", 5), 5);      // no change for all narrow
  EXPECT_EQ(wideSetw("abクcカ", 8), 10); // 3 narrow + 2 wide (each 3 bytes)
  EXPECT_EQ(wideSetw("。、Ｈ", 8), 11);  // 2 wide punctuation + 1 wide letter
  // a 4 byte rare kanji, a 3 byte common kanji, 4 narrow numbers and a 3 byte
  // wide space is 14 bytes, but has a displaySize of 10 (2 + 2 + 4 + 2)
  std::string s = "𫠜中1234　";
  EXPECT_EQ(s.size(), 14);
  EXPECT_EQ(displaySize(s), 10);
  // in order to get a 'width' of 11, std::setw would need to be given 15, i.e.,
  // one byte more than 14 (15 makes the stream add one more space which gets
  // 'display size' from 10 to 11). See comments for 'wideSetw' for details.
  EXPECT_EQ(wideSetw(s, 11), 15);
  // don't include non-spacing characters
  s = "逸︁";
  EXPECT_EQ(s.size(), 6);               // two 3-byte sequences
  EXPECT_EQ(toUnicode(s), "9038 FE01"); // 'FE01' is a variation selector
  // need to add 2 spaces for setw so result is '6 + 2'
  EXPECT_EQ(wideSetw(s, 4), 8);
  // try a character beyond BMP
  EXPECT_EQ(wideSetw("𠮟", 3), 5); // character is 4 bytes so return '4 + 1'
  EXPECT_EQ(wideSetw("𠮟", 2), 4); // return '4 + 0'
  EXPECT_EQ(wideSetw("𠮟", 1), 3); // shorter than wide char (return '4 + -1')
}

} // namespace kanji_tools
