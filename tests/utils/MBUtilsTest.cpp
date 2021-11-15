#include <gtest/gtest.h>

#include <kanji_tools/utils/MBUtils.h>

namespace kanji_tools {

namespace {

using BlockSet = std::set<UnicodeBlock>;

template<typename T> void checkRange(const T& blocks, BlockSet* allBlocks = nullptr, bool officialBlock = true) {
  int oldEnd = 0;
  for (const auto& i : blocks) {
    EXPECT_LT(oldEnd, i.start);
    oldEnd = i.end;
    if (officialBlock) {
      EXPECT_LT(i.start, i.end);
      EXPECT_EQ(i.start % 16, 0); // Unicode blocks must start with a value having MOD 16 of zero
      EXPECT_EQ(i.end % 16, 15);  // Unicode blocks must end with a value having MOD 16 of 15 (hex f)
      if (allBlocks) EXPECT_TRUE(allBlocks->insert(i).second);
    } else
      // some 'WideBlocks' have only one value
      EXPECT_LE(i.start, i.end);
  }
}

} // namespace

TEST(MBUtilsTest, CheckNoOverlappingRanges) {
  BlockSet allBlocks;
  checkRange(HiraganaBlocks, &allBlocks);
  checkRange(KatakanaBlocks, &allBlocks);
  checkRange(PunctuationBlocks, &allBlocks);
  checkRange(SymbolBlocks, &allBlocks);
  checkRange(LetterBlocks, &allBlocks);
  checkRange(CommonKanjiBlocks, &allBlocks);
  checkRange(RareKanjiBlocks, &allBlocks);
  checkRange(NonSpacingBlocks, &allBlocks);
  checkRange(allBlocks);
  // make sure 'WideBlocks' (from generated code) has no overlaps
  checkRange(WideBlocks, nullptr, false);
  // check 'range' strings (used in regex calls to remove furigana)
  ASSERT_EQ(CommonKanjiBlocks.size(), 3);
  ASSERT_EQ(RareKanjiBlocks.size(), 5);
  ASSERT_EQ(NonSpacingBlocks.size(), 1);
  // KanjiRange should include all the common and rare kanji + variant selectors and a null terminator
  ASSERT_EQ(std::size(KanjiRange), (CommonKanjiBlocks.size() + RareKanjiBlocks.size() + 1) * 3 + 1);
  EXPECT_EQ(CommonKanjiBlocks[0].range(), 20992);
  EXPECT_EQ(CommonKanjiBlocks[1].range(), 512);
  EXPECT_EQ(CommonKanjiBlocks[2].range(), 42720);
  EXPECT_EQ(RareKanjiBlocks[0].range(), 128);
  EXPECT_EQ(RareKanjiBlocks[1].range(), 6592);
  EXPECT_EQ(RareKanjiBlocks[2].range(), 17648);
  EXPECT_EQ(RareKanjiBlocks[3].range(), 544);
  EXPECT_EQ(NonSpacingBlocks[0].range(), 16);
  int pos = 0;
  auto checkKanjiRange = [&pos](auto& blocks) {
    for (auto& i : blocks) {
      EXPECT_EQ(KanjiRange[pos++], i.start) << pos;
      EXPECT_EQ(KanjiRange[pos++], L'-') << pos;
      EXPECT_EQ(KanjiRange[pos++], i.end) << pos;
    }
  };
  checkKanjiRange(CommonKanjiBlocks);
  checkKanjiRange(NonSpacingBlocks);
  checkKanjiRange(RareKanjiBlocks);
  EXPECT_EQ(KanjiRange[pos], L'\0');
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
  // first katakana block immediately follows hiragana block so can use a bigger range
  // but check the assumption by comparing 'end + 1' to 'start'
  EXPECT_EQ(HiraganaBlocks[0].end + 1, KatakanaBlocks[0].start);
  EXPECT_EQ(KanaRange[2], KatakanaBlocks[0].end);
  EXPECT_EQ(KanaRange[3], KatakanaBlocks[1].start);
  EXPECT_EQ(KanaRange[5], KatakanaBlocks[1].end);
}

TEST(MBUtilsTest, IsKana) {
  EXPECT_TRUE(isHiragana("ゑ"));
  EXPECT_FALSE(isHiragana("ゑあ"));
  EXPECT_TRUE(isHiragana("ゑあ", false)); // checkLengthOne=false
  EXPECT_TRUE(isHiragana("ゑク", false)); // checkLengthOne=false
  EXPECT_TRUE(isAllHiragana("ゑあ"));
  EXPECT_FALSE(isAllHiragana("ゑク"));
  EXPECT_FALSE(isKatakana("ゑ"));
  EXPECT_TRUE(isKatakana("ヰ"));
  EXPECT_FALSE(isHiragana("ヰ"));
  EXPECT_TRUE(isRecognizedCharacter("ー"));
  EXPECT_TRUE(isRecognizedCharacter("さ"));
}

TEST(MBUtilsTest, IsMBLetter) {
  EXPECT_FALSE(isMBLetter("ー"));
  EXPECT_FALSE(isMBLetter("さ"));
  // Note: half-width katakana is included in Unicode wide letter area
  EXPECT_FALSE(isKatakana("ｶ"));
  EXPECT_TRUE(isMBLetter("ｶ"));
  EXPECT_FALSE(isMBLetter("ｶＺ"));
  EXPECT_TRUE(isMBLetter("ｶＺ", false)); // checkLengthOne=false
  EXPECT_TRUE(isAllMBLetter("ｶＺ"));
  EXPECT_FALSE(isAllMBLetter("ｶＺ犬"));
  // 'isMBLetter' check also includes extended latin letters and enclosed letters
  EXPECT_TRUE(isMBLetter("ã"));
  EXPECT_TRUE(isMBLetter("⑦"));
  EXPECT_TRUE(isMBLetter("Ⅰ")); // Roman Numeral 'One'
  EXPECT_TRUE(isMBLetter("ｄ"));
  EXPECT_TRUE(isMBLetter("Ｚ"));
  EXPECT_TRUE(isMBLetter("１"));
  EXPECT_TRUE(isRecognizedCharacter("。"));
}

TEST(MBUtilsTest, IsMBPunctuation) {
  EXPECT_TRUE(isMBPunctuation("—"));  // from General Punctuation block
  EXPECT_TRUE(isMBPunctuation("。")); // from Wide Punctuation block
  EXPECT_FALSE(isMBPunctuation("。d"));
  EXPECT_TRUE(isMBPunctuation("。d", true, false)); // checkLengthOne=false
  EXPECT_TRUE(isMBPunctuation("、"));               // from Wide Punctuation block
  EXPECT_TRUE(isMBPunctuation("　"));
  EXPECT_FALSE(isMBPunctuation("　", false)); // includeSpace=false
  EXPECT_FALSE(isMBPunctuation("　x", true));
  EXPECT_TRUE(isMBPunctuation("　x", true, false)); // checkLengthOne=false
  EXPECT_FALSE(isAllMBPunctuation("　x"));
  EXPECT_TRUE(isAllMBPunctuation("　。　、"));
  EXPECT_TRUE(isMBPunctuation(toUtf8(L"\ufffc"))); // from Specials block
  EXPECT_TRUE(isRecognizedCharacter("—"));
  EXPECT_TRUE(isRecognizedCharacter("　"));
}

TEST(MBUtilsTest, IsMBSymbol) {
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

TEST(MBUtilsTest, IsKanji) {
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
  EXPECT_FALSE(isCommonKanji("㐀"));
  EXPECT_TRUE(isRareKanji("⺠"));
  EXPECT_TRUE(isRareKanji("輸")); // Compatibility Supplement
  EXPECT_FALSE(isRareKanji("⺠h"));
  EXPECT_TRUE(isRareKanji("⺠h", false)); // checkLengthOne=false
  EXPECT_FALSE(isAllRareKanji("⺠h"));
  EXPECT_FALSE(isAllRareKanji("⺠猫"));
  EXPECT_TRUE(isAllRareKanji("⺠㐀"));
  EXPECT_TRUE(isRareKanji("㐀"));
  EXPECT_TRUE(isKanji("厭"));
  EXPECT_TRUE(isKanji("⺠"));
  EXPECT_TRUE(isKanji("㐀"));
  EXPECT_TRUE(isRecognizedCharacter("厭"));
  EXPECT_TRUE(isRecognizedCharacter("⺠"));
  EXPECT_TRUE(isRecognizedCharacter("㐀"));
  EXPECT_FALSE(isRecognizedCharacter("㐀馬イヌねこ"));
  EXPECT_TRUE(isRecognizedCharacter("㐀馬イヌねこ", false));
  EXPECT_TRUE(isAllRecognizedCharacters("㐀馬イヌねこ"));
  EXPECT_FALSE(isAllRecognizedCharacters("㐀馬イxヌねこ"));
}

TEST(MBUtilsTest, FromUTF8String) {
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

TEST(MBUtilsTest, FromUTF8CharArray) {
  const char s[] = {'\xef', '\xbf', '\xbc', 0};
  auto w = fromUtf8(s);
  ASSERT_EQ(w.length(), 1);
  EXPECT_EQ(w[0], L'\ufffc');
  auto r = toUtf8(w);
  ASSERT_EQ(r.length(), std::size(s) - 1);
  for (int i = 0; i < std::size(s) - 1; ++i)
    EXPECT_EQ(r[i], s[i]);
}

TEST(MBUtilsTest, ToHex) {
  EXPECT_EQ(toHex(L'\ufffc'), "fffc");
  auto s = toUtf8(L"\ufffc");
  ASSERT_EQ(s.length(), 3);
  EXPECT_EQ(toHex(s[0]), "ef");
  EXPECT_EQ(toHex(s[1]), "bf");
  EXPECT_EQ(toHex(s[2]), "bc");
  EXPECT_EQ(toHex(s[2], true), "BC");
  EXPECT_EQ(toHex(s[2], false, true), "[bc]");
  EXPECT_EQ(toHex(s[2], true, true), "[BC]");
}

TEST(MBUtilsTest, ToUnicode) {
  EXPECT_EQ(toUnicode("ぁ"), "3041");
  EXPECT_EQ(toUnicode("すずめ"), "3059 305A 3081");
}

TEST(MBUtilsTest, ToBinary) {
  EXPECT_EQ(toBinary(L'\ufffc'), "1111111111111100");
  auto s = toUtf8(L"\ufffc");
  ASSERT_EQ(s.length(), 3);
  EXPECT_EQ(toBinary(s[0]), "11101111");
  EXPECT_EQ(toBinary(s[1]), "10111111");
  EXPECT_EQ(toBinary(s[2]), "10111100");
}

TEST(MBUtilsTest, CheckSingleByte) {
  // normal char
  EXPECT_TRUE(isSingleByteChar('a'));
  EXPECT_FALSE(isSingleByteChar('\x80'));
  // wide char
  EXPECT_TRUE(isSingleByteChar(L'a'));
  EXPECT_FALSE(isSingleByteChar(L'か'));
  // normal string
  EXPECT_TRUE(isSingleByte("x"));
  EXPECT_FALSE(isSingleByte("く"));
  EXPECT_FALSE(isSingleByte("xx"));
  EXPECT_TRUE(isSingleByte("xx", false));
  EXPECT_TRUE(isAllSingleByte("xx"));
  EXPECT_FALSE(isAllSingleByte("xxこ"));
  EXPECT_TRUE(isAnySingleByte("xxこ"));
  EXPECT_FALSE(isAnySingleByte("こ"));
  // wide string
  EXPECT_TRUE(isSingleByte(L"x"));
  EXPECT_FALSE(isSingleByte(L"く"));
  EXPECT_FALSE(isSingleByte(L"xx"));
  EXPECT_TRUE(isSingleByte(L"xx", false));
  EXPECT_TRUE(isAllSingleByte(L"")); // true for empty strings
  EXPECT_TRUE(isAllSingleByte(L"xx"));
  EXPECT_FALSE(isAllSingleByte(L"xxこ"));
  EXPECT_TRUE(isAnySingleByte(L"xxこ"));
  EXPECT_FALSE(isAnySingleByte(L"こ"));
}

TEST(MBUtilsTest, DisplayLength) {
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

TEST(MBUtilsTest, WideSetw) {
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

TEST(MBUtilsTest, SortKatakana) {
  std::set<std::string> s{"ケン、トウ", "カ", "カ、サ", "ガ", "ゲン、カン"};
  ASSERT_EQ(s.size(), 5);
  auto i = s.begin();
  EXPECT_EQ(*i++, "カ");
  // The following two entries should be reversed, i.e., "ガ" then "カ、サ" - works fine with bash 'sort'.
  // Later maybe try using https://github.com/unicode-org/icu collate functions.
  EXPECT_EQ(*i++, "カ、サ");
  EXPECT_EQ(*i++, "ガ");
  EXPECT_EQ(*i++, "ケン、トウ");
  EXPECT_EQ(*i++, "ゲン、カン");
  EXPECT_EQ(i, s.end());
}

TEST(MBUtilsTest, SortKanaAndRomaji) {
  // Default sort order for Japanese Kana and Rōmaji seems to be:
  // - Rōmaji: normal latin letters
  // - Hiragana: in Unicode order so しょう (incorrectly) comes before じょ
  // - Katakana: should mix with Hiragana instead of always coming after
  // - Full-width Rōmaji: should probably come before Kana
  // - Half-width Katakana: should mix with other Kana instead
  std::set<std::string> s{"しょう", "Ｐａｒａ", "はら", "ﾊﾗ",   "バラ",    "ばら",
                          "ぱら",   "para",     "じょ", "しょ", "ｐａｒａ"};
  ASSERT_EQ(s.size(), 11);
  auto i = s.begin();
  EXPECT_EQ(*i++, "para");
  EXPECT_EQ(*i++, "しょ");
  EXPECT_EQ(*i++, "しょう");
  EXPECT_EQ(*i++, "じょ");
  EXPECT_EQ(*i++, "はら");
  EXPECT_EQ(*i++, "ばら");
  EXPECT_EQ(*i++, "ぱら");
  EXPECT_EQ(*i++, "バラ");
  EXPECT_EQ(*i++, "Ｐａｒａ");
  EXPECT_EQ(*i++, "ｐａｒａ");
  EXPECT_EQ(*i++, "ﾊﾗ");
  EXPECT_EQ(i, s.end());
}

TEST(MBUtilsTest, SortKanji) {
  // Kanji sort order seems to follow Unicode code points instead of 'radical/stroke' ordering.
  // Calling std::setlocale with values like ja_JP or ja_JP.UTF-8 doesn't make any difference.
  std::set<std::string> s{"些", "丑", "云", "丞", "乃", "𠮟", "廿", "⺠", "輸", "鳩"};
  ASSERT_EQ(s.size(), 10);
  auto i = s.begin();
  EXPECT_EQ(toUnicode(*i), "2EA0"); // Rare Kanji (Radical Supplement)
  EXPECT_EQ(*i++, "⺠");
  EXPECT_EQ(toUnicode(*i), "4E11"); // Common Kanji with radical 1 (一), strokes 4 (1+3)
  EXPECT_EQ(*i++, "丑");
  EXPECT_EQ(toUnicode(*i), "4E1E"); // Common Kanji with radical 1 (一), strokes 6 (1+5)
  EXPECT_EQ(*i++, "丞");
  EXPECT_EQ(toUnicode(*i), "4E43"); // Common Kanji with radical 4 (丿), strokes 2 (1+1)
  EXPECT_EQ(*i++, "乃");
  EXPECT_EQ(toUnicode(*i), "4E91"); // Common Kanji with radical 7 (二), strokes 4 (2+2)
  EXPECT_EQ(*i++, "云");
  EXPECT_EQ(toUnicode(*i), "4E9B"); // Common Kanji with radical 7 (二), strokes 7 (2+5)
  EXPECT_EQ(*i++, "些");
  // 5EFF is a Common Kanji (Jinmei) with radical 55 (廾), strokes 4 (3+1), but it can also
  // be classified as having radical 24 (十) with strokes 4 (2+2)
  EXPECT_EQ(toUnicode(*i), "5EFF");
  EXPECT_EQ(*i++, "廿");
  EXPECT_EQ(toUnicode(*i), "9CE9"); // Common kanji with radical 196 (鳥), strokes 13 (11+2)
  EXPECT_EQ(*i++, "鳩");
  // 20B9F is a Common Kanji (in Extension B) with radical 30 (口), strokes 5 (2+3) which would
  // normally come before the previous two Kanji in the set since it has radical 30.
  EXPECT_EQ(toUnicode(*i), "20B9F");
  EXPECT_EQ(*i++, "𠮟");
  // 2F9DF is a Rare Kanji with radical 159 (車), strokes 16 (7+9) which would come before
  // before '9CE9' if sorting was based on radical numbers.
  EXPECT_EQ(toUnicode(*i), "2F9DF");
  EXPECT_EQ(*i++, "輸");
  EXPECT_EQ(i, s.end());
}

} // namespace kanji_tools
