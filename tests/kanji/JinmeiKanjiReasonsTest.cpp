#include <gtest/gtest.h>
#include <kanji_tools/kanji/JinmeiKanjiReasons.h>

namespace kanji_tools {

TEST(JinmeiKanjiReasonsTest, CheckStrings) {
  EXPECT_EQ(toString(JinmeiKanjiReasons::Names), "Names");
  EXPECT_EQ(toString(JinmeiKanjiReasons::Print), "Print");
  EXPECT_EQ(toString(JinmeiKanjiReasons::Variant), "Variant");
  EXPECT_EQ(toString(JinmeiKanjiReasons::Moved), "Moved");
  EXPECT_EQ(toString(JinmeiKanjiReasons::Simple), "Simple");
  EXPECT_EQ(toString(JinmeiKanjiReasons::Other), "Other");
}

TEST(JinmeiKanjiReasonsTest, CheckValues) {
  size_t i = 0;
  EXPECT_EQ(AllJinmeiKanjiReasons[i], JinmeiKanjiReasons::Names);
  EXPECT_EQ(AllJinmeiKanjiReasons[++i], JinmeiKanjiReasons::Print);
  EXPECT_EQ(AllJinmeiKanjiReasons[++i], JinmeiKanjiReasons::Variant);
  EXPECT_EQ(AllJinmeiKanjiReasons[++i], JinmeiKanjiReasons::Moved);
  EXPECT_EQ(AllJinmeiKanjiReasons[++i], JinmeiKanjiReasons::Simple);
  EXPECT_EQ(AllJinmeiKanjiReasons[++i], JinmeiKanjiReasons::Other);
}

} // namespace kanji_tools
