#include <gtest/gtest.h>

#include <kanji_tools/utils/Bitmask.h>

namespace kanji_tools {

namespace {

enum class TestEnum : uint8_t { One = 1, Two, Four = 4, Eight = 8, All = 15 };

} // namespace

template<> inline constexpr auto is_bitmask<TestEnum>{true};

TEST(BitmaskTest, BitwiseAndOperator) {
  EXPECT_EQ(TestEnum::All & TestEnum::Two, TestEnum::Two);
  EXPECT_EQ(TestEnum::One & TestEnum::Two, to_enum<TestEnum>(0));
}

TEST(BitmaskTest, BitwiseOrOperator) {
  EXPECT_EQ(TestEnum::Two | TestEnum::Four, to_enum<TestEnum>(2 | 4));
  EXPECT_EQ(TestEnum::One | TestEnum::Two | TestEnum::Four | TestEnum::Eight,
      TestEnum::All);
}

TEST(BitmaskTest, BitwiseXOrOperator) {
  EXPECT_EQ(TestEnum::All ^ TestEnum::Eight, to_enum<TestEnum>(15 ^ 8));
  EXPECT_EQ(TestEnum::Four ^ TestEnum::Four, to_enum<TestEnum>(0));
}

TEST(BitmaskTest, BitwiseComplementOperator) {
  constexpr uint8_t four{4};
  EXPECT_EQ(~TestEnum::Four,
      to_enum<TestEnum>(four ^ std::numeric_limits<uint8_t>::max()));
  auto x{TestEnum::Two | TestEnum::Four};
  EXPECT_EQ(x &= ~TestEnum::Two, TestEnum::Four);
}

TEST(BitmaskTest, BitwiseAndEqualOperator) {
  auto x{TestEnum::One};
  EXPECT_EQ(x &= TestEnum::All, TestEnum::One);
  // test chaining
  auto y{TestEnum::All};
  ((y &= TestEnum::Two) |= TestEnum::Four) &= TestEnum::Two;
  EXPECT_EQ(y, TestEnum::Two);
}

TEST(BitmaskTest, BitwiseOrEqualOperator) {
  auto x{TestEnum::Two};
  EXPECT_EQ(x |= TestEnum::One, to_enum<TestEnum>(1 | 2));
  EXPECT_EQ(x |= TestEnum::Four, to_enum<TestEnum>(1 | 2 | 4));
  // test chaining
  auto y{TestEnum::One};
  ((y |= TestEnum::Two) |= TestEnum::Four) |= TestEnum::Eight;
  EXPECT_EQ(y, TestEnum::All);
}

TEST(BitmaskTest, BitwiseXOrEqualOperator) {
  auto x{TestEnum::All};
  EXPECT_EQ(x ^= TestEnum::Four, to_enum<TestEnum>(1 | 2 | 8));
  EXPECT_EQ(x ^= TestEnum::Eight, to_enum<TestEnum>(1 | 2));
  // test chaining
  auto y{TestEnum::All};
  ((y ^= TestEnum::Two) ^= TestEnum::Four) ^= TestEnum::Eight;
  EXPECT_EQ(y, TestEnum::One);
}

TEST(BitmaskTest, HasValue) {
  EXPECT_TRUE(hasValue(TestEnum::One));
  const auto x{TestEnum::Two};
  EXPECT_TRUE(hasValue(x));
  EXPECT_FALSE(hasValue(x ^ TestEnum::Two));
}

TEST(BitmaskTest, NotOperator) {
  EXPECT_FALSE(!TestEnum::One);
  const auto x{TestEnum::Two};
  EXPECT_FALSE(!x);
  EXPECT_TRUE(!(x ^ TestEnum::Two));
}

} // namespace kanji_tools
