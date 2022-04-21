#include <gtest/gtest.h>
#include <kanji_tools/kanji/UcdLinkTypes.h>

namespace kanji_tools {

TEST(UcdLinkTypesTest, CheckStrings) {
  EXPECT_EQ(toString(UcdLinkTypes::Compatibility_R), "Compatibility*");
  EXPECT_EQ(toString(UcdLinkTypes::Definition_R), "Definition*");
  EXPECT_EQ(toString(UcdLinkTypes::Jinmei_R), "Jinmei*");
  EXPECT_EQ(toString(UcdLinkTypes::Semantic_R), "Semantic*");
  EXPECT_EQ(toString(UcdLinkTypes::Simplified_R), "Simplified*");
  EXPECT_EQ(toString(UcdLinkTypes::Traditional_R), "Traditional*");
  EXPECT_EQ(toString(UcdLinkTypes::Compatibility), "Compatibility");
  EXPECT_EQ(toString(UcdLinkTypes::Definition), "Definition");
  EXPECT_EQ(toString(UcdLinkTypes::Jinmei), "Jinmei");
  EXPECT_EQ(toString(UcdLinkTypes::Simplified), "Simplified");
  EXPECT_EQ(toString(UcdLinkTypes::Traditional), "Traditional");
  EXPECT_EQ(toString(UcdLinkTypes::None), "None");
}

TEST(UcdLinkTypesTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(AllUcdLinkTypes[i], UcdLinkTypes::Compatibility_R);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Definition_R);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Jinmei_R);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Semantic_R);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Simplified_R);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Traditional_R);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Compatibility);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Definition);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Jinmei);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Simplified);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::Traditional);
  EXPECT_EQ(AllUcdLinkTypes[++i], UcdLinkTypes::None);
}

} // namespace kanji_tools
