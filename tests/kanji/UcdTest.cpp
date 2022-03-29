#include <gtest/gtest.h>
#include <kanji_tools/kanji/Ucd.h>

namespace kanji_tools {

TEST(UcdTest, CodeAndName) {
  const Ucd ucd{u'\x5b66', "学", "CJK", "1.1", 39, 8, 0, "xué", "6974", "1271",
      "GHJKT", "J0-3358", true, false, {}, UcdLinkTypes::None, false,
      "learning, knowledge; school", "GAKU", "MANABU"};
  EXPECT_EQ(ucd.code(), u'\x5b66');
  EXPECT_EQ(ucd.block(), "CJK");
  EXPECT_EQ(ucd.version(), "1.1");
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::None);
  EXPECT_EQ(ucd.codeAndName(), "[5B66] 学");
}

TEST(UcdLinkTypesTest, LinkCodeAndNames) {
  const Ucd ucd{u'\x5b78', "學", "CJK", "1.1", 39, 16, 0, "xué", "7033", "1275",
      "GHJKTV", "J0-555C", false, false, {{u'\x5b66', "学"}},
      UcdLinkTypes::Simplified, false, "learning, knowledge; school", "GAKU",
      "MANABU"};
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::Simplified);
  EXPECT_EQ(ucd.linkCodeAndNames(), "[5B66] 学");
}

TEST(UcdLinkTypesTest, MultipleLinkCodeAndNames) {
  const Ucd ucd{u'\x5e76', "并", "CJK", "1.1", 51, 6, 0, "bìng", "9170", "580",
      "GHJKT", "J0-5675", false, false, {{u'\x4e26', "並"}, {u'\x4f75', "併"}},
      UcdLinkTypes::Traditional, false, "combine, annex; also, what's more",
      "HEI HYOU", "AWASERU NARABU"};
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::Traditional);
  EXPECT_EQ(ucd.linkCodeAndNames(), "[4E26] 並, [4F75] 併");
}

} // namespace kanji_tools
