#include <gtest/gtest.h>
#include <kanji_tools/stats/MBCount.h>
#include <kanji_tools/tests/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace {

auto removeFurigana(const std::wstring& s) {
  return std::regex_replace(s, MBCount::RemoveFurigana, MBCount::DefaultReplace);
}

} // namespace

namespace fs = std::filesystem;

class MBCountTest : public ::testing::Test {
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
  MBCount c;
  fs::path _testDir = "testDir";
  fs::path _testFile1 = _testDir / "testFile甲";
  fs::path _testFile2 = _testDir / "testFile乙";
  fs::path _bracketFile = _testDir / "bracketFile";
  fs::path _testSubDir = _testDir / "test下";
  fs::path _testSubFile1 = _testSubDir / "testSubFile1";
  fs::path _testSubFile2 = _testSubDir / "testSubFile2.txt";
};

TEST_F(MBCountTest, CheckRemovingFurigana) {
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

TEST_F(MBCountTest, Add) {
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

TEST_F(MBCountTest, AddWithErrors) {
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

TEST_F(MBCountTest, AddWithVariants) {
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

TEST_F(MBCountTest, AddWithCombiningMarks) {
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
  EXPECT_EQ(c.add(noMarks), 9);
  EXPECT_EQ(c.combiningMarks(), 2);
  EXPECT_EQ(c.add(marks), 9);
  EXPECT_EQ(c.combiningMarks(), 3);
  EXPECT_EQ(c.errors(), 1);
}

TEST_F(MBCountTest, AddWithPredicate) {
  auto pred = [](const auto& s) { return s != "。" && s != "は"; };
  MBCountIf cPred(pred);
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

TEST_F(MBCountTest, AddFile) {
  EXPECT_EQ(c.addFile(_testFile1, false, false, false), 3);
  EXPECT_EQ(c.uniqueEntries(), 3);
  EXPECT_EQ(c.files(), 1);
  EXPECT_EQ(c.directories(), 0);
  EXPECT_EQ(c.count("北"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
}

TEST_F(MBCountTest, AddFileIncludingFile) {
  EXPECT_EQ(c.addFile(_testFile1, false, true, false), 4);
  EXPECT_EQ(c.uniqueEntries(), 4);
  EXPECT_EQ(c.count("北"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
  EXPECT_EQ(c.count("甲"), 1);
}

TEST_F(MBCountTest, AddMissingFile) {
  EXPECT_THROW(call([this] { c.addFile(_testDir / "missing"); }, "file not found: testDir/missing"), std::domain_error);
  EXPECT_EQ(c.files(), 0);
  EXPECT_EQ(c.directories(), 0);
}

TEST_F(MBCountTest, AddDirectoryNoRecurse) {
  EXPECT_EQ(c.addFile(_testDir, false, false, false), 5);
  EXPECT_EQ(c.uniqueEntries(), 4);
  EXPECT_EQ(c.files(), 2);
  EXPECT_EQ(c.directories(), 1);
  EXPECT_EQ(c.count("北"), 2);
  EXPECT_EQ(c.count("南"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
}

TEST_F(MBCountTest, AddDirectoryNoRecurseIncludingFileNames) {
  EXPECT_EQ(c.addFile(_testDir, false, true, false), 7);
  EXPECT_EQ(c.uniqueEntries(), 6);
  EXPECT_EQ(c.count("北"), 2);
  EXPECT_EQ(c.count("南"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
  EXPECT_EQ(c.count("甲"), 1);
  EXPECT_EQ(c.count("乙"), 1);
}

TEST_F(MBCountTest, AddDirectoryRecurse) {
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

TEST_F(MBCountTest, AddDirectoryRecurseIncludingFileNamesButNoTags) {
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

TEST_F(MBCountTest, CheckTags) {
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

TEST_F(MBCountTest, Regex) {
  EXPECT_EQ(sizeof(U'a'), 4);
  std::wregex regex(L"（[^）]+）");
  MBCount r(regex);
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

TEST_F(MBCountTest, BracketsAcrossLines) {
  std::ofstream of(_bracketFile);
  of << "安寿が亡きあとはねんごろに弔（\n";
  of << "とむら）われ、また入水した沼の畔（ほとり）には尼寺が立つことになった。\n";
  of.close();
  std::wregex regex(L"（[^）]+）");
  MBCount r(regex);
  EXPECT_EQ(r.addFile(_bracketFile), 40);
  EXPECT_EQ(r.count("（"), 0);
  EXPECT_EQ(r.count("）"), 0);
}

TEST_F(MBCountTest, BracketsAtStartOfLine) {
  std::ofstream of(_bracketFile);
  of << "安寿が亡きあとはねんごろに弔（と\n";
  of << "むら）われ、また入水した沼の畔\n";
  of << "（ほとり）には尼寺が立つことになった。\n";
  of.close();
  std::wregex regex(L"（[^）]+）");
  MBCount r(regex);
  EXPECT_EQ(r.addFile(_bracketFile), 40);
  EXPECT_EQ(r.count("（"), 0);
  EXPECT_EQ(r.count("）"), 0);
}

} // namespace kanji_tools
