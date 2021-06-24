#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <kanji/Kanji.h>

#include <fstream>

namespace kanji {

namespace fs = std::filesystem;

using ::testing::_;
using ::testing::Return;

class MockData : public Data {
public:
  MockData(const fs::path& p) : Data(p, false) {}
  MOCK_METHOD(int, getFrequency, (const std::string&), (const, override));
  MOCK_METHOD(Levels, getLevel, (const std::string&), (const, override));
};

class KanjiTest : public ::testing::Test {
protected:
  KanjiTest()
    : _testDir("testDir"), _extra(_testDir / "extras.txt"), _radical(0, "", Radical::AltForms(), "", ""),
      _data(_testDir) {}

  void SetUp() override {
    if (fs::exists(_testDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(_testDir));
    std::array files = {std::make_pair(_extra, "\
Number	Name	Radical	Strokes	Meaning	Reading\n\
1	霙	雨	16	sleet	エイ、ヨウ、みぞれ")};
    for (auto& i : files) {
      std::ofstream of(i.first);
      of << i.second;
      of.close();
    }
  }
  void TearDown() override { fs::remove_all(_testDir); }

  fs::path _testDir;
  fs::path _extra;
  Radical _radical;
  MockData _data;
};

/*
TEST_F(KanjiTest, GoodOnePerLine) {
  EXPECT_CALL(_data, getLevel(_)).WillOnce(Return(Levels::None));
  auto results = FileListKanji::fromFile(_data, Types::Extra, _extra);
  EXPECT_EQ(results.size(), 1);
}
*/

} // namespace kanji
