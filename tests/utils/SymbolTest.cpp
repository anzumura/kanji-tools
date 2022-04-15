#include <gtest/gtest.h>
#include <kanji_tools/utils/Symbol.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

namespace {

class TestSymbol : public Symbol<TestSymbol> {
public:
  inline static const std::string Type{"TestSymbol"};
  TestSymbol(const std::string& name) : Symbol<TestSymbol>{name} {}
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
  EXPECT_EQ(t1.id(), 0);
  EXPECT_EQ(t2.name(), "t2");
  EXPECT_EQ(t2.id(), 1);
  EXPECT_EQ(TestSymbol::size(), 2);
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

TEST(SymbolTest, TooManySymbols) {
  size_t name{};
  while (TestSymbol::size() < std::numeric_limits<BaseSymbol::Id>::max())
    TestSymbol{"name-" + std::to_string(++name)};
  const auto before{TestSymbol::size()};
  EXPECT_THROW(call([] { TestSymbol{"foo"}; },
                   "TestSymbol: can't add 'foo' - max capacity"),
      std::domain_error);
  // make sure nothing new was added
  EXPECT_EQ(TestSymbol::size(), before);
  EXPECT_FALSE(TestSymbol::exists("foo"));
}

} // namespace kanji_tools
