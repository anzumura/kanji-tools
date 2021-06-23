#include <gtest/gtest.h>

#include <kanji/MBChar.h>

#include <array>

namespace kanji {

TEST(MBChar, Length) {
  EXPECT_EQ(MBChar("").length(), 0);
  EXPECT_EQ(MBChar("abc").length(), 0);
  EXPECT_EQ(MBChar("abc").length(false), 3);
  EXPECT_EQ(MBChar("大blue空").length(), 2);
  EXPECT_EQ(MBChar("大blue空").length(false), 6);
}

TEST(MBChar, ValidOne) {
  EXPECT_FALSE(MBChar("").validOne());
  EXPECT_FALSE(MBChar("a").validOne());
  std::string x("雪");
  EXPECT_EQ(x.length(), 3);
  EXPECT_TRUE(MBChar(x).validOne());
  EXPECT_FALSE(MBChar("吹雪").validOne());
  EXPECT_FALSE(MBChar("a猫").validOne());
  EXPECT_FALSE(MBChar("猫s").validOne());
}

TEST(MBChar, ValidOneWithTwoByte) {
  std::string x("©");
  EXPECT_EQ(x.length(), 2);
  EXPECT_TRUE(MBChar(x).validOne());
}

TEST(MBChar, ValidOneWithFourByte) {
  std::string x("𒀄"); // a four byte sumerian cuneiform symbol
  EXPECT_EQ(x.length(), 4);
  EXPECT_TRUE(MBChar(x).validOne());
}

TEST(MBChar, GetNext) {
  MBChar s("todayトロントの天気is nice。");
  std::string x;
  std::array expected = {"ト", "ロ", "ン", "ト", "の", "天", "気", "。"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.getNext(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.getNext(x));
}

TEST(MBChar, GetNextIncludingSingleByte) {
  MBChar s("a天気b");
  std::string x;
  std::array expected = {"a", "天", "気", "b"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.getNext(x, false));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.getNext(x, false));
}

TEST(MBChar, Reset) {
  MBChar s("a天気b");
  std::string x;
  std::array expected = {"天", "気"};
  for (const auto& i : expected) {
    EXPECT_TRUE(s.getNext(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.getNext(x));
  s.reset();
  for (const auto& i : expected) {
    EXPECT_TRUE(s.getNext(x));
    EXPECT_EQ(x, i);
  }
  EXPECT_FALSE(s.getNext(x));
}

} // namespace kanji
