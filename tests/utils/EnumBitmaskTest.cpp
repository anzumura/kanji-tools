#include <gtest/gtest.h>

#include <kanji_tools/utils/EnumBitmask.h>

namespace kanji_tools {

enum class TestEnum { One = 1, Two, Four = 4, Eight = 8, All = 15 };
template<> struct enum_bitmask<TestEnum> { static constexpr bool value = true; };

TEST(EnumBitmaskTest, BitwiseAndOperator) {
  EXPECT_EQ(TestEnum::All & TestEnum::Two, TestEnum::Two);
  EXPECT_EQ(TestEnum::One & TestEnum::Two, static_cast<TestEnum>(0));
}

TEST(EnumBitmaskTest, BitwiseOrOperator) {
  EXPECT_EQ(TestEnum::Two | TestEnum::Four, static_cast<TestEnum>(2 | 4));
  EXPECT_EQ(TestEnum::One | TestEnum::Two | TestEnum::Four | TestEnum::Eight, TestEnum::All);
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
}

TEST(EnumBitmaskTest, BitwiseOrEqualOperator) {
  auto x = TestEnum::Two;
  EXPECT_EQ(x |= TestEnum::One, static_cast<TestEnum>(1 | 2));
  EXPECT_EQ(x |= TestEnum::Four, static_cast<TestEnum>(1 | 2 | 4));
}

TEST(EnumBitmaskTest, BitwiseXOrEqualOperator) {
  auto x = TestEnum::All;
  EXPECT_EQ(x ^= TestEnum::Four, static_cast<TestEnum>(1 | 2 | 8));
  EXPECT_EQ(x ^= TestEnum::Eight, static_cast<TestEnum>(1 | 2));
}

} // namespace kanji_tools
