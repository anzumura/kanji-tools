#include <gtest/gtest.h>
#include <kt_tests/MockKanjiData.h>
#include <kt_tests/TestKanji.h>
#include <kt_tests/TestUcd.h>
#include <kt_tests/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace {

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class KanjiTest : public ::testing::Test {
protected:
  static constexpr Kanji::Frequency Freq2362{2362};

  inline static const Radical Rad1{1, "TestRadical", {}, {}, {}},
      Rad2{1, "二", {}, {}, {}}, RadRain{1, "雨", {}, {}, {}};
  inline static const Strokes Strokes4{4}, Strokes6{6}, Strokes7{7},
      Strokes8{8}, Strokes16{16}, Strokes19{19}, Strokes24{24};

  KanjiTest() = default;

  [[nodiscard]] const MockKanjiData& data() { return _data; }
private:
  const MockKanjiData _data;
};

} // namespace

TEST_F(KanjiTest, Equals) {
  const TestKanji first{"甲", "三"}, sameName{"甲", "山"}, diffName{"乙", "三"};
  // equality only depends on 'name' field - Kanji with same 'name' (even if any
  // other fields are different) can't be added to 'KanjiData' class
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
  EXPECT_EQ(k.qualifiedName(), "呑\"");
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
  EXPECT_EQ(k.qualifiedName(), "蘋#");
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
  const String sampleLink{"犬"};
  const Ucd ucd{TestUcd{"侭"}
                    .ids("123P", "456 789")
                    .links({{0x72ac, sampleLink}}, Ucd::LinkTypes::Simplified)
                    .meaningAndReadings("utmost", "JIN", "MAMA")};
  const UcdKanji k{data(), ucd};
  EXPECT_EQ(k.type(), KanjiTypes::Ucd);
  EXPECT_EQ(k.name(), "侭");
  EXPECT_EQ(k.qualifiedName(), "侭*");
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
  const Ucd ucd{TestUcd{"侭"}
                    .sources("GJ", "J0-4B79")
                    .links({{0x72ac, "犬"}, {0x732b, "猫"}},
                        Ucd::LinkTypes::Traditional_R)
                    .meaningAndReadings("utmost", "JIN", "MAMA")};
  EXPECT_EQ(ucd.sources(), "GJ");
  EXPECT_EQ(ucd.jSource(), "J0-4B79");
  const UcdKanji k{data(), ucd};
  ASSERT_FALSE(k.newName());
  EXPECT_EQ(k.oldNames(), (Kanji::LinkNames{"犬", "猫"}));
  EXPECT_EQ(k.info(), "Rad TestRadical(1), Strokes 8, Old 犬*／猫");
}

} // namespace kanji_tools
