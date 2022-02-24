#include <gtest/gtest.h>
#include <kanji_tools/kana/MBChar.h>

namespace kanji_tools {

TEST(MBCharTest, Length) {
  EXPECT_EQ(MBChar("").length(), 0);
  EXPECT_EQ(MBChar::length(nullptr), 0);
  EXPECT_EQ(MBChar("abc").length(), 0);
  EXPECT_EQ(MBChar("abc").length(false), 3);
  EXPECT_EQ(MBChar("大blue空").length(), 2);
  EXPECT_EQ(MBChar("大blue空").length(false), 6);
  // variation selectors are considered part of the previous character so don't
  // affect length
  auto mbCharWithVariant = U"\u9038\ufe01";
  auto s = toUtf8(mbCharWithVariant);
  EXPECT_EQ(s.length(), 6);
  EXPECT_EQ(MBChar::length(s), 1);
  // combining marks are not included in length
  std::string noMarks("愛詞（あいことば）");
  std::string marks("愛詞（あいことば）");
  EXPECT_EQ(noMarks.length(), 27);
  EXPECT_EQ(marks.length(), 30);
  EXPECT_EQ(MBChar::length(noMarks), 9);
  EXPECT_EQ(MBChar::length(marks), 9);
}

TEST(MBCharTest, GetFirst) {
  EXPECT_EQ(MBChar::getFirst(""), "");
  EXPECT_EQ(MBChar::getFirst("abc"), "");
  EXPECT_EQ(MBChar::getFirst("大blue空"), "大");
  // variation selectors are considered part of a character
  auto mbCharWithVariant = U"\u9038\ufe01";
  auto s = toUtf8(mbCharWithVariant);
  auto r = MBChar::getFirst(s);
  EXPECT_EQ(r, s);
}

TEST(MBCharTest, Next) {
  MBChar s("todayトロントの天気is nice。");
  std::string x;
  for (const std::array _ = {"ト", "ロ", "ン", "ト", "の", "天", "気", "。"};
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
  MBChar s("憎︀憎む朗︀");
  std::string x;
  for (const std::array expected = {"憎︀", "憎", "む", "朗︀"};
       auto& i : expected) {
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
  std::string ga("ガ"), gi("ギ"), combinedGi("ギ"), gu("グ"), po("ポ"),
    combinedPo("ポ");
  EXPECT_EQ(combinedGi.length(), 6);
  EXPECT_EQ(combinedPo.length(), 6);
  const std::string c = ga + combinedGi + gu + combinedPo;
  EXPECT_EQ(c.length(), 18);
  MBChar s(c);
  std::string x;
  // combining marks ashould get replaced by normal versions
  for (const std::array expected = {ga, gi, gu, po}; auto& i : expected) {
    EXPECT_EQ(i.length(), 3);
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
  MBChar s("a天気b");
  std::string x;
  std::array expected = {"a", "天", "気", "b"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x, false));
}

TEST(MBCharTest, Reset) {
  MBChar s("a天気b");
  std::string x;
  std::array expected = {"天", "気"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  s.reset();
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
}

TEST(MBCharTest, ErrorCount) {
  std::string original("甲乙丙丁");
  // there should be 4 '3-byte' characters
  ASSERT_EQ(original.length(), 12);
  // introduce some errors
  original[1] = 'x'; // change middle of 甲 makes 2 errors (first and last byte)
  original[6] = 'z'; // change first byte of 丙 makes 2 errors (2nd + 3rd bytes)
  MBChar s(original);
  std::string x;
  std::array expected = {"乙", "丁"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.next(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  EXPECT_EQ(s.errors(), 4);
  s.reset();
  // make sure 'reset' also clears errors
  EXPECT_EQ(s.errors(), 0);
  // now loop again looking for single byte results as well
  std::array expectedWithSingle = {"x", "乙", "z", "丁"};
  for (const auto& i : expectedWithSingle) {
    EXPECT_TRUE(s.next(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.next(x));
  EXPECT_EQ(s.errors(), 4);
}

TEST(MBCharTest, Valid) {
  EXPECT_EQ(MBChar("").valid(), MBUtf8Result::NotMBUtf8);
  EXPECT_EQ(MBChar("a").valid(), MBUtf8Result::NotMBUtf8);
  std::string x("雪");
  EXPECT_EQ(x.length(), 3);
  EXPECT_EQ(MBChar(x).valid(), MBUtf8Result::Valid);
  EXPECT_TRUE(MBChar(x).isValid());

  // longer strings are not considered valid by default
  EXPECT_EQ(MBChar("吹雪").valid(), MBUtf8Result::StringTooLong);
  EXPECT_EQ(MBChar("猫s").valid(), MBUtf8Result::StringTooLong);
  EXPECT_EQ(MBChar("a猫").valid(), MBUtf8Result::NotMBUtf8);

  // however, longer strings can be valid if 'sizeOne' is false
  EXPECT_TRUE(MBChar("吹雪").isValid(false));
  EXPECT_TRUE(MBChar("猫s").isValid(false));
  // but the first char must be a multi-byte
  EXPECT_FALSE(MBChar("a猫").isValid(false));
}

} // namespace kanji_tools
