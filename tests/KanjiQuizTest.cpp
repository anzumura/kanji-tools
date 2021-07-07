#include <gtest/gtest.h>

#include <kanji/Kanji.h>
#include <kanji/KanjiQuiz.h>

#include <sstream>

namespace kanji {

class KanjiQuizTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "../../data";
    static const char* args[] = {arg0, arg1};
    return args;
  }
  // Contructs KanjiData using the real data files
  KanjiQuizTest() : _quiz(2, argv(), _os, _es, _is) {}

  void startGradeQuiz() {
    _is << "g\n1\nb\n4\nk\n";
    // the above statement passes a string to '_is' so that '_quiz' can read the follow options:
    // 'g' for Grade Quiz
    // '1' for Grade 1
    // '4' for 4 choices
    // 'k' for kanji to reading quiz
  }
  void quit() { _is << "/\n"; } // '/' is the option to quit
  void skip() { _is << ".\n"; } // '.' is the option to skip

  std::stringstream _os;
  std::stringstream _es;
  std::stringstream _is;
  KanjiQuiz _quiz;
};

TEST_F(KanjiQuizTest, GroupsLoaded) {
  EXPECT_FALSE(_quiz.meaningGroupList().empty());
  EXPECT_FALSE(_quiz.patternGroupList().empty());
}

TEST_F(KanjiQuizTest, StartGradeQuiz) {
  startGradeQuiz();
  quit();
  _quiz.quiz();
  std::string line, lastLine;
  while (std::getline(_os, line))
    lastLine = line;
  // test the line sent to _os
  EXPECT_EQ(lastLine, "Final score: 0/0");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(KanjiQuizTest, SkipQuestions) {
  for (int i = 2; i < 4; ++i) {
    startGradeQuiz();
    for (int j = 0; j < i; ++j)
      skip();
    quit();
    _quiz.quiz();
    std::string line, lastLine;
    while (std::getline(_os, line))
      lastLine = line;
    auto expected = "Final score: 0/" + std::to_string(i) + ", skipped: " + std::to_string(i);
    EXPECT_EQ(lastLine, expected);
    // need to clear eofbit and failbit (before looping again)
    EXPECT_TRUE(_os.eof() && _os.fail());
    EXPECT_FALSE(_os.good() || _os.bad());
    _os.clear();
    EXPECT_TRUE(_os.good());
    EXPECT_FALSE(_os.eof() || _os.fail() || _os.bad());
  }
}

} // namespace kanji
