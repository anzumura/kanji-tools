#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiTypes.h>

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

} // namespace kanji_tools
