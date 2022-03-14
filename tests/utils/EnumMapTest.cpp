#include <gtest/gtest.h>

#include <kanji_tools/tests/WhatMismatch.h>
#include <kanji_tools/utils/EnumMap.h>

namespace kanji_tools {

namespace {

enum class Colors { Red, Green, Blue, None };

class EnumMapTest : public ::testing::Test {
protected:
  EnumMapTest() {
    _map[Colors::Red] = 2;
    _map[Colors::Green] = 4;
    _map[Colors::Blue] = 7;
  }
  EnumMap<Colors, int> _map;
};

} // namespace

TEST_F(EnumMapTest, SquareOperator) {
  _map[Colors::Green] = 6;
  const auto& cMap = _map;
  EXPECT_EQ(_map[Colors::Green], 6);
  EXPECT_EQ(cMap[Colors::Green], 6);
}

TEST_F(EnumMapTest, NoneReturnsEmptyForConstOperator) {
  const auto& cMap = _map;
  EXPECT_EQ(cMap[Colors::None], 0);
  const EnumMap<Colors, std::string> stringMap;
  EXPECT_EQ(stringMap[Colors::None], std::string{});
}

TEST_F(EnumMapTest, NoneThrowsErrorForNonConstOperator) {
  EXPECT_THROW(call([this] { return _map[Colors::None]; },
                    "index 'enum value 3' is out of range"),
               std::out_of_range);
}

TEST_F(EnumMapTest, RangeBasedForLoop) {
  std::vector<int> values;
  for (auto i : _map) values.push_back(i);
  EXPECT_EQ(values, std::vector({2, 4, 7}));
}

TEST_F(EnumMapTest, UninitializedIterator) {
  auto i = EnumMap<Colors, int>::Iterator();
  EXPECT_THROW(call([&] { return *i; }, "not initialized"), std::domain_error);
}

TEST_F(EnumMapTest, BadAccess) {
  EXPECT_THROW(call([this] { return _map[static_cast<Colors>(4UL)]; },
                    "index 'enum value 4' is out of range"),
               std::out_of_range);
}

TEST_F(EnumMapTest, BadIncrement) {
  auto i = _map.begin();
  i = i + 1;
  EXPECT_EQ(i[1], 7);
  i += 1;
  EXPECT_EQ(*i, 7);
  EXPECT_EQ(++i, _map.end());
  EXPECT_THROW(call([&] { return *i; }, "index '3' is out of range"),
               std::out_of_range);
  EXPECT_THROW(call([&] { i++; }, "can't increment past end"),
               std::out_of_range);
  EXPECT_THROW(call([&] { i += 1; }, "can't increment past end"),
               std::out_of_range);
  EXPECT_THROW(call([&] { return i[1]; }, "can't increment past end"),
               std::out_of_range);
}

TEST_F(EnumMapTest, BadDecrement) {
  auto i = _map.end();
  EXPECT_THROW(call([&] { i -= 4; }, "can't decrement past zero"),
               std::out_of_range);
  i -= 3;
  EXPECT_EQ(*i, 2);
  EXPECT_THROW(call([&] { --i; }, "can't decrement past zero"),
               std::out_of_range);
}

TEST_F(EnumMapTest, IteratorCompare) {
  auto i = _map.begin();
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
  // iterator arithmetic
  EXPECT_EQ(j - i, 2);
}

TEST_F(EnumMapTest, CompareIteratorFromDifferentCollections) {
  EnumMap<Colors, int> other;
  auto i = _map.begin();
  auto j = other.begin();
  EXPECT_THROW(call([&] { return i == j; }, "not comparable"),
               std::domain_error);
  EXPECT_THROW(call([&] { return i - j; }, "not comparable"),
               std::domain_error);
}

} // namespace kanji_tools
