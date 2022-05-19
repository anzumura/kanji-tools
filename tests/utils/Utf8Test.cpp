#include <gtest/gtest.h>
#include <kanji_tools/utils/Utf8.h>

namespace kanji_tools {

namespace {

void fromUtf8Error(const String& s, const CodeString& result = U"\ufffd") {
  EXPECT_EQ(fromUtf8(s), result);
}

void toUtf8Error(const CodeString& s, const String& result = "\xEF\xBF\xBD") {
  EXPECT_EQ(toUtf8(s), result);
}

namespace bytes {

const auto GoodFirst{toChar(0b11'11'01'00)}, GoodSecond{toChar(0b10'00'11'11)},
    GoodNext{toChar(0b10'11'11'11)};
const auto BadSecond{toChar(GoodSecond + 1)}, BadNext{toChar(Bit1)};

const String MaxUnicodeUtf8{GoodFirst, GoodSecond, GoodNext, GoodNext},
    BeyondMaxUnicodeUtf8{GoodFirst, BadSecond, BadNext, BadNext};

} // namespace bytes

using bytes::MaxUnicodeUtf8, bytes::BeyondMaxUnicodeUtf8;

const String BeforeSurrogateRange{'\xED', '\x9F', '\xBF'}, // U+D7FF
    SurrogateRangeStart{'\xED', '\xA0', '\x80'},           // U+D800
    SurrogateRangeEnd{'\xED', '\xBF', '\xBF'},             // U+DFFF
    AfterSurrogateRange{'\xEE', '\x80', '\x80'},           // U+E000
    Dog{"犬"};

const Code MaxUnicodePoint{0x10ffff}, BeyondMaxUnicodePoint{0x110000};

} // namespace

TEST(Utf8Test, ValidMBUtf8) {
  EXPECT_EQ(validateMBUtf8(nullptr), MBUtf8Result::NotMultiByte);
  const String x{"雪"};
  EXPECT_EQ(x.size(), 3);
  EXPECT_EQ(validateUtf8(x), Utf8Result::Valid);
  EXPECT_EQ(validateMBUtf8(x), MBUtf8Result::Valid);
  // badly formed strings:
  EXPECT_EQ(validateUtf8(x.substr(0, 1)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(0, 2)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(1, 1)), Utf8Result::ContinuationByte);
  EXPECT_EQ(validateUtf8(x.substr(1, 2)), Utf8Result::ContinuationByte);
}

TEST(Utf8Test, ValidWithTwoByte) {
  const String x{"©"};
  EXPECT_EQ(x.size(), 2);
  EXPECT_TRUE(isValidUtf8(x));
  EXPECT_TRUE(isValidMBUtf8(x));
  // badly formed strings:
  EXPECT_EQ(validateUtf8(x.substr(0, 1)), Utf8Result::MissingBytes);
  EXPECT_EQ(validateUtf8(x.substr(1)), Utf8Result::ContinuationByte);
}

TEST(Utf8Test, ValidWithFourByte) {
  const String x{"𒀄"}; // a four byte Sumerian cuneiform symbol
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

TEST(Utf8Test, NotValidWithFiveByte) {
  String x{"𒀄"};
  EXPECT_EQ(x.size(), 4);
  EXPECT_TRUE(isValidMBUtf8(x));
  // try to make a 'fake valid' string with 5 bytes (which is not valid)
  constexpr auto fakeValid{0b11'11'10'10};
  x[0] = toChar(fakeValid);
  EXPECT_EQ(x.size(), 4);
  auto e{Utf8Result::Valid};
  EXPECT_EQ(validateMBUtf8(x, e), MBUtf8Result::NotValid);
  EXPECT_EQ(e, Utf8Result::CharTooLong);
  x += x[3];
  EXPECT_EQ(x.size(), 5);
  EXPECT_EQ(validateUtf8(x), Utf8Result::CharTooLong);
}

TEST(Utf8Test, ValidateMaxUnicode) {
  EXPECT_EQ(BeyondMaxUnicodePoint - MaxUnicodePoint, 1);
  EXPECT_EQ(toBinary(MaxUnicodePoint, 21), "100001111111111111111");
  EXPECT_EQ(toBinary(BeyondMaxUnicodePoint, 21), "100010000000000000000");
  EXPECT_EQ(validateUtf8(MaxUnicodeUtf8), Utf8Result::Valid);
  EXPECT_EQ(validateUtf8(BeyondMaxUnicodeUtf8), Utf8Result::InvalidCodePoint);
}

TEST(Utf8Test, ValidateSurrogateRange) {
  EXPECT_EQ(validateUtf8(BeforeSurrogateRange), Utf8Result::Valid);
  EXPECT_EQ(validateUtf8(SurrogateRangeStart), Utf8Result::InvalidCodePoint);
  EXPECT_EQ(validateUtf8(SurrogateRangeEnd), Utf8Result::InvalidCodePoint);
  EXPECT_EQ(validateUtf8(AfterSurrogateRange), Utf8Result::Valid);
}

TEST(Utf8Test, NotValidForOverlong) {
  // overlong single byte ascii
  const unsigned char bang{33};
  EXPECT_EQ(toBinary(bang), "00100001"); // decimal 33 which is ascii '!'
  EXPECT_EQ(validateMBUtf8(String{toChar(bang)}), MBUtf8Result::NotMultiByte);
  EXPECT_EQ(validateUtf8(String{toChar(TwoBits), toChar(Bit1 | bang)}),
      Utf8Result::Overlong);
  // overlong ō with 3 bytes
  const String o{"ō"};
  EXPECT_EQ(o.size(), 2);
  EXPECT_EQ(validateUtf8(o), Utf8Result::Valid);
  EXPECT_EQ(toUnicode(o), "014D");
  constexpr uint32_t macronO{0x014d};
  EXPECT_EQ(toBinary(macronO, 16), "0000000101001101");
  const String overlongO{
      toChar(ThreeBits), toChar(Bit1 | 0b101U), toChar(Bit1 | 0b1101U)};
  EXPECT_EQ(validateUtf8(overlongO), Utf8Result::Overlong);
  // overlong Euro symbol with 4 bytes
  const String x{"\xF0\x82\x82\xAC"};
  EXPECT_EQ(validateUtf8(x), Utf8Result::Overlong);
}

TEST(Utf8Test, ConvertEmptyStrings) {
  EXPECT_EQ(fromUtf8(EmptyString), EmptyCodeString);
  EXPECT_EQ(fromUtf8(""), EmptyCodeString);
  EXPECT_EQ(toUtf8(EmptyCodeString), EmptyString);
  EXPECT_EQ(toUtf8(U""), EmptyString);
}

TEST(Utf8Test, FromUTF8String) {
  const auto wideSingle{fromUtf8("single .")};
  ASSERT_EQ(wideSingle, U"single .");
  // first byte error cases
  fromUtf8Error(String{toChar(Bit1)});
  fromUtf8Error(String{toChar(FiveBits)});
  // second byte not continuation - cSpell:ignore ufffda
  fromUtf8Error(String{toChar(TwoBits), 'a'}, U"\ufffda");
  const auto cont{toChar(Bit1)};
  // third byte not continuation
  fromUtf8Error(String{toChar(ThreeBits), cont, 'a'}, U"\ufffda");
  // fourth byte not continuation
  fromUtf8Error(String{toChar(FourBits), cont, cont, 'a'}, U"\ufffda");
  ASSERT_EQ(Dog.size(), 3);
  EXPECT_EQ(Dog[0], '\xe7');
  EXPECT_EQ(Dog[1], '\x8a');
  EXPECT_EQ(Dog[2], '\xac');
  const auto wideDog{fromUtf8(Dog)};
  ASSERT_EQ(wideDog.size(), 1);
  EXPECT_EQ(wideDog[0], U'\u72ac');
  const auto newDog{toUtf8(wideDog)};
  EXPECT_EQ(Dog, newDog);
}

TEST(Utf8Test, fromUtf8WithMaxSize) {
  const String utf8{"生命尊重"};
  // default is '0' which means no max size
  EXPECT_EQ(fromUtf8(utf8), U"生命尊重");
  EXPECT_EQ(fromUtf8(utf8, 1), U"生");
  EXPECT_EQ(fromUtf8(utf8, 2), U"生命");
  EXPECT_EQ(fromUtf8(utf8, 3), U"生命尊");
  for (const auto i : {0UL, 4UL, 5UL})
    EXPECT_EQ(fromUtf8(utf8, i), U"生命尊重");
}

TEST(Utf8Test, GetCode) {
  EXPECT_EQ(getCode("朧"), U'\u6727');
  EXPECT_EQ(getCode(String{"朧"}), U'\u6727');
}

TEST(Utf8Test, ToUTF8IntAndUInt) {
  constexpr int intDog{0x72ac};
  EXPECT_EQ(toUtf8(intDog), Dog);
  constexpr uint32_t uIntDog{0x72ac};
  EXPECT_EQ(toUtf8(uIntDog), Dog);
}

TEST(Utf8Test, BeyondMaxUnicode) {
  // from UTF-8
  EXPECT_EQ(fromUtf8(MaxUnicodeUtf8), U"\x10ffff");
  fromUtf8Error(BeyondMaxUnicodeUtf8);
  // to UTF-8
  EXPECT_EQ(toUtf8(U'\x10ffff'), "\xF4\x8F\xBF\xBF");
  toUtf8Error(U"\x110000");
}

TEST(Utf8Test, InvalidSurrogateRange) {
  // from UTF-8
  EXPECT_EQ(fromUtf8(BeforeSurrogateRange), U"\ud7ff");
  fromUtf8Error(SurrogateRangeStart);
  fromUtf8Error(SurrogateRangeEnd);
  EXPECT_EQ(fromUtf8(AfterSurrogateRange), U"\ue000");
  // to UTF-8
  EXPECT_EQ(toUtf8(U"\ud7ff"), BeforeSurrogateRange);
  toUtf8Error(U"\xd800");
  toUtf8Error(U"\xdfff");
  EXPECT_EQ(toUtf8(U"\ue000"), AfterSurrogateRange);
}

TEST(Utf8Test, ErrorForOverlong) {
  // overlong single byte ascii
  const unsigned char bang{33};
  EXPECT_EQ(toBinary(bang), "00100001"); // decimal 33 which is ascii '!'
  fromUtf8Error(String{toChar(TwoBits), toChar(Bit1 | bang)}, U"\ufffd");
  // overlong ō with 3 bytes
  constexpr auto Byte2{Bit1 | 0b101U}, Byte3{Bit1 | 0b1101U};
  String overlongO{toChar(ThreeBits), toChar(Byte2), toChar(Byte3)};
  fromUtf8Error(overlongO, U"\ufffd");
  // overlong Euro symbol with 4 bytes
  String x{"\xF0\x82\x82\xAC"};
  fromUtf8Error(x, U"\ufffd");
}

TEST(Utf8Test, FromUTF8CharArray) {
  const char s[]{'\xef', '\xbf', '\xbc', 0};
  const auto wideChar{fromUtf8(s)};
  ASSERT_EQ(wideChar.size(), 1);
  EXPECT_EQ(wideChar[0], U'\ufffc');
  const auto utf8String{toUtf8(wideChar)};
  // make sure round-trip conversion gets back to the original char array
  ASSERT_EQ(utf8String.size(), std::size(s) - 1);
  for (size_t i{}; i < std::size(s) - 1; ++i) EXPECT_EQ(utf8String[i], s[i]);
}

TEST(Utf8Test, SortKatakana) {
  const std::set<String> s{"ケン、トウ", "カ", "カ、サ", "ガ", "ゲン、カン"};
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

TEST(Utf8Test, SortKanaAndRomaji) {
  // Default sort order for Japanese Kana and Rōmaji seems to be:
  // - Rōmaji: normal latin letters
  // - Hiragana: in Unicode order so しょう (incorrectly) comes before じょ
  // - Katakana: should mix with Hiragana instead of always coming after
  // - Full-width Rōmaji: should probably come before Kana
  // - Half-width Katakana: should mix with other Kana instead
  // cSpell:ignore Ｐａｒａ
  const std::set<String> s{"しょう", "Ｐａｒａ", "はら", "ﾊﾗ", "バラ", "ばら",
      "ぱら", "para", "じょ", "しょ", "ｐａｒａ"};
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

TEST(Utf8Test, SortKanji) {
  // Kanji sort order seems to follow Unicode code points instead of
  // 'radical/stroke' ordering. Calling std::setlocale with values like ja_JP or
  // ja_JP.UTF-8 doesn't make any difference.
  const std::set<String> s{
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
