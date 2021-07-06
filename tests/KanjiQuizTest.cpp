#include <gtest/gtest.h>

#include <kanji/KanjiQuiz.h>
#include <kanji/Kanji.h>

namespace kanji {

class KanjiQuizTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "../../data";
    static const char* args[] = { arg0, arg1 };
    return args;
  }
  // Contructs KanjiData using the real data files
  KanjiQuizTest() : _quiz(2, argv()) {}

  KanjiQuiz _quiz;
};

TEST_F(KanjiQuizTest, GroupsLoaded) {
  EXPECT_FALSE(_quiz.meaningGroupList().empty());
  EXPECT_FALSE(_quiz.patternGroupList().empty());
}

} // namespace kanji
