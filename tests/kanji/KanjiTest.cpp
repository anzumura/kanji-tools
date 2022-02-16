#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <kanji_tools/kanji/CustomFileKanji.h>
#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/kanji/UcdFileKanji.h>
#include <kanji_tools/tests/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class MockData : public Data {
public:
  MockData(const fs::path& p) : Data(p, Data::DebugMode::None) {
    strokes("亘", 6);
    strokes("亙", 6);
    strokes("云", 6);
  }
  MOCK_METHOD(Kanji::OptInt, getFrequency, (const std::string&), (const, override));
  MOCK_METHOD(JlptLevels, getLevel, (const std::string&), (const, override));
  MOCK_METHOD(KenteiKyus, getKyu, (const std::string&), (const, override));
  MOCK_METHOD(const Radical&, ucdRadical, (const std::string&, const Ucd*), (const, override));
  MOCK_METHOD(const Radical&, getRadicalByName, (const std::string&), (const, override));
private:
  void strokes(const std::string& kanjiName, int count) { _strokes.emplace(kanjiName, count); }
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
    EXPECT_FALSE(k.hasGrade());
    EXPECT_FALSE(k.hasLevel());
    EXPECT_FALSE(k.frequency());
    EXPECT_EQ(k.kyu(), KenteiKyus::K1);
    EXPECT_EQ(k.name(), "霙");
    EXPECT_EQ(k.strokes(), 16);
    EXPECT_EQ(k.meaning(), "sleet");
    EXPECT_EQ(k.reading(), "エイ、ヨウ、みぞれ");
    EXPECT_EQ(k.info(), "Rad 雨(1), Strokes 16, K1");
    EXPECT_FALSE(k.hasGrade());
    EXPECT_FALSE(k.hasLevel());
    EXPECT_TRUE(k.hasMeaning());
    EXPECT_TRUE(k.hasReading());
    ASSERT_EQ(k.type(), KanjiTypes::Extra);
    auto& e = static_cast<const ExtraKanji&>(k);
    EXPECT_EQ(e.radical().name(), "雨");
  }

  void checkJinmeiKanji(const Kanji& k) const {
    EXPECT_FALSE(k.hasGrade());
    EXPECT_EQ(k.level(), JlptLevels::N1);
    EXPECT_EQ(k.kyu(), KenteiKyus::KJ1);
    EXPECT_EQ(k.frequency(), Kanji::OptInt(1728));
    EXPECT_EQ(k.name(), "亘");
    EXPECT_EQ(k.reading(), "コウ、カン、わた-る、もと-める");
    EXPECT_FALSE(k.hasMeaning());
    EXPECT_EQ(k.strokes(), 6);
    ASSERT_EQ(k.type(), KanjiTypes::Jinmei);
    EXPECT_EQ(k.extraTypeInfo(), "#8 1951 [Names]");
    EXPECT_EQ(k.info(), "Rad 二(1), Strokes 6, N1, Frq 1728, Old 亙, KJ1");
    auto& e = static_cast<const JinmeiKanji&>(k);
    EXPECT_EQ(e.radical().name(), "二");
    EXPECT_EQ(e.oldNames(), Kanji::LinkNames{"亙"});
    EXPECT_EQ(e.year(), 1951);
    EXPECT_EQ(e.reason(), JinmeiKanji::Reasons::Names);
  }

  const fs::path _testDir;
  const fs::path _testFile;
  const MockData _data;
};

TEST_F(KanjiTest, FrequencyKanji) {
  auto frequency = 2362;
  auto kyu = KenteiKyus::KJ1;
  EXPECT_CALL(_data, getKyu("呑")).WillOnce(Return(kyu));
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, ucdRadical(_, _)).WillOnce(ReturnRef(rad));
  FrequencyKanji k(_data, "呑", frequency);
  EXPECT_EQ(k.type(), KanjiTypes::Frequency);
  EXPECT_EQ(k.name(), "呑");
  EXPECT_EQ(k.radical(), rad);
  EXPECT_EQ(k.frequency(), Kanji::OptInt(frequency));
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_EQ(k.kyu(), kyu);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Frq 2362, KJ1");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_FALSE(k.hasReading());
}

