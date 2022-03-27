#include <gtest/gtest.h>
#include <kanji_tools/utils/DataFile.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path TestDir{"testDir"};
const fs::path GoodOnePerLine{TestDir / "goodOnePerLine"},
    GoodOnePerLineLevel{TestDir / "goodOnePerLineLevel"},
    MultiplePerLine{TestDir / "multiplePerLine"},
    BadOnePerLine{TestDir / "badOnePerLine"}, BadSymbol{TestDir / "badSymbol"},
    DuplicateSymbol{TestDir / "duplicateSymbol"};

} // namespace

class DataFileTest : public ::testing::Test {
protected:
  void SetUp() override {
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
  void TearDown() override { fs::remove_all(TestDir); }
};

TEST(DataTest, Usage) {
  const std::string msg{"error msg"};
  EXPECT_THROW(call([&msg] { DataFile::usage(msg); }, msg), std::domain_error);
}

TEST_F(DataFileTest, GoodOnePerLine) {
  const DataFile f{GoodOnePerLine};
  EXPECT_EQ(f.level(), JlptLevels::None);
  EXPECT_EQ(f.kyu(), KenteiKyus::None);
  EXPECT_EQ(f.name(), "GoodOnePerLine");
  const auto results = {"北", "海", "道"};
  EXPECT_EQ(f.list().size(), results.size());
  for (size_t pos{}; auto r : results) {
    EXPECT_TRUE(f.exists(r));
    // numbers start at 1
    EXPECT_EQ(f.get(r), ++pos);
  }
  EXPECT_EQ(f.toString(), "北海道");
}

TEST_F(DataFileTest, GoodOnePerLineLevel) {
  const LevelDataFile f{GoodOnePerLineLevel, JlptLevels::N2};
  EXPECT_EQ(f.level(), JlptLevels::N2);
  EXPECT_EQ(f.kyu(), KenteiKyus::None);
  EXPECT_EQ(f.name(), "N2");
  const auto results = {"犬", "猫", "虎"};
  EXPECT_EQ(f.list().size(), results.size());
  for (size_t pos{}; auto r : results) {
    EXPECT_TRUE(f.exists(r));
    EXPECT_EQ(f.get(r), ++pos);
  }
}

TEST_F(DataFileTest, BadOnePerLine) {
  EXPECT_THROW(
      call([] { DataFile{BadOnePerLine}; },
          "got multiple tokens - line: 1, file: testDir/badOnePerLine"),
      std::domain_error);
}

TEST_F(DataFileTest, MultiplePerLine) {
  const DataFile f{MultiplePerLine, DataFile::FileType::MultiplePerLine};
  EXPECT_EQ(f.level(), JlptLevels::None);
  EXPECT_EQ(f.name(), "MultiplePerLine");
  const auto results = {"東", "西", "線"};
  EXPECT_EQ(f.list().size(), results.size());
  for (size_t pos{}; auto r : results) {
    EXPECT_TRUE(f.exists(r));
    EXPECT_EQ(f.get(r), ++pos);
  }
}

TEST_F(DataFileTest, GlobalDuplicate) {
  EXPECT_THROW(
      call(
          [] {
            DataFile{MultiplePerLine, DataFile::FileType::MultiplePerLine};
          },
          "found globally non-unique entry '東' - line: 1, file: "
          "testDir/multiplePerLine"),
      std::domain_error);
}

TEST_F(DataFileTest, GlobalDuplicateLevel) {
  EXPECT_THROW(
      call(
          [] {
            LevelDataFile{GoodOnePerLineLevel, JlptLevels::N3};
          },
          "found 3 duplicates in N3, file: testDir/goodOnePerLineLevel"),
      std::domain_error);
}

TEST_F(DataFileTest, BadSymbol) {
  EXPECT_THROW(
      call([] { DataFile f(BadSymbol); },
          "invalid multi-byte token 'a' - line: 1, file: testDir/badSymbol"),
      std::domain_error);
}

TEST_F(DataFileTest, DuplicateSymbol) {
  EXPECT_THROW(
      call([] { DataFile f(DuplicateSymbol); },
          "got duplicate token '車 - line: 2, file: testDir/duplicateSymbol"),
      std::domain_error);
}

} // namespace kanji_tools
