#include <gtest/gtest.h>
#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/MBUtils.h>

#include <fstream>

namespace kanji_tools {

namespace {

auto removeFurigana(const std::wstring& s) {
  return std::regex_replace(s, MBCharCount::RemoveFurigana, MBCharCount::DefaultReplace);
}

} // namespace

TEST(MBCharTest, CheckRemovingFurigana) {
  // replace furigana - must be kanji followed by kana in wide brackets
  EXPECT_EQ(removeFurigana(L"犬（いぬ）"), L"犬");
  EXPECT_EQ(removeFurigana(L"犬（イヌ）"), L"犬");
  // don't replace after non-kanji
  EXPECT_EQ(removeFurigana(L"いぬ（いぬ）"), L"いぬ（いぬ）");
  // don't replace at start of string
  EXPECT_EQ(removeFurigana(L"（いぬ）"), L"（いぬ）");
  // replace one furigana set in a longer string
  EXPECT_EQ(removeFurigana(L"記された文（ふみ）だけがこの世に残って"), L"記された文だけがこの世に残って");
  // replace multiple furigana sets (for compound words)
  EXPECT_EQ(removeFurigana(L"子供たちは茫漠（ぼうばく）と見霽（みはる）かす"), L"子供たちは茫漠と見霽かす");
}

TEST(MBCharTest, Length) {
  EXPECT_EQ(MBChar("").length(), 0);
  EXPECT_EQ(MBChar::length(nullptr), 0);
  EXPECT_EQ(MBChar("abc").length(), 0);
  EXPECT_EQ(MBChar("abc").length(false), 3);
  EXPECT_EQ(MBChar("大blue空").length(), 2);
  EXPECT_EQ(MBChar("大blue空").length(false), 6);
  // variation selectors are considered part of the previous character so don't affect length
  auto mbCharWithVariant = U"\u9038\ufe01";
  auto s = toUtf8(mbCharWithVariant);
  EXPECT_EQ(s.length(), 6);
  EXPECT_EQ(MBChar::length(s), 1);
}

TEST(MBCharTest, GetFirst) {
  EXPECT_EQ(MBChar::getFirst(""), "");
  EXPECT_EQ(MBChar::getFirst("abc"), "");
  EXPECT_EQ(MBChar::getFirst("大blue空"), "大");
  // variation selectors are considered part of the previous character so get as well
  auto mbCharWithVariant = U"\u9038\ufe01";
  auto s = toUtf8(mbCharWithVariant);
  auto r = MBChar::getFirst(s);
  EXPECT_EQ(r, s);
}

TEST(MBCharTest, Valid) {
  EXPECT_EQ(MBChar("").valid(), MBChar::Results::NotMBChar);
  EXPECT_EQ(MBChar::validateUtf8(nullptr), MBChar::Results::NotMBChar);
  EXPECT_EQ(MBChar("a").valid(), MBChar::Results::NotMBChar);
  std::string x("雪");
  EXPECT_EQ(x.length(), 3);
  EXPECT_EQ(MBChar(x).valid(), MBChar::Results::Valid);
  EXPECT_TRUE(MBChar(x).isValid());

  // longer strings are not considered valid by default
  EXPECT_EQ(MBChar("吹雪").valid(), MBChar::Results::StringTooLong);
  EXPECT_EQ(MBChar("猫s").valid(), MBChar::Results::StringTooLong);
  EXPECT_EQ(MBChar("a猫").valid(), MBChar::Results::NotMBChar);

  // however, longer strings can be valid if 'checkLengthOne' is false
  EXPECT_TRUE(MBChar("吹雪").isValid(false));
  EXPECT_TRUE(MBChar("猫s").isValid(false));
  // but the first char must be a multi-byte
  EXPECT_FALSE(MBChar("a猫").isValid(false));

  // badly formed strings:
  EXPECT_EQ(MBChar::validateUtf8(x.substr(0, 1)), MBChar::Results::MBCharMissingBytes);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(0, 2)), MBChar::Results::MBCharMissingBytes);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(1, 1)), MBChar::Results::ContinuationByte);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(1, 2)), MBChar::Results::ContinuationByte);
}

