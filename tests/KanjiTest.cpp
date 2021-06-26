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
  MockData(const fs::path& p) : Data(p, false), _radical(0, "", Radical::AltForms(), "", "") {
    _radicals.insert(std::make_pair("雨", _radical));
    _radicals.insert(std::make_pair("二", _radical));
    _strokes.insert(std::make_pair("亘", 6));
    _strokes.insert(std::make_pair("亙", 6));
  }
  MOCK_METHOD(int, getFrequency, (const std::string&), (const, override));
  MOCK_METHOD(Levels, getLevel, (const std::string&), (const, override));

  Radical _radical;
};

class KanjiTest : public ::testing::Test {
protected:
  KanjiTest() : _testDir("testDir"), _testFile(_testDir / "test.txt"), _data(_testDir) {}

  void SetUp() override {
    if (fs::exists(_testDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(_testDir));
  }
  void TearDown() override { fs::remove_all(_testDir); }

  void writeTestFile(const std::string& s) {
    std::ofstream of(_testFile);
    of << s;
    of.close();
  }
  void checkExtraKanji(const Data::Entry& k) const {
    EXPECT_EQ(k->type(), Types::Extra);
    EXPECT_EQ(k->grade(), Grades::None);
    EXPECT_EQ(k->level(), Levels::None);
    EXPECT_EQ(k->frequency(), 0);
    EXPECT_EQ(k->name(), "霙");
    auto& e = static_cast<const ExtraKanji&>(*k);
    EXPECT_EQ(e.radical(), _data._radical);
    EXPECT_EQ(e.strokes(), 16);
    EXPECT_EQ(e.meaning(), "sleet");
    EXPECT_EQ(e.reading(), "エイ、ヨウ、みぞれ");
  }
  void checkJinmeiKanji(const Data::Entry& k) const {
    EXPECT_EQ(k->type(), Types::Jinmei);
    EXPECT_EQ(k->grade(), Grades::None);
    EXPECT_EQ(k->level(), Levels::N1);
    EXPECT_EQ(k->frequency(), 1728);
    EXPECT_EQ(k->name(), "亘");
    auto& e = static_cast<const JinmeiKanji&>(*k);
    EXPECT_EQ(e.radical(), _data._radical);
    EXPECT_EQ(e.strokes(), 6);
    EXPECT_EQ(e.oldName(), "亙");
    EXPECT_EQ(e.year(), 1951);
    EXPECT_EQ(e.reason(), JinmeiKanji::Reasons::Names);
  }

  fs::path _testDir;
  fs::path _testFile;
  MockData _data;
};

TEST_F(KanjiTest, OtherKanji) {
  int frequency = 2362;
  EXPECT_CALL(_data, getFrequency(_)).WillOnce(Return(frequency));
  Kanji k(_data, 4, "呑");
  EXPECT_EQ(k.type(), Types::Other);
  EXPECT_EQ(k.name(), "呑");
  EXPECT_EQ(k.number(), 4);
  EXPECT_EQ(k.frequency(), frequency);
  EXPECT_EQ(k.level(), Levels::None);
  EXPECT_EQ(k.grade(), Grades::None);
}

TEST_F(KanjiTest, ExtraFile) {
  writeTestFile("\
Number\tName\tRadical\tStrokes\tMeaning\tReading\n\
1\t霙\t雨\t16\tsleet\tエイ、ヨウ、みぞれ");
  EXPECT_CALL(_data, getLevel(_)).WillOnce(Return(Levels::None));
  auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
  EXPECT_EQ(results.size(), 1);
  checkExtraKanji(results[0]);
}

TEST_F(KanjiTest, ExtraFileWithDifferentColumnOrder) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_CALL(_data, getLevel(_)).WillOnce(Return(Levels::None));
  auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
  EXPECT_EQ(results.size(), 1);
  checkExtraKanji(results[0]);
}

TEST_F(KanjiTest, ExtraFileWithUnrecognizedColumn) {
  writeTestFile("\
Name\tNumber\tRdical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  try {
    auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("unrecognized column: Rdical, file: testDir/test.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(KanjiTest, ExtraFileWithDuplicateColumn) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tName\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  try {
    auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("duplicate column: Name, file: testDir/test.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(KanjiTest, ExtraFileWithToManyColumn) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16\t16");
  try {
    auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("too many columns - line: 2, file: testDir/test.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(KanjiTest, ExtraFileWithNotEnoughColumn) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ");
  try {
    auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("not enough columns - line: 2, file: testDir/test.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(KanjiTest, ExtraFileWithInvalidData) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\ta\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  try {
    auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(std::string(err.what()),
              "got exception while creating kanji 'failed to convert to int: a' - line: 2, file: testDir/test.txt");
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(KanjiTest, JinmeiFile) {
  writeTestFile("\
Number	Name	Radical	OldName	Year	Reason\n\
1	亘	二	亙	1951	Names");
  EXPECT_CALL(_data, getLevel(_)).WillOnce(Return(Levels::N1));
  EXPECT_CALL(_data, getFrequency(_)).WillOnce(Return(1728));
  auto results = FileListKanji::fromFile(_data, Types::Jinmei, _testFile);
  EXPECT_EQ(results.size(), 1);
  checkJinmeiKanji(results[0]);
}

TEST_F(KanjiTest, LinkedJinmei) {
  writeTestFile("\
Number	Name	Radical	OldName	Year	Reason\n\
1	亘	二	亙	1951	Names");
  EXPECT_CALL(_data, getLevel("亘")).WillOnce(Return(Levels::N1));
  EXPECT_CALL(_data, getFrequency("亘")).WillOnce(Return(1728));
  EXPECT_CALL(_data, getFrequency("亙")).WillOnce(Return(0));
  auto results = FileListKanji::fromFile(_data, Types::Jinmei, _testFile);
  EXPECT_EQ(results.size(), 1);
  LinkedJinmeiKanji k(_data, 7, "亙", results[0]);
  EXPECT_EQ(k.type(), Types::LinkedJinmei);
  EXPECT_EQ(k.name(), "亙");
  EXPECT_EQ(k.level(), Levels::None);
  EXPECT_EQ(k.grade(), Grades::None);
  EXPECT_EQ(k.frequency(), 0);
  EXPECT_EQ(k.link(), results[0]);
}

TEST_F(KanjiTest, BadLinkedJinmei) {
  EXPECT_CALL(_data, getFrequency(_)).WillOnce(Return(2362));
  auto other = std::make_shared<Kanji>(_data, 4, "呑");
  try {
    LinkedJinmeiKanji k(_data, 7, "亙", other);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(std::string(err.what()), "LinkedKanji 亙 wanted type 'Jouyou' or 'Jinmei' for link 呑, but got 'Other'");
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(KanjiTest, JinmeiFileWithMissingReason) {
  writeTestFile("\
Number	Name	Radical	OldName	Year\n\
1	亘	二	亙	1951");
  try {
    auto results = FileListKanji::fromFile(_data, Types::Jinmei, _testFile);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(std::string(err.what()), "missing required column: Reason, file: testDir/test.txt");
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

} // namespace kanji
