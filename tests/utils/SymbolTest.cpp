#include <gtest/gtest.h>
#include <kt_tests/WhatMismatch.h>
#include <kt_utils/Exception.h>
#include <kt_utils/Symbol.h>

namespace kanji_tools {

namespace {

class TestSymbol : public Symbol<TestSymbol> {
public:
  inline static const String Type{"TestSymbol"};
  using Symbol::Symbol;
};

} // namespace

TEST(SymbolTest, SymbolSizeAndType) {
  EXPECT_EQ(sizeof(TestSymbol), 2);
  EXPECT_EQ(TestSymbol::type(), "TestSymbol");
  EXPECT_EQ(TestSymbol::size(), 0);
}

TEST(SymbolTest, CreateSymbols) {
  ASSERT_FALSE(TestSymbol::exists("t1"));
  ASSERT_FALSE(TestSymbol::exists("t2"));
  const auto before{TestSymbol::size()};
  const TestSymbol t1{"t1"}, t2{"t2"};
  EXPECT_EQ(t1.name(), "t1");
  EXPECT_EQ(t1.id(), before + 1);
  EXPECT_EQ(t2.name(), "t2");
  EXPECT_EQ(t2.id(), before + 2);
  EXPECT_EQ(TestSymbol::size(), before + 2);
  EXPECT_TRUE(TestSymbol::exists("t1"));
  EXPECT_TRUE(TestSymbol::exists("t2"));
}

TEST(SymbolTest, CreateDuplicateSymbols) {
  ASSERT_FALSE(TestSymbol::exists("t3"));
  const auto before{TestSymbol::size()};
  const TestSymbol t1{"t3"}, t2{"t3"};
  EXPECT_EQ(t1.name(), "t3");
  EXPECT_EQ(t2.name(), "t3");
  EXPECT_EQ(&t1.name(), &t2.name());
  EXPECT_EQ(t1.id(), t2.id());
  EXPECT_EQ(TestSymbol::size(), before + 1);
}

TEST(SymbolTest, OperatorBool) {
  const TestSymbol nonEmpty{"nonEmpty"}, empty{};
  EXPECT_TRUE(nonEmpty);
  EXPECT_FALSE(empty);
  EXPECT_EQ(empty.name(), emptyString());
  EXPECT_TRUE(TestSymbol::exists(nonEmpty.name()));
  EXPECT_FALSE(TestSymbol::exists(empty.name()));
}

TEST(SymbolTest, OstreamOperator) {
  std::stringstream s;
  const TestSymbol x{"outTest"};
  s << x;
  EXPECT_EQ(s.str(), x.name());
}

TEST(SymbolTest, Equality) {
  const TestSymbol a1{"a1"}, a2{"a2"}, anotherA1{"a1"};
  EXPECT_NE(a1, a2);
  EXPECT_EQ(a1, anotherA1);
}

TEST(SymbolTest, TooManySymbols) {
  size_t name{};
  while (TestSymbol::size() < BaseSymbol::Max)
    TestSymbol{"name-" + std::to_string(++name)};
  const auto before{TestSymbol::size()};
  EXPECT_THROW(call([] { TestSymbol{"foo"}; },
                   "TestSymbol: can't add 'foo' - max capacity"),
      DomainError);
  // make sure nothing new was added
  EXPECT_EQ(TestSymbol::size(), before);
  EXPECT_FALSE(TestSymbol::exists("foo"));
  // allow creating a new symbol with an existing name (so just a lookup)
  const TestSymbol oneMore{"name-1"};
  EXPECT_EQ(oneMore.name(), "name-1");
}

} // namespace kanji_tools
