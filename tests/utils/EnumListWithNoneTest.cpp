#include <gtest/gtest.h>

#include <kt_tests/WhatMismatch.h>
#include <kt_utils/EnumList.h>

#include <sstream>

namespace kanji_tools {

namespace {

enum class Colors : Enum::Size { Red, Green, Blue, None };

constexpr Colors BadColor{29};

enum class TestEnum : Enum::Size { A, B, C, None };

} // namespace

template <> inline constexpr auto is_enumlist_with_none<Colors>{true};

inline const auto AllColors{
    BaseEnumList<Colors>::create("Red", "Green", "Blue")};

template <> inline constexpr auto is_enumlist_with_none<TestEnum>{true};

TEST(EnumListWithNoneTest, FailForDuplicateName) {
  EXPECT_THROW(
      call([] { return BaseEnumList<TestEnum>::create("A", "B", "B"); },
          "duplicate name 'B'"),
      DomainError);
}

TEST(EnumListWithNoneTest, FailForNoneName) {
  EXPECT_THROW(
      call([] { return BaseEnumList<TestEnum>::create("A", "B", "None"); },
          "'None' should not be specified"),
      DomainError);
}

TEST(EnumListWithNoneTest, CallInstanceBeforeCreate) {
  // 'toString' calls 'instance'
  EXPECT_THROW(call([] { return toString(TestEnum::A); },
                   "must call 'create' before calling 'instance'"),
      DomainError);
}

TEST(EnumListWithNoneTest, DestructorClearsInstance) {
  for (auto _{2}; _--;) {
    EXPECT_FALSE(BaseEnumList<TestEnum>::isCreated());
    const auto x{BaseEnumList<TestEnum>::create("A", "B", "C")};
    EXPECT_TRUE(BaseEnumList<TestEnum>::isCreated());
  }
}

TEST(EnumListWithNoneTest, CallCreateTwice) {
  const auto enumArray{BaseEnumList<TestEnum>::create("A", "B", "C")};
  // 'create' returns an 'EnumList' for the given enum with a second template
  // parameter for the number of names provided (the actual enum should have one
  // more 'None' value at the end)
  EXPECT_EQ(typeid(enumArray), typeid(EnumListWithNone<TestEnum, 3>));
  EXPECT_EQ(enumArray.size(), 4); // 'size' includes final 'None' value
  auto& instance{enumArray.instance()};
  EXPECT_EQ(
      typeid(
          std::invoke_result_t<decltype (&BaseEnumList<TestEnum>::instance)()>),
      typeid(BaseEnumList<TestEnum> const& (*)()));
  EXPECT_EQ(typeid(instance), typeid(const EnumListWithNone<TestEnum, 3>&));
  // calling 'create' again should throw an exception
  EXPECT_THROW(
      call([] { return BaseEnumList<TestEnum>::create("A", "B", "C"); },
          "'create' should only be called once"),
      DomainError);
}

TEST(EnumListWithNoneTest, Iteration) {
  std::vector<Colors> colors;
  for (size_t i{}; i < AllColors.size(); ++i) colors.emplace_back(AllColors[i]);
  EXPECT_EQ(colors,
      (std::vector{Colors::Red, Colors::Green, Colors::Blue, Colors::None}));
}

TEST(EnumListWithNoneTest, BadAccess) {
  EXPECT_THROW(call([] { return AllColors[4]; }, "index '4' is out of range"),
      RangeError);
}

TEST(EnumListWithNoneTest, RangeBasedForLoop) {
  std::vector<Colors> colors;
  for (auto c : AllColors) colors.emplace_back(c);
  EXPECT_EQ(colors,
      (std::vector{Colors::Red, Colors::Green, Colors::Blue, Colors::None}));
}

TEST(EnumListWithNoneTest, BadIncrement) {
  auto i{AllColors.begin()};
  i = i + 1;
  EXPECT_EQ(i[2], Colors::None);
  i += 2;
  EXPECT_EQ(*i, Colors::None);
  EXPECT_EQ(++i, AllColors.end());
  EXPECT_THROW(
      call([&] { return *i; }, "index '4' is out of range"), RangeError);
  EXPECT_THROW(call([&] { i++; }, "can't increment past end"), RangeError);
  EXPECT_THROW(call([&] { i += 1; }, "can't increment past end"), RangeError);
  EXPECT_THROW(
      call([&] { return i[1]; }, "can't increment past end"), RangeError);
}

TEST(EnumListWithNoneTest, BadDecrement) {
  auto i{AllColors.end()};
  EXPECT_THROW(call([&] { i -= 5; }, "can't decrement past zero"), RangeError);
  i -= 4;
  EXPECT_EQ(*i, Colors::Red);
  EXPECT_THROW(call([&] { --i; }, "can't decrement past zero"), RangeError);
}

TEST(EnumListWithNoneTest, IteratorCompare) {
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

TEST(EnumListWithNoneTest, ThreeWayCompare) {
  auto i{AllColors.begin()};
  auto j{i};
  EXPECT_EQ(i <=> j, std::strong_ordering::equal);
  j += 2;
  EXPECT_EQ(i <=> j, std::strong_ordering::less);
  EXPECT_EQ(j <=> i, std::strong_ordering::greater);
}

TEST(EnumListWithNoneTest, ToString) {
  EXPECT_EQ(toString(Colors::Red), "Red");
  EXPECT_EQ(toString(Colors::Green), "Green");
  EXPECT_EQ(toString(Colors::Blue), "Blue");
  EXPECT_EQ(toString(Colors::None), "None");
}

TEST(EnumListWithNoneTest, BadToString) {
  EXPECT_THROW(
      call([] { return toString(BadColor); }, "enum '29' is out of range"),
      RangeError);
}

TEST(EnumListWithNoneTest, Stream) {
  std::stringstream s;
  s << Colors::Green << ' ' << Colors::None;
  EXPECT_EQ(s.str(), "Green None");
}

TEST(EnumListWithNoneTest, FromString) {
  EXPECT_EQ(AllColors.fromString("Red"), Colors::Red);
  EXPECT_EQ(AllColors.fromString("Green"), Colors::Green);
  EXPECT_EQ(AllColors.fromString("Blue"), Colors::Blue);
  EXPECT_EQ(AllColors.fromStringAllowEmpty(""), Colors::None);
  EXPECT_EQ(AllColors.fromStringAllowNone("None"), Colors::None);
  EXPECT_EQ(AllColors.fromStringAllowEmptyAndNone(""), Colors::None);
  EXPECT_EQ(AllColors.fromStringAllowEmptyAndNone("None"), Colors::None);
}

TEST(EnumListWithNoneTest, BadFromString) {
  for (auto i : {"", "None", "Blah"}) {
    const String s{i};
    const String msg{"name '" + s + "' not found"};
    EXPECT_THROW(
        call([&s] { return AllColors.fromString(s); }, msg), DomainError);
    if (s != "None")
      EXPECT_THROW(call([&s] { return AllColors.fromStringAllowNone(s); }, msg),
          DomainError);
    if (!s.empty())
      EXPECT_THROW(
          call([&s] { return AllColors.fromStringAllowEmpty(s); }, msg),
          DomainError);
    if (!s.empty() && s != "None")
      EXPECT_THROW(
          call([&s] { return AllColors.fromStringAllowEmptyAndNone(s); }, msg),
          DomainError);
  }
}

TEST(EnumListWithNoneTest, HasValue) {
  EXPECT_FALSE(hasValue(Colors::None)); // only 'None' is false
  EXPECT_TRUE(hasValue(Colors::Blue));
  EXPECT_TRUE(hasValue(BadColor)); // bad value
}

TEST(EnumListWithNoneTest, OperatorNot) {
  EXPECT_TRUE(!Colors::None); // only 'None' is true
  EXPECT_FALSE(!Colors::Blue);
  EXPECT_FALSE(!BadColor); // bad value
}

TEST(EnumListWithNoneTest, IsNextNone) {
  EXPECT_FALSE(isNextNone(Colors::Red));
  EXPECT_FALSE(isNextNone(Colors::Green));
  EXPECT_TRUE(isNextNone(Colors::Blue));
  EXPECT_FALSE(isNextNone(Colors::None));
  EXPECT_FALSE(isNextNone(BadColor)); // bad value
}

} // namespace kanji_tools
