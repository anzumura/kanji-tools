#include <gtest/gtest.h>
#include <kanji_tools/utils/Symbol.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

namespace {

class TestSymbol : public Symbol<TestSymbol> {
public:
  TestSymbol(const std::string& s) : Symbol<TestSymbol>{s} {}
};

} // namespace

template<> const std::string Symbol<TestSymbol>::Type{"Test"};

TEST(SymbolTest, SymbolSizeAndType) {
  EXPECT_EQ(sizeof(TestSymbol), 2);
  EXPECT_EQ(TestSymbol::type(), "Test");
  EXPECT_EQ(TestSymbol::symbols(), 0);
}

TEST(SymbolTest, CreateSymbols) {
  const TestSymbol t1{"t1"}, t2{"t2"};
  EXPECT_EQ(t1.name(), "t1");
  EXPECT_EQ(t1.id(), 0);
  EXPECT_EQ(t2.name(), "t2");
  EXPECT_EQ(t2.id(), 1);
  EXPECT_EQ(TestSymbol::symbols(), 2);
}

TEST(SymbolTest, TooManySymbols) {
  size_t name{};
  while (TestSymbol::symbols() < std::numeric_limits<BaseSymbol::Id>::max())
    TestSymbol{"name-" + std::to_string(++name)};
  EXPECT_THROW(
      call([] { TestSymbol{"foo"}; }, "Test: can't add 'foo' - max capacity"),
      std::domain_error);
}

} // namespace kanji_tools
