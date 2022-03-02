#include <gtest/gtest.h>
#include <kanji_tools/utils/MBUtils.h>

#ifdef USE_CODECVT_FOR_UTF_8
#include <kanji_tools/tests/WhatMismatch.h>
#endif

namespace kanji_tools {

namespace {

void fromUtf8Error(const std::string& s,
                   const std::u32string& result = U"\ufffd") {
#ifdef USE_CODECVT_FOR_UTF_8
  // 'result' isn't used by codecvt since it throws an exception, but use it
  // below to avoid a compile warning
  EXPECT_THROW(
    call([&] { return fromUtf8(s) + result; }, "wstring_convert::from_bytes"),
    std::range_error);
#else
  EXPECT_EQ(fromUtf8(s), result);
#endif
}

void toUtf8Error(const std::u32string& s,
                 const std::string& result = "\xEF\xBF\xBD") {
#ifdef USE_CODECVT_FOR_UTF_8
  EXPECT_THROW(
    call([&] { return toUtf8(s) + result; }, "wstring_convert::to_bytes"),
    std::range_error);
#else
  EXPECT_EQ(toUtf8(s), result);
#endif
}

} // namespace

TEST(MBUtilsTest, ValidMBUtf8) {
  EXPECT_EQ(validateMBUtf8(nullptr), MBUtf8Result::NotMBUtf8);
  std::string x("雪");
  EXPECT_EQ(x.size(), 3);
  // badly formed strings:
  EXPECT_EQ(validateMBUtf8(x.substr(0, 1)), MBUtf8Result::MBCharMissingBytes);
  EXPECT_EQ(validateMBUtf8(x.substr(0, 2)), MBUtf8Result::MBCharMissingBytes);
  EXPECT_EQ(validateMBUtf8(x.substr(1, 1)), MBUtf8Result::ContinuationByte);
  EXPECT_EQ(validateMBUtf8(x.substr(1, 2)), MBUtf8Result::ContinuationByte);
}

TEST(MBUtilsTest, ValidWithTwoByte) {
  std::string x("©");
  EXPECT_EQ(x.size(), 2);
  EXPECT_TRUE(isValidMBUtf8(x));
  // badly formed strings:
  EXPECT_EQ(validateMBUtf8(x.substr(0, 1)), MBUtf8Result::MBCharMissingBytes);
  EXPECT_EQ(validateMBUtf8(x.substr(1)), MBUtf8Result::ContinuationByte);
}

TEST(MBUtilsTest, ValidWithFourByte) {
  std::string x("𒀄"); // a four byte sumerian cuneiform symbol
  EXPECT_EQ(x.size(), 4);
  EXPECT_TRUE(isValidMBUtf8(x));
  // badly formed strings:
  EXPECT_EQ(validateMBUtf8(x.substr(0, 1)), MBUtf8Result::MBCharMissingBytes);
  EXPECT_EQ(validateMBUtf8(x.substr(0, 2)), MBUtf8Result::MBCharMissingBytes);
  EXPECT_EQ(validateMBUtf8(x.substr(0, 3)), MBUtf8Result::MBCharMissingBytes);
  EXPECT_EQ(validateMBUtf8(x.substr(1, 1)), MBUtf8Result::ContinuationByte);
  EXPECT_EQ(validateMBUtf8(x.substr(1, 2)), MBUtf8Result::ContinuationByte);
  EXPECT_EQ(validateMBUtf8(x.substr(1, 3)), MBUtf8Result::ContinuationByte);
  EXPECT_EQ(validateMBUtf8(x.substr(2, 1)), MBUtf8Result::ContinuationByte);
  EXPECT_EQ(validateMBUtf8(x.substr(2, 2)), MBUtf8Result::ContinuationByte);
  EXPECT_EQ(validateMBUtf8(x.substr(3, 1)), MBUtf8Result::ContinuationByte);
}

TEST(MBUtilsTest, NotValidWithFiveByte) {
  std::string x("𒀄");
  EXPECT_EQ(x.size(), 4);
  EXPECT_TRUE(isValidMBUtf8(x));
  // try to make a 'fake valid' string with 5 bytes (which is not valid)
  x[0] = static_cast<char>(0b11'11'10'10);
  EXPECT_EQ(x.size(), 4);
  EXPECT_EQ(validateMBUtf8(x), MBUtf8Result::MBCharTooLong);
  x += x[3];
  EXPECT_EQ(x.size(), 5);
  EXPECT_EQ(validateMBUtf8(x), MBUtf8Result::MBCharTooLong);
}

TEST(MBUtilsTest, ValidateMaxUnicode) {
  const char32_t ok = 0x10ffff;
  const char32_t bad = 0x110000;
  EXPECT_EQ(bad - ok, 1);
  EXPECT_EQ(toBinary(ok, 21), "100001111111111111111");
  EXPECT_EQ(toBinary(bad, 21), "100010000000000000000");
  const char firstByte = static_cast<char>(0b11'11'01'00);
  const auto okS = std::string({firstByte, static_cast<char>(0b10'00'11'11),
                                static_cast<char>(0b10'11'11'11),
                                static_cast<char>(0b10'11'11'11)});
  const auto badS =
    std::string({firstByte, static_cast<char>(0b10'01'00'00),
                 static_cast<char>(Bit1), static_cast<char>(Bit1)});
  EXPECT_EQ(validateMBUtf8(okS), MBUtf8Result::Valid);
  EXPECT_EQ(validateMBUtf8(badS), MBUtf8Result::InvalidCodePoint);
}

TEST(MBUtilsTest, ValidateSurrogateRange) {
  const auto beforeRange = std::string({'\xED', '\x9F', '\xBF'}); // U+D7FF
  const auto rangeStart = std::string({'\xED', '\xA0', '\x80'});  // U+D800
  const auto rangeEnd = std::string({'\xED', '\xBF', '\xBF'});    // U+DFFF
  const auto afterRange = std::string({'\xEE', '\x80', '\x80'});  // U+E000
  EXPECT_EQ(validateMBUtf8(beforeRange), MBUtf8Result::Valid);
  EXPECT_EQ(validateMBUtf8(rangeStart), MBUtf8Result::InvalidCodePoint);
  EXPECT_EQ(validateMBUtf8(rangeEnd), MBUtf8Result::InvalidCodePoint);
  EXPECT_EQ(validateMBUtf8(afterRange), MBUtf8Result::Valid);
}

TEST(MBUtilsTest, NotValidForOverlong) {
  // overlong single byte ascii
  const unsigned char bang = 33;
  EXPECT_EQ(toBinary(bang), "00100001"); // decimal 33 which is ascii '!'
  EXPECT_EQ(validateMBUtf8(std::string({static_cast<char>(bang)})),
            MBUtf8Result::NotMBUtf8);
  EXPECT_EQ(validateMBUtf8(std::string(
              {static_cast<char>(TwoBits), static_cast<char>(Bit1 | bang)})),
            MBUtf8Result::Overlong);
  // overlong ō with 3 bytes
  std::string o("ō");
  EXPECT_EQ(o.size(), 2);
  EXPECT_EQ(validateMBUtf8(o), MBUtf8Result::Valid);
  EXPECT_EQ(toUnicode(o), "014D");
  EXPECT_EQ(toBinary(0x014d, 16), "0000000101001101");
  std::string overlongO({static_cast<char>(ThreeBits),
                         static_cast<char>(Bit1 | 0b101),
                         static_cast<char>(Bit1 | 0b1101)});
  EXPECT_EQ(validateMBUtf8(overlongO), MBUtf8Result::Overlong);
  // overlong Euro symbol with 4 bytes
  std::string x("\xF0\x82\x82\xAC");
  EXPECT_EQ(validateMBUtf8(x), MBUtf8Result::Overlong);
}

TEST(MBUtilsTest, ConvertEmptyStrings) {
  std::string emptyString;
  std::u32string emptyU32String;
  EXPECT_EQ(fromUtf8(emptyString), emptyU32String);
  EXPECT_EQ(fromUtf8(""), emptyU32String);
  EXPECT_EQ(toUtf8(emptyU32String), emptyString);
  EXPECT_EQ(toUtf8(U""), emptyString);
}

TEST(MBUtilsTest, FromUTF8String) {
  auto wideSingle = fromUtf8("single .");
  ASSERT_EQ(wideSingle, U"single .");
  // first byte error cases
  fromUtf8Error(std::string({static_cast<char>(Bit1)}));
  fromUtf8Error(std::string({static_cast<char>(FiveBits)}));
  // second byte not continuation
  fromUtf8Error(std::string({static_cast<char>(TwoBits), 'a'}), U"\ufffda");
  const char cont = static_cast<char>(Bit1);
  // third byte not continuation
  fromUtf8Error(std::string({static_cast<char>(ThreeBits), cont, 'a'}),
                U"\ufffda");
  // fourth byte not continuation
  fromUtf8Error(std::string({static_cast<char>(FourBits), cont, cont, 'a'}),
                U"\ufffda");
  std::string dog("犬");
  auto wideDog = fromUtf8(dog);
  ASSERT_EQ(dog.size(), 3);
  EXPECT_EQ(dog[0], '\xe7');
  EXPECT_EQ(dog[1], '\x8a');
  EXPECT_EQ(dog[2], '\xac');
  ASSERT_EQ(wideDog.size(), 1);
  EXPECT_EQ(wideDog[0], U'\u72ac');
  auto newDog = toUtf8(wideDog);
  EXPECT_EQ(dog, newDog);
}

TEST(MBUtilsTest, BeyondMaxUnicode) {
  const char firstByte = static_cast<char>(0b11'11'01'00);
  const auto okS = std::string({firstByte, static_cast<char>(0b10'00'11'11),
                                static_cast<char>(0b10'11'11'11),
                                static_cast<char>(0b10'11'11'11)});
  const auto badS =
    std::string({firstByte, static_cast<char>(0b10'01'00'00),
                 static_cast<char>(Bit1), static_cast<char>(Bit1)});
  // from UTF-8
  EXPECT_EQ(fromUtf8(okS), U"\x10ffff");
  fromUtf8Error(badS);
  // to UTF-8
  EXPECT_EQ(toUtf8(U'\x10ffff'), "\xF4\x8F\xBF\xBF");
  toUtf8Error(U"\x110000");
}

TEST(MBUtilsTest, InvalidSurrogateRange) {
  const auto beforeRange = std::string({'\xED', '\x9F', '\xBF'}); // U+D7FF
  const auto rangeStart = std::string({'\xED', '\xA0', '\x80'});  // U+D800
  const auto rangeEnd = std::string({'\xED', '\xBF', '\xBF'});    // U+DFFF
  const auto afterRange = std::string({'\xEE', '\x80', '\x80'});  // U+E000
  // from UTF-8
  EXPECT_EQ(fromUtf8(beforeRange), U"\ud7ff");
#ifndef USE_CODECVT_FOR_UTF_8
  fromUtf8Error(rangeStart);
  fromUtf8Error(rangeEnd);
  EXPECT_EQ(fromUtf8(afterRange), U"\ue000");
  // to UTF-8
  EXPECT_EQ(toUtf8(U"\ud7ff"), beforeRange);
  toUtf8Error(U"\xd800");
  toUtf8Error(U"\xdfff");
#endif
  EXPECT_EQ(toUtf8(U"\ue000"), afterRange);
}

TEST(MBUtilsTest, ErrorForOverlong) {
  // overlong single byte ascii
  const unsigned char bang = 33;
  EXPECT_EQ(toBinary(bang), "00100001"); // decimal 33 which is ascii '!'
  fromUtf8Error(
    std::string({static_cast<char>(TwoBits), static_cast<char>(Bit1 | bang)}),
    U"\ufffd");
  // overlong ō with 3 bytes
  std::string overlongO({static_cast<char>(ThreeBits),
                         static_cast<char>(Bit1 | 0b101),
                         static_cast<char>(Bit1 | 0b1101)});
  fromUtf8Error(overlongO, U"\ufffd");
  // overlong Euro symbol with 4 bytes
  std::string x("\xF0\x82\x82\xAC");
  fromUtf8Error(x, U"\ufffd");
}

TEST(MBUtilsTest, FromUTF8CharArray) {
  const char s[] = {'\xef', '\xbf', '\xbc', 0};
  auto w = fromUtf8(s);
  ASSERT_EQ(w.size(), 1);
  EXPECT_EQ(w[0], U'\ufffc');
  auto r = toUtf8(w);
  ASSERT_EQ(r.size(), std::size(s) - 1);
  for (size_t i = 0; i < std::size(s) - 1; ++i) EXPECT_EQ(r[i], s[i]);
}

TEST(MBUtilsTest, ToHex) {
  EXPECT_EQ(toHex(U'\ufffc'), "0000fffc");
  auto s = toUtf8(U"\ufffc");
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
  EXPECT_EQ(toUnicode("すずめ-雀", BracketType::Square),
            "[3059 305A 3081 002D 96C0]");
}

TEST(MBUtilsTest, U32ToUnicode) {
  EXPECT_EQ(toUnicode(U'a'), "0061");
  EXPECT_EQ(toUnicode(U"ぁ"), "3041");
  EXPECT_EQ(toUnicode(U"ぁ", BracketType::Square), "[3041]");
  EXPECT_EQ(toUnicode(U"すずめ-雀"), "3059 305A 3081 002D 96C0");
  EXPECT_EQ(toUnicode(U"すずめ-雀", BracketType::Square),
            "[3059 305A 3081 002D 96C0]");
}

TEST(MBUtilsTest, ToBinary) {
  EXPECT_EQ(toBinary(U'\ufffc'), "00000000000000001111111111111100");
  EXPECT_EQ(toBinary(U'\ufffc', 1), "1111111111111100");
  EXPECT_EQ(toBinary(U'\ufffc', BracketType::Square, 1), "[1111111111111100]");
  auto s = toUtf8(U"\ufffc");
  ASSERT_EQ(s.size(), 3);
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

TEST(MBUtilsTest, SortKatakana) {
  std::set<std::string> s{"ケン、トウ", "カ", "カ、サ", "ガ", "ゲン、カン"};
  ASSERT_EQ(s.size(), 5);
  auto i = s.begin();
  EXPECT_EQ(*i++, "カ");
  // The following two entries should be reversed, i.e., "ガ" then "カ、サ" -
  // works fine with bash 'sort'. Later maybe try using
  // https://github.com/unicode-org/icu collate functions.
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
  std::set<std::string> s{"しょう", "Ｐａｒａ", "はら",    "ﾊﾗ",
                          "バラ",   "ばら",     "ぱら",    "para",
                          "じょ",   "しょ",     "ｐａｒａ"};
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
  // Kanji sort order seems to follow Unicode code points instead of
  // 'radical/stroke' ordering. Calling std::setlocale with values like ja_JP or
  // ja_JP.UTF-8 doesn't make any difference.
  std::set<std::string> s{"些", "丑", "云", "丞", "乃",
                          "𠮟", "廿", "⺠", "輸", "鳩"};
  ASSERT_EQ(s.size(), 10);
  auto i = s.begin();
  EXPECT_EQ(toUnicode(*i), "2EA0"); // Rare Kanji (Radical Supplement)
  EXPECT_EQ(*i++, "⺠");
  // Common Kanji with radical 1 (一), strokes 4 (1+3)
  EXPECT_EQ(toUnicode(*i), "4E11");
  EXPECT_EQ(*i++, "丑");
  // Common Kanji with radical 1 (一), strokes 6 (1+5)
  EXPECT_EQ(toUnicode(*i), "4E1E");
  EXPECT_EQ(*i++, "丞");
  // Common Kanji with radical 4 (丿), strokes 2 (1+1)
  EXPECT_EQ(toUnicode(*i), "4E43");
  EXPECT_EQ(*i++, "乃");
  // Common Kanji with radical 7 (二), strokes 4 (2+2)
  EXPECT_EQ(toUnicode(*i), "4E91");
  EXPECT_EQ(*i++, "云");
  // Common Kanji with radical 7 (二), strokes 7 (2+5)
  EXPECT_EQ(toUnicode(*i), "4E9B");
  EXPECT_EQ(*i++, "些");
  // 5EFF is a Common Kanji (Jinmei) with radical 55 (廾), strokes 4 (3+1), but
  // it can also be classified as having radical 24 (十) with strokes 4 (2+2)
  EXPECT_EQ(toUnicode(*i), "5EFF");
  EXPECT_EQ(*i++, "廿");
  // Common kanji with radical 196 (鳥), strokes 13 (11+2)
  EXPECT_EQ(toUnicode(*i), "9CE9");
  EXPECT_EQ(*i++, "鳩");
  // 20B9F is a Common Kanji (in Extension B) with radical 30 (口), strokes 5
  // (2+3) which would normally come before the previous two Kanji in the set
  // since it has radical 30.
  EXPECT_EQ(toUnicode(*i), "20B9F");
  EXPECT_EQ(*i++, "𠮟");
  // 2F9DF is a Rare Kanji with radical 159 (車), strokes 16 (7+9) which would
  // come before before '9CE9' if sorting was based on radical numbers.
  EXPECT_EQ(toUnicode(*i), "2F9DF");
  EXPECT_EQ(*i++, "輸");
  EXPECT_EQ(i, s.end());
}

} // namespace kanji_tools
