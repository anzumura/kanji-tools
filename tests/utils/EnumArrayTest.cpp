#include <gtest/gtest.h>

#include <kanji_tools/tests/WhatMismatch.h>
#include <kanji_tools/utils/EnumArray.h>

#include <sstream>

namespace kanji_tools {

namespace {

enum class Colors { Red, Green, Blue };

enum class TestEnum { A, B, C };

} // namespace

template<> inline constexpr bool is_enumarray<Colors> = true;
inline const auto AllColors = BaseEnumArray<Colors>::create("Red", "Green", "Blue");

template<> inline constexpr bool is_enumarray<TestEnum> = true;

TEST(EnumArrayTest, FailForDuplicateName) {
  EXPECT_THROW(call([] { auto x = BaseEnumArray<TestEnum>::create("A", "B", "B"); }, "duplicate name 'B'"),
               std::domain_error);
}

TEST(EnumArrayTest, CallInstanceBeforeCreate) {
  // 'toString' calls 'instance'
  EXPECT_THROW(call([] { return toString(TestEnum::A); }, "must call 'create' before calling 'instance'"),
               std::domain_error);
}

TEST(EnumArrayTest, DestructorClearsInstance) {
  for (auto _ = 2; _--;) {
    EXPECT_FALSE(BaseEnumArray<TestEnum>::isCreated());
    auto x = BaseEnumArray<TestEnum>::create("A", "B", "C");
    EXPECT_TRUE(BaseEnumArray<TestEnum>::isCreated());
  }
}

TEST(EnumArrayTest, CallCreateTwice) {
  auto enumArray = BaseEnumArray<TestEnum>::create("A", "B", "C");
  // 'create' returns an 'EnumArray' for the given enum with a second template parameter for the
  // number of names provided.
  EXPECT_EQ(typeid(enumArray), typeid(EnumArray<TestEnum, 3>));
  EXPECT_EQ(enumArray.size(), 3);
  // 'instance' returns 'const BaseEnumArray&', but the object returned is 'EnumArray' (typeid ignores
  // 'const', but put it in for clarity)
  auto& instance = enumArray.instance();
  EXPECT_EQ(typeid(std::result_of_t<decltype (&BaseEnumArray<TestEnum>::instance)()>),
            typeid(const BaseEnumArray<TestEnum>&));
  EXPECT_EQ(typeid(instance), typeid(const EnumArray<TestEnum, 3>&));
  // calling 'create' again should throw an exception
  EXPECT_THROW(
    call([] { auto x = BaseEnumArray<TestEnum>::create("A", "B", "C"); }, "'create' should only be called once"),
    std::domain_error);
}

TEST(EnumArrayTest, Iteration) {
  std::vector<Colors> colors;
  for (size_t i = 0; i < AllColors.size(); ++i) colors.push_back(AllColors[i]);
  EXPECT_EQ(colors, std::vector({Colors::Red, Colors::Green, Colors::Blue}));
}

TEST(EnumArrayTest, BadAccess) {
  EXPECT_THROW(call([] { return AllColors[4]; }, "index '4' is out of range"), std::out_of_range);
}

TEST(EnumArrayTest, RangeBasedForLoop) {
  std::vector<Colors> colors;
  for (auto c : AllColors) colors.push_back(c);
  EXPECT_EQ(colors, std::vector({Colors::Red, Colors::Green, Colors::Blue}));
}

TEST(EnumArrayTest, BadIncrement) {
  auto i = AllColors.begin();
  i = i + 1;
  EXPECT_EQ(i[1], Colors::Blue);
  i += 1;
  EXPECT_EQ(*i, Colors::Blue);
  EXPECT_EQ(++i, AllColors.end());
  EXPECT_THROW(call([&] { return *i; }, "index '3' is out of range"), std::out_of_range);
  EXPECT_THROW(call([&] { i++; }, "can't increment past end"), std::out_of_range);
  EXPECT_THROW(call([&] { i += 1; }, "can't increment past end"), std::out_of_range);
  EXPECT_THROW(call([&] { return i[1]; }, "can't increment past end"), std::out_of_range);
}

TEST(EnumArrayTest, BadDecrement) {
  auto i = AllColors.end();
  i = i - 3;
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
  // iterator arithmetic
  EXPECT_EQ(j - i, 2);
}

TEST(EnumArrayTest, ThreeWayCompare) {
  auto i = AllColors.begin();
  auto j = i;
  EXPECT_EQ(i <=> j, std::strong_ordering::equal);
  j += 2;
  EXPECT_EQ(i <=> j, std::strong_ordering::less);
  EXPECT_EQ(j <=> i, std::strong_ordering::greater);
}

TEST(EnumArrayTest, ToString) {
  EXPECT_EQ(toString(Colors::Red), "Red");
  EXPECT_EQ(toString(Colors::Green), "Green");
  EXPECT_EQ(toString(Colors::Blue), "Blue");
}

TEST(EnumArrayTest, BadToString) {
  EXPECT_THROW(call([] { return toString(static_cast<Colors>(7)); }, "enum '7' is out of range"), std::out_of_range);
}

TEST(EnumArrayTest, Stream) {
  std::stringstream s;
  s << Colors::Green << ' ' << Colors::Blue;
  EXPECT_EQ(s.str(), "Green Blue");
}

TEST(EnumArrayTest, FromString) {
  EXPECT_EQ(AllColors.fromString("Red"), Colors::Red);
  EXPECT_EQ(AllColors.fromString("Green"), Colors::Green);
  EXPECT_EQ(AllColors.fromString("Blue"), Colors::Blue);
}

TEST(EnumArrayTest, BadFromString) {
  EXPECT_THROW(call([] { return AllColors.fromString(""); }, "name '' not found"), std::domain_error);
  EXPECT_THROW(call([] { return AllColors.fromString("Blah"); }, "name 'Blah' not found"), std::domain_error);
}

} // namespace kanji_tools
