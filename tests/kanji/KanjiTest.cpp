#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <kanji_tools/kanji/CustomFileKanji.h>
#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/kanji/UcdFileKanji.h>
#include <tests/kanji_tools/TestKanji.h>
#include <tests/kanji_tools/TestUcd.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

class MockData : public Data {
public:
  explicit MockData(const Path& p) : Data{p, Data::DebugMode::None} {}
  MOCK_METHOD(
      Kanji::Frequency, frequency, (const std::string&), (const, override));
  MOCK_METHOD(JlptLevels, level, (const std::string&), (const, override));
  MOCK_METHOD(KenteiKyus, kyu, (const std::string&), (const, override));
  MOCK_METHOD(
      RadicalRef, ucdRadical, (const std::string&, UcdPtr), (const, override));
  MOCK_METHOD(
      Strokes, ucdStrokes, (const std::string&, UcdPtr), (const, override));
  MOCK_METHOD(
      RadicalRef, getRadicalByName, (const std::string&), (const, override));
};

class KanjiTest : public ::testing::Test {
protected:
  static constexpr Kanji::Frequency Freq640{640}, Freq1728{1728},
      Freq2362{2362}, Freq2207{2207};
  static constexpr Kanji::Year Year1951{1951}, Year2004{2004};

  inline static const Radical Rad1{1, "TestRadical", {}, {}, {}},
      Rad2{1, "二", {}, {}, {}}, RadRain{1, "雨", {}, {}, {}};
  inline static const Strokes Strokes4{4}, Strokes6{6}, Strokes7{7},
      Strokes8{8}, Strokes16{16}, Strokes19{19}, Strokes24{24};

  inline static const fs::path TestDir{"testDir"};
  inline static const fs::path TestFile{TestDir / "test.txt"};

  KanjiTest() : _data{TestDir} {}

  void SetUp() override {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
  }

  void TearDown() override { fs::remove_all(TestDir); }

  template<typename T> [[nodiscard]] Data::KanjiList fromFile() {
    return CustomFileKanji::fromFile<T>(_data, TestFile);
  }

  static void write(const std::string& s) {
    std::ofstream of{TestFile};
    of << s;
    of.close();
  }

  static void checkExtraKanji(const Kanji& k) { // NOLINT
    EXPECT_EQ(k.type(), KanjiTypes::Extra);
    EXPECT_EQ(k.name(), "霙");
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

  [[nodiscard]] const MockData& data() { return _data; }
private:
  const MockData _data;
};

} // namespace

TEST_F(KanjiTest, Equals) {
  const TestKanji first{"甲", "三"}, sameName{"甲", "山"}, diffName{"乙", "三"};
  // equality only depends on 'name' field - Kanji with same 'name' (even if any
  // other fields are different) can't be added to 'Data' class
  EXPECT_EQ(first, sameName);
  EXPECT_NE(first, diffName);
}

TEST_F(KanjiTest, Size) {
  EXPECT_EQ(sizeof(Kanji::Frequency), 2);
  EXPECT_EQ(sizeof(Kanji::Year), 2);
  EXPECT_EQ(sizeof(KanjiPtr), 16);
#ifdef __clang__
  EXPECT_EQ(sizeof(Kanji::OptString), 32);
  EXPECT_EQ(sizeof(Kanji), 104);
#else
  EXPECT_EQ(sizeof(Kanji::OptString), 40);
  EXPECT_EQ(sizeof(Kanji), 120);
#endif
}

TEST_F(KanjiTest, FrequencyKanji) {
  constexpr auto kyu{KenteiKyus::KJ1};
  EXPECT_CALL(data(), kyu("呑")).WillOnce(Return(kyu));
  EXPECT_CALL(data(), ucdRadical(_, _)).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), ucdStrokes(_, _)).WillOnce(Return(Strokes7));
  const FrequencyKanji k{data(), "呑", Freq2362};
  EXPECT_EQ(k.type(), KanjiTypes::Frequency);
  EXPECT_EQ(k.name(), "呑");
  EXPECT_EQ(k.radical(), Rad1);
  EXPECT_EQ(k.strokes(), Strokes7);
  EXPECT_FALSE(k.link());
  EXPECT_EQ(k.frequency(), Freq2362);
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_EQ(k.kyu(), kyu);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Strokes 7, Frq 2362, KJ1");
  EXPECT_FALSE(k.extraTypeInfo());
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_FALSE(k.hasReading());
}

