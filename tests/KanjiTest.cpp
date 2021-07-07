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
  MockData(const fs::path& p) : Data(p, false) {
    radical("雨");
    radical("二");
    radical("心");
    radical("色");
    strokes("亘", 6);
    strokes("亙", 6);
    strokes("云", 6);
  }
  MOCK_METHOD(int, getFrequency, (const std::string&), (const, override));
  MOCK_METHOD(Levels, getLevel, (const std::string&), (const, override));
private:
  void radical(const std::string& name) {
    static int count;
    _radicals.insert(std::make_pair(name, Radical(++count, name, Radical::AltForms(), "", "")));
  }
  void strokes(const std::string& name, int count) {
    _strokes.insert(std::make_pair(name, count));
  }
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

  void checkExtraKanji(const Kanji& k) const {
    EXPECT_EQ(k.grade(), Grades::None);
    EXPECT_EQ(k.level(), Levels::None);
    EXPECT_EQ(k.frequency(), 0);
    EXPECT_EQ(k.name(), "霙");
    EXPECT_EQ(k.strokes(), 16);
    EXPECT_EQ(k.meaning(), "sleet");
    EXPECT_EQ(k.reading(), "エイ、ヨウ、みぞれ");
    EXPECT_EQ(k.info(), "Rad 雨, Strokes 16");
    EXPECT_FALSE(k.hasGrade());
    EXPECT_FALSE(k.hasLevel());
    EXPECT_TRUE(k.hasMeaning());
    EXPECT_TRUE(k.hasReading());
    ASSERT_EQ(k.type(), Types::Extra);
    auto& e = static_cast<const ExtraKanji&>(k);
    EXPECT_EQ(e.radical().name(), "雨");
  }

  void checkJinmeiKanji(const Kanji& k) const {
    EXPECT_EQ(k.grade(), Grades::None);
    EXPECT_EQ(k.level(), Levels::N1);
    EXPECT_EQ(k.frequency(), 1728);
    EXPECT_EQ(k.name(), "亘");
    EXPECT_EQ(k.reading(), "コウ、カン、わた-る、もと-める");
    EXPECT_FALSE(k.hasMeaning());
    EXPECT_EQ(k.strokes(), 6);
    ASSERT_EQ(k.type(), Types::Jinmei);
    EXPECT_EQ(k.info(), "Rad 二, Strokes 6, Level N1, Freq 1728, Old 亙");
    auto& e = static_cast<const JinmeiKanji&>(k);
    EXPECT_EQ(e.radical().name(), "二");
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
  EXPECT_EQ(k.info(), "Freq 2362");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_FALSE(k.hasReading());
}

TEST_F(KanjiTest, OtherKanjiWithReading) {
  int frequency = 2362;
  EXPECT_CALL(_data, getFrequency(_)).WillOnce(Return(frequency));
  ReadingKanji k(_data, 4, "呑", "トン、ドン、の-む");
  EXPECT_EQ(k.type(), Types::Other);
  EXPECT_TRUE(k.is(Types::Other));
  EXPECT_EQ(k.name(), "呑");
  EXPECT_EQ(k.number(), 4);
  EXPECT_EQ(k.frequency(), frequency);
  EXPECT_EQ(k.level(), Levels::None);
  EXPECT_EQ(k.grade(), Grades::None);
  EXPECT_EQ(k.info(), "Freq 2362");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_TRUE(k.hasReading());
  EXPECT_EQ(k.reading(), "トン、ドン、の-む");
}

TEST_F(KanjiTest, ExtraFile) {
  writeTestFile("\
Number\tName\tRadical\tStrokes\tMeaning\tReading\n\
1\t霙\t雨\t16\tsleet\tエイ、ヨウ、みぞれ");
  EXPECT_CALL(_data, getLevel(_)).WillOnce(Return(Levels::None));
  auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
  ASSERT_EQ(results.size(), 1);
  checkExtraKanji(*results[0]);
}

