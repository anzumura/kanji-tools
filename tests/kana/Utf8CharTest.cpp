#include <gtest/gtest.h>
#include <kt_kana/Utf8Char.h>

namespace kanji_tools {

TEST(Utf8CharTest, Size) {
  EXPECT_EQ(Utf8Char{""}.size(), 0);
  EXPECT_EQ(Utf8Char::size(nullptr), 0);
  EXPECT_EQ(Utf8Char{"abc"}.size(), 0);
  EXPECT_EQ(Utf8Char{"abc"}.size(false), 3);
  EXPECT_EQ(Utf8Char{"大blue空"}.size(), 2);
  EXPECT_EQ(Utf8Char{"大blue空"}.size(false), 6);
  // variation selectors are considered part of the previous character so don't
  // affect 'size'
  auto Utf8CharWithVariant{U"\u9038\ufe01"};
  auto s{toUtf8(Utf8CharWithVariant)};
  EXPECT_EQ(s.size(), 6);
  EXPECT_EQ(Utf8Char::size(s), 1);
  s = Utf8Char::noVariationSelector(s); // strip off the variation selector
  EXPECT_EQ(s.size(), 3);
  // char is unchanged if it doesn't have a variation selector
  EXPECT_EQ(Utf8Char::noVariationSelector(s), s);
  // combining marks are not included in 'size'
  const String noMarks{"愛詞（あいことば）"}, marks{"愛詞（あいことば）"};
  EXPECT_EQ(noMarks.size(), 27);
  EXPECT_EQ(marks.size(), 30);
  EXPECT_EQ(Utf8Char::size(noMarks), 9);
  EXPECT_EQ(Utf8Char::size(marks), 9);
}

TEST(Utf8CharTest, GetFirst) {
  EXPECT_EQ(Utf8Char::getFirst(""), "");
  EXPECT_EQ(Utf8Char::getFirst("abc"), "");
  EXPECT_EQ(Utf8Char::getFirst("大blue空"), "大");
  // variation selectors are considered part of a character
  auto Utf8CharWithVariant{U"\u9038\ufe01"};
  auto s{toUtf8(Utf8CharWithVariant)};
  auto r{Utf8Char::getFirst(s)};
  EXPECT_EQ(r, s);
}

TEST(Utf8CharTest, Next) {
  Utf8Char s{"todayトロントの天気is nice。"};
  String x;
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

TEST(Utf8CharTest, NextWithVariationSelectors) {
  Utf8Char s{"憎︀憎む朗︀"};
  String x;
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

TEST(Utf8CharTest, NextWithCombiningMarks) {
  const String ga{"ガ"}, gi{"ギ"}, combinedGi{"ギ"}, gu{"グ"}, po{"ポ"},
      combinedPo{"ポ"};
  EXPECT_EQ(combinedGi.size(), 6);
  EXPECT_EQ(combinedPo.size(), 6);
  const auto c{ga + combinedGi + gu + combinedPo};
  EXPECT_EQ(c.size(), 18);
  Utf8Char s{c};
  String x;
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

TEST(Utf8CharTest, GetNextIncludingSingleByte) {
  Utf8Char s{"a天気b"};
  String x;
  for (const auto _ = {"a", "天", "気", "b"}; auto& i : _) {
    EXPECT_TRUE(s.peek(x, false));
    EXPECT_EQ(x, i);
    x.clear();
    EXPECT_TRUE(s.next(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x, false));
}

TEST(Utf8CharTest, Reset) {
  Utf8Char s{"a天気b"};
  String x;
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

TEST(Utf8CharTest, ErrorCount) {
  String original{"甲乙丙丁"};
  // there should be 4 '3-byte' characters
  ASSERT_EQ(original.size(), 12);
  // introduce some errors
  original[1] = 'x'; // change middle of 甲 makes 2 errors (first and last byte)
  original[original.size() / 2] =
      'z'; // change first byte of 丙 makes 2 errors (2nd + 3rd bytes)
  Utf8Char s{original};
  String x;
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

TEST(Utf8CharTest, ErrorWithVariationSelectors) {
  const auto variantSelector{toUtf8(U"\ufe01")};
  // put a variation selector after a single byte char which is invalid
  Utf8Char s{"a" + variantSelector + "ご"};
  String x;
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

TEST(Utf8CharTest, ErrorWithCombiningMarks) {
  // put combining marks at the start which isn't valid
  Utf8Char s{(String{CombiningVoiced} += CombiningSemiVoiced) += "じ"};
  String x;
  EXPECT_TRUE(s.peek(x));
  EXPECT_EQ(x, "じ");
  EXPECT_EQ(s.errors(), 0);
  x.clear();
  EXPECT_TRUE(s.next(x));
  EXPECT_EQ(x, "じ");
  EXPECT_EQ(s.errors(), 2); // each combining mark causes a error
  EXPECT_FALSE(s.next(x));
}

TEST(Utf8CharTest, Valid) {
  EXPECT_EQ(Utf8Char{""}.valid(), MBUtf8Result::NotMultiByte);
  EXPECT_EQ(Utf8Char{"a"}.valid(), MBUtf8Result::NotMultiByte);
  const String x{"雪"};
  EXPECT_EQ(x.size(), 3);
  EXPECT_EQ(Utf8Char(x).valid(), MBUtf8Result::Valid);
  EXPECT_TRUE(Utf8Char(x).isValid());

  // longer strings are not considered valid by default
  EXPECT_EQ(Utf8Char{"吹雪"}.valid(), MBUtf8Result::NotValid);
  EXPECT_EQ(Utf8Char{"猫s"}.valid(), MBUtf8Result::NotValid);
  EXPECT_EQ(Utf8Char{"a猫"}.valid(), MBUtf8Result::NotMultiByte);

  // however, longer strings can be valid if 'sizeOne' is false
  EXPECT_TRUE(Utf8Char{"吹雪"}.isValid(false));
  EXPECT_TRUE(Utf8Char{"猫s"}.isValid(false));
  // but the first char must be a multi-byte
  EXPECT_FALSE(Utf8Char{"a猫"}.isValid(false));
}

} // namespace kanji_tools
