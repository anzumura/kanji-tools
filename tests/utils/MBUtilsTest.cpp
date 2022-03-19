#include <gtest/gtest.h>
#include <kanji_tools/utils/MBUtils.h>

#ifdef USE_CODECVT_FOR_UTF_8
#include <kanji_tools/tests/WhatMismatch.h>
#endif

namespace kanji_tools {

namespace {

void fromUtf8Error(
    const std::string& s, const std::u32string& result = U"\ufffd") {
#ifdef USE_CODECVT_FOR_UTF_8
  static constexpr auto Msg{
#ifdef __clang__
      "wstring_convert: from_bytes error"
#else
      "wstring_convert::from_bytes"
#endif
  };
  // 'result' isn't used by codecvt since it throws an exception, but use it
  // below to avoid a compile warning
  EXPECT_THROW(
      call([&] { return fromUtf8(s) + result; }, Msg), std::range_error);
#else
  EXPECT_EQ(fromUtf8(s), result);
#endif
}

void toUtf8Error(
    const std::u32string& s, const std::string& result = "\xEF\xBF\xBD") {
#ifdef USE_CODECVT_FOR_UTF_8
  static constexpr auto Msg{
#ifdef __clang__
      "wstring_convert: to_bytes error"
#else
      "wstring_convert::to_bytes"
#endif
  };
  EXPECT_THROW(call([&] { return toUtf8(s) + result; }, Msg), std::range_error);
#else
  EXPECT_EQ(toUtf8(s), result);
#endif
}

namespace bytes {

const auto GoodFirst{static_cast<char>(0b11'11'01'00)},
    GoodSecond{static_cast<char>(0b10'00'11'11)},
    GoodNext{static_cast<char>(0b10'11'11'11)};
const auto BadSecond{static_cast<char>(GoodSecond + 1)},
    BadNext{static_cast<char>(Bit1)};

const std::string MaxUnicodeUtf8{GoodFirst, GoodSecond, GoodNext, GoodNext},
    BeyondMaxUnicodeUtf8{GoodFirst, BadSecond, BadNext, BadNext};

} // namespace bytes

using bytes::MaxUnicodeUtf8, bytes::BeyondMaxUnicodeUtf8;

const std::string BeforeSurrogateRange{'\xED', '\x9F', '\xBF'}, // U+D7FF
    SurrogateRangeStart{'\xED', '\xA0', '\x80'},                // U+D800
    SurrogateRangeEnd{'\xED', '\xBF', '\xBF'},                  // U+DFFF
    AfterSurrogateRange{'\xEE', '\x80', '\x80'};                // U+E000

const char32_t MaxUnicodePoint{0x10ffff}, BeyondMaxUnicodePoint{0x110000};

} // namespace

TEST(MBUtilsTest, ValidMBUtf8) {
  EXPECT_EQ(validateMBUtf8(nullptr), MBUtf8Result::NotMultiByte);
  const std::string x{"雪"};
  EXPECT_EQ(x.size(), 3);
  EXPECT_EQ(validateUtf8(x), Utf8Result::Valid);
  EXPECT_EQ(validateMBUtf8(x), MBUtf8Result::Valid);
  // badly formed strings:
  EXPECT_EQ(validateUtf8(x.substr(0, 1)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(0, 2)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(1, 1)), Utf8Result::ContinuationByte);
  EXPECT_EQ(validateUtf8(x.substr(1, 2)), Utf8Result::ContinuationByte);
}

TEST(MBUtilsTest, ValidWithTwoByte) {
  const std::string x{"©"};
  EXPECT_EQ(x.size(), 2);
  EXPECT_TRUE(isValidUtf8(x));
  EXPECT_TRUE(isValidMBUtf8(x));
  // badly formed strings:
  EXPECT_EQ(validateUtf8(x.substr(0, 1)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(1)), Utf8Result::ContinuationByte);
}

TEST(MBUtilsTest, ValidWithFourByte) {
  const std::string x{"𒀄"}; // a four byte Sumerian cuneiform symbol
  EXPECT_EQ(x.size(), 4);
  EXPECT_TRUE(isValidUtf8(x));
  EXPECT_TRUE(isValidMBUtf8(x));
  // badly formed strings:
  EXPECT_EQ(validateUtf8(x.substr(0, 1)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(0, 2)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(0, 3)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(1, 1)), Utf8Result::ContinuationByte);
  EXPECT_EQ(validateUtf8(x.substr(1, 2)), Utf8Result::ContinuationByte);
  EXPECT_EQ(validateUtf8(x.substr(1, 3)), Utf8Result::ContinuationByte);
  EXPECT_EQ(validateUtf8(x.substr(2, 1)), Utf8Result::ContinuationByte);
  EXPECT_EQ(validateUtf8(x.substr(2, 2)), Utf8Result::ContinuationByte);
  EXPECT_EQ(validateUtf8(x.substr(3, 1)), Utf8Result::ContinuationByte);
}

TEST(MBUtilsTest, NotValidWithFiveByte) {
  std::string x{"𒀄"};
  EXPECT_EQ(x.size(), 4);
  EXPECT_TRUE(isValidMBUtf8(x));
  // try to make a 'fake valid' string with 5 bytes (which is not valid)
  x[0] = static_cast<char>(0b11'11'10'10);
  EXPECT_EQ(x.size(), 4);
  auto e{Utf8Result::Valid};
  EXPECT_EQ(validateMBUtf8(x, e), MBUtf8Result::NotValid);
  EXPECT_EQ(e, Utf8Result::CharTooLong);
  x += x[3];
  EXPECT_EQ(x.size(), 5);
  EXPECT_EQ(validateUtf8(x), Utf8Result::CharTooLong);
}

TEST(MBUtilsTest, ValidateMaxUnicode) {
  EXPECT_EQ(BeyondMaxUnicodePoint - MaxUnicodePoint, 1);
  EXPECT_EQ(toBinary(MaxUnicodePoint, 21), "100001111111111111111");
  EXPECT_EQ(toBinary(BeyondMaxUnicodePoint, 21), "100010000000000000000");
  EXPECT_EQ(validateUtf8(MaxUnicodeUtf8), Utf8Result::Valid);
  EXPECT_EQ(validateUtf8(BeyondMaxUnicodeUtf8), Utf8Result::InvalidCodePoint);
}

TEST(MBUtilsTest, ValidateSurrogateRange) {
  EXPECT_EQ(validateUtf8(BeforeSurrogateRange), Utf8Result::Valid);
  EXPECT_EQ(validateUtf8(SurrogateRangeStart), Utf8Result::InvalidCodePoint);
  EXPECT_EQ(validateUtf8(SurrogateRangeEnd), Utf8Result::InvalidCodePoint);
  EXPECT_EQ(validateUtf8(AfterSurrogateRange), Utf8Result::Valid);
}

TEST(MBUtilsTest, NotValidForOverlong) {
  // overlong single byte ascii
  const unsigned char bang{33};
  EXPECT_EQ(toBinary(bang), "00100001"); // decimal 33 which is ascii '!'
  EXPECT_EQ(validateMBUtf8(std::string{static_cast<char>(bang)}),
      MBUtf8Result::NotMultiByte);
  EXPECT_EQ(validateUtf8(std::string{
                static_cast<char>(TwoBits), static_cast<char>(Bit1 | bang)}),
      Utf8Result::Overlong);
  // overlong ō with 3 bytes
  const std::string o{"ō"};
  EXPECT_EQ(o.size(), 2);
  EXPECT_EQ(validateUtf8(o), Utf8Result::Valid);
  EXPECT_EQ(toUnicode(o), "014D");
  EXPECT_EQ(toBinary(0x014d, 16), "0000000101001101");
  const std::string overlongO{static_cast<char>(ThreeBits),
      static_cast<char>(Bit1 | 0b101), static_cast<char>(Bit1 | 0b1101)};
  EXPECT_EQ(validateUtf8(overlongO), Utf8Result::Overlong);
  // overlong Euro symbol with 4 bytes
  const std::string x{"\xF0\x82\x82\xAC"};
  EXPECT_EQ(validateUtf8(x), Utf8Result::Overlong);
}

TEST(MBUtilsTest, ConvertEmptyStrings) {
  EXPECT_EQ(fromUtf8(EmptyString), EmptyU32String);
  EXPECT_EQ(fromUtf8(""), EmptyU32String);
  EXPECT_EQ(toUtf8(EmptyU32String), EmptyString);
  EXPECT_EQ(toUtf8(U""), EmptyString);
}

TEST(MBUtilsTest, FromUTF8String) {
  const auto wideSingle{fromUtf8("single .")};
  ASSERT_EQ(wideSingle, U"single .");
  // first byte error cases
  fromUtf8Error(std::string{static_cast<char>(Bit1)});
  fromUtf8Error(std::string{static_cast<char>(FiveBits)});
  // second byte not continuation
  fromUtf8Error(std::string{static_cast<char>(TwoBits), 'a'}, U"\ufffda");
  const auto cont{static_cast<char>(Bit1)};
  // third byte not continuation
  fromUtf8Error(
      std::string{static_cast<char>(ThreeBits), cont, 'a'}, U"\ufffda");
  // fourth byte not continuation
  fromUtf8Error(
      std::string{static_cast<char>(FourBits), cont, cont, 'a'}, U"\ufffda");
  const std::string dog{"犬"};
  ASSERT_EQ(dog.size(), 3);
  EXPECT_EQ(dog[0], '\xe7');
  EXPECT_EQ(dog[1], '\x8a');
  EXPECT_EQ(dog[2], '\xac');
  const auto wideDog{fromUtf8(dog)};
  ASSERT_EQ(wideDog.size(), 1);
  EXPECT_EQ(wideDog[0], U'\u72ac');
  const auto newDog{toUtf8(wideDog)};
  EXPECT_EQ(dog, newDog);
}

TEST(MBUtilsTest, BeyondMaxUnicode) {
  // from UTF-8
  EXPECT_EQ(fromUtf8(MaxUnicodeUtf8), U"\x10ffff");
  fromUtf8Error(BeyondMaxUnicodeUtf8);
  // to UTF-8
  EXPECT_EQ(toUtf8(U'\x10ffff'), "\xF4\x8F\xBF\xBF");
  toUtf8Error(U"\x110000");
}

TEST(MBUtilsTest, InvalidSurrogateRange) {
  // from UTF-8
  EXPECT_EQ(fromUtf8(BeforeSurrogateRange), U"\ud7ff");
#ifndef USE_CODECVT_FOR_UTF_8
  fromUtf8Error(SurrogateRangeStart);
  fromUtf8Error(SurrogateRangeEnd);
  EXPECT_EQ(fromUtf8(AfterSurrogateRange), U"\ue000");
  // to UTF-8
  EXPECT_EQ(toUtf8(U"\ud7ff"), BeforeSurrogateRange);
  toUtf8Error(U"\xd800");
  toUtf8Error(U"\xdfff");
#endif
  EXPECT_EQ(toUtf8(U"\ue000"), AfterSurrogateRange);
}

TEST(MBUtilsTest, ErrorForOverlong) {
  // overlong single byte ascii
  const unsigned char bang{33};
  EXPECT_EQ(toBinary(bang), "00100001"); // decimal 33 which is ascii '!'
  fromUtf8Error(
      std::string{static_cast<char>(TwoBits), static_cast<char>(Bit1 | bang)},
      U"\ufffd");
  // overlong ō with 3 bytes
  std::string overlongO{static_cast<char>(ThreeBits),
      static_cast<char>(Bit1 | 0b101), static_cast<char>(Bit1 | 0b1101)};
  fromUtf8Error(overlongO, U"\ufffd");
  // overlong Euro symbol with 4 bytes
  std::string x{"\xF0\x82\x82\xAC"};
  fromUtf8Error(x, U"\ufffd");
}

TEST(MBUtilsTest, FromUTF8CharArray) {
  const char s[]{'\xef', '\xbf', '\xbc', 0};
  const auto wideChar{fromUtf8(s)};
  ASSERT_EQ(wideChar.size(), 1);
  EXPECT_EQ(wideChar[0], U'\ufffc');
  const auto utf8String{toUtf8(wideChar)};
  // make sure round-trip convertion gets back to the original char array
  ASSERT_EQ(utf8String.size(), std::size(s) - 1);
  for (size_t i{}; i < std::size(s) - 1; ++i) EXPECT_EQ(utf8String[i], s[i]);
}

TEST(MBUtilsTest, ToHex) {
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
  auto i{s.begin()};
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
  const std::set<std::string> s{"しょう", "Ｐａｒａ", "はら", "ﾊﾗ", "バラ",
      "ばら", "ぱら", "para", "じょ", "しょ", "ｐａｒａ"};
  ASSERT_EQ(s.size(), 11);
  auto i{s.begin()};
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
  const std::set<std::string> s{
      "些", "丑", "云", "丞", "乃", "𠮟", "廿", "⺠", "輸", "鳩"};
  ASSERT_EQ(s.size(), 10);
  auto i{s.begin()};
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
