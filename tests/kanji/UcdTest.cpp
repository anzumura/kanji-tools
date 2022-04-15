#include <gtest/gtest.h>
#include <tests/kanji_tools/TestUcd.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

TEST(UcdTest, Size) {
#ifdef __clang__
  EXPECT_EQ(sizeof(Ucd), 232);
  EXPECT_EQ(sizeof(UcdLinks), 32);
  EXPECT_EQ(sizeof(UcdLinks::Links), 24);
  EXPECT_EQ(sizeof(UcdEntry), 32);
  EXPECT_EQ(sizeof(UcdLinkTypes), 4);
  EXPECT_EQ(sizeof(std::string), 24);
#else
  EXPECT_EQ(sizeof(Ucd), 288);
  EXPECT_EQ(sizeof(UcdLinks), 32);
  EXPECT_EQ(sizeof(UcdLinks::Links), 24);
  EXPECT_EQ(sizeof(UcdEntry), 40);
  EXPECT_EQ(sizeof(UcdLinkTypes), 4);
  EXPECT_EQ(sizeof(std::string), 32);
#endif
}

TEST(UcdTest, SourcesTooLong) {
  const std::string s{"GHJKHJK"};
  EXPECT_THROW(call([&s] { return Ucd{TestUcd{}.sources(s)}; },
                   "sources '" + s + "' exceeds max size"),
      std::domain_error);
}

TEST(UcdTest, SourcesHasDuplicate) {
  const std::string s{"GHH"};
  EXPECT_THROW(call([&s] { return Ucd{TestUcd{}.sources(s)}; },
                   "sources '" + s + "' has duplicate value: H"),
      std::domain_error);
}

TEST(UcdTest, SourcesUnrecognized) {
  const std::string s{"JKL"};
  EXPECT_THROW(call([&s] { return Ucd{TestUcd{}.sources(s)}; },
                   "sources '" + s + "' has unrecognized value: L"),
      std::domain_error);
}

TEST(UcdTest, SetSources) {
  const Ucd noSources{TestUcd{}};
  EXPECT_EQ(noSources.sources(), "");
  EXPECT_FALSE(noSources.joyo());
  EXPECT_FALSE(noSources.jinmei());
  for (auto i : {std::pair{false, false}, std::pair{false, true},
           std::pair{true, false}, std::pair{true, true}}) {
    const Ucd ucd{TestUcd{}.sources("VTKJHG").joyo(i.first).jinmei(i.second)};
    EXPECT_EQ(ucd.sources(), "GHJKTV"); // sources are returned in alpha order
    EXPECT_EQ(ucd.joyo(), i.first);
    EXPECT_EQ(ucd.jinmei(), i.second);
  }
}

TEST(UcdTest, CodeAndName) {
  const Ucd ucd{TestUcd{"学"}.code(u'\x5b66').block("CJK").version("1.1")};
  EXPECT_EQ(ucd.code(), u'\x5b66');
  EXPECT_EQ(ucd.block(), "CJK");
  EXPECT_EQ(ucd.version(), "1.1");
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::None);
  EXPECT_EQ(ucd.codeAndName(), "[5B66] 学");
}

TEST(UcdTest, LinkCodeAndNames) {
  const Ucd ucd{
      TestUcd{"學"}.links({{u'\x5b66', "学"}}, UcdLinkTypes::Simplified)};
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::Simplified);
  EXPECT_EQ(ucd.linkCodeAndNames(), "[5B66] 学");
}

TEST(UcdTest, MultipleLinkCodeAndNames) {
  const Ucd ucd{TestUcd("并").links(
      {{u'\x4e26', "並"}, {u'\x4f75', "併"}}, UcdLinkTypes::Traditional)};
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::Traditional);
  EXPECT_EQ(ucd.linkCodeAndNames(), "[4E26] 並, [4F75] 併");
}

} // namespace kanji_tools
