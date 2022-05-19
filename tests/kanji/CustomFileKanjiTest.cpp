#include <gtest/gtest.h>
#include <kanji_tools/kanji/CustomFileKanji.h>
#include <tests/kanji_tools/MockKanjiData.h>
#include <tests/kanji_tools/TestKanji.h>
#include <tests/kanji_tools/TestUcd.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace {

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace fs = std::filesystem;

class CustomFileKanjiTest : public ::testing::Test {
protected:
  static constexpr Kanji::Frequency Freq640{640}, Freq1728{1728},
      Freq2207{2207};
  static constexpr Kanji::Year Year1951{1951}, Year2004{2004};

  inline static const Radical Rad1{1, "TestRadical", {}, {}, {}},
      Rad2{1, "二", {}, {}, {}}, RadRain{1, "雨", {}, {}, {}};
  inline static const Strokes Strokes4{4}, Strokes6{6}, Strokes7{7},
      Strokes8{8}, Strokes16{16}, Strokes19{19}, Strokes24{24};

  inline static const fs::path TestDir{"testDir"};
  inline static const fs::path TestFile{TestDir / "test.txt"};

  CustomFileKanjiTest() = default;

  void SetUp() override {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
  }

  void TearDown() override { fs::remove_all(TestDir); }

  template<typename T> [[nodiscard]] KanjiData::List fromFile() {
    return CustomFileKanji::fromFile<T>(_data, TestFile);
  }

  static void write(const String& s) {
    std::ofstream of{TestFile};
    of << s;
    of.close();
  }

  static void checkExtraKanji(const Kanji& k) { // NOLINT
    EXPECT_EQ(k.type(), KanjiTypes::Extra);
    EXPECT_EQ(k.name(), "霙");
    EXPECT_EQ(k.qualifiedName(), "霙+");
    EXPECT_EQ(k.radical().name(), "雨");
    EXPECT_EQ(k.strokes(), Strokes16);
    EXPECT_EQ(k.meaning(), "sleet");
    EXPECT_EQ(k.reading(), "エイ、ヨウ、みぞれ");
    EXPECT_FALSE(k.hasGrade());
    EXPECT_FALSE(k.hasLevel());
    EXPECT_FALSE(k.frequency());
    EXPECT_EQ(k.frequencyOrMax(), std::numeric_limits<Kanji::Frequency>::max());
    EXPECT_EQ(k.kyu(), KenteiKyus::K1);
    EXPECT_EQ(k.reason(), JinmeiReasons::None);
    EXPECT_FALSE(k.year());
    EXPECT_EQ(k.info(), "Rad 雨(1), Strokes 16, K1");
    EXPECT_EQ(k.extraTypeInfo(), "#1");
  }

  static void checkJinmeiKanji(const Kanji& k) { // NOLINT
    ASSERT_EQ(k.type(), KanjiTypes::Jinmei);
    EXPECT_EQ(k.name(), "亘");
    EXPECT_EQ(k.radical().name(), "二");
    EXPECT_EQ(k.strokes(), Strokes6);
    EXPECT_EQ(k.reading(), "コウ、カン、わた-る、もと-める");
    EXPECT_FALSE(k.hasMeaning());
    EXPECT_FALSE(k.hasGrade());
    EXPECT_EQ(k.level(), JlptLevels::N1);
    EXPECT_EQ(k.frequency(), Freq1728);
    EXPECT_EQ(k.frequencyOrMax(), Freq1728);
    EXPECT_EQ(k.kyu(), KenteiKyus::KJ1);
    EXPECT_EQ(k.oldNames(), Kanji::LinkNames{"亙"});
    EXPECT_EQ(k.reason(), JinmeiReasons::Names);
    EXPECT_EQ(k.year(), Year1951);
    EXPECT_EQ(k.info(), "Rad 二(1), Strokes 6, N1, Frq 1728, Old 亙, KJ1");
    EXPECT_EQ(k.extraTypeInfo(), "#8 1951 [Names]");
  }

  [[nodiscard]] const MockKanjiData& data() { return _data; }
private:
  const MockKanjiData _data;
};

} // namespace

