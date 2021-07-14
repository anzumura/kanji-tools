#include <gtest/gtest.h>

#include <kanji/MBChar.h>

#include <fstream>

namespace kanji {

namespace {

using BlockSet = std::set<UnicodeBlock>;

template<typename T> void checkRange(const T& blocks, BlockSet* allBlocks = nullptr, bool expectInsert = true) {
  int oldEnd = 0;
  for (const auto& i : blocks) {
    EXPECT_LT(oldEnd, i.start);
    EXPECT_LT(i.start, i.end);
    oldEnd = i.end;
    if (allBlocks) EXPECT_EQ(allBlocks->insert(i).second, expectInsert);
  }
}

auto removeFurigana(const std::wstring& s) {
  return std::regex_replace(s, MBCharCount::RemoveFurigana, MBCharCount::DefaultReplace);
}

} // namespace

TEST(MBChar, CheckNoOverlappingRanges) {
  BlockSet allBlocks;
  checkRange(HiraganaBlocks, &allBlocks);
  checkRange(KatakanaBlocks, &allBlocks);
  // All kana blocks should already be part of hiragana or katakana so make sure size is the same
  // as well as insert 'false' into allBlocks (which means equal blocks were already inserted)
  EXPECT_EQ(allBlocks.size(), KanaBlocks.size());
  checkRange(KanaBlocks, &allBlocks, false);
  checkRange(MBPunctuationBlocks, &allBlocks);
  checkRange(MBLetterBlocks, &allBlocks);
  checkRange(KanjiBlocks, &allBlocks);
  checkRange(allBlocks);
  // check 'range' strings (used in regex calls to remove furigana)
  ASSERT_EQ(std::size(KanjiRange), 7);
  ASSERT_EQ(KanjiBlocks.size(), 2);
  EXPECT_EQ(KanjiRange[0], KanjiBlocks[0].start);
  EXPECT_EQ(KanjiRange[2], KanjiBlocks[0].end);
  EXPECT_EQ(KanjiRange[3], KanjiBlocks[1].start);
  EXPECT_EQ(KanjiRange[5], KanjiBlocks[1].end);
  ASSERT_EQ(std::size(HiraganaRange), 4);
  ASSERT_EQ(HiraganaBlocks.size(), 1);
  EXPECT_EQ(HiraganaRange[0], HiraganaBlocks[0].start);
  EXPECT_EQ(HiraganaRange[2], HiraganaBlocks[0].end);
}

TEST(MBChar, CheckRemovingFurigana) {
  // replace furigana - must be kanji followed by hiragana in wide brackets
  EXPECT_EQ(removeFurigana(L"犬（いぬ）"), L"犬");
  // don't replace katakana
  EXPECT_EQ(removeFurigana(L"犬（イヌ）"), L"犬（イヌ）");
  // don't replace after non-kanji
  EXPECT_EQ(removeFurigana(L"いぬ（いぬ）"), L"いぬ（いぬ）");
  // don't replace at start of string
  EXPECT_EQ(removeFurigana(L"（いぬ）"), L"（いぬ）");
  // replace one furigana set in a longer string
  EXPECT_EQ(removeFurigana(L"記された文（ふみ）だけがこの世に残って"), L"記された文だけがこの世に残って");
  // replace multiple furigana sets (for compound words)
  EXPECT_EQ(removeFurigana(L"子供たちは茫漠（ぼうばく）と見霽（みはる）かす"), L"子供たちは茫漠と見霽かす");
}

TEST(MBChar, CheckFunctions) {
  EXPECT_TRUE(isHiragana("ゑ"));
  EXPECT_FALSE(isKatakana("ゑ"));
  EXPECT_TRUE(isKatakana("ヰ"));
  EXPECT_FALSE(isHiragana("ヰ"));
  EXPECT_TRUE(isKana("ー"));
  EXPECT_TRUE(isKana("さ"));
  EXPECT_FALSE(isMBLetter("ー"));
  EXPECT_FALSE(isMBLetter("さ"));
  EXPECT_FALSE(isKana("ｶ"));
  // Note: half-width katakana is included in Unicode wide letter area
  EXPECT_TRUE(isMBLetter("ｶ"));
  // 'isMBLetter' check also includes extended latin letters and enclosed letters
  EXPECT_TRUE(isMBLetter("ã"));
  EXPECT_TRUE(isMBLetter("⑦"));
  EXPECT_TRUE(isMBLetter("Ⅰ")); // Roman Numeral 'One'
  EXPECT_TRUE(isKana("こ"));
  EXPECT_TRUE(isKana("コ"));
  EXPECT_FALSE(isKana("。"));
  EXPECT_TRUE(isMBPunctuation("—")); // from General Punctuation block
  EXPECT_TRUE(isMBPunctuation("∀")); // from Math Symbols block
  EXPECT_TRUE(isMBPunctuation("☆")); // from Misc Symbols block
  EXPECT_TRUE(isMBPunctuation("○")); // from Geometric Shapes block
  EXPECT_TRUE(isMBPunctuation("。"));
  EXPECT_TRUE(isMBPunctuation("、"));
  EXPECT_TRUE(isMBPunctuation("　"));
  EXPECT_FALSE(isMBPunctuation("ｺ"));
  EXPECT_TRUE(isMBLetter("ｄ"));
  EXPECT_TRUE(isMBLetter("Ｚ"));
  EXPECT_TRUE(isMBLetter("１"));
}

TEST(MBChar, Length) {
  EXPECT_EQ(MBChar("").length(), 0);
  EXPECT_EQ(MBChar::length(nullptr), 0);
  EXPECT_EQ(MBChar("abc").length(), 0);
  EXPECT_EQ(MBChar("abc").length(false), 3);
  EXPECT_EQ(MBChar("大blue空").length(), 2);
  EXPECT_EQ(MBChar("大blue空").length(false), 6);
}

TEST(MBChar, Valid) {
  EXPECT_FALSE(MBChar("").valid());
  EXPECT_FALSE(MBChar::valid(nullptr));
  EXPECT_FALSE(MBChar("a").valid());
  std::string x("雪");
  EXPECT_EQ(x.length(), 3);
  EXPECT_TRUE(MBChar(x).valid());

  // longer strings are not considered valid by default
  EXPECT_FALSE(MBChar("吹雪").valid());
  EXPECT_FALSE(MBChar("猫s").valid());
  EXPECT_FALSE(MBChar("a猫").valid());

  // however, longer strings can be valid if 'checkLengthOne' is false
  EXPECT_TRUE(MBChar("吹雪").valid(false));
  EXPECT_TRUE(MBChar("猫s").valid(false));
  // but the first char must be a multi-byte
  EXPECT_FALSE(MBChar("a猫").valid(false));

  // badly formed strings:
  EXPECT_FALSE(MBChar::valid(x.substr(0, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(0, 2)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 2)));
}

TEST(MBChar, ValidWithTwoByte) {
  std::string x("©");
  EXPECT_EQ(x.length(), 2);
  EXPECT_TRUE(MBChar(x).valid());
  // badly formed strings:
  EXPECT_FALSE(MBChar::valid(x.substr(0, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(1)));
}

TEST(MBChar, ValidWithFourByte) {
  std::string x("𒀄"); // a four byte sumerian cuneiform symbol
  EXPECT_EQ(x.length(), 4);
  EXPECT_TRUE(MBChar(x).valid());
  // badly formed strings:
  EXPECT_FALSE(MBChar::valid(x.substr(0, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(0, 2)));
  EXPECT_FALSE(MBChar::valid(x.substr(0, 3)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 2)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 3)));
  EXPECT_FALSE(MBChar::valid(x.substr(2, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(2, 2)));
  EXPECT_FALSE(MBChar::valid(x.substr(3, 1)));
}

TEST(MBChar, NotValidWithFiveByte) {
  std::string x("𒀄");
  EXPECT_EQ(x.length(), 4);
  EXPECT_TRUE(MBChar(x).valid());
  // try to make a 'fake valid' string with 5 bytes (which is not valid)
  x[0] = 0b11'11'10'10;
  EXPECT_EQ(x.length(), 4);
  EXPECT_FALSE(MBChar::valid(x));
  x += x[3];
  EXPECT_EQ(x.length(), 5);
  EXPECT_FALSE(MBChar::valid(x));
}

TEST(MBChar, GetNext) {
  MBChar s("todayトロントの天気is nice。");
  std::string x;
  std::array expected = {"ト", "ロ", "ン", "ト", "の", "天", "気", "。"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
}

TEST(MBChar, GetNextIncludingSingleByte) {
  MBChar s("a天気b");
  std::string x;
  std::array expected = {"a", "天", "気", "b"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x, false));
}

TEST(MBChar, Reset) {
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

namespace fs = std::filesystem;

class MBCharCountTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (fs::exists(_testDir)) TearDown();
    EXPECT_TRUE(fs::create_directories(_testSubDir));
    std::array files = {std::make_pair(_testFile1, "北海道"), std::make_pair(_testFile2, "南北"),
                        std::make_pair(_testSubFile1, "東西線"), std::make_pair(_testSubFile2, "東北")};
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
  } catch (std::domain_error& err) {
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
  EXPECT_EQ(sizeof(L'a'), 4);
  std::wregex regex(L"（[^）]+）");
  MBCharCount r(regex);
  EXPECT_EQ(r.add("a仰（あお）ぐbc仰（あお）ぐ）"), 5);
  EXPECT_EQ(r.count("仰"), 2);
  EXPECT_EQ(r.count("ぐ"), 2);
  EXPECT_EQ(r.count("）"), 1);
  EXPECT_EQ(r.count("あ"), 0);
  EXPECT_EQ(r.count("お"), 0);
  EXPECT_EQ(r.count("（"), 0);
}

} // namespace kanji
