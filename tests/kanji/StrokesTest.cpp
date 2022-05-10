#include <gtest/gtest.h>
#include <kanji_tools/kanji/Strokes.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

[[nodiscard]] String error(Strokes::Size s, bool variant = false) {
  return (variant ? "variant " : "") + String{"strokes '"} + std::to_string(s) +
         "' out of range";
}

} // namespace

TEST(StrokesTest, ValidStrokes) {
  const Strokes s{1};
  EXPECT_EQ(s.value(), 1);
  EXPECT_EQ(s.variant(), 0);
  EXPECT_FALSE(s.hasVariant());
  EXPECT_EQ(s.toString(), "1");
  EXPECT_EQ(s.toString(true), "1");
}

TEST(StrokesTest, ValidStrokesWithVariant) {
  const Strokes s{2, 3};
  EXPECT_EQ(s.value(), 2);
  EXPECT_EQ(s.variant(), 3);
  EXPECT_TRUE(s.hasVariant());
  EXPECT_EQ(s.toString(), "2");
  EXPECT_EQ(s.toString(true), "2/3");
}

TEST(StrokesTest, MaxStrokes) {
  const Strokes s{Strokes::Max, Strokes::MaxVariant};
  EXPECT_EQ(s.toString(true), "53/33");
}

TEST(StrokesTest, InvalidStrokes) {
  // 0 is not allowed for strokes
  EXPECT_THROW(call([] { Strokes{0}; }, error(0)), std::range_error);
  const Strokes::Size s{Strokes::Max + 1};
  EXPECT_THROW(call([] { Strokes{s}; }, error(s)), std::range_error);
}

TEST(StrokesTest, InvalidVariantStrokes) {
  // variant strokes must be > 2
  EXPECT_THROW(call([] { Strokes{2, 0}; }, error(0, true)), std::range_error);
  EXPECT_THROW(call([] { Strokes{2, 1}; }, error(1, true)), std::range_error);
  // note: range check is done before 'same value' check
  EXPECT_THROW(call([] { Strokes{2, 2}; }, error(2, true)), std::range_error);
  // strokes must be > 1 if there are variant strokes
  EXPECT_THROW(call([] { Strokes{0, 3}; }, error(0)), std::range_error);
  EXPECT_THROW(call([] { Strokes{1, 3}; }, error(1)), std::range_error);
  const Strokes::Size s{Strokes::MaxVariant + 1};
  EXPECT_THROW(call([] { Strokes{2, s}; }, error(s, true)), std::range_error);
}

TEST(StrokesTest, SameStrokesAndVariant) {
  const String msg{"strokes and variant strokes are the same '3'"};
  EXPECT_THROW(call([] { Strokes{3, 3}; }, msg), std::domain_error);
}

TEST(StrokesTest, StreamOperator) {
  const Strokes s1{3}, s2{4, 5};
  std::stringstream s;
  s << s1 << ' ' << s2;
  // variants are not included in output to operator<<
  EXPECT_EQ(s.str(), "3 4");
}

TEST(StrokesTest, Equals) {
  const Strokes s{5}, diff1{6}, diff2{5, 6}, same{5};
  EXPECT_NE(s, diff1);
  EXPECT_NE(s, diff2);
  EXPECT_EQ(s, same);
}

TEST(StrokesTest, Compare) {
  // sort by 'value', then 'variant'
  const Strokes s1{3}, s1v{3, 4}, s2{4, 3};
  EXPECT_LT(s1, s1v);
  EXPECT_LE(s1, s1v);
  EXPECT_LE(s1v, s1v);
  EXPECT_GE(s1v, s1v);
  EXPECT_GE(s2, s1v);
  EXPECT_GT(s2, s1v);
}

} // namespace kanji_tools
