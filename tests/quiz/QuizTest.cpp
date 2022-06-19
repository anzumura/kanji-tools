#include <gtest/gtest.h>
#include <kt_quiz/Quiz.h>
#include <kt_tests/Utils.h>

#include <sstream>

namespace kanji_tools {

TEST(QuizTest, Info) {
  const char* args[]{"", KanjiData::InfoArg.c_str()};
  std::stringstream os;
  Quiz::run(args, os);
  // look for a few strings instead of comparing the whole output
  const auto expected = {">>> Loaded 1460 kanji into 88 groups",
      ">>> Loaded 5703 kanji into 1038 groups",
      ">>> Total Kanji with Jukugo: 2910, unique jukugo: 18490"};
  EXPECT_EQ(findEqualMatches(os, expected), std::nullopt);
}

TEST(QuizTest, Debug) {
  const char* args[]{"", KanjiData::DebugArg.c_str()};
  std::stringstream os;
  Quiz::run(args, os);
  // look for a few strings instead of comparing the whole output
  const auto expected = {"団体　 (5 )   : 団. 社. 派. 組. 群.", // short name
      "時間：十干 (10)   : 甲. 乙. 丙. 丁. 戊^ 己. 庚^ 辛. 壬\" 癸+", // 'meaning'
      "阿：ア( 3)   阿': 婀# 痾#",         // 'pattern' with parent
      "  ：ジュン( 3)   　 : 準. 准. 隼'", // 'pattern' with no parent
      "畏：ワイ、イ( 3)   畏.: 隈\" 猥#"}; // 'pattern' with multiple readings
  EXPECT_EQ(findEndMatches(os, expected), std::nullopt);
}

} // namespace kanji_tools
