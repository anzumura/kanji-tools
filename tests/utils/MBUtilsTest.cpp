#include <gtest/gtest.h>
#include <kanji_tools/utils/MBUtils.h>

namespace kanji_tools {

void fromUtf8Error(const std::string& s, const std::wstring& result = L"\ufffd") {
  try {
    EXPECT_EQ(fromUtf8(s), result);
#ifdef USE_CODECVT_FOR_UTF_8
    FAIL() << "Expected std::range_error";
#endif
  } catch (std::range_error& err) {
    EXPECT_EQ(err.what(), std::string("wstring_convert: from_bytes error"));
  } catch (...) {
    FAIL() << "Expected std::range_error";
  }
}

void toUtf8Error(const std::wstring& s, const std::string& result = "\xEF\xBF\xBD") {
  try {
    EXPECT_EQ(toUtf8(s), result);
#ifdef USE_CODECVT_FOR_UTF_8
    FAIL() << "Expected std::range_error";
#endif
  } catch (std::range_error& err) {
    EXPECT_EQ(err.what(), std::string("wstring_convert: to_bytes error"));
  } catch (...) {
    FAIL() << "Expected std::range_error";
  }
}

TEST(MBUtilsTest, FromUTF8String) {
  auto wideSingle = fromUtf8("single");
  ASSERT_EQ(wideSingle, L"single");
  // first byte error cases
  fromUtf8Error(std::string({static_cast<char>(Bit1)}));
  fromUtf8Error(std::string({static_cast<char>(FiveBits)}));
  // second byte not continuation
  fromUtf8Error(std::string({static_cast<char>(TwoBits), 'a'}), L"\ufffda");
  const char cont = static_cast<char>(Bit1);
  // third byte not continuation
  fromUtf8Error(std::string({static_cast<char>(ThreeBits), cont, 'a'}), L"\ufffda");
  // fourth byte not continuation
  fromUtf8Error(std::string({static_cast<char>(FourBits), cont, cont, 'a'}), L"\ufffda");
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

// see similar tests in MBCharTest.cpp
TEST(MBUtilsTest, BeyondMaxUnicode) {
  const char firstByte = static_cast<char>(0b11'11'01'00);
  const auto okS = std::string(
    {firstByte, static_cast<char>(0b10'00'11'11), static_cast<char>(0b10'11'11'11), static_cast<char>(0b10'11'11'11)});
  const auto badS =
    std::string({firstByte, static_cast<char>(0b10'01'00'00), static_cast<char>(Bit1), static_cast<char>(Bit1)});
  // from UTF-8
  EXPECT_EQ(fromUtf8(okS), L"\x10ffff");
  fromUtf8Error(badS);
  // to UTF-8
  EXPECT_EQ(toUtf8(L'\x10ffff'), "\xF4\x8F\xBF\xBF");
  toUtf8Error(L"\x110000");
}

TEST(MBUtilsTest, InvalidSurrogateRange) {
  const auto beforeRange = std::string({'\xED', '\x9F', '\xBF'}); // U+D7FF
  const auto rangeStart = std::string({'\xED', '\xA0', '\x80'});  // U+D800
  const auto rangeEnd = std::string({'\xED', '\xBF', '\xBF'});    // U+DFFF
  const auto afterRange = std::string({'\xEE', '\x80', '\x80'});  // U+E000
  // from UTF-8
  EXPECT_EQ(fromUtf8(beforeRange), L"\ud7ff");
  fromUtf8Error(rangeStart);
  fromUtf8Error(rangeEnd);
  EXPECT_EQ(fromUtf8(afterRange), L"\ue000");
  // to UTF-8
  EXPECT_EQ(toUtf8(L"\ud7ff"), beforeRange);
  toUtf8Error(L"\xd800");
  toUtf8Error(L"\xdfff");
  EXPECT_EQ(toUtf8(L"\ue000"), afterRange);
}

// see similar tests in MBCharTest.cpp (NotValidForOverlong)
TEST(MBUtilsTest, ErrorForOverlong) {
  // overlong single byte ascii
  const unsigned char bang = 33;
  EXPECT_EQ(toBinary(bang), "00100001"); // decimal 33 which is ascii '!'
  fromUtf8Error(std::string({static_cast<char>(TwoBits), static_cast<char>(Bit1 | bang)}), L"\ufffd");
  // overlong ō with 3 bytes
  std::string overlongO(
    {static_cast<char>(ThreeBits), static_cast<char>(Bit1 | 0b101), static_cast<char>(Bit1 | 0b1101)});
  fromUtf8Error(overlongO, L"\ufffd");
  // overlong Euro symbol with 4 bytes
  std::string x("\xF0\x82\x82\xAC");
  fromUtf8Error(x, L"\ufffd");
}

TEST(MBUtilsTest, FromUTF8CharArray) {
  const char s[] = {'\xef', '\xbf', '\xbc', 0};
  auto w = fromUtf8(s);
  ASSERT_EQ(w.length(), 1);
  EXPECT_EQ(w[0], L'\ufffc');
  auto r = toUtf8(w);
  ASSERT_EQ(r.length(), std::size(s) - 1);
  for (size_t i = 0; i < std::size(s) - 1; ++i) EXPECT_EQ(r[i], s[i]);
}

TEST(MBUtilsTest, ToHex) {
  EXPECT_EQ(toHex(L'\ufffc'), "0000fffc");
  auto s = toUtf8(L"\ufffc");
  ASSERT_EQ(s.length(), 3);
  EXPECT_EQ(toHex(s[0]), "ef");
  EXPECT_EQ(toHex(s[1]), "bf");
  EXPECT_EQ(toHex(s[2]), "bc");
  EXPECT_EQ(toHex(s[2], HexCase::Upper), "BC");
  EXPECT_EQ(toHex(s[2], BracketType::Curly), "{bc}");
  EXPECT_EQ(toHex(s[2], BracketType::Round), "(bc)");
  EXPECT_EQ(toHex(s[2], BracketType::Square), "[bc]");
  EXPECT_EQ(toHex(s[2], BracketType::Square, HexCase::Upper), "[BC]");
  // test converting 'char' values to hex
  EXPECT_EQ(toHex('~'), "7e");
  char nullChar = 0x0, newline = '\n';
  EXPECT_EQ(toHex(nullChar), "00");
  EXPECT_EQ(toHex(nullChar, 1), "0");
  EXPECT_EQ(toHex(newline), "0a");
  EXPECT_EQ(toHex(newline, 1), "a");
}

TEST(MBUtilsTest, ToUnicode) {
  EXPECT_EQ(toUnicode('a'), "0061");
  EXPECT_EQ(toUnicode("ぁ"), "3041");
  EXPECT_EQ(toUnicode("ぁ", BracketType::Square), "[3041]");
  EXPECT_EQ(toUnicode("すずめ-雀"), "3059 305A 3081 002D 96C0");
  EXPECT_EQ(toUnicode("すずめ-雀", BracketType::Square), "[3059 305A 3081 002D 96C0]");
}

TEST(MBUtilsTest, ToBinary) {
  EXPECT_EQ(toBinary(L'\ufffc'), "00000000000000001111111111111100");
  EXPECT_EQ(toBinary(L'\ufffc', 1), "1111111111111100");
  EXPECT_EQ(toBinary(L'\ufffc', BracketType::Square, 1), "[1111111111111100]");
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
