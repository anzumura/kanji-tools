#include <gtest/gtest.h>
#include <kanji_tools/kana/MBChar.h>

namespace kanji_tools {

TEST(MBCharTest, Size) {
  EXPECT_EQ(MBChar{""}.size(), 0);
  EXPECT_EQ(MBChar::size(nullptr), 0);
  EXPECT_EQ(MBChar{"abc"}.size(), 0);
  EXPECT_EQ(MBChar{"abc"}.size(false), 3);
  EXPECT_EQ(MBChar{"大blue空"}.size(), 2);
  EXPECT_EQ(MBChar{"大blue空"}.size(false), 6);
  // variation selectors are considered part of the previous character so don't
  // affect 'size'
  auto mbCharWithVariant{U"\u9038\ufe01"};
  auto s{toUtf8(mbCharWithVariant)};
  EXPECT_EQ(s.size(), 6);
  EXPECT_EQ(MBChar::size(s), 1);
  s = MBChar::noVariationSelector(s); // strip off the variation selector
  EXPECT_EQ(s.size(), 3);
  // char is unchanged if it doesn't have a variation selector
  EXPECT_EQ(MBChar::noVariationSelector(s), s);
  // combining marks are not included in 'size'
  const std::string noMarks{"愛詞（あいことば）"}, marks{"愛詞（あいことば）"};
  EXPECT_EQ(noMarks.size(), 27);
  EXPECT_EQ(marks.size(), 30);
  EXPECT_EQ(MBChar::size(noMarks), 9);
  EXPECT_EQ(MBChar::size(marks), 9);
}

TEST(MBCharTest, GetFirst) {
  EXPECT_EQ(MBChar::getFirst(""), "");
  EXPECT_EQ(MBChar::getFirst("abc"), "");
  EXPECT_EQ(MBChar::getFirst("大blue空"), "大");
  // variation selectors are considered part of a character
  auto mbCharWithVariant{U"\u9038\ufe01"};
  auto s{toUtf8(mbCharWithVariant)};
  auto r{MBChar::getFirst(s)};
  EXPECT_EQ(r, s);
}

TEST(MBCharTest, Next) {
  MBChar s{"todayトロントの天気is nice。"};
  std::string x;
  for (const auto _ = {"ト", "ロ", "ン", "ト", "の", "天", "気", "。"};
       auto& i : _) {
    EXPECT_TRUE(s.peek(x));
    EXPECT_EQ(x, i);
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.peek(x));
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, NextWithVariationSelectors) {
  MBChar s{"憎︀憎む朗︀"};
  std::string x;
  for (const auto _ = {"憎︀", "憎", "む", "朗︀"}; auto& i : _) {
    EXPECT_TRUE(s.peek(x));
    EXPECT_EQ(x, i);
    x.clear();
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(s.errors(), 0);
    EXPECT_EQ(x, i);
    x.clear();
  }
  EXPECT_FALSE(s.peek(x));
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, NextWithCombiningMarks) {
  const std::string ga{"ガ"}, gi{"ギ"}, combinedGi{"ギ"}, gu{"グ"}, po{"ポ"},
      combinedPo{"ポ"};
  EXPECT_EQ(combinedGi.size(), 6);
  EXPECT_EQ(combinedPo.size(), 6);
  const auto c{ga + combinedGi + gu + combinedPo};
  EXPECT_EQ(c.size(), 18);
  MBChar s{c};
  std::string x;
  // combining marks should get replaced by normal versions
  for (const auto _ = {ga, gi, gu, po}; auto& i : _) {
    EXPECT_EQ(i.size(), 3);
    EXPECT_TRUE(s.peek(x));
    EXPECT_EQ(x, i);
    x.clear();
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(s.errors(), 0);
    EXPECT_EQ(x, i);
    x.clear();
  }
  EXPECT_FALSE(s.peek(x));
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, GetNextIncludingSingleByte) {
  MBChar s{"a天気b"};
  std::string x;
  for (const auto _ = {"a", "天", "気", "b"}; auto& i : _) {
    EXPECT_TRUE(s.peek(x, false));
    EXPECT_EQ(x, i);
    x.clear();
    EXPECT_TRUE(s.next(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x, false));
}

TEST(MBCharTest, Reset) {
  MBChar s{"a天気b"};
  std::string x;
  const auto expected = {"天", "気"};
  for (auto& i : expected) {
    EXPECT_TRUE(s.peek(x));
    EXPECT_EQ(x, i);
    x.clear();
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  s.reset();
  for (auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, ErrorCount) {
  std::string original{"甲乙丙丁"};
  // there should be 4 '3-byte' characters
  ASSERT_EQ(original.size(), 12);
  // introduce some errors
  original[1] = 'x'; // change middle of 甲 makes 2 errors (first and last byte)
  original[3 * 2] =
      'z'; // change first byte of 丙 makes 2 errors (2nd + 3rd bytes)
  MBChar s{original};
  std::string x;
  for (const auto _ = {"乙", "丁"}; auto& i : _) {
    EXPECT_TRUE(s.peek(x));
    EXPECT_EQ(x, i);
    x.clear();
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  EXPECT_EQ(s.errors(), 4);
  s.reset();
  // make sure 'reset' also clears errors
  EXPECT_EQ(s.errors(), 0);
  // now loop again looking for single byte results as well
  for (const auto _ = {"x", "乙", "z", "丁"}; auto& i : _) {
    EXPECT_TRUE(s.next(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  EXPECT_EQ(s.errors(), 4);
}

TEST(MBCharTest, ErrorWithVariationSelectors) {
  const auto variantSelector{toUtf8(U"\ufe01")};
  // put a variation selector after a single byte char which is invalid
  MBChar s{"a" + variantSelector + "ご"};
  std::string x;
  EXPECT_TRUE(s.next(x, false));
  EXPECT_EQ(x, "a");
  EXPECT_TRUE(s.peek(x));
  EXPECT_EQ(x, "ご");
  EXPECT_EQ(s.errors(), 0); // peek doesn't increment errors
  x.clear();
  EXPECT_TRUE(s.next(x));
  EXPECT_EQ(x, "ご");
  EXPECT_EQ(s.errors(), 1);
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, ErrorWithCombiningMarks) {
  // put combining marks at the start which isn't valid
  MBChar s{CombiningVoiced + CombiningSemiVoiced + "じ"};
  std::string x;
  EXPECT_TRUE(s.peek(x));
  EXPECT_EQ(x, "じ");
  EXPECT_EQ(s.errors(), 0);
  x.clear();
  EXPECT_TRUE(s.next(x));
  EXPECT_EQ(x, "じ");
  EXPECT_EQ(s.errors(), 2); // each combining mark causes a error
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, Valid) {
  EXPECT_EQ(MBChar{""}.valid(), MBUtf8Result::NotMultiByte);
  EXPECT_EQ(MBChar{"a"}.valid(), MBUtf8Result::NotMultiByte);
  std::string x{"雪"};
  EXPECT_EQ(x.size(), 3);
  EXPECT_EQ(MBChar(x).valid(), MBUtf8Result::Valid);
  EXPECT_TRUE(MBChar(x).isValid());

  // longer strings are not considered valid by default
  EXPECT_EQ(MBChar{"吹雪"}.valid(), MBUtf8Result::NotValid);
  EXPECT_EQ(MBChar{"猫s"}.valid(), MBUtf8Result::NotValid);
  EXPECT_EQ(MBChar{"a猫"}.valid(), MBUtf8Result::NotMultiByte);

  // however, longer strings can be valid if 'sizeOne' is false
  EXPECT_TRUE(MBChar{"吹雪"}.isValid(false));
  EXPECT_TRUE(MBChar{"猫s"}.isValid(false));
  // but the first char must be a multi-byte
  EXPECT_FALSE(MBChar{"a猫"}.isValid(false));
}

} // namespace kanji_tools
