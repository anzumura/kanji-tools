#include <gtest/gtest.h>

#include <kanji/MBChar.h>

#include <array>
#include <fstream>

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

namespace fs = std::filesystem;

class MBCharCountTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (fs::exists(_testDir)) TearDown();
    EXPECT_TRUE(fs::create_directories(_testSubDir));
    std::array files = {std::make_pair(_testFile1, "北海道"), std::make_pair(_testFile2, "南北"),
                        std::make_pair(_testSubFile1, "東西線"), std::make_pair(_testSubFile2, "東北")};
    for (auto& i : files) {
      std::ofstream of(i.first);
      of << i.second;
      of.close();
    }
  }
  void TearDown() override { fs::remove_all(_testDir); }
  MBCharCount c;
  fs::path _testDir = "testDir";
  fs::path _testFile1 = _testDir / "testFile1";
  fs::path _testFile2 = _testDir / "testFile2";
  fs::path _testSubDir = _testDir / "testSubDir";
  fs::path _testSubFile1 = _testSubDir / "testSubFile1";
  fs::path _testSubFile2 = _testSubDir / "testSubFile2";
};

TEST_F(MBCharCountTest, Add) {
  EXPECT_EQ(c.add("hello空は青い"), 4);
  EXPECT_EQ(c.add("箱は空です"), 5);
  EXPECT_EQ(c.add("今日は涼しい。good bye"), 7);
  // map only includes MB chars
  EXPECT_EQ(c.uniqueEntries(), 12);
  EXPECT_EQ(c.count("空"), 2);
  EXPECT_EQ(c.count("は"), 3);
  EXPECT_EQ(c.count("青"), 1);
  EXPECT_EQ(c.count("い"), 2);
  EXPECT_EQ(c.count("箱"), 1);
  EXPECT_EQ(c.count("で"), 1);
  EXPECT_EQ(c.count("す"), 1);
  EXPECT_EQ(c.count("今"), 1);
  EXPECT_EQ(c.count("日"), 1);
  EXPECT_EQ(c.count("涼"), 1);
  EXPECT_EQ(c.count("し"), 1);
  EXPECT_EQ(c.count("。"), 1);
}

TEST_F(MBCharCountTest, AddFile) {
  EXPECT_EQ(c.addFile(_testFile1), 3);
  EXPECT_EQ(c.uniqueEntries(), 3);
  EXPECT_EQ(c.count("北"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
}

TEST_F(MBCharCountTest, AddDirectoryNoRecurse) {
  EXPECT_EQ(c.addFile(_testDir, false), 5);
  EXPECT_EQ(c.uniqueEntries(), 4);
  EXPECT_EQ(c.count("北"), 2);
  EXPECT_EQ(c.count("南"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("道"), 1);
}

TEST_F(MBCharCountTest, AddDirectoryRecurse) {
  EXPECT_EQ(c.addFile(_testDir), 10);
  EXPECT_EQ(c.uniqueEntries(), 7);
  EXPECT_EQ(c.count("北"), 3);
  EXPECT_EQ(c.count("東"), 2);
  EXPECT_EQ(c.count("南"), 1);
  EXPECT_EQ(c.count("海"), 1);
  EXPECT_EQ(c.count("西"), 1);
  EXPECT_EQ(c.count("道"), 1);
  EXPECT_EQ(c.count("線"), 1);
}

} // namespace kanji