TEST_F(CustomFileKanjiTest, ExtraFile) {
  write("\
Number\tName\tRadical\tStrokes\tMeaning\tReading\n\
1\t霙\t雨\t16\tsleet\tエイ、ヨウ、みぞれ");
  EXPECT_CALL(data(), kyu(_)).WillOnce(Return(KenteiKyus::K1));
  EXPECT_CALL(data(), getRadicalByName("雨")).WillOnce(ReturnRef(RadRain));
  const auto results{fromFile<ExtraKanji>()};
  ASSERT_EQ(results.size(), 1);
  checkExtraKanji(*results[0]);
}

TEST_F(CustomFileKanjiTest, ExtraFileWithDifferentColumnOrder) {
  write("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_CALL(data(), kyu(_)).WillOnce(Return(KenteiKyus::K1));
  EXPECT_CALL(data(), getRadicalByName("雨")).WillOnce(ReturnRef(RadRain));
  const auto results{fromFile<ExtraKanji>()};
  ASSERT_EQ(results.size(), 1);
  checkExtraKanji(*results[0]);
}

TEST_F(CustomFileKanjiTest, ExtraFileWithUnrecognizedColumn) {
  // cSpell:ignore Rdical
  write("\
Name\tNumber\tRdical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "unrecognized header 'Rdical' - file: test.txt"),
      std::domain_error);
}

TEST_F(CustomFileKanjiTest, ExtraFileWithDuplicateColumn) {
  write("\
Name\tNumber\tRadical\tMeaning\tName\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "duplicate header 'Name' - file: test.txt"),
      std::domain_error);
}

TEST_F(CustomFileKanjiTest, ExtraFileWithToManyColumns) {
  write("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16\t16");
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "too many columns - file: test.txt, row: 1"),
      std::domain_error);
}

TEST_F(CustomFileKanjiTest, ExtraFileWithNotEnoughColumns) {
  write("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ");
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "not enough columns - file: test.txt, row: 1"),
      std::domain_error);
}

TEST_F(CustomFileKanjiTest, ExtraFileWithInvalidData) {
  write("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\ta\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_CALL(data(), kyu(_)).WillOnce(Return(KenteiKyus::K1));
  EXPECT_CALL(data(), getRadicalByName("雨")).WillOnce(ReturnRef(RadRain));
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "failed to convert to unsigned long - file: test.txt, row: "
                   "1, column: 'Number', value: 'a'"),
      std::domain_error);
}

TEST_F(CustomFileKanjiTest, JinmeiFile) {
  write("\
Number\tName\tRadical\tOldNames\tYear\tReason\tReading\n\
7\t云\t二\t\t2004\tPrint\tウン、い-う、ここに\n\
8\t亘\t二\t亙\t1951\tNames\tコウ、カン、わた-る、もと-める");
  EXPECT_CALL(data(), level("云")).WillOnce(Return(JlptLevels::None));
  EXPECT_CALL(data(), frequency("云")).WillOnce(Return(0));
  EXPECT_CALL(data(), kyu("云")).WillOnce(Return(KenteiKyus::KJ1));
  EXPECT_CALL(data(), level("亘")).WillOnce(Return(JlptLevels::N1));
  EXPECT_CALL(data(), frequency("亘")).WillOnce(Return(Freq1728));
  EXPECT_CALL(data(), kyu("亘")).WillOnce(Return(KenteiKyus::KJ1));
  EXPECT_CALL(data(), getRadicalByName("二")).WillRepeatedly(ReturnRef(Rad2));
  EXPECT_CALL(data(), ucdStrokes("云", nullptr)).WillOnce(Return(Strokes4));
  EXPECT_CALL(data(), ucdStrokes("亘", nullptr)).WillOnce(Return(Strokes6));
  const auto results{fromFile<JinmeiKanji>()};
  ASSERT_EQ(results.size(), 2);

  auto& k{*results[0]};
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.frequency());
  EXPECT_EQ(k.name(), "云");
  EXPECT_EQ(k.qualifiedName(), "云^");
  EXPECT_EQ(k.strokes(), Strokes4);
  EXPECT_EQ(k.kyu(), KenteiKyus::KJ1);
  ASSERT_EQ(k.type(), KanjiTypes::Jinmei);
  EXPECT_EQ(k.radical().name(), "二");
  EXPECT_EQ(k.extraTypeInfo(), "#7 2004 [Print]");
  EXPECT_TRUE(k.oldNames().empty());
  EXPECT_EQ(k.year(), Year2004);
  EXPECT_EQ(k.reason(), JinmeiReasons::Print);
  checkJinmeiKanji(*results[1]);
}