TEST(MBCharTest, ValidWithTwoByte) {
  std::string x("©");
  EXPECT_EQ(x.length(), 2);
  EXPECT_TRUE(MBChar(x).isValid());
  // badly formed strings:
  EXPECT_EQ(MBChar::validateUtf8(x.substr(0, 1)), MBChar::Results::MBCharMissingBytes);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(1)), MBChar::Results::ContinuationByte);
}

TEST(MBCharTest, ValidWithFourByte) {
  std::string x("𒀄"); // a four byte sumerian cuneiform symbol
  EXPECT_EQ(x.length(), 4);
  EXPECT_TRUE(MBChar(x).isValid());
  // badly formed strings:
  EXPECT_EQ(MBChar::validateUtf8(x.substr(0, 1)), MBChar::Results::MBCharMissingBytes);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(0, 2)), MBChar::Results::MBCharMissingBytes);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(0, 3)), MBChar::Results::MBCharMissingBytes);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(1, 1)), MBChar::Results::ContinuationByte);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(1, 2)), MBChar::Results::ContinuationByte);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(1, 3)), MBChar::Results::ContinuationByte);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(2, 1)), MBChar::Results::ContinuationByte);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(2, 2)), MBChar::Results::ContinuationByte);
  EXPECT_EQ(MBChar::validateUtf8(x.substr(3, 1)), MBChar::Results::ContinuationByte);
}

TEST(MBCharTest, NotValidWithFiveByte) {
  std::string x("𒀄");
  EXPECT_EQ(x.length(), 4);
  EXPECT_TRUE(MBChar(x).isValid());
  // try to make a 'fake valid' string with 5 bytes (which is not valid)
  x[0] = 0b11'11'10'10;
  EXPECT_EQ(x.length(), 4);
  EXPECT_EQ(MBChar::validateUtf8(x), MBChar::Results::MBCharTooLong);
  x += x[3];
  EXPECT_EQ(x.length(), 5);
  EXPECT_EQ(MBChar::validateUtf8(x), MBChar::Results::MBCharTooLong);
}