TEST_F(KanjiTest, FrequencyKanjiWithReading) {
  auto frequency = 2362;
  auto kyu = KenteiKyus::KJ1;
  EXPECT_CALL(_data, getKyu("呑")).WillOnce(Return(kyu));
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, ucdRadical(_, _)).WillOnce(ReturnRef(rad));
  FrequencyKanji k(_data, "呑", "トン、ドン、の-む", frequency);
  EXPECT_EQ(k.type(), KanjiTypes::Frequency);
  EXPECT_TRUE(k.is(KanjiTypes::Frequency));
  EXPECT_EQ(k.name(), "呑");
  EXPECT_EQ(k.radical(), rad);
  EXPECT_EQ(k.frequency(), Kanji::OptInt(frequency));
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_EQ(k.kyu(), kyu);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Frq 2362, KJ1");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_TRUE(k.hasReading());
  EXPECT_EQ(k.reading(), "トン、ドン、の-む");
}

TEST_F(KanjiTest, KenteiKanji) {
  auto kyu = KenteiKyus::K1;
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, ucdRadical(_, _)).WillOnce(ReturnRef(rad));
  KenteiKanji k(_data, "蘋", kyu);
  EXPECT_EQ(k.type(), KanjiTypes::Kentei);
  EXPECT_EQ(k.name(), "蘋");
  EXPECT_EQ(k.radical(), rad);
  EXPECT_FALSE(k.frequency());
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_EQ(k.kyu(), kyu);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), K1");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_FALSE(k.hasReading());
}

TEST_F(KanjiTest, UcdKanjiWithNewName) {
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, ucdRadical(_, _)).WillOnce(ReturnRef(rad));
  const std::string sampleLink("sampleLink");
  Ucd ucd(0, "侭", "", "", 0, 0, 0, "", "123P", "456 789", false, false, Ucd::Links({Ucd::Link(1, sampleLink)}),
          UcdLinkTypes::Simplified, false, "utmost", "JIN", "MAMA");
  UcdKanji k(_data, ucd);
  EXPECT_EQ(k.type(), KanjiTypes::Ucd);
  EXPECT_EQ(k.name(), "侭");
  EXPECT_EQ(k.radical(), rad);
  EXPECT_FALSE(k.frequency());
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.hasKyu());
  EXPECT_EQ(k.morohashiId(), Kanji::OptString("123P"));
  EXPECT_EQ(k.nelsonIds(), Kanji::NelsonIds({456, 789}));
  EXPECT_EQ(k.meaning(), "utmost");
  EXPECT_EQ(k.reading(), "ジン、まま");
  ASSERT_TRUE(k.newName());
  EXPECT_EQ(*k.newName(), sampleLink);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), New sampleLink");
}

TEST_F(KanjiTest, UcdKanjiWithLinkedReadingOldNames) {
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, ucdRadical(_, _)).WillOnce(ReturnRef(rad));
  Ucd ucd(0, "侭", "", "", 0, 0, 0, "", "", "", false, false, Ucd::Links({Ucd::Link(1, "old1"), Ucd::Link(2, "old2")}),
          UcdLinkTypes::Traditional, true, "utmost", "JIN", "MAMA");
  UcdKanji k(_data, ucd);
  ASSERT_FALSE(k.newName());
  EXPECT_EQ(k.oldNames(), Kanji::LinkNames({"old1", "old2"}));
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Old old1*／old2");
}

TEST_F(KanjiTest, ExtraFile) {
  writeTestFile("\
Number\tName\tRadical\tStrokes\tMeaning\tReading\n\
1\t霙\t雨\t16\tsleet\tエイ、ヨウ、みぞれ");
  EXPECT_CALL(_data, getKyu(_)).WillOnce(Return(KenteiKyus::K1));
  Radical rad(1, "雨", {}, "", "");
  EXPECT_CALL(_data, getRadicalByName("雨")).WillOnce(ReturnRef(rad));
  auto results = ExtraKanji::fromFile(_data, KanjiTypes::Extra, _testFile);
  ASSERT_EQ(results.size(), 1);
  checkExtraKanji(*results[0]);
}

TEST_F(KanjiTest, ExtraFileWithDifferentColumnOrder) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_CALL(_data, getKyu(_)).WillOnce(Return(KenteiKyus::K1));
  Radical rad(1, "雨", {}, "", "");
  EXPECT_CALL(_data, getRadicalByName("雨")).WillOnce(ReturnRef(rad));
  auto results = ExtraKanji::fromFile(_data, KanjiTypes::Extra, _testFile);
  ASSERT_EQ(results.size(), 1);
  checkExtraKanji(*results[0]);
}