TEST_F(KanjiTest, ExtraFileWithDifferentColumnOrder) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_CALL(_data, getLevel(_)).WillOnce(Return(Levels::None));
  auto results = FileListKanji::fromFile(_data, Types::Extra, _testFile);
  ASSERT_EQ(results.size(), 1);
  checkExtraKanji(*results[0]);
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
Number\tName\tRadical\tOldName\tYear\tReason\tReading\n\
7\t云\t二\t\t2004\tPrint\tウン、い-う、ここに\n\
8\t亘\t二\t亙\t1951\tNames\tコウ、カン、わた-る、もと-める");
  EXPECT_CALL(_data, getLevel("云")).WillOnce(Return(Levels::None));
  EXPECT_CALL(_data, getFrequency("云")).WillOnce(Return(0));
  EXPECT_CALL(_data, getLevel("亘")).WillOnce(Return(Levels::N1));
  EXPECT_CALL(_data, getFrequency("亘")).WillOnce(Return(1728));
  auto results = FileListKanji::fromFile(_data, Types::Jinmei, _testFile);
  ASSERT_EQ(results.size(), 2);

  auto& k = *results[0];
  EXPECT_EQ(k.grade(), Grades::None);
  EXPECT_FALSE(k.hasLevel());
  EXPECT_EQ(k.frequency(), 0);
  EXPECT_EQ(k.name(), "云");
  EXPECT_EQ(k.strokes(), 6);
  ASSERT_EQ(k.type(), Types::Jinmei);
  auto& e = static_cast<const JinmeiKanji&>(k);
  EXPECT_EQ(e.radical().name(), "二");
  EXPECT_EQ(e.oldName(), std::nullopt);
  EXPECT_EQ(e.year(), 2004);
  EXPECT_EQ(e.reason(), JinmeiKanji::Reasons::Print);
  checkJinmeiKanji(*results[1]);
}

