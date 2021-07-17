#include <gtest/gtest.h>

#include <kanji/MBUtils.h>

namespace kanji {

namespace {

using BlockSet = std::set<UnicodeBlock>;

template<typename T> void checkRange(const T& blocks, BlockSet* allBlocks = nullptr) {
  int oldEnd = 0;
  for (const auto& i : blocks) {
    EXPECT_LT(oldEnd, i.start);
    EXPECT_LT(i.start, i.end);
    oldEnd = i.end;
    if (allBlocks) EXPECT_TRUE(allBlocks->insert(i).second);
  }
}

} // namespace

TEST(MBUtils, CheckNoOverlappingRanges) {
  BlockSet allBlocks;
  checkRange(HiraganaBlocks, &allBlocks);
  checkRange(KatakanaBlocks, &allBlocks);
  checkRange(PunctuationBlocks, &allBlocks);
  checkRange(SymbolBlocks, &allBlocks);
  checkRange(LetterBlocks, &allBlocks);
  checkRange(CommonKanjiBlocks, &allBlocks);
  checkRange(RareKanjiBlocks, &allBlocks);
  checkRange(allBlocks);
  // check 'range' strings (used in regex calls to remove furigana)
  ASSERT_EQ(std::size(KanjiRange), 10);
  ASSERT_EQ(CommonKanjiBlocks.size(), 1);
  ASSERT_EQ(RareKanjiBlocks.size(), 2);
  EXPECT_EQ(CommonKanjiBlocks[0].range(), 20989);
  EXPECT_EQ(RareKanjiBlocks[0].range(), 128);
  EXPECT_EQ(RareKanjiBlocks[1].range(), 6592);
  EXPECT_EQ(KanjiRange[0], RareKanjiBlocks[0].start);
  EXPECT_EQ(KanjiRange[2], RareKanjiBlocks[0].end);
  EXPECT_EQ(KanjiRange[3], RareKanjiBlocks[1].start);
  EXPECT_EQ(KanjiRange[5], RareKanjiBlocks[1].end);
  EXPECT_EQ(KanjiRange[6], CommonKanjiBlocks[0].start);
  EXPECT_EQ(KanjiRange[8], CommonKanjiBlocks[0].end);
  ASSERT_EQ(std::size(HiraganaRange), 4);
  ASSERT_EQ(HiraganaBlocks.size(), 1);
  EXPECT_EQ(HiraganaRange[0], HiraganaBlocks[0].start);
  EXPECT_EQ(HiraganaRange[2], HiraganaBlocks[0].end);
}

TEST(MBUtils, IsKana) {
  EXPECT_TRUE(isHiragana("ゑ"));
  EXPECT_FALSE(isKatakana("ゑ"));
  EXPECT_TRUE(isKatakana("ヰ"));
  EXPECT_FALSE(isHiragana("ヰ"));
  EXPECT_TRUE(isRecognizedMB("ー"));
  EXPECT_TRUE(isRecognizedMB("さ"));
}

TEST(MBUtils, IsMBLetter) {
  EXPECT_FALSE(isMBLetter("ー"));
  EXPECT_FALSE(isMBLetter("さ"));
  // Note: half-width katakana is included in Unicode wide letter area
  EXPECT_FALSE(isKatakana("ｶ"));
  EXPECT_TRUE(isMBLetter("ｶ"));
  // 'isMBLetter' check also includes extended latin letters and enclosed letters
  EXPECT_TRUE(isMBLetter("ã"));
  EXPECT_TRUE(isMBLetter("⑦"));
  EXPECT_TRUE(isMBLetter("Ⅰ")); // Roman Numeral 'One'
  EXPECT_TRUE(isMBLetter("ｄ"));
  EXPECT_TRUE(isMBLetter("Ｚ"));
  EXPECT_TRUE(isMBLetter("１"));
  EXPECT_TRUE(isRecognizedMB("。"));
}

TEST(MBUtils, IsMBPunctuation) {
  EXPECT_TRUE(isMBPunctuation("—")); // from General Punctuation block
  EXPECT_TRUE(isMBPunctuation("。")); // from Wide Punctuation block
  EXPECT_TRUE(isMBPunctuation("、")); // from Wide Punctuation block
  EXPECT_TRUE(isMBPunctuation("　"));
  EXPECT_TRUE(isMBPunctuation(toUtf8(L"\ufffc"))); // from Specials block
  EXPECT_TRUE(isRecognizedMB("—"));
  EXPECT_TRUE(isRecognizedMB("　"));
}

TEST(MBUtils, IsMBSymbol) {
  EXPECT_TRUE(isMBSymbol("∀")); // from Math Symbols block
  EXPECT_TRUE(isMBSymbol("☆")); // from Misc Symbols block
  EXPECT_TRUE(isMBSymbol("○")); // from Geometric Shapes block
  EXPECT_TRUE(isMBSymbol("⿱")); // CJK Ideographic Description Character
  EXPECT_TRUE(isMBSymbol("㆑")); // Kanbun (annotations)
  EXPECT_TRUE(isMBSymbol("㇁")); // CJK Stokes
  EXPECT_FALSE(isMBSymbol("ｺ"));
  EXPECT_TRUE(isRecognizedMB("☆"));
}

TEST(MBUtils, IsKanji) {
  // test common and rare kanji
  EXPECT_TRUE(isCommonKanji("厭"));
  EXPECT_FALSE(isRareKanji("厭"));
  EXPECT_FALSE(isCommonKanji("⺠"));
  EXPECT_FALSE(isCommonKanji("㐀"));
  EXPECT_TRUE(isRareKanji("⺠"));
  EXPECT_TRUE(isRareKanji("㐀"));
  EXPECT_TRUE(isKanji("厭"));
  EXPECT_TRUE(isKanji("⺠"));
  EXPECT_TRUE(isKanji("㐀"));
  EXPECT_TRUE(isRecognizedMB("厭"));
  EXPECT_TRUE(isRecognizedMB("⺠"));
  EXPECT_TRUE(isRecognizedMB("㐀"));
}

TEST(MBUtils, FromUTF8String) {
  std::string dog("犬");
  auto wideDog = fromUtf8(dog);
  ASSERT_EQ(dog.length(), 3);
  EXPECT_EQ(dog[0], '\xe7');
  EXPECT_EQ(dog[1], '\x8a');
  EXPECT_EQ(dog[2], '\xac');
  ASSERT_EQ(wideDog.length(), 1);
  EXPECT_EQ(wideDog[0], L'\u72ac');
  auto newDog = toUtf8(wideDog);
  EXPECT_EQ(dog, newDog);
}

TEST(MBUtils, FromUTF8CharArray) {
  const char s[] = {'\xef', '\xbf', '\xbc', 0};
  auto w = fromUtf8(s);
  ASSERT_EQ(w.length(), 1);
  EXPECT_EQ(w[0], L'\ufffc');
  auto r = toUtf8(w);
  ASSERT_EQ(r.length(), std::size(s) - 1);
  for (int i = 0; i < std::size(s) - 1; ++i)
    EXPECT_EQ(r[i], s[i]);
}

TEST(MBUtils, ToHex) {
  EXPECT_EQ(toHex(L'\ufffc'), "fffc");
  auto s = toUtf8(L"\ufffc");
  ASSERT_EQ(s.length(), 3);
  EXPECT_EQ(toHex(s[0]), "ef");
  EXPECT_EQ(toHex(s[1]), "bf");
  EXPECT_EQ(toHex(s[2]), "bc");
}

TEST(MBUtils, ToBinary) {
  EXPECT_EQ(toBinary(L'\ufffc'), "1111111111111100");
  auto s = toUtf8(L"\ufffc");
  ASSERT_EQ(s.length(), 3);
  EXPECT_EQ(toBinary(s[0]), "11101111");
  EXPECT_EQ(toBinary(s[1]), "10111111");
  EXPECT_EQ(toBinary(s[2]), "10111100");
}

} // namespace kanji