TEST_F(KanjiTest, ExtraFileWithUnrecognizedColumn) {
  writeTestFile("\
Name\tNumber\tRdical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_THROW(call([this] { return ExtraKanji::fromFile(_data, KanjiTypes::Extra, _testFile); },
                    "unrecognized header 'Rdical' - file: test.txt"),
               std::domain_error);
}

TEST_F(KanjiTest, ExtraFileWithDuplicateColumn) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tName\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_THROW(call([this] { return ExtraKanji::fromFile(_data, KanjiTypes::Extra, _testFile); },
                    "duplicate header 'Name' - file: test.txt"),
               std::domain_error);
}

TEST_F(KanjiTest, ExtraFileWithToManyColumns) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16\t16");
  EXPECT_THROW(call([this] { return ExtraKanji::fromFile(_data, KanjiTypes::Extra, _testFile); },
                    "too many columns - file: test.txt, row: 1"),
               std::domain_error);
}

TEST_F(KanjiTest, ExtraFileWithNotEnoughColumns) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ");
  EXPECT_THROW(call([this] { return ExtraKanji::fromFile(_data, KanjiTypes::Extra, _testFile); },
                    "not enough columns - file: test.txt, row: 1"),
               std::domain_error);
}

TEST_F(KanjiTest, ExtraFileWithInvalidData) {
  writeTestFile("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\ta\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_CALL(_data, getKyu(_)).WillOnce(Return(KenteiKyus::K1));
  Radical rad(1, "雨", {}, "", "");
  EXPECT_CALL(_data, getRadicalByName("雨")).WillOnce(ReturnRef(rad));
  EXPECT_THROW(call([this] { return ExtraKanji::fromFile(_data, KanjiTypes::Extra, _testFile); },
                    "failed to convert to int - file: test.txt, row: 1, column: 'Number', value: 'a'"),
               std::domain_error);
}

TEST_F(KanjiTest, JinmeiFile) {
  writeTestFile("\
Number\tName\tRadical\tOldNames\tYear\tReason\tReading\n\
7\t云\t二\t\t2004\tPrint\tウン、い-う、ここに\n\
8\t亘\t二\t亙\t1951\tNames\tコウ、カン、わた-る、もと-める");
  EXPECT_CALL(_data, getLevel("云")).WillOnce(Return(JlptLevels::None));
  EXPECT_CALL(_data, getFrequency("云")).WillOnce(Return(std::nullopt));
  EXPECT_CALL(_data, getKyu("云")).WillOnce(Return(KenteiKyus::KJ1));
  EXPECT_CALL(_data, getLevel("亘")).WillOnce(Return(JlptLevels::N1));
  EXPECT_CALL(_data, getFrequency("亘")).WillOnce(Return(Kanji::OptInt(1728)));
  EXPECT_CALL(_data, getKyu("亘")).WillOnce(Return(KenteiKyus::KJ1));
  Radical rad(1, "二", {}, "", "");
  EXPECT_CALL(_data, getRadicalByName("二")).WillRepeatedly(ReturnRef(rad));
  auto results = ExtraKanji::fromFile(_data, KanjiTypes::Jinmei, _testFile);
  ASSERT_EQ(results.size(), 2);

  auto& k = *results[0];
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.frequency());
  EXPECT_EQ(k.name(), "云");
  EXPECT_EQ(k.strokes(), 6);
  EXPECT_EQ(k.kyu(), KenteiKyus::KJ1);
  ASSERT_EQ(k.type(), KanjiTypes::Jinmei);
  EXPECT_EQ(k.radical().name(), "二");
  EXPECT_EQ(k.extraTypeInfo(), "#7 2004 [Print]");
  auto& e = static_cast<const JinmeiKanji&>(k);
  EXPECT_TRUE(e.oldNames().empty());
  EXPECT_EQ(e.year(), 2004);
  EXPECT_EQ(e.reason(), JinmeiKanji::Reasons::Print);
  checkJinmeiKanji(*results[1]);
}

