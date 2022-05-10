#include <gtest/gtest.h>
#include <kanji_tools/stats/MBCount.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>

#include <sys/socket.h>
#include <sys/un.h>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path TestDir{"testDir"};
const fs::path TestFile1{TestDir / "testFile甲"},
    TestFile2{TestDir / "testFile乙"}, BracketFile{TestDir / "bracketFile"},
    TestSubDir{TestDir / "test下"};
const fs::path TestSubFile1{TestSubDir / "testSubFile1"},
    TestSubFile2{TestSubDir / "testSubFile2.txt"};

auto removeFurigana(const std::wstring& s) {
  return std::regex_replace(
      s, MBCount::RemoveFurigana, MBCount::DefaultReplace);
}

class MBCountTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directories(TestSubDir));
    const auto files = {std::pair{TestFile1, "北海道"},
        std::pair{TestFile2, "南北"}, std::pair{TestSubFile1, "東西線"},
        std::pair{TestSubFile2, "東北"}};
    for (auto& i : files) {
      std::ofstream of{i.first};
      of << i.second;
      of.close();
    }
  }

  void TearDown() override { fs::remove_all(TestDir); }

  auto& mbCount() { return _mbCount; }
private:
  MBCount _mbCount;
};

} // namespace

TEST_F(MBCountTest, CheckRemovingFurigana) {
  // replace furigana - must be kanji followed by kana in wide brackets
  EXPECT_EQ(removeFurigana(L"犬（いぬ）"), L"犬");
  EXPECT_EQ(removeFurigana(L"犬（イヌ）"), L"犬");
  // don't replace after non-kanji
  EXPECT_EQ(removeFurigana(L"いぬ（いぬ）"), L"いぬ（いぬ）");
  // don't replace at start of string
  EXPECT_EQ(removeFurigana(L"（いぬ）"), L"（いぬ）");
  // replace one furigana set in a longer string
  EXPECT_EQ(removeFurigana(L"記された文（ふみ）だけがこの世に残って"),
      L"記された文だけがこの世に残って");
  // replace multiple furigana sets (for compound words)
  EXPECT_EQ(removeFurigana(L"子供たちは茫漠（ぼうばく）と見霽（みはる）かす"),
      L"子供たちは茫漠と見霽かす");
}

TEST_F(MBCountTest, Add) {
  EXPECT_EQ(mbCount().add("hello空は青い"), 4);
  EXPECT_EQ(mbCount().add("箱は空です"), 5);
  EXPECT_EQ(mbCount().add("今日は涼しい。good bye"), 7);
  // map only includes MB chars
  EXPECT_EQ(mbCount().uniqueEntries(), 12);
  EXPECT_EQ(mbCount().count("空"), 2);
  EXPECT_EQ(mbCount().count("は"), 3);
  EXPECT_EQ(mbCount().count("青"), 1);
  EXPECT_EQ(mbCount().count("い"), 2);
  EXPECT_EQ(mbCount().count("箱"), 1);
  EXPECT_EQ(mbCount().count("で"), 1);
  EXPECT_EQ(mbCount().count("す"), 1);
  EXPECT_EQ(mbCount().count("今"), 1);
  EXPECT_EQ(mbCount().count("日"), 1);
  EXPECT_EQ(mbCount().count("涼"), 1);
  EXPECT_EQ(mbCount().count("し"), 1);
  EXPECT_EQ(mbCount().count("。"), 1);
}

TEST_F(MBCountTest, AddWithErrors) {
  String s1{"hello空は青い"}, s2{"箱は空です"};
  s1[s1.size() - 2] = 'x'; // mess up い introducing 2 errors
  s2[0] = 'y';             // mess up 箱 introducing 2 errors
  EXPECT_EQ(mbCount().add(s1), 3);
  EXPECT_EQ(mbCount().add(s2), 4);
  EXPECT_EQ(mbCount().add("今日は涼しい。good bye"), 7);
  // map only includes MB chars
  EXPECT_EQ(mbCount().uniqueEntries(), 11);
  EXPECT_EQ(mbCount().errors(), 4);
  EXPECT_EQ(mbCount().count("空"), 2);
  EXPECT_EQ(mbCount().count("は"), 3);
  EXPECT_EQ(mbCount().count("青"), 1);
  EXPECT_EQ(mbCount().count("い"), 1);
  EXPECT_EQ(mbCount().count("で"), 1);
  EXPECT_EQ(mbCount().count("す"), 1);
  EXPECT_EQ(mbCount().count("今"), 1);
  EXPECT_EQ(mbCount().count("日"), 1);
  EXPECT_EQ(mbCount().count("涼"), 1);
  EXPECT_EQ(mbCount().count("し"), 1);
  EXPECT_EQ(mbCount().count("。"), 1);
}

