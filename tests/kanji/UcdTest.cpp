#include <gtest/gtest.h>
#include <kanji_tools/kanji/Ucd.h>
#include <tests/kanji_tools/TestUcd.h>

namespace kanji_tools {

TEST(UcdTest, CodeAndName) {
  const Ucd ucd{TestUcd{"学"}.code(u'\x5b66').block("CJK").version("1.1")};
  EXPECT_EQ(ucd.code(), u'\x5b66');
  EXPECT_EQ(ucd.block(), "CJK");
  EXPECT_EQ(ucd.version(), "1.1");
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::None);
  EXPECT_EQ(ucd.codeAndName(), "[5B66] 学");
}

TEST(UcdLinkTypesTest, LinkCodeAndNames) {
  const Ucd ucd{
      TestUcd{"學"}.links({{u'\x5b66', "学"}}, UcdLinkTypes::Simplified)};
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::Simplified);
  EXPECT_EQ(ucd.linkCodeAndNames(), "[5B66] 学");
}

TEST(UcdLinkTypesTest, MultipleLinkCodeAndNames) {
  const Ucd ucd{TestUcd("并").links(
      {{u'\x4e26', "並"}, {u'\x4f75', "併"}}, UcdLinkTypes::Traditional)};
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::Traditional);
  EXPECT_EQ(ucd.linkCodeAndNames(), "[4E26] 並, [4F75] 併");
}

} // namespace kanji_tools
