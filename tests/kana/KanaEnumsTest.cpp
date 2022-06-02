#include <gtest/gtest.h>
#include <kanji_tools/kana/KanaEnums.h>
#include <kanji_tools/kana/Utf8Char.h>

namespace kanji_tools {

using enum CharType;

TEST(CharTypeTest, CheckStrings) {
  EXPECT_EQ(toString(Hiragana), "Hiragana");
  EXPECT_EQ(toString(Katakana), "Katakana");
  EXPECT_EQ(toString(Romaji), "Romaji");
}

TEST(CharTypeTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(CharTypes[i], Hiragana);
  EXPECT_EQ(CharTypes[++i], Katakana);
  EXPECT_EQ(CharTypes[++i], Romaji);
}

} // namespace kanji_tools
