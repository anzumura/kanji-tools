#include <gtest/gtest.h>
#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

namespace {

using BlockSet = std::set<const UnicodeBlock*>;

template<typename T>
void checkRange(const T& blocks, BlockSet* allBlocks = {}) {
  for (Code oldEnd{}; auto& i : blocks) {
    EXPECT_LT(oldEnd, i.start());
    oldEnd = i.end();
    if (allBlocks) EXPECT_TRUE(allBlocks->insert(&i).second);
  }
}

} // namespace

TEST(UnicodeBlockTest, UnicodeVersionStreamOperator) {
  std::stringstream s;
  s << UVer1_1;
  EXPECT_EQ(s.str(), "v1.1: 6, 1993");
  s.str("");
  s << UVer13_0;
  EXPECT_EQ(s.str(), "v13.0: 3, 2020");
}

TEST(UnicodeBlockTest, UnicodeBlockStreamOperator) {
  std::stringstream s;
  s << CommonKanjiBlocks[0];
  EXPECT_EQ(s.str(), "CJK Extension A (v3.0: 9, 1999)");
  s.str("");
  const auto noName{makeBlock<0x26A1>()};
  s << noName;
  EXPECT_EQ(s.str(), "start=26A1, end=26A1");
  // a block can't be created with a name, but no version
}

TEST(UnicodeBlockTest, CheckNoOverlappingBlocks) {
  BlockSet allBlocks;
  checkRange(HiraganaBlocks, &allBlocks);
  checkRange(KatakanaBlocks, &allBlocks);
  checkRange(PunctuationBlocks, &allBlocks);
  checkRange(SymbolBlocks, &allBlocks);
  checkRange(LetterBlocks, &allBlocks);
  checkRange(CommonKanjiBlocks, &allBlocks);
  checkRange(RareKanjiBlocks, &allBlocks);
  checkRange(NonSpacingBlocks, &allBlocks);
}

TEST(UnicodeBlockTest, CheckKanjiBlocks) {
  ASSERT_EQ(CommonKanjiBlocks.size(), 4);
  ASSERT_EQ(NonSpacingBlocks.size(), 1);
  ASSERT_EQ(RareKanjiBlocks.size(), 4);
  EXPECT_EQ(CommonKanjiBlocks[0].range(), 6592);
  EXPECT_EQ(CommonKanjiBlocks[1].range(), 20992);
  EXPECT_EQ(CommonKanjiBlocks[2].range(), 512);
  EXPECT_EQ(CommonKanjiBlocks[3].range(), 42720);
  EXPECT_EQ(RareKanjiBlocks[0].range(), 128);
  EXPECT_EQ(RareKanjiBlocks[1].range(), 17648);
  EXPECT_EQ(RareKanjiBlocks[2].range(), 544);
  EXPECT_EQ(RareKanjiBlocks[3].range(), 4944);
  EXPECT_EQ(NonSpacingBlocks[0].range(), 16);
}

TEST(UnicodeBlockTest, IsNonSpacing) {
  CodeString s{U"\x3078\x3099"}; // へ and dakuten combining mark
  EXPECT_EQ(s.size(), 2);
  EXPECT_FALSE(isNonSpacing(s[0]));
  EXPECT_TRUE(isNonSpacing(s[1]));
  s = U"\x3078\x309a"; // へ and han=dakuten combining mark
  EXPECT_EQ(s.size(), 2);
  EXPECT_FALSE(isNonSpacing(s[0]));
  EXPECT_TRUE(isNonSpacing(s[1]));
}

TEST(UnicodeBlockTest, IsKana) {
  EXPECT_TRUE(isHiragana("ゑ"));
  EXPECT_FALSE(isHiragana("ゑあ"));
  EXPECT_TRUE(isHiragana("ゑあ", false)); // sizeOne=false
  EXPECT_TRUE(isHiragana("ゑク", false)); // sizeOne=false
  EXPECT_TRUE(isAllHiragana("ゑあ"));
  EXPECT_FALSE(isAllHiragana("ゑク"));
  EXPECT_TRUE(isAllHiragana("ゑは" + CombiningVoiced + "あ"));
  EXPECT_TRUE(isAllKatakana("ヱハ" + CombiningSemiVoiced + "ア"));
  EXPECT_FALSE(isKatakana("ゑ"));
  EXPECT_TRUE(isKatakana("ヰ"));
  EXPECT_FALSE(isHiragana("ヰ"));
  EXPECT_TRUE(isRecognizedUtf8("ー"));
  EXPECT_TRUE(isRecognizedUtf8("さ"));
  EXPECT_TRUE(isKana("は"));
  EXPECT_TRUE(isKana("ハ"));
  EXPECT_FALSE(isKana("犬"));
  EXPECT_TRUE(isAllKana("あア"));
  EXPECT_FALSE(isAllKana("あaア"));
}

