#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiEnums.h>

namespace kanji_tools {

TEST(KanjiTypesTest, CheckStrings) {
  using enum KanjiTypes;
  EXPECT_EQ(toString(Jouyou), "Jouyou");
  EXPECT_EQ(toString(Jinmei), "Jinmei");
  EXPECT_EQ(toString(LinkedJinmei), "LinkedJinmei");
  EXPECT_EQ(toString(LinkedOld), "LinkedOld");
  EXPECT_EQ(toString(Frequency), "Frequency");
  EXPECT_EQ(toString(Extra), "Extra");
  EXPECT_EQ(toString(Kentei), "Kentei");
  EXPECT_EQ(toString(Ucd), "Ucd");
  EXPECT_EQ(toString(None), "None");
}

TEST(KanjiTypesTest, CheckValues) {
  using enum KanjiTypes;
  size_t i{};
  EXPECT_EQ(AllKanjiTypes[i], Jouyou);
  EXPECT_EQ(AllKanjiTypes[++i], Jinmei);
  EXPECT_EQ(AllKanjiTypes[++i], LinkedJinmei);
  EXPECT_EQ(AllKanjiTypes[++i], LinkedOld);
  EXPECT_EQ(AllKanjiTypes[++i], Frequency);
  EXPECT_EQ(AllKanjiTypes[++i], Extra);
  EXPECT_EQ(AllKanjiTypes[++i], Kentei);
  EXPECT_EQ(AllKanjiTypes[++i], Ucd);
  EXPECT_EQ(AllKanjiTypes[++i], None);
}

TEST(KanjiGradesTest, CheckStrings) {
  using enum KanjiGrades;
  EXPECT_EQ(toString(G1), "G1");
  EXPECT_EQ(toString(G2), "G2");
  EXPECT_EQ(toString(G3), "G3");
  EXPECT_EQ(toString(G4), "G4");
  EXPECT_EQ(toString(G5), "G5");
  EXPECT_EQ(toString(G6), "G6");
  EXPECT_EQ(toString(S), "S");
  EXPECT_EQ(toString(None), "None");
}

TEST(KanjiGradesTest, CheckValues) {
  using enum KanjiGrades;
  size_t i{};
  EXPECT_EQ(AllKanjiGrades[i], G1);
  EXPECT_EQ(AllKanjiGrades[++i], G2);
  EXPECT_EQ(AllKanjiGrades[++i], G3);
  EXPECT_EQ(AllKanjiGrades[++i], G4);
  EXPECT_EQ(AllKanjiGrades[++i], G5);
  EXPECT_EQ(AllKanjiGrades[++i], G6);
  EXPECT_EQ(AllKanjiGrades[++i], S);
  EXPECT_EQ(AllKanjiGrades[++i], None);
}

TEST(JlptLevelsTest, CheckStrings) {
  using enum JlptLevels;
  EXPECT_EQ(toString(N5), "N5");
  EXPECT_EQ(toString(N4), "N4");
  EXPECT_EQ(toString(N3), "N3");
  EXPECT_EQ(toString(N2), "N2");
  EXPECT_EQ(toString(N1), "N1");
  EXPECT_EQ(toString(None), "None");
}

TEST(JlptLevelsTest, CheckValues) {
  using enum JlptLevels;
  size_t i{};
  EXPECT_EQ(AllJlptLevels[i], N5);
  EXPECT_EQ(AllJlptLevels[++i], N4);
  EXPECT_EQ(AllJlptLevels[++i], N3);
  EXPECT_EQ(AllJlptLevels[++i], N2);
  EXPECT_EQ(AllJlptLevels[++i], N1);
  EXPECT_EQ(AllJlptLevels[++i], None);
}

TEST(KenteiKyusTest, CheckStrings) {
  using enum KenteiKyus;
  EXPECT_EQ(toString(K10), "K10");
  EXPECT_EQ(toString(K9), "K9");
  EXPECT_EQ(toString(K8), "K8");
  EXPECT_EQ(toString(K7), "K7");
  EXPECT_EQ(toString(K6), "K6");
  EXPECT_EQ(toString(K5), "K5");
  EXPECT_EQ(toString(K4), "K4");
  EXPECT_EQ(toString(K3), "K3");
  EXPECT_EQ(toString(KJ2), "KJ2");
  EXPECT_EQ(toString(K2), "K2");
  EXPECT_EQ(toString(KJ1), "KJ1");
  EXPECT_EQ(toString(K1), "K1");
  EXPECT_EQ(toString(None), "None");
}

TEST(KenteiKyusTest, CheckValues) {
  using enum KenteiKyus;
  size_t i{};
  EXPECT_EQ(AllKenteiKyus[i], K10);
  EXPECT_EQ(AllKenteiKyus[++i], K9);
  EXPECT_EQ(AllKenteiKyus[++i], K8);
  EXPECT_EQ(AllKenteiKyus[++i], K7);
  EXPECT_EQ(AllKenteiKyus[++i], K6);
  EXPECT_EQ(AllKenteiKyus[++i], K5);
  EXPECT_EQ(AllKenteiKyus[++i], K4);
  EXPECT_EQ(AllKenteiKyus[++i], K3);
  EXPECT_EQ(AllKenteiKyus[++i], KJ2);
  EXPECT_EQ(AllKenteiKyus[++i], K2);
  EXPECT_EQ(AllKenteiKyus[++i], KJ1);
  EXPECT_EQ(AllKenteiKyus[++i], K1);
  EXPECT_EQ(AllKenteiKyus[++i], None);
}

TEST(JinmeiReasonsTest, CheckStrings) {
  using enum JinmeiReasons;
  EXPECT_EQ(toString(Names), "Names");
  EXPECT_EQ(toString(Print), "Print");
  EXPECT_EQ(toString(Variant), "Variant");
  EXPECT_EQ(toString(Moved), "Moved");
  EXPECT_EQ(toString(Simple), "Simple");
  EXPECT_EQ(toString(Other), "Other");
  EXPECT_EQ(toString(None), "None");
}

TEST(JinmeiReasonsTest, CheckValues) {
  using enum JinmeiReasons;
  size_t i{};
  EXPECT_EQ(AllJinmeiReasons[i], Names);
  EXPECT_EQ(AllJinmeiReasons[++i], Print);
  EXPECT_EQ(AllJinmeiReasons[++i], Variant);
  EXPECT_EQ(AllJinmeiReasons[++i], Moved);
  EXPECT_EQ(AllJinmeiReasons[++i], Simple);
  EXPECT_EQ(AllJinmeiReasons[++i], Other);
  EXPECT_EQ(AllJinmeiReasons[++i], None);
}

} // namespace kanji_tools