TEST_F(MBCountTest, AddWithVariants) {
  const String s1{"normal中variant逸︁"}, s2{"あア謁︀"};
  EXPECT_EQ(mbCount().add(s1), 2);
  EXPECT_EQ(mbCount().add(s2), 3);
  EXPECT_EQ(mbCount().count("中"), 1);
  EXPECT_EQ(mbCount().count("逸︁"), 1);
  EXPECT_EQ(mbCount().count("あ"), 1);
  EXPECT_EQ(mbCount().count("ア"), 1);
  EXPECT_EQ(mbCount().count("謁︀"), 1);
  EXPECT_EQ(mbCount().errors(), 0);
  EXPECT_EQ(mbCount().variants(), 2);
}

TEST_F(MBCountTest, AddWithCombiningMarks) {
  const String s1{"て\xe3\x82\x99"}, // with dakuten
      s2{"フ\xe3\x82\x9a"},          // with han-dakuten
      bad{"や\xe3\x82\x9aく"};       // error, but still add や and く
  EXPECT_EQ(mbCount().add(s1), 1);
  EXPECT_EQ(mbCount().add(s2), 1);
  EXPECT_EQ(mbCount().combiningMarks(), 2);
  EXPECT_EQ(mbCount().add(bad), 2);
  EXPECT_EQ(mbCount().combiningMarks(), 2);
  EXPECT_EQ(mbCount().errors(), 1);
  const String noMarks{"愛詞（あいことば）"}, marks{"愛詞（あいことば）"};
  EXPECT_EQ(noMarks.size(), 27);
  EXPECT_EQ(marks.size(), 30);
  EXPECT_EQ(mbCount().add(noMarks), 9);
  EXPECT_EQ(mbCount().combiningMarks(), 2);
  EXPECT_EQ(mbCount().add(marks), 9);
  EXPECT_EQ(mbCount().combiningMarks(), 3);
  EXPECT_EQ(mbCount().errors(), 1);
}

TEST_F(MBCountTest, AddWithPredicate) {
  const auto pred{[](const auto& s) { return s != "。" && s != "は"; }};
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
  EXPECT_EQ(mbCount().addFile(TestFile1, false, false, false), 3);
  EXPECT_EQ(mbCount().uniqueEntries(), 3);
  EXPECT_EQ(mbCount().files(), 1);
  EXPECT_EQ(mbCount().directories(), 0);
  EXPECT_EQ(mbCount().count("北"), 1);
  EXPECT_EQ(mbCount().count("海"), 1);
  EXPECT_EQ(mbCount().count("道"), 1);
}

TEST_F(MBCountTest, AddFileIncludingFile) {
  EXPECT_EQ(mbCount().addFile(TestFile1, false, true, false), 4);
  EXPECT_EQ(mbCount().uniqueEntries(), 4);
  EXPECT_EQ(mbCount().count("北"), 1);
  EXPECT_EQ(mbCount().count("海"), 1);
  EXPECT_EQ(mbCount().count("道"), 1);
  EXPECT_EQ(mbCount().count("甲"), 1);
}

TEST_F(MBCountTest, AddMissingFile) {
  EXPECT_THROW(call([this] { mbCount().addFile(TestDir / "missing"); },
                   "file not found: testDir/missing"),
      std::domain_error);
  EXPECT_EQ(mbCount().files(), 0);
  EXPECT_EQ(mbCount().directories(), 0);
}

TEST_F(MBCountTest, AddDirectoryNoRecurse) {
  EXPECT_EQ(mbCount().addFile(TestDir, false, false, false), 5);
  EXPECT_EQ(mbCount().uniqueEntries(), 4);
  EXPECT_EQ(mbCount().files(), 2);
  EXPECT_EQ(mbCount().directories(), 1);
  EXPECT_EQ(mbCount().count("北"), 2);
  EXPECT_EQ(mbCount().count("南"), 1);
  EXPECT_EQ(mbCount().count("海"), 1);
  EXPECT_EQ(mbCount().count("道"), 1);
}

TEST_F(MBCountTest, AddDirectoryNoRecurseIncludingFileNames) {
  EXPECT_EQ(mbCount().addFile(TestDir, false, true, false), 7);
  EXPECT_EQ(mbCount().uniqueEntries(), 6);
  EXPECT_EQ(mbCount().count("北"), 2);
  EXPECT_EQ(mbCount().count("南"), 1);
  EXPECT_EQ(mbCount().count("海"), 1);
  EXPECT_EQ(mbCount().count("道"), 1);
  EXPECT_EQ(mbCount().count("甲"), 1);
  EXPECT_EQ(mbCount().count("乙"), 1);
}