TEST(UnicodeBlockTest, IsMBLetter) {
  EXPECT_FALSE(isMBLetter("ー"));
  EXPECT_FALSE(isMBLetter("さ"));
  // Note: half-width katakana is included in Unicode wide letter area
  EXPECT_FALSE(isKatakana("ｶ"));
  EXPECT_TRUE(isMBLetter("ｶ"));
  EXPECT_FALSE(isMBLetter("ｶＺ"));
  EXPECT_TRUE(isMBLetter("ｶＺ", false)); // sizeOne=false
  EXPECT_TRUE(isAllMBLetter("ｶＺ"));
  EXPECT_FALSE(isAllMBLetter("ｶＺ犬"));
  // 'isMBLetter' also includes extended latin letters and enclosed letters
  EXPECT_TRUE(isMBLetter("ã"));
  EXPECT_TRUE(isMBLetter("⑦"));
  EXPECT_TRUE(isMBLetter("Ⅰ")); // Roman Numeral 'One'
  EXPECT_TRUE(isMBLetter("ｄ"));
  EXPECT_TRUE(isMBLetter("Ｚ"));
  EXPECT_TRUE(isMBLetter("１"));
  EXPECT_TRUE(isRecognizedUtf8("。"));
}

TEST(UnicodeBlockTest, IsMBPunctuation) {
  EXPECT_TRUE(isMBPunctuation("—"));  // from General Punctuation block
  EXPECT_TRUE(isMBPunctuation("。")); // from Wide Punctuation block
  EXPECT_FALSE(isMBPunctuation("。d"));
  EXPECT_TRUE(isMBPunctuation("。d", false, false)); // sizeOne=false
  EXPECT_TRUE(isMBPunctuation("、")); // from Wide Punctuation block
  EXPECT_TRUE(isMBPunctuation("　", true));
  EXPECT_FALSE(isMBPunctuation("　")); // includeSpace=false
  EXPECT_FALSE(isMBPunctuation("　x", true));
  EXPECT_TRUE(isMBPunctuation("　x", true, false)); // sizeOne=false
  EXPECT_FALSE(isAllMBPunctuation("　x"));
  EXPECT_TRUE(isAllMBPunctuation("　。　、"));
  EXPECT_TRUE(isMBPunctuation(toUtf8(U"\ufffc"))); // from Specials block
  EXPECT_TRUE(isRecognizedUtf8("—"));
  EXPECT_TRUE(isRecognizedUtf8("　"));
}

TEST(UnicodeBlockTest, IsMBSymbol) {
  EXPECT_TRUE(isMBSymbol("∀"));  // from Math Symbols block
  EXPECT_TRUE(isMBSymbol("☆"));  // from Misc Symbols block
  EXPECT_TRUE(isMBSymbol("○"));  // from Geometric Shapes block
  EXPECT_TRUE(isMBSymbol("⿱")); // CJK Ideographic Description Character
  EXPECT_TRUE(isMBSymbol("㆑")); // Kanbun (annotations)
  EXPECT_TRUE(isMBSymbol("㇁")); // CJK Stokes
  EXPECT_FALSE(isMBSymbol("㇁ぶ"));
  EXPECT_TRUE(isMBSymbol("㇁ぶ", false));
  EXPECT_FALSE(isAllMBSymbol("㇁ぶ"));
  EXPECT_TRUE(isAllMBSymbol("㇁☆"));
  EXPECT_FALSE(isMBSymbol("ｺ"));
  EXPECT_TRUE(isRecognizedUtf8("☆"));
}

TEST(UnicodeBlockTest, IsKanji) {
  // test common and rare kanji
  EXPECT_TRUE(isCommonKanji("厭")); // in Unified block
  EXPECT_TRUE(isCommonKanji("琢")); // in Compatibility block
  EXPECT_TRUE(isCommonKanji("𠮟")); // in Extension B (beyond BMP)
  EXPECT_FALSE(isCommonKanji("厭が"));
  EXPECT_TRUE(isCommonKanji("厭が", false));
  EXPECT_FALSE(isAllCommonKanji("厭が"));
  EXPECT_TRUE(isAllCommonKanji("厭猫"));
  EXPECT_FALSE(isRareKanji("厭"));
  EXPECT_FALSE(isCommonKanji("⺠"));
  EXPECT_FALSE(isCommonKanji("𫠜"));
  EXPECT_TRUE(isRareKanji("⺠"));
  EXPECT_TRUE(isRareKanji("輸")); // Compatibility Supplement
  EXPECT_FALSE(isRareKanji("⺠h"));
  EXPECT_TRUE(isRareKanji("⺠h", false)); // sizeOne=false
  EXPECT_FALSE(isAllRareKanji("⺠h"));
  EXPECT_FALSE(isAllRareKanji("⺠猫"));
  EXPECT_TRUE(isAllRareKanji("⺠𫠜"));
  EXPECT_TRUE(isRareKanji("𫠜"));
  EXPECT_TRUE(isKanji("厭"));
  EXPECT_TRUE(isKanji("⺠"));
  EXPECT_TRUE(isKanji("𫠜"));
  EXPECT_TRUE(isAllKanji("𫠜𠮟"));
  EXPECT_FALSE(isAllKanji("𫠜か𠮟"));
  EXPECT_TRUE(isRecognizedUtf8("厭"));
  EXPECT_TRUE(isRecognizedUtf8("⺠"));
  EXPECT_TRUE(isRecognizedUtf8("𫠜"));
  EXPECT_FALSE(isRecognizedUtf8("𫠜馬イヌねこ"));
  EXPECT_TRUE(isRecognizedUtf8("𫠜馬イヌねこ", false));
  EXPECT_TRUE(isAllRecognizedUtf8("𫠜馬イヌねこ"));
  EXPECT_FALSE(isAllRecognizedUtf8("𫠜馬イxヌねこ"));
}

} // namespace kanji_tools
