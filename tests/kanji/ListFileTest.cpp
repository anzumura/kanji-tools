#include <gtest/gtest.h>
#include <kt_kanji/ListFile.h>
#include <kt_tests/WhatMismatch.h>
#include <kt_utils/UnicodeBlock.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path TestDir{"testDir"};
const fs::path GoodOnePerLine{TestDir / "goodOnePerLine"},
    GoodOnePerLineLevel{TestDir / "goodOnePerLineLevel"},
    MultiplePerLine{TestDir / "multiplePerLine"},
    BadOnePerLine{TestDir / "badOnePerLine"}, BadSymbol{TestDir / "badSymbol"},
    DuplicateSymbol{TestDir / "duplicateSymbol"}, BigFile{TestDir / "bigFile"};

class ListFileTest : public ::testing::Test {
protected:
  void SetUp() final {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
    const auto files = {std::pair{GoodOnePerLine, "北\n海\n道"},
        std::pair{GoodOnePerLineLevel, "犬\n猫\n虎"},
        std::pair{BadOnePerLine, "焼 肉"},
        std::pair{MultiplePerLine, "東 西 線"}, std::pair{BadSymbol, "a"},
        std::pair{DuplicateSymbol, "車\n車"}};
    for (auto& i : files) {
      std::ofstream of{i.first};
      of << i.second;
      of.close();
    }
  }

  void TearDown() final {
    ListFile::clearUniqueCheckData();
    fs::remove_all(TestDir);
  }
};

} // namespace

TEST_F(ListFileTest, Usage) {
  const String msg{"error msg"};
  EXPECT_THROW(call([&msg] { ListFile::usage(msg); }, msg), DomainError);
}

TEST_F(ListFileTest, MissingFileWithExtension) {
  const String msg{"testDir must contain 'missing.txt'"};
  EXPECT_THROW(
      call([] { return ListFile::getFile(TestDir, "missing.txt"); }, msg),
      DomainError);
}

TEST_F(ListFileTest, MissingFileWithoutExtension) {
  const String msg{
      "testDir must contain 'missing' (also tried '.txt' extension)"};
  EXPECT_THROW(call([] { return ListFile::getFile(TestDir, "missing"); }, msg),
      DomainError);
}

TEST_F(ListFileTest, PrintEmptyList) {
  std::stringstream s;
  ListFile::print(s, {}, "items", {});
  EXPECT_EQ(s.str(), "");
}

TEST_F(ListFileTest, PrintNonEmptyList) {
  std::stringstream s;
  ListFile::print(s, {"foo", "bar"}, "items", {});
  EXPECT_EQ(s.str(), ">>> Found 2 items: foo bar\n");
}

TEST_F(ListFileTest, PrintWithGroupName) {
  std::stringstream s;
  ListFile::print(s, {"a", "b", "c"}, "items", "bag");
  EXPECT_EQ(s.str(), ">>> Found 3 items in bag: a b c\n");
}

TEST_F(ListFileTest, GoodOnePerLine) {
  const ListFile f{GoodOnePerLine};
  EXPECT_EQ(f.level(), JlptLevels::None);
  EXPECT_EQ(f.kyu(), KenteiKyus::None);
  EXPECT_EQ(f.name(), "GoodOnePerLine");
  const auto results = {"北", "海", "道"};
  EXPECT_EQ(f.list().size(), results.size());
  for (size_t pos{}; auto r : results) {
    EXPECT_TRUE(f.exists(r));
    // numbers start at 1
    EXPECT_EQ(f.getIndex(r), ++pos);
  }
  EXPECT_EQ(f.toString(), "北海道");
}

TEST_F(ListFileTest, GoodOnePerLineLevel) {
  const LevelListFile f{GoodOnePerLineLevel, JlptLevels::N2};
  EXPECT_EQ(f.level(), JlptLevels::N2);
  EXPECT_EQ(f.kyu(), KenteiKyus::None);
  EXPECT_EQ(f.name(), "N2");
  const auto results = {"犬", "猫", "虎"};
  EXPECT_EQ(f.list().size(), results.size());
  for (size_t pos{}; auto r : results) {
    EXPECT_TRUE(f.exists(r));
    EXPECT_EQ(f.getIndex(r), ++pos);
  }
}

TEST_F(ListFileTest, BadOnePerLine) {
  EXPECT_THROW(
      call([] { ListFile{BadOnePerLine}; },
          "got multiple tokens - line: 1, file: testDir/badOnePerLine"),
      DomainError);
}

TEST_F(ListFileTest, MultiplePerLine) {
  const ListFile f{MultiplePerLine, ListFile::FileType::MultiplePerLine};
  EXPECT_EQ(f.level(), JlptLevels::None);
  EXPECT_EQ(f.name(), "MultiplePerLine");
  const auto results = {"東", "西", "線"};
  EXPECT_EQ(f.list().size(), results.size());
  for (size_t pos{}; auto r : results) {
    EXPECT_TRUE(f.exists(r));
    EXPECT_EQ(f.getIndex(r), ++pos);
  }
}

TEST_F(ListFileTest, GlobalDuplicate) {
  const ListFile file{MultiplePerLine, ListFile::FileType::MultiplePerLine};
  const auto f{[] {
    // trying to load the same file causes global duplicate error
    ListFile{MultiplePerLine, ListFile::FileType::MultiplePerLine};
  }};
  EXPECT_THROW(call(f, "found globally non-unique entry '東' - line: 1, file: "
                       "testDir/multiplePerLine"),
      DomainError);
}

TEST_F(ListFileTest, GlobalDuplicateLevel) {
  const LevelListFile file{GoodOnePerLineLevel, JlptLevels::N2};
  const auto f{[] {
    // trying to load the same 'typed' file causes duplicate error
    LevelListFile{GoodOnePerLineLevel, JlptLevels::N3};
  }};
  EXPECT_THROW(call(f, "found 3 duplicates in N3: 犬 猫 虎, file: "
                       "testDir/goodOnePerLineLevel"),
      DomainError);
}

TEST_F(ListFileTest, BadSymbol) {
  EXPECT_THROW(
      call([] { ListFile{BadSymbol}; },
          "invalid multi-byte token 'a' - line: 1, file: testDir/badSymbol"),
      DomainError);
}

TEST_F(ListFileTest, DuplicateSymbol) {
  EXPECT_THROW(
      call([] { ListFile{DuplicateSymbol}; },
          "got duplicate token '車 - line: 2, file: testDir/duplicateSymbol"),
      DomainError);
}

TEST_F(ListFileTest, MaxEntries) {
  // need to write more than 65K unique multi-byte characters to a file so loop
  // over all 'CommonKanjiBlocks' (even though some aren't real characters)
  std::ofstream f{BigFile};
  for (ListFile::Index c{}; auto& i : CommonKanjiBlocks)
    for (auto j = i.start(); j < i.end() && c <= ListFile::MaxEntries + 1;
         ++j, ++c)
      f << toUtf8(j) << '\n';
  f.close();
  EXPECT_THROW(call([] { ListFile{BigFile}; },
                   "exceeded '65534' entries, file: testDir/bigFile"),
      DomainError);
}

} // namespace kanji_tools
