#include <gtest/gtest.h>
#include <kanji_tools/utils/JlptLevels.h>

namespace kanji_tools {

TEST(JlptLevelsTest, CheckStrings) {
  EXPECT_EQ(toString(JlptLevels::N5), "N5");
  EXPECT_EQ(toString(JlptLevels::N4), "N4");
  EXPECT_EQ(toString(JlptLevels::N3), "N3");
  EXPECT_EQ(toString(JlptLevels::N2), "N2");
  EXPECT_EQ(toString(JlptLevels::N1), "N1");
  EXPECT_EQ(toString(JlptLevels::None), "None");
}

TEST(JlptLevelsTest, CheckValues) {
  size_t i = 0;
  EXPECT_EQ(AllJlptLevels[i], JlptLevels::N5);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::N4);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::N3);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::N2);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::N1);
  EXPECT_EQ(AllJlptLevels[++i], JlptLevels::None);
}

} // namespace kanji_tools
