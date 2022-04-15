#include <gtest/gtest.h>

#include <kanji_tools/utils/EnumArray.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

enum class Colors { Red, Green, Blue, None };

enum class TestEnum { A, B, C, None };

} // namespace

template<> inline constexpr auto is_enumarray_with_none<Colors>{true};

inline const auto AllColors{
    TypedEnumArray<Colors>::create("Red", "Green", "Blue")};

template<> inline constexpr auto is_enumarray_with_none<TestEnum>{true};

TEST(EnumArrayWithNoneTest, FailForDuplicateName) {
  EXPECT_THROW(
      call([] { return TypedEnumArray<TestEnum>::create("A", "B", "B"); },
          "duplicate name 'B'"),
      std::domain_error);
}

TEST(EnumArrayWithNoneTest, FailForNoneName) {
  EXPECT_THROW(
      call([] { return TypedEnumArray<TestEnum>::create("A", "B", "None"); },
          "'None' should not be specified"),
      std::domain_error);
}

TEST(EnumArrayWithNoneTest, CallInstanceBeforeCreate) {
  // 'toString' calls 'instance'
  EXPECT_THROW(call([] { return toString(TestEnum::A); },
                   "must call 'create' before calling 'instance'"),
      std::domain_error);
}

TEST(EnumArrayWithNoneTest, DestructorClearsInstance) {
  for (auto _{2}; _--;) {
    EXPECT_FALSE(TypedEnumArray<TestEnum>::isCreated());
    const auto x{TypedEnumArray<TestEnum>::create("A", "B", "C")};
    EXPECT_TRUE(TypedEnumArray<TestEnum>::isCreated());
  }
}

TEST(EnumArrayWithNoneTest, CallCreateTwice) {
  const auto enumArray{TypedEnumArray<TestEnum>::create("A", "B", "C")};
  // 'create' returns an 'EnumArray' for the given enum with a second template
  // parameter for the number of names provided (the actual enum should have one
  // more 'None' value at the end)
  EXPECT_EQ(typeid(enumArray), typeid(EnumArrayWithNone<TestEnum, 3>));
  EXPECT_EQ(enumArray.size(), 4); // 'size' includes final 'None' value
  // 'instance' returns 'const TypedEnumArray&', but the object returned is
  // 'EnumArrayWithNone'
  auto& instance{enumArray.instance()};
  EXPECT_EQ(
      typeid(
          std::result_of_t<decltype (&TypedEnumArray<TestEnum>::instance)()>),
      typeid(const TypedEnumArray<TestEnum>&));
  EXPECT_EQ(typeid(instance), typeid(const EnumArrayWithNone<TestEnum, 3>&));
  // calling 'create' again should throw an exception
  EXPECT_THROW(
      call([] { return TypedEnumArray<TestEnum>::create("A", "B", "C"); },
          "'create' should only be called once"),
      std::domain_error);
}

TEST(EnumArrayWithNoneTest, Iteration) {
  std::vector<Colors> colors;
  for (size_t i{}; i < AllColors.size(); ++i) colors.emplace_back(AllColors[i]);
  EXPECT_EQ(colors,
      (std::vector{Colors::Red, Colors::Green, Colors::Blue, Colors::None}));
}

TEST(EnumArrayWithNoneTest, BadAccess) {
  EXPECT_THROW(call([] { return AllColors[4]; }, "index '4' is out of range"),
      std::out_of_range);
}

TEST(EnumArrayWithNoneTest, RangeBasedForLoop) {
  std::vector<Colors> colors;
  for (auto c : AllColors) colors.emplace_back(c);
  EXPECT_EQ(colors,
      (std::vector{Colors::Red, Colors::Green, Colors::Blue, Colors::None}));
}

