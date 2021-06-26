#include <gtest/gtest.h>

#include <kanji/KanjiData.h>
#include <kanji/Kanji.h>

#include <type_traits>

namespace kanji {

namespace fs = std::filesystem;

class DataTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "../../data";
    static const char* args[] = { arg0, arg1 };
    return args;
  }
  // Contructs KanjiData using the real data files
  DataTest() : _data(2, argv()) {}

  KanjiData _data;
};

TEST_F(DataTest, SanityChecks) {
  // basic checks
  EXPECT_EQ(_data.getLevel("院"), Levels::N4);
  EXPECT_EQ(_data.getFrequency("蝦"), 2501);
  EXPECT_EQ(_data.getStrokes("廳"), 25);
  // radical
  auto radical = _data.getRadical("鹿");
  EXPECT_EQ(radical.number(), 198);
  EXPECT_EQ(radical.name(), "鹿");
  EXPECT_EQ(radical.longName(), "鹿部（ろくぶ）");
  EXPECT_EQ(radical.reading(), "しか");
  // find
  auto result = _data.findKanji("響");
  ASSERT_TRUE(result.has_value());
  auto& k = **result;
  EXPECT_EQ(k.type(), Types::LinkedOld);
  EXPECT_EQ(k.name(), "響");
  EXPECT_EQ(k.level(), Levels::None);
  EXPECT_EQ(k.grade(), Grades::None);
  EXPECT_EQ(k.frequency(), 0);
  // MB character types
  EXPECT_TRUE(_data.isHiragana("ゑ"));
  EXPECT_FALSE(_data.isKatakana("ゑ"));
  EXPECT_TRUE(_data.isKatakana("ヰ"));
  EXPECT_FALSE(_data.isHiragana("ヰ"));  
  EXPECT_TRUE(_data.isFullWidthKana("ー"));
  EXPECT_TRUE(_data.isFullWidthKana("さ"));
  EXPECT_FALSE(_data.isHalfWidthKana("ー"));
  EXPECT_FALSE(_data.isHalfWidthKana("さ"));
  EXPECT_FALSE(_data.isFullWidthKana("ｶ"));
  EXPECT_TRUE(_data.isHalfWidthKana("ｶ"));
  EXPECT_TRUE(_data.isKana("こ"));
  EXPECT_TRUE(_data.isKana("コ"));
  EXPECT_TRUE(_data.isKana("ｺ"));
  EXPECT_FALSE(_data.isKana("。"));
  EXPECT_TRUE(_data.isWidePunctuation("。"));
  EXPECT_TRUE(_data.isWidePunctuation("、"));
}

} // namespace kanji