TEST_F(MBCountTest, AddDirectoryRecurse) {
  EXPECT_EQ(mbCount().addFile(TestDir, false, false), 10);
  EXPECT_EQ(mbCount().uniqueEntries(), 7);
  EXPECT_EQ(mbCount().files(), 4);
  EXPECT_EQ(mbCount().directories(), 2);
  EXPECT_EQ(mbCount().count("北"), 3);
  EXPECT_EQ(mbCount().count("東"), 2);
  EXPECT_EQ(mbCount().count("南"), 1);
  EXPECT_EQ(mbCount().count("海"), 1);
  EXPECT_EQ(mbCount().count("西"), 1);
  EXPECT_EQ(mbCount().count("道"), 1);
  EXPECT_EQ(mbCount().count("線"), 1);
}

TEST_F(MBCountTest, AddDirectoryRecurseIncludingFileNamesButNoTags) {
  EXPECT_EQ(mbCount().addFile(TestDir, false), 13);
  EXPECT_EQ(mbCount().uniqueEntries(), 10);
  EXPECT_EQ(mbCount().count("北"), 3);
  EXPECT_EQ(mbCount().tags("北"), nullptr);
  EXPECT_EQ(mbCount().count("東"), 2);
  EXPECT_EQ(mbCount().count("南"), 1);
  EXPECT_EQ(mbCount().count("海"), 1);
  EXPECT_EQ(mbCount().count("西"), 1);
  EXPECT_EQ(mbCount().count("道"), 1);
  EXPECT_EQ(mbCount().count("線"), 1);
  EXPECT_EQ(mbCount().count("甲"), 1);
  EXPECT_EQ(mbCount().count("乙"), 1);
  EXPECT_EQ(mbCount().count("下"), 1);
}

TEST_F(MBCountTest, SkipSimlinksWhenRecursing) {
  const auto link{TestDir / "link"};
  fs::create_symlink(TestSubDir.filename(), link);
  EXPECT_TRUE(fs::is_symlink(link));
  EXPECT_EQ(mbCount().addFile(TestDir, false), 13);
  EXPECT_EQ(mbCount().directories(), 2);
  EXPECT_EQ(mbCount().files(), 4);
}

TEST_F(MBCountTest, SkipNonRegularFiles) {
  const auto file{TestDir / "socket"};
  const auto fd{socket(AF_UNIX, SOCK_STREAM, 0)};
  ASSERT_NE(fd, -1);
  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, file.c_str(), sizeof(addr.sun_path) - 1);
  ASSERT_FALSE(fs::is_socket(file));
  ASSERT_NE(bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), -1);
  ASSERT_TRUE(fs::is_socket(file));
  EXPECT_EQ(mbCount().addFile(file), 0);
  EXPECT_EQ(mbCount().directories(), 0);
  EXPECT_EQ(mbCount().files(), 0);
}

TEST_F(MBCountTest, CheckTags) {
  EXPECT_EQ(mbCount().addFile(TestDir), 13);
  EXPECT_EQ(mbCount().uniqueEntries(), 10);
  const auto tags{mbCount().tags("北")};
  ASSERT_TRUE(tags != nullptr);
  ASSERT_EQ(tags->size(), 3);
  auto i{tags->find("testFile甲")};
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
  const std::wregex regex{L"（[^）]+）"};
  MBCount r{regex};
  EXPECT_EQ(r.replacements(), 0);
  EXPECT_EQ(r.add("a仰（あお）ぐbc仰（あお）ぐ）"), 5);
  EXPECT_EQ(r.replacements(), 1);
  EXPECT_EQ(r.count("仰"), 2);
  EXPECT_EQ(r.count("ぐ"), 2);
  EXPECT_EQ(r.count("）"), 1);
  EXPECT_EQ(r.count("あ"), 0);
  EXPECT_EQ(r.count("お"), 0);
  EXPECT_EQ(r.count("（"), 0);
}

TEST_F(MBCountTest, BracketsAcrossLines) {
  std::ofstream of{BracketFile};
  of << "安寿が亡きあとはねんごろに弔（\n";
  of << "とむら）われ、また入水した沼の畔（ほとり）には尼寺が立つことになった。"
        "\n";
  of.close();
  const std::wregex regex{L"（[^）]+）"};
  MBCount r{regex};
  EXPECT_EQ(r.addFile(BracketFile), 40);
  EXPECT_EQ(r.count("（"), 0);
  EXPECT_EQ(r.count("）"), 0);
}

TEST_F(MBCountTest, BracketsAtStartOfLine) {
  std::ofstream of{BracketFile};
  of << "安寿が亡きあとはねんごろに弔（と\n";
  of << "むら）われ、また入水した沼の畔\n";
  of << "（ほとり）には尼寺が立つことになった。\n";
  of.close();
  const std::wregex regex{L"（[^）]+）"};
  MBCount r{regex};
  EXPECT_EQ(r.addFile(BracketFile), 40);
  EXPECT_EQ(r.count("（"), 0);
  EXPECT_EQ(r.count("）"), 0);
}

} // namespace kanji_tools
