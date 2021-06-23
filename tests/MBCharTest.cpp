#include <gtest/gtest.h>

#include <kanji/MBChar.h>

#include <array>

namespace kanji {

TEST(MBChar, Length) {
  EXPECT_EQ(MBChar("").length(), 0);
  EXPECT_EQ(MBChar::length(nullptr), 0);
  EXPECT_EQ(MBChar("abc").length(), 0);
  EXPECT_EQ(MBChar("abc").length(false), 3);
  EXPECT_EQ(MBChar("大blue空").length(), 2);
  EXPECT_EQ(MBChar("大blue空").length(false), 6);
}

TEST(MBChar, Valid) {
  EXPECT_FALSE(MBChar("").valid());
  EXPECT_FALSE(MBChar::valid(nullptr));
  EXPECT_FALSE(MBChar("a").valid());
  std::string x("雪");
  EXPECT_EQ(x.length(), 3);
  EXPECT_TRUE(MBChar(x).valid());

  // longer strings are not considered valid by default
  EXPECT_FALSE(MBChar("吹雪").valid());
  EXPECT_FALSE(MBChar("猫s").valid());
  EXPECT_FALSE(MBChar("a猫").valid());

  // however, longer strings can be valid if 'checkLengthOne' is false
  EXPECT_TRUE(MBChar("吹雪").valid(false));
  EXPECT_TRUE(MBChar("猫s").valid(false));
  // but the first char must be a multi-byte
  EXPECT_FALSE(MBChar("a猫").valid(false));

  // badly formed strings:
  EXPECT_FALSE(MBChar::valid(x.substr(0, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(0, 2)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 2)));
}

TEST(MBChar, ValidWithTwoByte) {
  std::string x("©");
  EXPECT_EQ(x.length(), 2);
  EXPECT_TRUE(MBChar(x).valid());
  // badly formed strings:
  EXPECT_FALSE(MBChar::valid(x.substr(0, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(1)));
}

TEST(MBChar, ValidWithFourByte) {
  std::string x("𒀄"); // a four byte sumerian cuneiform symbol
  EXPECT_EQ(x.length(), 4);
  EXPECT_TRUE(MBChar(x).valid());
  // badly formed strings:
  EXPECT_FALSE(MBChar::valid(x.substr(0, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(0, 2)));
  EXPECT_FALSE(MBChar::valid(x.substr(0, 3)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 2)));
  EXPECT_FALSE(MBChar::valid(x.substr(1, 3)));
  EXPECT_FALSE(MBChar::valid(x.substr(2, 1)));
  EXPECT_FALSE(MBChar::valid(x.substr(2, 2)));
  EXPECT_FALSE(MBChar::valid(x.substr(3, 1)));
}

TEST(MBChar, NotValidWithFiveByte) {
  std::string x("𒀄");
  EXPECT_EQ(x.length(), 4);
  EXPECT_TRUE(MBChar(x).valid());
  // try to make a 'fake valid' string with 5 bytes (which is not valid)
  x[0] = 0b11'11'10'10;
  EXPECT_EQ(x.length(), 4);
  EXPECT_FALSE(MBChar::valid(x));
  x += x[3];
  EXPECT_EQ(x.length(), 5);
  EXPECT_FALSE(MBChar::valid(x));
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
