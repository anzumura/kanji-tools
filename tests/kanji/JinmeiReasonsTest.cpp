#include <gtest/gtest.h>
#include <kanji_tools/kanji/JinmeiReasons.h>

namespace kanji_tools {

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