TEST(EnumArrayWithNoneTest, BadIncrement) {
  auto i{AllColors.begin()};
  i = i + 1;
  EXPECT_EQ(i[2], Colors::None);
  i += 2;
  EXPECT_EQ(*i, Colors::None);
  EXPECT_EQ(++i, AllColors.end());
  EXPECT_THROW(
      call([&] { return *i; }, "index '4' is out of range"), std::out_of_range);
  EXPECT_THROW(
      call([&] { i++; }, "can't increment past end"), std::out_of_range);
  EXPECT_THROW(
      call([&] { i += 1; }, "can't increment past end"), std::out_of_range);
  EXPECT_THROW(call([&] { return i[1]; }, "can't increment past end"),
      std::out_of_range);
}

TEST(EnumArrayWithNoneTest, BadDecrement) {
  auto i{AllColors.end()};
  EXPECT_THROW(
      call([&] { i -= 5; }, "can't decrement past zero"), std::out_of_range);
  i -= 4;
  EXPECT_EQ(*i, Colors::Red);
  EXPECT_THROW(
      call([&] { --i; }, "can't decrement past zero"), std::out_of_range);
}

TEST(EnumArrayWithNoneTest, IteratorCompare) {
  auto i{AllColors.begin()};
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

TEST(EnumArrayWithNoneTest, ThreeWayCompare) {
  auto i{AllColors.begin()};
  auto j{i};
  EXPECT_EQ(i <=> j, std::strong_ordering::equal);
  j += 2;
  EXPECT_EQ(i <=> j, std::strong_ordering::less);
  EXPECT_EQ(j <=> i, std::strong_ordering::greater);
}

TEST(EnumArrayWithNoneTest, ToString) {
  EXPECT_EQ(toString(Colors::Red), "Red");
  EXPECT_EQ(toString(Colors::Green), "Green");
  EXPECT_EQ(toString(Colors::Blue), "Blue");
  EXPECT_EQ(toString(Colors::None), "None");
}

TEST(EnumArrayWithNoneTest, BadToString) {
  EXPECT_THROW(call([] { return toString(static_cast<Colors>(7)); },
                   "enum '7' is out of range"),
      std::out_of_range);
}

TEST(EnumArrayWithNoneTest, Stream) {
  std::stringstream s;
  s << Colors::Green << ' ' << Colors::None;
  EXPECT_EQ(s.str(), "Green None");
}

TEST(EnumArrayWithNoneTest, FromString) {
  EXPECT_EQ(AllColors.fromString("Red"), Colors::Red);
  EXPECT_EQ(AllColors.fromString("Green"), Colors::Green);
  EXPECT_EQ(AllColors.fromString("Blue"), Colors::Blue);
  EXPECT_EQ(AllColors.fromString("None"), Colors::None);
  // set allowEmptyAsNone to true
  EXPECT_EQ(AllColors.fromString("", true), Colors::None);
}

TEST(EnumArrayWithNoneTest, BadFromString) {
  EXPECT_THROW(
      call([] { return AllColors.fromString(""); }, "name '' not found"),
      std::domain_error);
  EXPECT_THROW(call([] { return AllColors.fromString("Blah"); },
                   "name 'Blah' not found"),
      std::domain_error);
}

TEST(EnumArrayWithNoneTest, HasValue) {
  EXPECT_FALSE(hasValue(Colors::None)); // only 'None' is false
  EXPECT_TRUE(hasValue(Colors::Blue));
  EXPECT_TRUE(hasValue(static_cast<Colors>(29))); // bad value
}

TEST(EnumArrayWithNoneTest, OperatorNot) {
  EXPECT_TRUE(!Colors::None); // only 'None' is true
  EXPECT_FALSE(!Colors::Blue);
  EXPECT_FALSE(!static_cast<Colors>(29)); // bad value
}

TEST(EnumArrayWithNoneTest, IsNextNone) {
  EXPECT_FALSE(isNextNone(Colors::Red));
  EXPECT_FALSE(isNextNone(Colors::Green));
  EXPECT_TRUE(isNextNone(Colors::Blue));
  EXPECT_FALSE(isNextNone(Colors::None));
  EXPECT_FALSE(isNextNone(static_cast<Colors>(4))); // bad value
}

} // namespace kanji_tools