TEST_F(CustomFileKanjiTest, LinkedJinmei) {
  write("\
Number\tName\tRadical\tOldNames\tYear\tReason\tReading\n\
1\t亘\t二\t亙\t1951\tNames\tコウ、カン、わた-る、もと-める");
  EXPECT_CALL(data(), level("亘")).WillOnce(Return(JlptLevels::N1));
  EXPECT_CALL(data(), frequency("亘")).WillOnce(Return(Freq1728));
  EXPECT_CALL(data(), kyu("亘")).WillOnce(Return(KenteiKyus::KJ1));
  EXPECT_CALL(data(), getRadicalByName("二")).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), ucdRadical("亙", _)).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), frequency("亙")).WillOnce(Return(0));
  EXPECT_CALL(data(), kyu("亙")).WillOnce(Return(KenteiKyus::KJ1));
  EXPECT_CALL(data(), ucdStrokes("亘", nullptr)).WillOnce(Return(Strokes6));
  EXPECT_CALL(data(), ucdStrokes("亙", nullptr)).WillOnce(Return(Strokes6));
  const auto results{fromFile<JinmeiKanji>()};
  ASSERT_EQ(results.size(), 1);
  const LinkedJinmeiKanji k{data(), "亙", results[0]};
  EXPECT_EQ(k.type(), KanjiTypes::LinkedJinmei);
  EXPECT_EQ(k.name(), "亙");
  EXPECT_EQ(k.qualifiedName(), "亙~");
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.frequency());
  EXPECT_EQ(k.reading(), "コウ、カン、わた-る、もと-める");
  EXPECT_EQ(k.info(Kanji::Info::New), "New 亘*");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_EQ(k.link(), results[0]);
}

TEST_F(CustomFileKanjiTest, BadLinkedJinmei) {
  const auto fKanji{std::make_shared<TestKanji>("呑")};
  EXPECT_THROW(
      call([&fKanji, this] { LinkedJinmeiKanji k(data(), "亙", fKanji); },
          "LinkedKanji 亙 wanted type 'Jouyou' or 'Jinmei' for link 呑, but "
          "got 'None'"),
      std::domain_error);
}

TEST_F(CustomFileKanjiTest, JinmeiFileWithMissingReason) {
  write("\
Number\tName\tRadical\tOldNames\tYear\tReading\n\
1\t亘\t二\t亙\t1951\tコウ、カン、わた-る、もと-める");
  EXPECT_THROW(call([this] { return fromFile<JinmeiKanji>(); },
                   "column 'Reason' not found - file: test.txt"),
      std::domain_error);
}

