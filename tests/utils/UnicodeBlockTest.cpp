#include <gtest/gtest.h>
#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

namespace {

using BlockSet = std::set<const UnicodeBlock*>;

template<typename T>
void checkRange(const T& blocks, BlockSet* allBlocks = nullptr) {
  for (char32_t oldEnd = 0; auto& i : blocks) {
    EXPECT_LT(oldEnd, i.start);
    oldEnd = i.end;
    if (allBlocks) EXPECT_TRUE(allBlocks->insert(&i).second);
  }
}

} // namespace

TEST(UnicodeBlockTest, CheckNoOverlappingRanges) {
  BlockSet allBlocks;
  checkRange(HiraganaBlocks, &allBlocks);
  checkRange(KatakanaBlocks, &allBlocks);
  checkRange(PunctuationBlocks, &allBlocks);
  checkRange(SymbolBlocks, &allBlocks);
  checkRange(LetterBlocks, &allBlocks);
  checkRange(CommonKanjiBlocks, &allBlocks);
  checkRange(RareKanjiBlocks, &allBlocks);
  checkRange(NonSpacingBlocks, &allBlocks);
  // check 'range' strings (used in regex calls to remove furigana)
  ASSERT_EQ(CommonKanjiBlocks.size(), 4);
  ASSERT_EQ(RareKanjiBlocks.size(), 4);
  ASSERT_EQ(NonSpacingBlocks.size(), 1);
  // KanjiRange should include all the common and rare kanji + variant selectors
  // and a null terminator
  ASSERT_EQ(std::size(KanjiRange),
            (CommonKanjiBlocks.size() + RareKanjiBlocks.size() + 1) * 3 + 1);
  EXPECT_EQ(CommonKanjiBlocks[0].range(), 6592);
  EXPECT_EQ(CommonKanjiBlocks[1].range(), 20992);
  EXPECT_EQ(CommonKanjiBlocks[2].range(), 512);
  EXPECT_EQ(CommonKanjiBlocks[3].range(), 42720);
  EXPECT_EQ(RareKanjiBlocks[0].range(), 128);
  EXPECT_EQ(RareKanjiBlocks[1].range(), 17648);
  EXPECT_EQ(RareKanjiBlocks[2].range(), 544);
  EXPECT_EQ(RareKanjiBlocks[3].range(), 4944);
  EXPECT_EQ(NonSpacingBlocks[0].range(), 16);
  int pos = 0;
  auto checkKanjiRange = [&pos](auto& blocks) {
    for (auto& i : blocks) {
      EXPECT_EQ(KanjiRange[pos++], i.start) << pos;
      EXPECT_EQ(KanjiRange[pos++], U'-') << pos;
      EXPECT_EQ(KanjiRange[pos++], i.end) << pos;
    }
  };
  checkKanjiRange(CommonKanjiBlocks);
  checkKanjiRange(NonSpacingBlocks);
  checkKanjiRange(RareKanjiBlocks);
  EXPECT_EQ(KanjiRange[pos], U'\0');
  ASSERT_EQ(std::size(HiraganaRange), 4);
  ASSERT_EQ(HiraganaBlocks.size(), 1);
  EXPECT_EQ(HiraganaRange[0], HiraganaBlocks[0].start);
  EXPECT_EQ(HiraganaRange[2], HiraganaBlocks[0].end);
  ASSERT_EQ(std::size(KatakanaRange), 7);
  ASSERT_EQ(KatakanaBlocks.size(), 2);
  EXPECT_EQ(KatakanaRange[0], KatakanaBlocks[0].start);
  EXPECT_EQ(KatakanaRange[2], KatakanaBlocks[0].end);
  EXPECT_EQ(KatakanaRange[3], KatakanaBlocks[1].start);
  EXPECT_EQ(KatakanaRange[5], KatakanaBlocks[1].end);
  ASSERT_EQ(std::size(KanaRange), 7);
  EXPECT_EQ(KanaRange[0], HiraganaBlocks[0].start);
  // first katakana block immediately follows hiragana block so can use a bigger
  // range but check the assumption by comparing 'end + 1' to 'start'
  EXPECT_EQ(HiraganaBlocks[0].end + 1, KatakanaBlocks[0].start);
  EXPECT_EQ(KanaRange[2], KatakanaBlocks[0].end);
  EXPECT_EQ(KanaRange[3], KatakanaBlocks[1].start);
  EXPECT_EQ(KanaRange[5], KatakanaBlocks[1].end);
}

TEST(UnicodeBlockTest, IsNonSpacing) {
  auto s = std::u32string(U"\x3078\x3099"); // へ and dakuten combining mark
  EXPECT_EQ(s.size(), 2);
  EXPECT_FALSE(isNonSpacing(s[0]));
  EXPECT_TRUE(isNonSpacing(s[1]));
  s = std::u32string(U"\x3078\x309a"); // へ and han=dakuten combining mark
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
  EXPECT_FALSE(isKatakana("ゑ"));
  EXPECT_TRUE(isKatakana("ヰ"));
  EXPECT_FALSE(isHiragana("ヰ"));
  EXPECT_TRUE(isRecognizedCharacter("ー"));
  EXPECT_TRUE(isRecognizedCharacter("さ"));
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
  // 'isMBLetter' check also includes extended latin letters and enclosed
  // letters
  EXPECT_TRUE(isMBLetter("ã"));
  EXPECT_TRUE(isMBLetter("⑦"));
  EXPECT_TRUE(isMBLetter("Ⅰ")); // Roman Numeral 'One'
  EXPECT_TRUE(isMBLetter("ｄ"));
  EXPECT_TRUE(isMBLetter("Ｚ"));
  EXPECT_TRUE(isMBLetter("１"));
  EXPECT_TRUE(isRecognizedCharacter("。"));
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
  EXPECT_TRUE(isRecognizedCharacter("—"));
  EXPECT_TRUE(isRecognizedCharacter("　"));
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
  EXPECT_TRUE(isRecognizedCharacter("☆"));
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
  EXPECT_TRUE(isRecognizedCharacter("厭"));
  EXPECT_TRUE(isRecognizedCharacter("⺠"));
  EXPECT_TRUE(isRecognizedCharacter("𫠜"));
  EXPECT_FALSE(isRecognizedCharacter("𫠜馬イヌねこ"));
  EXPECT_TRUE(isRecognizedCharacter("𫠜馬イヌねこ", false));
  EXPECT_TRUE(isAllRecognizedCharacters("𫠜馬イヌねこ"));
  EXPECT_FALSE(isAllRecognizedCharacters("𫠜馬イxヌねこ"));
}

} // namespace kanji_tools
