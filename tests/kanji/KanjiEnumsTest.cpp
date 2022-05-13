#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiEnums.h>

namespace kanji_tools {

TEST(KanjiTypesTest, CheckStrings) {
  EXPECT_EQ(toString(KanjiTypes::Jouyou), "Jouyou");
  EXPECT_EQ(toString(KanjiTypes::Jinmei), "Jinmei");
  EXPECT_EQ(toString(KanjiTypes::LinkedJinmei), "LinkedJinmei");
  EXPECT_EQ(toString(KanjiTypes::LinkedOld), "LinkedOld");
  EXPECT_EQ(toString(KanjiTypes::Frequency), "Frequency");
  EXPECT_EQ(toString(KanjiTypes::Extra), "Extra");
  EXPECT_EQ(toString(KanjiTypes::Kentei), "Kentei");
  EXPECT_EQ(toString(KanjiTypes::Ucd), "Ucd");
  EXPECT_EQ(toString(KanjiTypes::None), "None");
}

TEST(KanjiTypesTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(AllKanjiTypes[i], KanjiTypes::Jouyou);
  EXPECT_EQ(AllKanjiTypes[++i], KanjiTypes::Jinmei);
  EXPECT_EQ(AllKanjiTypes[++i], KanjiTypes::LinkedJinmei);
  EXPECT_EQ(AllKanjiTypes[++i], KanjiTypes::LinkedOld);
  EXPECT_EQ(AllKanjiTypes[++i], KanjiTypes::Frequency);
  EXPECT_EQ(AllKanjiTypes[++i], KanjiTypes::Extra);
  EXPECT_EQ(AllKanjiTypes[++i], KanjiTypes::Kentei);
  EXPECT_EQ(AllKanjiTypes[++i], KanjiTypes::Ucd);
  EXPECT_EQ(AllKanjiTypes[++i], KanjiTypes::None);
}

TEST(KanjiGradesTest, CheckStrings) {
  EXPECT_EQ(toString(KanjiGrades::G1), "G1");
  EXPECT_EQ(toString(KanjiGrades::G2), "G2");
  EXPECT_EQ(toString(KanjiGrades::G3), "G3");
  EXPECT_EQ(toString(KanjiGrades::G4), "G4");
  EXPECT_EQ(toString(KanjiGrades::G5), "G5");
  EXPECT_EQ(toString(KanjiGrades::G6), "G6");
  EXPECT_EQ(toString(KanjiGrades::S), "S");
  EXPECT_EQ(toString(KanjiGrades::None), "None");
}

TEST(KanjiGradesTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(AllKanjiGrades[i], KanjiGrades::G1);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G2);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G3);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G4);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G5);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G6);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::S);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::None);
}

TEST(JlptLevelsTest, CheckStrings) {
  EXPECT_EQ(toString(JlptLevels::N5), "N5");
  EXPECT_EQ(toString(JlptLevels::N4), "N4");
  EXPECT_EQ(toString(JlptLevels::N3), "N3");
  EXPECT_EQ(toString(JlptLevels::N2), "N2");
  EXPECT_EQ(toString(JlptLevels::N1), "N1");
  EXPECT_EQ(toString(JlptLevels::None), "None");
}

TEST(JlptLevelsTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(AllJlptLevels[i], JlptLevels::N5);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::N4);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::N3);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::N2);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::N1);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::None);
}

TEST(KenteiKyusTest, CheckStrings) {
  EXPECT_EQ(toString(KenteiKyus::K10), "K10");
  EXPECT_EQ(toString(KenteiKyus::K9), "K9");
  EXPECT_EQ(toString(KenteiKyus::K8), "K8");
  EXPECT_EQ(toString(KenteiKyus::K7), "K7");
  EXPECT_EQ(toString(KenteiKyus::K6), "K6");
  EXPECT_EQ(toString(KenteiKyus::K5), "K5");
  EXPECT_EQ(toString(KenteiKyus::K4), "K4");
  EXPECT_EQ(toString(KenteiKyus::K3), "K3");
  EXPECT_EQ(toString(KenteiKyus::KJ2), "KJ2");
  EXPECT_EQ(toString(KenteiKyus::K2), "K2");
  EXPECT_EQ(toString(KenteiKyus::KJ1), "KJ1");
  EXPECT_EQ(toString(KenteiKyus::K1), "K1");
  EXPECT_EQ(toString(KenteiKyus::None), "None");
}

TEST(KenteiKyusTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(AllKenteiKyus[i], KenteiKyus::K10);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K9);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K8);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K7);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K6);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K5);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K4);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K3);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::KJ2);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K2);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::KJ1);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K1);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::None);
}

TEST(JinmeiReasonsTest, CheckStrings) {
  EXPECT_EQ(toString(JinmeiReasons::Names), "Names");
  EXPECT_EQ(toString(JinmeiReasons::Print), "Print");
  EXPECT_EQ(toString(JinmeiReasons::Variant), "Variant");
  EXPECT_EQ(toString(JinmeiReasons::Moved), "Moved");
  EXPECT_EQ(toString(JinmeiReasons::Simple), "Simple");
  EXPECT_EQ(toString(JinmeiReasons::Other), "Other");
  EXPECT_EQ(toString(JinmeiReasons::None), "None");
}

TEST(JinmeiReasonsTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(AllJinmeiReasons[i], JinmeiReasons::Names);
  EXPECT_EQ(AllJinmeiReasons[++i], JinmeiReasons::Print);
  EXPECT_EQ(AllJinmeiReasons[++i], JinmeiReasons::Variant);
  EXPECT_EQ(AllJinmeiReasons[++i], JinmeiReasons::Moved);
  EXPECT_EQ(AllJinmeiReasons[++i], JinmeiReasons::Simple);
  EXPECT_EQ(AllJinmeiReasons[++i], JinmeiReasons::Other);
  EXPECT_EQ(AllJinmeiReasons[++i], JinmeiReasons::None);
}

} // namespace kanji_tools