TEST_F(CustomFileKanjiTest, JouyouFile) {
  write("\
Number\tName\tRadical\tOldNames\tYear\tStrokes\tGrade\tMeaning\tReading\n\
4\t愛\t心\t\t\t13\t4\tlove\tアイ\n\
103\t艶\t色\t艷\t2010\t19\tS\tglossy\tエン、つや");
  const Radical Heart{1, "心", {}, {}, {}}, Color{2, "色", {}, {}, {}};
  EXPECT_CALL(data(), getRadicalByName("心")).WillOnce(ReturnRef(Heart));
  EXPECT_CALL(data(), getRadicalByName("色")).WillOnce(ReturnRef(Color));
  EXPECT_CALL(data(), level("愛")).WillOnce(Return(JlptLevels::N3));
  EXPECT_CALL(data(), kyu("愛")).WillOnce(Return(KenteiKyus::K7));
  EXPECT_CALL(data(), frequency("愛")).WillOnce(Return(Freq640));
  EXPECT_CALL(data(), level("艶")).WillOnce(Return(JlptLevels::N1));
  EXPECT_CALL(data(), kyu("艶")).WillOnce(Return(KenteiKyus::K2));
  EXPECT_CALL(data(), frequency("艶")).WillOnce(Return(Freq2207));
  const auto results{fromFile<JouyouKanji>()};
  ASSERT_EQ(results.size(), 2);

  for (auto& i : results) {
    auto& k{*i};
    EXPECT_EQ(k.type(), KanjiTypes::Jouyou);
    if (k.grade() == KanjiGrades::G4) {
      EXPECT_EQ(k.level(), JlptLevels::N3);
      EXPECT_EQ(k.kyu(), KenteiKyus::K7);
      EXPECT_EQ(k.frequency(), Freq640);
      EXPECT_EQ(k.name(), "愛");
      EXPECT_EQ(k.qualifiedName(), "愛.");
      EXPECT_EQ(k.strokes().value(), 13);
      EXPECT_EQ(k.meaning(), "love");
      EXPECT_EQ(k.reading(), "アイ");
      EXPECT_EQ(k.radical().name(), "心");
      EXPECT_EQ(k.info(), "Rad 心(1), Strokes 13, G4, N3, Frq 640, K7");
      EXPECT_TRUE(k.oldNames().empty());
      EXPECT_EQ(k.extraTypeInfo(), "#4");
      EXPECT_EQ(k.year(), 0);
    } else {
      EXPECT_EQ(k.grade(), KanjiGrades::S);
      EXPECT_EQ(k.level(), JlptLevels::N1);
      EXPECT_EQ(k.kyu(), KenteiKyus::K2);
      EXPECT_EQ(k.frequency(), Freq2207);
      EXPECT_EQ(k.name(), "艶");
      EXPECT_EQ(k.meaning(), "glossy");
      EXPECT_EQ(k.reading(), "エン、つや");
      EXPECT_EQ(k.strokes(), Strokes19);
      EXPECT_EQ(k.radical().name(), "色");
      EXPECT_EQ(k.oldNames(), Kanji::LinkNames{"艷"});
      EXPECT_EQ(k.extraTypeInfo(), "#103 2010");
      EXPECT_EQ(k.info(), "Rad 色(2), Strokes 19, S, N1, Frq 2207, Old 艷, K2");
      EXPECT_EQ(k.info(Kanji::Info::Radical), "Rad 色(2)");
      EXPECT_EQ(k.info(Kanji::Info::Strokes), "Strokes 19");
      EXPECT_EQ(k.info(Kanji::Info::Grade), "S");
      EXPECT_EQ(k.info(Kanji::Info::Level), "N1");
      EXPECT_EQ(k.info(Kanji::Info::Kyu), "K2");
      EXPECT_EQ(k.info(Kanji::Info::Old), "Old 艷");
      EXPECT_EQ(k.info(Kanji::Info::New), "");
      EXPECT_EQ(k.info(Kanji::Info::Grade | Kanji::Info::Old), "S, Old 艷");
      EXPECT_EQ(
          k.info(Kanji::Info::Strokes | Kanji::Info::Level), "Strokes 19, N1");
      EXPECT_EQ(k.year(), 2010);
    }
  }
}

TEST_F(CustomFileKanjiTest, LinkedOld) {
  write("\
Number\tName\tRadical\tOldNames\tYear\tStrokes\tGrade\tMeaning\tReading\n\
103\t艶\t色\t艷\t2010\t19\tS\tglossy\tエン、つや");
  EXPECT_CALL(data(), level("艶")).WillOnce(Return(JlptLevels::N1));
  EXPECT_CALL(data(), kyu("艶")).WillOnce(Return(KenteiKyus::K2));
  EXPECT_CALL(data(), frequency("艶")).WillOnce(Return(Freq2207));
  EXPECT_CALL(data(), frequency("艷")).WillOnce(Return(0));
  EXPECT_CALL(data(), kyu("艷")).WillOnce(Return(KenteiKyus::None));
  EXPECT_CALL(data(), ucdRadical("艷", _)).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), ucdStrokes("艷", _)).WillOnce(Return(Strokes24));
  EXPECT_CALL(data(), getRadicalByName(_)).WillRepeatedly(ReturnRef(Rad1));
  const auto results{fromFile<JouyouKanji>()};
  ASSERT_EQ(results.size(), 1);
  const LinkedOldKanji k{data(), "艷", results[0]};
  EXPECT_EQ(k.type(), KanjiTypes::LinkedOld);
  EXPECT_EQ(k.name(), "艷");
  EXPECT_EQ(k.qualifiedName(), "艷%");
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasKyu());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.frequency());
  EXPECT_EQ(k.reading(), "エン、つや");
  EXPECT_EQ(k.meaning(), "glossy");
  EXPECT_EQ(k.link(), results[0]);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Strokes 24, New 艶*");
}

TEST_F(CustomFileKanjiTest, BadLinkedOld) {
  const auto freqKanji{std::make_shared<TestKanji>("呑")};
  EXPECT_THROW(call([&, this] { LinkedOldKanji k(data(), "艷", freqKanji); },
                   "LinkedKanji 艷 wanted type 'Jouyou' for link 呑, but got "
                   "'None'"),
      std::domain_error);
}

} // namespace kanji_tools
