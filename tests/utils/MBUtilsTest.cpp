#include <gtest/gtest.h>

#include <kanji_tools/utils/MBUtils.h>

namespace kanji_tools {

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
  EXPECT_EQ(toHex(L'\ufffc'), "0000fffc");
  auto s = toUtf8(L"\ufffc");
  ASSERT_EQ(s.length(), 3);
  EXPECT_EQ(toHex(s[0]), "ef");
  EXPECT_EQ(toHex(s[1]), "bf");
  EXPECT_EQ(toHex(s[2]), "bc");
  EXPECT_EQ(toHex(s[2], true), "BC");
  EXPECT_EQ(toHex(s[2], false, true), "[bc]");
  EXPECT_EQ(toHex(s[2], true, true), "[BC]");
  // test converting 'char' values to hex
  EXPECT_EQ(toHex('~'), "7e");
  char nullChar = 0x0, newline = '\n';
  EXPECT_EQ(toHex(nullChar), "00");
  EXPECT_EQ(toHex(nullChar, false, false, 0), "0");
  EXPECT_EQ(toHex(newline), "0a");
  EXPECT_EQ(toHex(newline, false, false, 0), "a");
}

TEST(MBUtilsTest, ToUnicode) {
  EXPECT_EQ(toUnicode('a'), "0061");
  EXPECT_EQ(toUnicode("ぁ"), "3041");
  EXPECT_EQ(toUnicode("ぁ", true), "[3041]");
  EXPECT_EQ(toUnicode("すずめ-雀"), "3059 305A 3081 002D 96C0");
  EXPECT_EQ(toUnicode("すずめ-雀", true), "[3059 305A 3081 002D 96C0]");
}

TEST(MBUtilsTest, ToBinary) {
  EXPECT_EQ(toBinary(L'\ufffc'), "00000000000000001111111111111100");
  EXPECT_EQ(toBinary(L'\ufffc', false), "1111111111111100");
  auto s = toUtf8(L"\ufffc");
  ASSERT_EQ(s.length(), 3);
  EXPECT_EQ(toBinary(s[0]), "11101111");
  EXPECT_EQ(toBinary(s[1]), "10111111");
  EXPECT_EQ(toBinary(s[2]), "10111100");
  // test converting 'char' values to binary
  EXPECT_EQ(toBinary('~'), "01111110");
  char nullChar = 0x0;
  EXPECT_EQ(toBinary(nullChar), "00000000");
  EXPECT_EQ(toBinary(nullChar, 2), "00");
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