// see similar tests in MBUtilsTest.cpp
TEST(MBCharTest, BeyondMaxUnicode) {
  const char32_t ok = 0x10ffff;
  const char32_t bad = 0x110000;
  EXPECT_EQ(bad - ok, 1);
  EXPECT_EQ(toBinary(ok, 21), "100001111111111111111");
  EXPECT_EQ(toBinary(bad, 21), "100010000000000000000");
  const char firstByte = static_cast<char>(0b11'11'01'00);
  const auto okS = std::string(
    {firstByte, static_cast<char>(0b10'00'11'11), static_cast<char>(0b10'11'11'11), static_cast<char>(0b10'11'11'11)});
  const auto badS =
    std::string({firstByte, static_cast<char>(0b10'01'00'00), static_cast<char>(Bit1), static_cast<char>(Bit1)});
  EXPECT_EQ(MBChar::validateUtf8(okS), MBChar::Results::Valid);
  EXPECT_EQ(MBChar::validateUtf8(badS), MBChar::Results::InvalidCodePoint);
}

TEST(MBCharTest, InvalidSurrogateRange) {
  const auto beforeRange = std::string({'\xED', '\x9F', '\xBF'}); // U+D7FF
  const auto rangeStart = std::string({'\xED', '\xA0', '\x80'});  // U+D800
  const auto rangeEnd = std::string({'\xED', '\xBF', '\xBF'});    // U+DFFF
  const auto afterRange = std::string({'\xEE', '\x80', '\x80'});  // U+E000
  EXPECT_EQ(MBChar::validateUtf8(beforeRange), MBChar::Results::Valid);
  EXPECT_EQ(MBChar::validateUtf8(rangeStart), MBChar::Results::InvalidCodePoint);
  EXPECT_EQ(MBChar::validateUtf8(rangeEnd), MBChar::Results::InvalidCodePoint);
  EXPECT_EQ(MBChar::validateUtf8(afterRange), MBChar::Results::Valid);
}

// see similar tests in MBUtilsTest.cpp (ErrorForOverlong)
TEST(MBCharTest, NotValidForOverlong) {
  // overlong single byte ascii
  const unsigned char bang = 33;
  EXPECT_EQ(toBinary(bang), "00100001"); // decimal 33 which is ascii '!'
  EXPECT_EQ(MBChar::validateUtf8(std::string({static_cast<char>(bang)})), MBChar::Results::NotMBChar);
  EXPECT_EQ(MBChar::validateUtf8(std::string({static_cast<char>(TwoBits), static_cast<char>(Bit1 | bang)})),
            MBChar::Results::Overlong);
  // overlong ō with 3 bytes
  std::string o("ō");
  EXPECT_EQ(o.length(), 2);
  EXPECT_EQ(MBChar::validateUtf8(o), MBChar::Results::Valid);
  EXPECT_EQ(toUnicode(o), "014D");
  EXPECT_EQ(toBinary(0x014d, 16), "0000000101001101");
  std::string overlongO(
    {static_cast<char>(ThreeBits), static_cast<char>(Bit1 | 0b101), static_cast<char>(Bit1 | 0b1101)});
  EXPECT_EQ(MBChar::validateUtf8(overlongO), MBChar::Results::Overlong);
  // overlong Euro symbol with 4 bytes
  std::string x("\xF0\x82\x82\xAC");
  EXPECT_EQ(MBChar::validateUtf8(x), MBChar::Results::Overlong);
}

TEST(MBCharTest, GetNext) {
  MBChar s("todayトロントの天気is nice。");
  std::string x;
  std::array expected = {"ト", "ロ", "ン", "ト", "の", "天", "気", "。"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, GetNextIncludingSingleByte) {
  MBChar s("a天気b");
  std::string x;
  std::array expected = {"a", "天", "気", "b"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x, false));
}

TEST(MBCharTest, Reset) {
  MBChar s("a天気b");
  std::string x;
  std::array expected = {"天", "気"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  s.reset();
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, ErrorCount) {
  std::string original("甲乙丙丁");
  // there should be 4 '3-byte' characters
  ASSERT_EQ(original.length(), 12);
  // introduce some errors
  original[1] = 'x'; // change the middle of 甲 resulting in 2 errors (first and last bytes)
  original[6] = 'z'; // change the first byte of 丙 resulting in 2 errors (second and third bytes)
  MBChar s(original);
  std::string x;
  std::array expected = {"乙", "丁"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  EXPECT_EQ(s.errors(), 4);
  s.reset();
  // make sure 'reset' also clears errors
  EXPECT_EQ(s.errors(), 0);
  // now loop again looking for single byte results as well
  std::array expectedWithSingle = {"x", "乙", "z", "丁"};
  for (const auto& i : expectedWithSingle) {
    EXPECT_TRUE(s.next(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  EXPECT_EQ(s.errors(), 4);
}

namespace fs = std::filesystem;

class MBCharCountTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (fs::exists(_testDir)) TearDown();
    EXPECT_TRUE(fs::create_directories(_testSubDir));
    std::array files = {std::pair(_testFile1, "北海道"), std::pair(_testFile2, "南北"),
                        std::pair(_testSubFile1, "東西線"), std::pair(_testSubFile2, "東北")};
    for (auto& i : files) {
      std::ofstream of(i.first);
      of << i.second;
      of.close();
    }
  }
  void TearDown() override { fs::remove_all(_testDir); }
  MBCharCount c;
  fs::path _testDir = "testDir";
  fs::path _testFile1 = _testDir / "testFile甲";
  fs::path _testFile2 = _testDir / "testFile乙";
  fs::path _bracketFile = _testDir / "bracketFile";
  fs::path _testSubDir = _testDir / "test下";
  fs::path _testSubFile1 = _testSubDir / "testSubFile1";
  fs::path _testSubFile2 = _testSubDir / "testSubFile2.txt";
};

TEST_F(MBCharCountTest, Add) {
  EXPECT_EQ(c.add("hello空は青い"), 4);
  EXPECT_EQ(c.add("箱は空です"), 5);
  EXPECT_EQ(c.add("今日は涼しい。good bye"), 7);
  // map only includes MB chars
  EXPECT_EQ(c.uniqueEntries(), 12);
  EXPECT_EQ(c.count("空"), 2);
  EXPECT_EQ(c.count("は"), 3);
  EXPECT_EQ(c.count("青"), 1);
  EXPECT_EQ(c.count("い"), 2);
  EXPECT_EQ(c.count("箱"), 1);
  EXPECT_EQ(c.count("で"), 1);
  EXPECT_EQ(c.count("す"), 1);
  EXPECT_EQ(c.count("今"), 1);
  EXPECT_EQ(c.count("日"), 1);
  EXPECT_EQ(c.count("涼"), 1);
  EXPECT_EQ(c.count("し"), 1);
  EXPECT_EQ(c.count("。"), 1);
}

TEST_F(MBCharCountTest, AddWithErrors) {
  std::string s1("hello空は青い"), s2("箱は空です");
  s1[s1.length() - 2] = 'x'; // mess up い introducing 2 errors
  s2[0] = 'y';               // mess up 箱 introducing 2 errors
  EXPECT_EQ(c.add(s1), 3);
  EXPECT_EQ(c.add(s2), 4);
  EXPECT_EQ(c.add("今日は涼しい。good bye"), 7);
  // map only includes MB chars
  EXPECT_EQ(c.uniqueEntries(), 11);
  EXPECT_EQ(c.errors(), 4);
  EXPECT_EQ(c.count("空"), 2);
  EXPECT_EQ(c.count("は"), 3);
  EXPECT_EQ(c.count("青"), 1);
  EXPECT_EQ(c.count("い"), 1);
  EXPECT_EQ(c.count("で"), 1);
  EXPECT_EQ(c.count("す"), 1);
  EXPECT_EQ(c.count("今"), 1);
  EXPECT_EQ(c.count("日"), 1);
  EXPECT_EQ(c.count("涼"), 1);
  EXPECT_EQ(c.count("し"), 1);
  EXPECT_EQ(c.count("。"), 1);
}

TEST_F(MBCharCountTest, AddWithVariants) {
  std::string s1("normal中variant逸︁"), s2("あア謁︀");
  EXPECT_EQ(c.add(s1), 2);
  EXPECT_EQ(c.add(s2), 3);
  EXPECT_EQ(c.count("中"), 1);
  EXPECT_EQ(c.count("逸︁"), 1);
  EXPECT_EQ(c.count("あ"), 1);
  EXPECT_EQ(c.count("ア"), 1);
  EXPECT_EQ(c.count("謁︀"), 1);
  EXPECT_EQ(c.errors(), 0);
  EXPECT_EQ(c.variants(), 2);
}

TEST_F(MBCharCountTest, AddWithCombiningMarks) {
  std::string s1("て\xe3\x82\x99"); // with dakuten
  std::string s2("フ\xe3\x82\x9a"); // with han-dakuten
  EXPECT_EQ(c.add(s1), 1);
  EXPECT_EQ(c.add(s2), 1);
  EXPECT_EQ(c.combiningMarks(), 2);
  std::string bad("や\xe3\x82\x9aく"); // error case, but still adds や and く
  EXPECT_EQ(c.add(bad), 2);
  EXPECT_EQ(c.combiningMarks(), 2);
  EXPECT_EQ(c.errors(), 1);
  std::string noMarks("愛詞（あいことば）");
  std::string marks("愛詞（あいことば）");
  EXPECT_EQ(noMarks.length(), 27);
  EXPECT_EQ(marks.length(), 30);
  EXPECT_EQ(MBChar::length(noMarks), 9);
  EXPECT_EQ(MBChar::length(marks), 9);
  EXPECT_EQ(c.add(noMarks), 9);
  EXPECT_EQ(c.combiningMarks(), 2);
  EXPECT_EQ(c.add(marks), 9);
  EXPECT_EQ(c.combiningMarks(), 3);
  EXPECT_EQ(c.errors(), 1);
}

TEST_F(MBCharCountTest, AddWithPredicate) {
  auto pred = [](const auto& s) { return s != "。" && s != "は"; };
  MBCharCountIf cPred(pred);
  EXPECT_EQ(cPred.add("これは模擬テストです。"), 9);
  EXPECT_EQ(cPred.count("こ"), 1);
  EXPECT_EQ(cPred.count("れ"), 1);
  EXPECT_EQ(cPred.count("模"), 1);
  EXPECT_EQ(cPred.count("擬"), 1);
  EXPECT_EQ(cPred.count("テ"), 1);
  EXPECT_EQ(cPred.count("ス"), 1);
  EXPECT_EQ(cPred.count("ト"), 1);
  EXPECT_EQ(cPred.count("で"), 1);
  EXPECT_EQ(cPred.count("す"), 1);
  EXPECT_EQ(cPred.count("は"), 0);
  EXPECT_EQ(cPred.count("。"), 0);
}

TEST_F(MBCharCountTest, AddFile) {
  EXPECT_EQ(c.addFile(_testFile1, false, false, false), 3);
  EXPECT_EQ(c.uniqueEntries(), 3);
  EXPECT_EQ(c.files(), 1);
  EXPECT_EQ(c.directories(), 0);
  EXPECT_EQ(c.count("北"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
}

TEST_F(MBCharCountTest, AddFileIncludingFile) {
  EXPECT_EQ(c.addFile(_testFile1, false, true, false), 4);
  EXPECT_EQ(c.uniqueEntries(), 4);
  EXPECT_EQ(c.count("北"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
  EXPECT_EQ(c.count("甲"), 1);
}

TEST_F(MBCharCountTest, AddMissingFile) {
  try {
    c.addFile(_testDir / "missing");
    FAIL() << "Expected std::domain_error";
  } catch (const std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("file not found: testDir/missing"));
    EXPECT_EQ(c.files(), 0);
    EXPECT_EQ(c.directories(), 0);
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(MBCharCountTest, AddDirectoryNoRecurse) {
  EXPECT_EQ(c.addFile(_testDir, false, false, false), 5);
  EXPECT_EQ(c.uniqueEntries(), 4);
  EXPECT_EQ(c.files(), 2);
  EXPECT_EQ(c.directories(), 1);
  EXPECT_EQ(c.count("北"), 2);
  EXPECT_EQ(c.count("南"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
}

TEST_F(MBCharCountTest, AddDirectoryNoRecurseIncludingFileNames) {
  EXPECT_EQ(c.addFile(_testDir, false, true, false), 7);
  EXPECT_EQ(c.uniqueEntries(), 6);
  EXPECT_EQ(c.count("北"), 2);
  EXPECT_EQ(c.count("南"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
  EXPECT_EQ(c.count("甲"), 1);
  EXPECT_EQ(c.count("乙"), 1);
}

TEST_F(MBCharCountTest, AddDirectoryRecurse) {
  EXPECT_EQ(c.addFile(_testDir, false, false), 10);
  EXPECT_EQ(c.uniqueEntries(), 7);
  EXPECT_EQ(c.files(), 4);
  EXPECT_EQ(c.directories(), 2);
  EXPECT_EQ(c.count("北"), 3);
  EXPECT_EQ(c.count("東"), 2);
  EXPECT_EQ(c.count("南"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("西"), 1);
  EXPECT_EQ(c.count("道"), 1);
  EXPECT_EQ(c.count("線"), 1);
}

TEST_F(MBCharCountTest, AddDirectoryRecurseIncludingFileNamesButNoTags) {
  EXPECT_EQ(c.addFile(_testDir, false), 13);
  EXPECT_EQ(c.uniqueEntries(), 10);
  EXPECT_EQ(c.count("北"), 3);
  EXPECT_EQ(c.tags("北"), nullptr);
  EXPECT_EQ(c.count("東"), 2);
  EXPECT_EQ(c.count("南"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("西"), 1);
  EXPECT_EQ(c.count("道"), 1);
  EXPECT_EQ(c.count("線"), 1);
  EXPECT_EQ(c.count("甲"), 1);
  EXPECT_EQ(c.count("乙"), 1);
  EXPECT_EQ(c.count("下"), 1);
}

TEST_F(MBCharCountTest, CheckTags) {
  EXPECT_EQ(c.addFile(_testDir), 13);
  EXPECT_EQ(c.uniqueEntries(), 10);
  auto tags = c.tags("北");
  ASSERT_TRUE(tags != nullptr);
  ASSERT_EQ(tags->size(), 3);
  auto i = tags->find("testFile甲");
  ASSERT_NE(i, tags->end());
  EXPECT_EQ(i->second, 1);
  i = tags->find("testFile乙");
  ASSERT_NE(i, tags->end());
  EXPECT_EQ(i->second, 1);
  i = tags->find("testSubFile2.txt");
  ASSERT_NE(i, tags->end());
  EXPECT_EQ(i->second, 1);
}

TEST_F(MBCharCountTest, Regex) {
  EXPECT_EQ(sizeof(U'a'), 4);
  std::wregex regex(L"（[^）]+）");
  MBCharCount r(regex);
  EXPECT_EQ(r.replaceCount(), 0);
  EXPECT_EQ(r.add("a仰（あお）ぐbc仰（あお）ぐ）"), 5);
  EXPECT_EQ(r.replaceCount(), 1);
  EXPECT_EQ(r.count("仰"), 2);
  EXPECT_EQ(r.count("ぐ"), 2);
  EXPECT_EQ(r.count("）"), 1);
  EXPECT_EQ(r.count("あ"), 0);
  EXPECT_EQ(r.count("お"), 0);
  EXPECT_EQ(r.count("（"), 0);
}

TEST_F(MBCharCountTest, BracketsAcrossLines) {
  std::ofstream of(_bracketFile);
  of << "安寿が亡きあとはねんごろに弔（\n";
  of << "とむら）われ、また入水した沼の畔（ほとり）には尼寺が立つことになった。\n";
  of.close();
  std::wregex regex(L"（[^）]+）");
  MBCharCount r(regex);
  EXPECT_EQ(r.addFile(_bracketFile), 40);
  EXPECT_EQ(r.count("（"), 0);
  EXPECT_EQ(r.count("）"), 0);
}

TEST_F(MBCharCountTest, BracketsAtStartOfLine) {
  std::ofstream of(_bracketFile);
  of << "安寿が亡きあとはねんごろに弔（と\n";
  of << "むら）われ、また入水した沼の畔\n";
  of << "（ほとり）には尼寺が立つことになった。\n";
  of.close();
  std::wregex regex(L"（[^）]+）");
  MBCharCount r(regex);
  EXPECT_EQ(r.addFile(_bracketFile), 40);
  EXPECT_EQ(r.count("（"), 0);
  EXPECT_EQ(r.count("）"), 0);
}

} // namespace kanji_tools
