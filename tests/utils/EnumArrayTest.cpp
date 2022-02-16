#include <gtest/gtest.h>

#include <kanji_tools/tests/WhatMismatch.h>
#include <kanji_tools/utils/EnumArray.h>

#include <sstream>

namespace kanji_tools {

namespace {

enum class Colors { Red, Green, Blue, None };

} // namespace

template<> inline constexpr bool is_enumarray<Colors> = true;
const auto AllColors = EnumArray<Colors>::initialize("Red", "Green", "Blue");

TEST(EnumArrayTest, Iteration) {
  std::vector<Colors> colors;
  for (size_t i = 0; i < AllColors.size(); ++i) colors.push_back(AllColors[i]);
  EXPECT_EQ(colors, std::vector({Colors::Red, Colors::Green, Colors::Blue, Colors::None}));
}

TEST(EnumArrayTest, BadAccess) {
  EXPECT_THROW(call([] { return AllColors[4]; }, "index value 4 is out of range"), std::out_of_range);
}

TEST(EnumArrayTest, RangeBasedForLoop) {
  std::vector<Colors> colors;
  for (auto c : AllColors) colors.push_back(c);
  EXPECT_EQ(colors, std::vector({Colors::Red, Colors::Green, Colors::Blue, Colors::None}));
}

TEST(EnumArrayTest, BadIncrement) {
  auto i = AllColors.begin();
  i += 3;
  EXPECT_EQ(*i, Colors::None);
  EXPECT_EQ(++i, AllColors.end());
  EXPECT_THROW(call([&] { i++; }, "can't increment past end"), std::out_of_range);
  EXPECT_THROW(call([&] { i += 1; }, "can't increment past end"), std::out_of_range);
}

TEST(EnumArrayTest, BadDecrement) {
  auto i = AllColors.end();
  i = i - 4;
  EXPECT_EQ(*i, Colors::Red);
  EXPECT_THROW(call([&] { --i; }, "can't decrement past zero"), std::out_of_range);
}

TEST(EnumArrayTest, IteratorCompare) {
  auto i = AllColors.begin();
  auto j = i;
  EXPECT_EQ(i, j);
  EXPECT_LE(i, j);
  EXPECT_GE(i, j);
  j += 2;
  EXPECT_NE(i, j);
  EXPECT_LE(i, j);
  EXPECT_LT(i, j);
  EXPECT_GE(j, i);
  EXPECT_GT(j, i);
  EXPECT_EQ(j - i, 2);
}

TEST(EnumArrayTest, ToString) {
  EXPECT_STREQ(toString(Colors::Red), "Red");
  EXPECT_STREQ(toString(Colors::Green), "Green");
  EXPECT_STREQ(toString(Colors::Blue), "Blue");
  EXPECT_STREQ(toString(Colors::None), "None");
}

TEST(EnumArrayTest, BadToString) {
  EXPECT_THROW(call([] { return toString(static_cast<Colors>(37)); }, "enum value 37 is out of range"),
               std::out_of_range);
}

TEST(EnumArrayTest, Stream) {
  std::stringstream s;
  s << Colors::Green << ' ' << Colors::None;
  EXPECT_EQ(s.str(), "Green None");
}

TEST(EnumArrayTest, HasValue) {
  EXPECT_FALSE(hasValue(Colors::None)); // only 'None' is false
  EXPECT_TRUE(hasValue(Colors::Blue));
  EXPECT_TRUE(hasValue(static_cast<Colors>(29))); // bad value
}

TEST(EnumArrayTest, OperatorNot) {
  EXPECT_TRUE(!Colors::None); // only 'None' is true
  EXPECT_FALSE(!Colors::Blue);
  EXPECT_FALSE(!static_cast<Colors>(29)); // bad value
}

} // namespace kanji_tools
