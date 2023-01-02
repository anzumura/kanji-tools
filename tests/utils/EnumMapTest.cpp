#include <gtest/gtest.h>

#include <kt_tests/WhatMismatch.h>
#include <kt_utils/EnumMap.h>
#include <kt_utils/Exception.h>

namespace kanji_tools {

namespace {

enum class Colors : Enum::Size { Red, Green, Blue, None };

class EnumMapTest : public ::testing::Test {
protected:
  // assign some sample values for testing
  static constexpr auto RedVal{2}, GreenVal{4}, BlueVal{7};

  EnumMapTest() {
    _map[Colors::Red] = RedVal;
    _map[Colors::Green] = GreenVal;
    _map[Colors::Blue] = BlueVal;
  }

  auto& map() { return _map; }

private:
  EnumMap<Colors, int> _map;
};

} // namespace

TEST_F(EnumMapTest, SquareOperator) {
  constexpr auto expected{6};
  map()[Colors::Green] = expected;
  const auto& cMap{map()};
  EXPECT_EQ(map()[Colors::Green], expected);
  EXPECT_EQ(cMap[Colors::Green], expected);
}

TEST_F(EnumMapTest, NoneReturnsEmptyForConstOperator) {
  const auto& cMap{map()};
  EXPECT_EQ(cMap[Colors::None], 0);
  const EnumMap<Colors, String> stringMap;
  EXPECT_EQ(stringMap[Colors::None], String{});
}

TEST_F(EnumMapTest, NoneThrowsErrorForNonConstOperator) {
  EXPECT_THROW(call([this] { return map()[Colors::None]; },
                   "index 'enum value 3' is out of range"),
      RangeError);
}

TEST_F(EnumMapTest, RangeBasedForLoop) {
  std::vector<int> values;
  for (auto i : map()) values.emplace_back(i);
  EXPECT_EQ(values, (std::vector{RedVal, GreenVal, BlueVal}));
}

TEST_F(EnumMapTest, UninitializedIterator) {
  auto i{EnumMap<Colors, int>::ConstIterator()};
  EXPECT_THROW(call([&] { return *i; }, "not initialized"), DomainError);
}

TEST_F(EnumMapTest, BadAccess) {
  EXPECT_THROW(call([this] { return map()[to_enum<Colors>(4U)]; },
                   "index 'enum value 4' is out of range"),
      RangeError);
}

TEST_F(EnumMapTest, IteratorIncrementAndDecrement) {
  auto i{map().begin()};
  ASSERT_NE(i, map().end());
  auto j{i};
  EXPECT_NE(++i, j);
  EXPECT_EQ(--i, j);
  EXPECT_EQ(i++, j);
  EXPECT_NE(i--, j);
}

TEST_F(EnumMapTest, IteratorAdditionAndSubtraction) {
  auto i{map().begin()};
  ASSERT_NE(i, map().end());
  auto j{i};
  EXPECT_NE(i + 1, j);
  EXPECT_EQ(i + 2, j += 2);
  EXPECT_NE(i, j - 1);
  EXPECT_EQ(i, j -= 2);
}

TEST_F(EnumMapTest, BadIncrement) {
  auto i{map().begin()};
  i = i + 1;
  EXPECT_EQ(i[1], BlueVal);
  i += 1;
  EXPECT_EQ(*i, BlueVal);
  EXPECT_EQ(++i, map().end());
  EXPECT_THROW(
      call([&] { return *i; }, "index '3' is out of range"), RangeError);
  EXPECT_THROW(call([&] { i++; }, "can't increment past end"), RangeError);
  EXPECT_THROW(call([&] { i += 1; }, "can't increment past end"), RangeError);
  EXPECT_THROW(
      call([&] { return i[1]; }, "can't increment past end"), RangeError);
}

TEST_F(EnumMapTest, BadDecrement) {
  auto i{map().end()};
  EXPECT_THROW(call([&] { i -= 4; }, "can't decrement past zero"), RangeError);
  i -= 3;
  EXPECT_EQ(*i, RedVal);
  EXPECT_THROW(call([&] { --i; }, "can't decrement past zero"), RangeError);
}

TEST_F(EnumMapTest, IteratorCompare) {
  auto i{map().begin()};
  auto j{i};
  EXPECT_EQ(i, j);
  EXPECT_LE(i, j);
  EXPECT_GE(i, j);
  j += 2;
  EXPECT_NE(i, j);
  EXPECT_LE(i, j);
  EXPECT_LT(i, j);
  EXPECT_GE(j, i);
  EXPECT_GT(j, i);
  // iterator arithmetic
  EXPECT_EQ(j - i, 2);
}

TEST_F(EnumMapTest, CompareIteratorFromDifferentCollections) {
  EnumMap<Colors, int>::ConstIterator i, j;
  // uninitialized iterators are considered equal
  EXPECT_EQ(i, j);
  EXPECT_FALSE(i != j);
  // an initialized iterator can't be compared to an initialized one
  i = map().begin();
  EXPECT_THROW(call([&] { return i == j; }, "not comparable"), DomainError);
  const EnumMap<Colors, int> other; // all values are initially set to zero
  EXPECT_EQ(map().size(), other.size());
  j = other.begin();
  // iterators for different collections can't be compared even if they both
  // point to the same locations (begin, middle or end)
  for (int distance{};; ++distance, ++i, ++j) {
    EXPECT_EQ(i - map().begin(), distance);
    EXPECT_EQ(j - other.begin(), distance);
    EXPECT_THROW(call([&] { return i == j; }, "not comparable"), DomainError);
    EXPECT_THROW(call([&] { return i - j; }, "not comparable"), DomainError);
    // put loop break condition here to let 'end' case get tested as well
    if (i == map().end()) break;
  }
  EXPECT_EQ(j, other.end());
  EXPECT_EQ(j - other.begin(), 3);
}

} // namespace kanji_tools