TEST_F(KanjiTest, FrequencyKanjiWithReading) {
  constexpr auto kyu{KenteiKyus::KJ1};
  EXPECT_CALL(data(), kyu("呑")).WillOnce(Return(kyu));
  EXPECT_CALL(data(), ucdRadical(_, _)).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), ucdStrokes(_, _)).WillOnce(Return(Strokes7));
  const FrequencyKanji k{data(), "呑", "トン、ドン、の-む", Freq2362};
  EXPECT_EQ(k.type(), KanjiTypes::Frequency);
  EXPECT_TRUE(k.is(KanjiTypes::Frequency));
  EXPECT_EQ(k.name(), "呑");
  EXPECT_EQ(k.radical(), Rad1);
  EXPECT_EQ(k.frequency(), Freq2362);
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_EQ(k.kyu(), kyu);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Strokes 7, Frq 2362, KJ1");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_TRUE(k.hasReading());
  EXPECT_EQ(k.reading(), "トン、ドン、の-む");
}

TEST_F(KanjiTest, KenteiKanji) {
  constexpr auto kyu{KenteiKyus::K1};
  EXPECT_CALL(data(), ucdRadical(_, _)).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), ucdStrokes(_, _)).WillOnce(Return(Strokes19));
  const KenteiKanji k{data(), "蘋", kyu};
  EXPECT_EQ(k.type(), KanjiTypes::Kentei);
  EXPECT_EQ(k.name(), "蘋");
  EXPECT_EQ(k.strokes(), Strokes19);
  EXPECT_EQ(k.radical(), Rad1);
  EXPECT_FALSE(k.frequency());
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_EQ(k.kyu(), kyu);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Strokes 19, K1");
  EXPECT_FALSE(k.extraTypeInfo());
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_FALSE(k.hasReading());
}

TEST_F(KanjiTest, UcdKanjiWithNewName) {
  EXPECT_CALL(data(), ucdRadical(_, _)).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), ucdStrokes(_, _)).WillOnce(Return(Strokes8));
  const std::string sampleLink{"犬"};
  const Ucd ucd{TestUcd{"侭"}
                    .ids("123P", "456 789")
                    .links({{0x72ac, sampleLink}}, UcdLinkTypes::Simplified)
                    .meaningAndReadings("utmost", "JIN", "MAMA")};
  const UcdKanji k{data(), ucd};
  EXPECT_EQ(k.type(), KanjiTypes::Ucd);
  EXPECT_EQ(k.name(), "侭");
  EXPECT_EQ(k.radical(), Rad1);
  EXPECT_FALSE(k.frequency());
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.hasKyu());
  EXPECT_EQ(k.morohashiId(), MorohashiId{"123P"});
  EXPECT_EQ(k.nelsonIds(), (Kanji::NelsonIds{456, 789}));
  EXPECT_EQ(k.meaning(), "utmost");
  EXPECT_EQ(k.reading(), "ジン、まま");
  ASSERT_TRUE(k.newName());
  EXPECT_EQ(*k.newName(), "犬");
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Strokes 8, New 犬");
  EXPECT_FALSE(k.extraTypeInfo());
}

TEST_F(KanjiTest, UcdKanjiWithLinkedReadingOldNames) {
  EXPECT_CALL(data(), ucdStrokes(_, _)).WillOnce(Return(Strokes8));
  EXPECT_CALL(data(), ucdRadical(_, _)).WillOnce(ReturnRef(Rad1));
  const Ucd ucd{
      TestUcd{"侭"}
          .sources("GJ", "J0-4B79")
          .links({{0x72ac, "犬"}, {0x732b, "猫"}}, UcdLinkTypes::Traditional_R)
          .meaningAndReadings("utmost", "JIN", "MAMA")};
  EXPECT_EQ(ucd.sources(), "GJ");
  EXPECT_EQ(ucd.jSource(), "J0-4B79");
  const UcdKanji k{data(), ucd};
  ASSERT_FALSE(k.newName());
  EXPECT_EQ(k.oldNames(), (Kanji::LinkNames{"犬", "猫"}));
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Strokes 8, Old 犬*／猫");
}

TEST_F(KanjiTest, ExtraFile) {
  write("\
Number\tName\tRadical\tStrokes\tMeaning\tReading\n\
1\t霙\t雨\t16\tsleet\tエイ、ヨウ、みぞれ");
  EXPECT_CALL(data(), kyu(_)).WillOnce(Return(KenteiKyus::K1));
  EXPECT_CALL(data(), getRadicalByName("雨")).WillOnce(ReturnRef(RadRain));
  const auto results{fromFile<ExtraKanji>()};
  ASSERT_EQ(results.size(), 1);
  checkExtraKanji(*results[0]);
}

TEST_F(KanjiTest, ExtraFileWithDifferentColumnOrder) {
  write("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_CALL(data(), kyu(_)).WillOnce(Return(KenteiKyus::K1));
  EXPECT_CALL(data(), getRadicalByName("雨")).WillOnce(ReturnRef(RadRain));
  const auto results{fromFile<ExtraKanji>()};
  ASSERT_EQ(results.size(), 1);
  checkExtraKanji(*results[0]);
}

TEST_F(KanjiTest, ExtraFileWithUnrecognizedColumn) {
  write("\
Name\tNumber\tRdical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "unrecognized header 'Rdical' - file: test.txt"),
      std::domain_error);
}

TEST_F(KanjiTest, ExtraFileWithDuplicateColumn) {
  write("\
Name\tNumber\tRadical\tMeaning\tName\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16");
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "duplicate header 'Name' - file: test.txt"),
      std::domain_error);
}

