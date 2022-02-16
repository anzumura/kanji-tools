#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiGrades.h>

namespace kanji_tools {

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
  size_t i = 0;
  EXPECT_EQ(AllKanjiGrades[i], KanjiGrades::G1);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G2);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G3);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G4);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G5);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::G6);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::S);
  EXPECT_EQ(AllKanjiGrades[++i], KanjiGrades::None);
}

} // namespace kanji_tools