TEST_F(KanjiTest, LinkedJinmei) {
  writeTestFile("\
Number\tName\tRadical\tOldName\tYear\tReason\tReading\n\
1\t亘\t二\t亙\t1951\tNames\tコウ、カン、わた-る、もと-める");
  EXPECT_CALL(_data, getLevel("亘")).WillOnce(Return(Levels::N1));
  EXPECT_CALL(_data, getFrequency("亘")).WillOnce(Return(1728));
  EXPECT_CALL(_data, getFrequency("亙")).WillOnce(Return(0));
  auto results = FileListKanji::fromFile(_data, Types::Jinmei, _testFile);
  ASSERT_EQ(results.size(), 1);
  LinkedJinmeiKanji k(_data, 7, "亙", results[0]);
  EXPECT_EQ(k.type(), Types::LinkedJinmei);
  EXPECT_EQ(k.name(), "亙");
  EXPECT_EQ(k.level(), Levels::None);
  EXPECT_EQ(k.grade(), Grades::None);
  EXPECT_EQ(k.frequency(), 0);
  EXPECT_EQ(k.reading(), "コウ、カン、わた-る、もと-める");
  EXPECT_EQ(k.info(Kanji::NewField), "New 亘");
  EXPECT_FALSE(k.hasMeaning());
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
Number\tName\tRadical\tOldName\tYear\tReading\n\
1\t亘\t二\t亙\t1951\tコウ、カン、わた-る、もと-める");
  try {
    auto results = FileListKanji::fromFile(_data, Types::Jinmei, _testFile);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(std::string(err.what()), "missing required column: Reason, file: testDir/test.txt");
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(KanjiTest, JouyouFile) {
  writeTestFile("\
Number\tName\tRadical\tOldName\tYear\tStrokes\tGrade\tMeaning\tReading\n\
4\t愛\t心\t\t\t13\t4\tlove\tアイ\n\
103\t艶\t色\t艷\t2010\t19\tS\tglossy\tエン、つや");
  EXPECT_CALL(_data, getLevel("愛")).WillOnce(Return(Levels::N3));
  EXPECT_CALL(_data, getFrequency("愛")).WillOnce(Return(640));
  EXPECT_CALL(_data, getLevel("艶")).WillOnce(Return(Levels::N1));
  EXPECT_CALL(_data, getFrequency("艶")).WillOnce(Return(2207));
  auto results = FileListKanji::fromFile(_data, Types::Jouyou, _testFile);
  ASSERT_EQ(results.size(), 2);

  for (auto& i : results) {
    auto& k = *i;
    ASSERT_EQ(k.type(), Types::Jouyou);
    auto& e = static_cast<const JouyouKanji&>(k);
    if (k.number() == 4) {
      EXPECT_EQ(k.grade(), Grades::G4);
      EXPECT_EQ(k.level(), Levels::N3);
      EXPECT_EQ(k.frequency(), 640);
      EXPECT_EQ(k.name(), "愛");
      EXPECT_EQ(k.strokes(), 13);
      EXPECT_EQ(k.meaning(), "love");
      EXPECT_EQ(k.reading(), "アイ");
      EXPECT_EQ(e.radical().name(), "心");
      EXPECT_EQ(e.oldName(), std::nullopt);
      EXPECT_EQ(e.year(), std::nullopt);
      EXPECT_EQ(k.info(), "Rad 心, Strokes 13, Grade G4, Level N3, Freq 640");
    } else {
      EXPECT_EQ(k.number(), 103);
      EXPECT_EQ(k.grade(), Grades::S);
      EXPECT_EQ(k.level(), Levels::N1);
      EXPECT_EQ(k.frequency(), 2207);
      EXPECT_EQ(k.name(), "艶");
      EXPECT_EQ(k.meaning(), "glossy");
      EXPECT_EQ(k.reading(), "エン、つや");
      EXPECT_EQ(k.strokes(), 19);
      EXPECT_EQ(e.radical().name(), "色");
      EXPECT_EQ(e.oldName(), "艷");
      EXPECT_EQ(e.year(), 2010);
      EXPECT_EQ(k.info(), "Rad 色, Strokes 19, Grade S, Level N1, Freq 2207, Old 艷");
      EXPECT_EQ(k.info(Kanji::RadicalField), "Rad 色");
      EXPECT_EQ(k.info(Kanji::StrokesField), "Strokes 19");
      EXPECT_EQ(k.info(Kanji::GradeField), "Grade S");
      EXPECT_EQ(k.info(Kanji::LevelField), "Level N1");
      EXPECT_EQ(k.info(Kanji::OldField), "Old 艷");
      EXPECT_EQ(k.info(Kanji::NewField), "");
      EXPECT_EQ(k.info(Kanji::GradeField | Kanji::OldField), "Grade S, Old 艷");
      EXPECT_EQ(k.info(Kanji::StrokesField | Kanji::LevelField), "Strokes 19, Level N1");
    }
  }
}

TEST_F(KanjiTest, LinkedOld) {
  writeTestFile("\
Number\tName\tRadical\tOldName\tYear\tStrokes\tGrade\tMeaning\tReading\n\
103\t艶\t色\t艷\t2010\t19\tS\tglossy\tエン、つや");
  EXPECT_CALL(_data, getLevel("艶")).WillOnce(Return(Levels::N1));
  EXPECT_CALL(_data, getFrequency("艶")).WillOnce(Return(2207));
  EXPECT_CALL(_data, getFrequency("艷")).WillOnce(Return(0));
  auto results = FileListKanji::fromFile(_data, Types::Jouyou, _testFile);
  ASSERT_EQ(results.size(), 1);
  LinkedOldKanji k(_data, 7, "艷", results[0]);
  EXPECT_EQ(k.type(), Types::LinkedOld);
  EXPECT_EQ(k.name(), "艷");
  EXPECT_EQ(k.level(), Levels::None);
  EXPECT_EQ(k.grade(), Grades::None);
  EXPECT_EQ(k.frequency(), 0);
  EXPECT_EQ(k.reading(), "エン、つや");
  EXPECT_EQ(k.meaning(), "glossy");
  EXPECT_EQ(k.link(), results[0]);
  EXPECT_EQ(k.info(), "New 艶");
}

TEST_F(KanjiTest, BadLinkedOld) {
  EXPECT_CALL(_data, getFrequency(_)).WillOnce(Return(2362));
  auto other = std::make_shared<Kanji>(_data, 4, "呑");
  try {
    LinkedOldKanji k(_data, 7, "艷", other);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(std::string(err.what()), "LinkedKanji 艷 wanted type 'Jouyou' for link 呑, but got 'Other'");
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

} // namespace kanji