TEST_F(KanjiTest, ExtraFileWithToManyColumns) {
  write("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ\t16\t16");
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "too many columns - file: test.txt, row: 1"),
      std::domain_error);
}

TEST_F(KanjiTest, ExtraFileWithNotEnoughColumns) {
  write("\
Name\tNumber\tRadical\tMeaning\tReading\tStrokes\n\
霙\t1\t雨\tsleet\tエイ、ヨウ、みぞれ");
  EXPECT_THROW(call([this] { return fromFile<ExtraKanji>(); },
                   "not enough columns - file: test.txt, row: 1"),
      std::domain_error);
}

TEST_F(KanjiTest, ExtraFileWithInvalidData) {
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

TEST_F(KanjiTest, JinmeiFile) {
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

TEST_F(KanjiTest, LinkedJinmei) {
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
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.frequency());
  EXPECT_EQ(k.reading(), "コウ、カン、わた-る、もと-める");
  EXPECT_EQ(k.info(KanjiInfo::New), "New 亘*");
  EXPECT_FALSE(k.hasMeaning());
  EXPECT_EQ(k.link(), results[0]);
}

TEST_F(KanjiTest, BadLinkedJinmei) {
  EXPECT_CALL(data(), ucdRadical("呑", _)).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), ucdStrokes("呑", _)).WillOnce(Return(Strokes7));
  EXPECT_CALL(data(), kyu("呑")).WillOnce(Return(KenteiKyus::KJ1));
  const auto fKanji{std::make_shared<FrequencyKanji>(data(), "呑", Freq2362)};
  EXPECT_THROW(
      call([&fKanji, this] { LinkedJinmeiKanji k(data(), "亙", fKanji); },
          "LinkedKanji 亙 wanted type 'Jouyou' or 'Jinmei' for link 呑, but "
          "got 'Frequency'"),
      std::domain_error);
}

TEST_F(KanjiTest, JinmeiFileWithMissingReason) {
  write("\
Number\tName\tRadical\tOldNames\tYear\tReading\n\
1\t亘\t二\t亙\t1951\tコウ、カン、わた-る、もと-める");
  EXPECT_THROW(call([this] { return fromFile<JinmeiKanji>(); },
                   "column 'Reason' not found - file: test.txt"),
      std::domain_error);
}

TEST_F(KanjiTest, JouyouFile) {
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
      EXPECT_EQ(k.info(KanjiInfo::Radical), "Rad 色(2)");
      EXPECT_EQ(k.info(KanjiInfo::Strokes), "Strokes 19");
      EXPECT_EQ(k.info(KanjiInfo::Grade), "S");
      EXPECT_EQ(k.info(KanjiInfo::Level), "N1");
      EXPECT_EQ(k.info(KanjiInfo::Kyu), "K2");
      EXPECT_EQ(k.info(KanjiInfo::Old), "Old 艷");
      EXPECT_EQ(k.info(KanjiInfo::New), "");
      EXPECT_EQ(k.info(KanjiInfo::Grade | KanjiInfo::Old), "S, Old 艷");
      EXPECT_EQ(
          k.info(KanjiInfo::Strokes | KanjiInfo::Level), "Strokes 19, N1");
      EXPECT_EQ(k.year(), 2010);
    }
  }
}

TEST_F(KanjiTest, LinkedOld) {
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
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasKyu());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.frequency());
  EXPECT_EQ(k.reading(), "エン、つや");
  EXPECT_EQ(k.meaning(), "glossy");
  EXPECT_EQ(k.link(), results[0]);
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Strokes 24, New 艶*");
}

TEST_F(KanjiTest, BadLinkedOld) {
  const std::string name{"呑"};
  EXPECT_CALL(data(), ucdRadical(name, _)).WillOnce(ReturnRef(Rad1));
  EXPECT_CALL(data(), ucdStrokes(name, _)).WillOnce(Return(Strokes7));
  EXPECT_CALL(data(), kyu("呑")).WillOnce(Return(KenteiKyus::KJ1));
  const auto freqKanji{std::make_shared<FrequencyKanji>(data(), name, 2362)};
  EXPECT_THROW(call([&, this] { LinkedOldKanji k(data(), "艷", freqKanji); },
                   "LinkedKanji 艷 wanted type 'Jouyou' for link 呑, but got "
                   "'Frequency'"),
      std::domain_error);
}

} // namespace kanji_tools