TEST_F(KanjiTest, LinkedJinmei) {
  writeTestFile("\
Number\tName\tRadical\tOldNames\tYear\tReason\tReading\n\
1\t亘\t二\t亙\t1951\tNames\tコウ、カン、わた-る、もと-める");
  EXPECT_CALL(_data, getLevel("亘")).WillOnce(Return(JlptLevels::N1));
  EXPECT_CALL(_data, getFrequency("亘")).WillOnce(Return(Kanji::OptInt(1728)));
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, getRadicalByName("二")).WillOnce(ReturnRef(rad));
  EXPECT_CALL(_data, ucdRadical("亙", _)).WillOnce(ReturnRef(rad));
  EXPECT_CALL(_data, getFrequency("亙")).WillOnce(Return(std::nullopt));
  auto results = ExtraKanji::fromFile(_data, KanjiTypes::Jinmei, _testFile);
  ASSERT_EQ(results.size(), 1);
  LinkedJinmeiKanji k(_data, "亙", results[0]);
  EXPECT_EQ(k.type(), KanjiTypes::LinkedJinmei);
  EXPECT_EQ(k.name(), "亙");
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.frequency());
  EXPECT_EQ(k.reading(), "コウ、カン、わた-る、もと-める");
  EXPECT_EQ(k.info(KanjiInfo::New), "New 亘*");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_EQ(k.link(), results[0]);
}

TEST_F(KanjiTest, BadLinkedJinmei) {
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, ucdRadical("呑", _)).WillOnce(ReturnRef(rad));
  auto frequencyKanji = std::make_shared<FrequencyKanji>(_data, "呑", 2362);
  EXPECT_THROW(call([&, this] { LinkedJinmeiKanji k(_data, "亙", frequencyKanji); },
                    "LinkedKanji 亙 wanted type 'Jouyou' or 'Jinmei' for link 呑, but got 'Frequency'"),
               std::domain_error);
}

TEST_F(KanjiTest, JinmeiFileWithMissingReason) {
  writeTestFile("\
Number\tName\tRadical\tOldNames\tYear\tReading\n\
1\t亘\t二\t亙\t1951\tコウ、カン、わた-る、もと-める");
  EXPECT_THROW(call([this] { return ExtraKanji::fromFile(_data, KanjiTypes::Jinmei, _testFile); },
                    "column 'Reason' not found - file: test.txt"),
               std::domain_error);
}

TEST_F(KanjiTest, JouyouFile) {
  writeTestFile("\
Number\tName\tRadical\tOldNames\tYear\tStrokes\tGrade\tMeaning\tReading\n\
4\t愛\t心\t\t\t13\t4\tlove\tアイ\n\
103\t艶\t色\t艷\t2010\t19\tS\tglossy\tエン、つや");
  Radical heart(1, "心", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, getRadicalByName("心")).WillOnce(ReturnRef(heart));
  Radical color(2, "色", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, getRadicalByName("色")).WillOnce(ReturnRef(color));
  EXPECT_CALL(_data, getLevel("愛")).WillOnce(Return(JlptLevels::N3));
  EXPECT_CALL(_data, getKyu("愛")).WillOnce(Return(KenteiKyus::K7));
  EXPECT_CALL(_data, getFrequency("愛")).WillOnce(Return(Kanji::OptInt(640)));
  EXPECT_CALL(_data, getLevel("艶")).WillOnce(Return(JlptLevels::N1));
  EXPECT_CALL(_data, getKyu("艶")).WillOnce(Return(KenteiKyus::K2));
  EXPECT_CALL(_data, getFrequency("艶")).WillOnce(Return(Kanji::OptInt(2207)));
  auto results = ExtraKanji::fromFile(_data, KanjiTypes::Jouyou, _testFile);
  ASSERT_EQ(results.size(), 2);

  for (auto& i : results) {
    auto& k = *i;
    ASSERT_EQ(k.type(), KanjiTypes::Jouyou);
    auto& e = static_cast<const JouyouKanji&>(k);
    if (k.grade() == KanjiGrades::G4) {
      EXPECT_EQ(k.level(), JlptLevels::N3);
      EXPECT_EQ(k.kyu(), KenteiKyus::K7);
      EXPECT_EQ(k.frequency(), Kanji::OptInt(640));
      EXPECT_EQ(k.name(), "愛");
      EXPECT_EQ(k.strokes(), 13);
      EXPECT_EQ(k.meaning(), "love");
      EXPECT_EQ(k.reading(), "アイ");
      EXPECT_EQ(k.radical().name(), "心");
      EXPECT_EQ(k.info(), "Rad 心(1), Strokes 13, G4, N3, Frq 640, K7");
      EXPECT_TRUE(e.oldNames().empty());
      EXPECT_EQ(e.year(), std::nullopt);
      EXPECT_EQ(e.extraTypeInfo(), "#4");
    } else {
      EXPECT_EQ(k.grade(), KanjiGrades::S);
      EXPECT_EQ(k.level(), JlptLevels::N1);
      EXPECT_EQ(k.kyu(), KenteiKyus::K2);
      EXPECT_EQ(k.frequency(), Kanji::OptInt(2207));
      EXPECT_EQ(k.name(), "艶");
      EXPECT_EQ(k.meaning(), "glossy");
      EXPECT_EQ(k.reading(), "エン、つや");
      EXPECT_EQ(k.strokes(), 19);
      EXPECT_EQ(k.radical().name(), "色");
      EXPECT_EQ(e.oldNames(), Kanji::LinkNames{"艷"});
      EXPECT_EQ(e.year(), 2010);
      EXPECT_EQ(e.extraTypeInfo(), "#103 2010");
      EXPECT_EQ(k.info(), "Rad 色(2), Strokes 19, S, N1, Frq 2207, Old 艷, K2");
      EXPECT_EQ(k.info(KanjiInfo::Radical), "Rad 色(2)");
      EXPECT_EQ(k.info(KanjiInfo::Strokes), "Strokes 19");
      EXPECT_EQ(k.info(KanjiInfo::Grade), "S");
      EXPECT_EQ(k.info(KanjiInfo::Level), "N1");
      EXPECT_EQ(k.info(KanjiInfo::Kyu), "K2");
      EXPECT_EQ(k.info(KanjiInfo::Old), "Old 艷");
      EXPECT_EQ(k.info(KanjiInfo::New), "");
      EXPECT_EQ(k.info(KanjiInfo::Grade | KanjiInfo::Old), "S, Old 艷");
      EXPECT_EQ(k.info(KanjiInfo::Strokes | KanjiInfo::Level), "Strokes 19, N1");
    }
  }
}

