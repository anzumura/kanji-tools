#include <gtest/gtest.h>
#include <kanji_tools/utils/MBUtils.h>
#include <kanji_tools/utils/Utils.h>

namespace kanji_tools {

namespace {

const std::string LowerString{"aBcD"}, UpperString{"EfGh"}, MBString{"雪sNow"};

} // namespace

TEST(UtilsTest, AddLeadingZeroes) {
  // addLeadingZeroes returns '0' when given an empty string, otherwise it pads
  // the string with zeroes if size is less than 'minSize' (the second param)
  EXPECT_EQ(addLeadingZeroes("", 0), "0");
  EXPECT_EQ(addLeadingZeroes("", 1), "0");
  EXPECT_EQ(addLeadingZeroes("", 2), "00");
  EXPECT_EQ(addLeadingZeroes("abc", 5), "00abc");
  EXPECT_EQ(addLeadingZeroes("abc", 3), "abc");
  EXPECT_EQ(addLeadingZeroes("abc", 2), "abc");
}

TEST(UtilsTest, U32AddLeadingZeroes) {
  // addLeadingZeroes returns '0' when given an empty string, otherwise it pads
  // the string with zeroes if size is less than 'minSize' (the second param)
  EXPECT_EQ(addLeadingZeroes(U"", 0), U"0");
  EXPECT_EQ(addLeadingZeroes(U"", 1), U"0");
  EXPECT_EQ(addLeadingZeroes(U"", 2), U"00");
  EXPECT_EQ(addLeadingZeroes(U"abc", 5), U"00abc");
  EXPECT_EQ(addLeadingZeroes(U"abc", 3), U"abc");
  EXPECT_EQ(addLeadingZeroes(U"abc", 2), U"abc");
}

TEST(UtilsTest, ToUnicode) {
  EXPECT_EQ(toUnicode('a'), "0061");
  EXPECT_EQ(toUnicode("ぁ"), "3041");
  EXPECT_EQ(toUnicode("ぁ", BracketType::Square), "[3041]");
  EXPECT_EQ(toUnicode("すずめ-雀"), "3059 305A 3081 002D 96C0");
  EXPECT_EQ(toUnicode("すずめ-雀", BracketType::Square),
      "[3059 305A 3081 002D 96C0]");
}

TEST(UtilsTest, U32ToUnicode) {
  EXPECT_EQ(toUnicode(U'a'), "0061");
  EXPECT_EQ(toUnicode(U"ぁ"), "3041");
  EXPECT_EQ(toUnicode(U"ぁ", BracketType::Square), "[3041]");
  EXPECT_EQ(toUnicode(U"すずめ-雀"), "3059 305A 3081 002D 96C0");
  EXPECT_EQ(toUnicode(U"すずめ-雀", BracketType::Square),
      "[3059 305A 3081 002D 96C0]");
}

TEST(UtilsTest, ToHex) {
  EXPECT_EQ(toHex(U'\ufffc'), "0000fffc");
  const auto s{toUtf8(U"\ufffc")};
  ASSERT_EQ(s.size(), 3);
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
  const char nullChar{0x0}, newline{'\n'};
  EXPECT_EQ(toHex(nullChar), "00");
  EXPECT_EQ(toHex(nullChar, 1), "0");
  EXPECT_EQ(toHex(newline), "0a");
  EXPECT_EQ(toHex(newline, 1), "a");
}

TEST(UtilsTest, ToBinary) {
  EXPECT_EQ(toBinary(U'\ufffc'), "00000000000000001111111111111100");
  EXPECT_EQ(toBinary(U'\ufffc', 1), "1111111111111100");
  EXPECT_EQ(toBinary(U'\ufffc', BracketType::Square, 1), "[1111111111111100]");
  const auto s{toUtf8(U"\ufffc")};
  ASSERT_EQ(s.size(), 3);
  EXPECT_EQ(toBinary(s[0]), "11101111");
  EXPECT_EQ(toBinary(s[1]), "10111111");
  EXPECT_EQ(toBinary(s[2]), "10111100");
  // test converting 'char' values to binary
  EXPECT_EQ(toBinary('~'), "01111110");
  const char nullChar{0x0};
  EXPECT_EQ(toBinary(nullChar), "00000000");
  EXPECT_EQ(toBinary(nullChar, 2), "00");
}

TEST(UtilsTest, CheckSingleByte) {
  // normal char
  EXPECT_TRUE(isSingleByteChar('a'));
  EXPECT_FALSE(isSingleByteChar('\x80'));
  // wide char
  EXPECT_TRUE(isSingleByteChar(U'a'));
  EXPECT_FALSE(isSingleByteChar(U'か'));
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
  EXPECT_TRUE(isSingleByte(U"x"));
  EXPECT_FALSE(isSingleByte(U"く"));
  EXPECT_FALSE(isSingleByte(U"xx"));
  EXPECT_TRUE(isSingleByte(U"xx", false));
  EXPECT_TRUE(isAllSingleByte(U"")); // true for empty strings
  EXPECT_TRUE(isAllSingleByte(U"xx"));
  EXPECT_FALSE(isAllSingleByte(U"xxこ"));
  EXPECT_TRUE(isAnySingleByte(U"xxこ"));
  EXPECT_FALSE(isAnySingleByte(U"こ"));
}

TEST(UtilsTest, FirstLower) {
  EXPECT_EQ(firstLower(EmptyString), EmptyString);
  EXPECT_EQ(firstLower(LowerString), LowerString);
  EXPECT_EQ(firstLower(UpperString), "efGh");
  EXPECT_EQ(firstLower(MBString), MBString);
}

TEST(UtilsTest, FirstUpper) {
  EXPECT_EQ(firstUpper(EmptyString), EmptyString);
  EXPECT_EQ(firstUpper(LowerString), "ABcD");
  EXPECT_EQ(firstUpper(UpperString), UpperString);
  EXPECT_EQ(firstUpper(MBString), MBString);
}

TEST(UtilsTest, ToLower) {
  EXPECT_EQ(toLower(EmptyString), EmptyString);
  EXPECT_EQ(toLower(LowerString), "abcd");
  EXPECT_EQ(toLower(UpperString), "efgh");
  EXPECT_EQ(toLower(MBString), "雪snow");
}

TEST(UtilsTest, ToUpper) {
  EXPECT_EQ(toUpper(EmptyString), EmptyString);
  EXPECT_EQ(toUpper(LowerString), "ABCD");
  EXPECT_EQ(toUpper(UpperString), "EFGH");
  EXPECT_EQ(toUpper(MBString), "雪SNOW");
}

} // namespace kanji_tools