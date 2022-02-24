#include <gtest/gtest.h>

#include <kanji_tools/utils/EnumBitmask.h>

namespace kanji_tools {

namespace {

enum class TestEnum { One = 1, Two, Four = 4, Eight = 8, All = 15 };

} // namespace

template<> inline constexpr bool is_bitmask<TestEnum> = true;

TEST(EnumBitmaskTest, BitwiseAndOperator) {
  EXPECT_EQ(TestEnum::All & TestEnum::Two, TestEnum::Two);
  EXPECT_EQ(TestEnum::One & TestEnum::Two, static_cast<TestEnum>(0));
}

TEST(EnumBitmaskTest, BitwiseOrOperator) {
  EXPECT_EQ(TestEnum::Two | TestEnum::Four, static_cast<TestEnum>(2 | 4));
  EXPECT_EQ(TestEnum::One | TestEnum::Two | TestEnum::Four | TestEnum::Eight,
            TestEnum::All);
}

TEST(EnumBitmaskTest, BitwiseXOrOperator) {
  EXPECT_EQ(TestEnum::All ^ TestEnum::Eight, static_cast<TestEnum>(15 ^ 8));
  EXPECT_EQ(TestEnum::Four ^ TestEnum::Four, static_cast<TestEnum>(0));
}

TEST(EnumBitmaskTest, BitwiseComplementOperator) {
  EXPECT_EQ(~TestEnum::Four, static_cast<TestEnum>(~4));
  auto x = TestEnum::Two | TestEnum::Four;
  EXPECT_EQ(x &= ~TestEnum::Two, TestEnum::Four);
}

TEST(EnumBitmaskTest, BitwiseAndEqualOperator) {
  auto x = TestEnum::One;
  EXPECT_EQ(x &= TestEnum::All, TestEnum::One);
  // test chaining
  auto y = TestEnum::All;
  ((y &= TestEnum::Two) |= TestEnum::Four) &= TestEnum::Two;
  EXPECT_EQ(y, TestEnum::Two);
}

TEST(EnumBitmaskTest, BitwiseOrEqualOperator) {
  auto x = TestEnum::Two;
  EXPECT_EQ(x |= TestEnum::One, static_cast<TestEnum>(1 | 2));
  EXPECT_EQ(x |= TestEnum::Four, static_cast<TestEnum>(1 | 2 | 4));
  // test chaining
  auto y = TestEnum::One;
  ((y |= TestEnum::Two) |= TestEnum::Four) |= TestEnum::Eight;
  EXPECT_EQ(y, TestEnum::All);
}

TEST(EnumBitmaskTest, BitwiseXOrEqualOperator) {
  auto x = TestEnum::All;
  EXPECT_EQ(x ^= TestEnum::Four, static_cast<TestEnum>(1 | 2 | 8));
  EXPECT_EQ(x ^= TestEnum::Eight, static_cast<TestEnum>(1 | 2));
  // test chaining
  auto y = TestEnum::All;
  ((y ^= TestEnum::Two) ^= TestEnum::Four) ^= TestEnum::Eight;
  EXPECT_EQ(y, TestEnum::One);
}

TEST(EnumBitmaskTest, HasValue) {
  EXPECT_TRUE(hasValue(TestEnum::One));
  auto x = TestEnum::Two;
  EXPECT_TRUE(hasValue(x));
  EXPECT_FALSE(hasValue(x ^ TestEnum::Two));
}

TEST(EnumBitmaskTest, NotOperator) {
  EXPECT_FALSE(!TestEnum::One);
  auto x = TestEnum::Two;
  EXPECT_FALSE(!x);
  EXPECT_TRUE(!(x ^ TestEnum::Two));
}

} // namespace kanji_tools