TEST_F(KanjiTest, LinkedOld) {
  writeTestFile("\
Number\tName\tRadical\tOldNames\tYear\tStrokes\tGrade\tMeaning\tReading\n\
103\t艶\t色\t艷\t2010\t19\tS\tglossy\tエン、つや");
  EXPECT_CALL(_data, getLevel("艶")).WillOnce(Return(JlptLevels::N1));
  EXPECT_CALL(_data, getKyu("艶")).WillOnce(Return(KenteiKyus::K2));
  EXPECT_CALL(_data, getFrequency("艶")).WillOnce(Return(Kanji::OptInt(2207)));
  EXPECT_CALL(_data, getFrequency("艷")).WillOnce(Return(std::nullopt));
  EXPECT_CALL(_data, getKyu("艷")).WillOnce(Return(KenteiKyus::None));
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  EXPECT_CALL(_data, ucdRadical("艷", _)).WillOnce(ReturnRef(rad));
  EXPECT_CALL(_data, getRadicalByName(_)).WillRepeatedly(ReturnRef(rad));
  auto results = ExtraKanji::fromFile(_data, KanjiTypes::Jouyou, _testFile);
  ASSERT_EQ(results.size(), 1);
  LinkedOldKanji k(_data, "艷", results[0]);
  EXPECT_EQ(k.type(), KanjiTypes::LinkedOld);
  EXPECT_EQ(k.name(), "艷");
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasKyu());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.frequency());
  EXPECT_EQ(k.reading(), "エン、つや");
  EXPECT_EQ(k.meaning(), "glossy");
  EXPECT_EQ(k.link(), results[0]);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), New 艶*");
}

TEST_F(KanjiTest, BadLinkedOld) {
  Radical rad(1, "TestRadical", Radical::AltForms(), "", "");
  std::string name("呑");
  EXPECT_CALL(_data, ucdRadical(name, _)).WillOnce(ReturnRef(rad));
  auto frequencyKanji = std::make_shared<FrequencyKanji>(_data, name, 2362);
  EXPECT_THROW(call([&, this] { LinkedOldKanji k(_data, "艷", frequencyKanji); },
                    "LinkedKanji 艷 wanted type 'Jouyou' for link 呑, but got 'Frequency'"),
               std::domain_error);
}

} // namespace kanji_tools
