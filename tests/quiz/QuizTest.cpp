#include <gtest/gtest.h>
#include <kanji_tools/quiz/Quiz.h>

#include <sstream>

namespace kanji_tools {

TEST(QuizTest, Info) {
  const char* args[]{"", Data::InfoArg.c_str()};
  std::stringstream os;
  Quiz::run(args, os);
  // look for a few strings instead of comparing the whole output
  const auto expected = {">>> Loaded 1460 kanji into 88 groups",
      ">>> Loaded 5700 kanji into 1038 groups",
      ">>> Total Kanji with Jukugo: 2910, unique jukugo: 18490"};
  size_t found{};
  for (String line; std::getline(os, line);)
    for (const auto& i : expected)
      if (line == i) {
        ++found;
        break;
      }
  EXPECT_EQ(found, std::size(expected));
}

TEST(QuizTest, Debug) {
  const char* args[]{"", Data::DebugArg.c_str()};
  std::stringstream os;
  Quiz::run(args, os);
  // look for a few strings instead of comparing the whole output
  const auto expected = {"阿：ア( 3)   阿': 婀# 痾#",
      "時間：十干 (10)   : 甲. 乙. 丙. 丁. 戊^ 己. 庚^ 辛. 壬\" 癸+",
      "畏：ワイ、イ( 3)   畏.: 隈\" 猥#"};
  size_t found{};
  for (String line; std::getline(os, line);)
    for (const auto& i : expected)
      if (line.ends_with(i)) {
        ++found;
        break;
      }
  EXPECT_EQ(found, std::size(expected));
}

} // namespace kanji_tools
