#include <gtest/gtest.h>

#include <kanji_tools/utils/DataFile.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

class DataFileTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (fs::exists(_testDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(_testDir));
    std::array files = {std::make_pair(_goodOnePerLine, "北\n海\n道"),
                        std::make_pair(_goodOnePerLineLevel, "犬\n猫\n虎"),
                        std::make_pair(_badOnePerLine, "焼 肉"),
                        std::make_pair(_multiplePerLine, "東 西 線"),
                        std::make_pair(_badSymbol, "a"),
                        std::make_pair(_duplicateSymbol, "車\n車")};
    for (auto& i : files) {
      std::ofstream of(i.first);
      of << i.second;
      of.close();
    }
  }
  void TearDown() override { fs::remove_all(_testDir); }
  fs::path _testDir = "testDir";
  fs::path _goodOnePerLine = _testDir / "goodOnePerLine";
  fs::path _goodOnePerLineLevel = _testDir / "goodOnePerLineLevel";
  fs::path _multiplePerLine = _testDir / "multiplePerLine";
  fs::path _badOnePerLine = _testDir / "badOnePerLine";
  fs::path _badSymbol = _testDir / "badSymbol";
  fs::path _duplicateSymbol = _testDir / "duplicateSymbol";
};

TEST_F(DataFileTest, GoodOnePerLine) {
  DataFile f(_goodOnePerLine);
  EXPECT_EQ(f.level(), JlptLevels::None);
  EXPECT_EQ(f.kyu(), KenteiKyus::None);
  EXPECT_EQ(f.name(), "GoodOnePerLine");
  std::array results = {"北", "海", "道"};
  EXPECT_EQ(f.list().size(), results.size());
  int pos = 0;
  for (auto r : results) {
    EXPECT_TRUE(f.exists(r));
    // numbers start at 1
    EXPECT_EQ(f.get(r), ++pos);
  }
  EXPECT_EQ(f.toString(), "北海道");
}

TEST_F(DataFileTest, GoodOnePerLineLevel) {
  LevelDataFile f(_goodOnePerLineLevel, JlptLevels::N2);
  EXPECT_EQ(f.level(), JlptLevels::N2);
  EXPECT_EQ(f.kyu(), KenteiKyus::None);
  EXPECT_EQ(f.name(), "N2");
  std::array results = {"犬", "猫", "虎"};
  EXPECT_EQ(f.list().size(), results.size());
  int pos = 0;
  for (auto r : results) {
    EXPECT_TRUE(f.exists(r));
    EXPECT_EQ(f.get(r), ++pos);
  }
}

TEST_F(DataFileTest, BadOnePerLine) {
  try {
    DataFile f(_badOnePerLine);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("got multiple tokens - line: 1, file: testDir/badOnePerLine"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(DataFileTest, MultiplePerLine) {
  DataFile f(_multiplePerLine, DataFile::FileType::MultiplePerLine);
  EXPECT_EQ(f.level(), JlptLevels::None);
  EXPECT_EQ(f.name(), "MultiplePerLine");
  std::array results = {"東", "西", "線"};
  EXPECT_EQ(f.list().size(), results.size());
  int pos = 0;
  for (auto r : results) {
    EXPECT_TRUE(f.exists(r));
    EXPECT_EQ(f.get(r), ++pos);
  }
}

TEST_F(DataFileTest, GlobalDuplicate) {
  try {
    DataFile f(_multiplePerLine, DataFile::FileType::MultiplePerLine);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("found globally non-unique entry '東' - line: 1, file: testDir/multiplePerLine"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(DataFileTest, GlobalDuplicateLevel) {
  try {
    LevelDataFile f(_goodOnePerLineLevel, JlptLevels::N3);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("found 3 duplicates in N3, file: testDir/goodOnePerLineLevel"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(DataFileTest, BadSymbol) {
  try {
    DataFile f(_badSymbol);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("invalid multi-byte token 'a' - line: 1, file: testDir/badSymbol"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(DataFileTest, DuplicateSymbol) {
  try {
    DataFile f(_duplicateSymbol);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("got duplicate token '車 - line: 2, file: testDir/duplicateSymbol"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

} // namespace kanji_tools
