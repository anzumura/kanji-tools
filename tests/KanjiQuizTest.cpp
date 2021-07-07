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
  // Contructs KanjiQuiz using the real data files
  KanjiQuizTest() : _quiz(2, argv(), _os, _es, _is) {}

  void gradeListQuiz() {
    // Send a string to '_is' so that '_quiz' can read the follow options:
    // 'g' for Grade List Quiz
    // 'b' for Beginning of list (instead of End or Random)
    // '1' for Grade 1
    // '4' for 4 choices
    // 'k' for kanji to reading quiz
    _is << "g\n1\nb\n4\nk\n";
  }
  void meaningGroupQuiz() {
    // Send a string to '_is' so that '_quiz' can read the follow options:
    // 'm' for Meaning Group Quiz
    // 'b' for Beginning of list (instead of End or Random)
    // '1' for including only Jōyō kanji
    _is << "m\nb\n1\n";
  }
  void skip() { _is << ".\n"; }           // '.' is the option to skip
  void toggleMeanings() { _is << "-\n"; } // '-' is the option to toggle meanings
  void runQuiz() {
    // clear eofbit and failbit for output streams in case quiz is run more than once during a test
    _os.clear();
    _es.clear();
    // final input needs to be '/' to 'quit' the quiz, otherwise test code will hang while quiz
    // is waiting for more input.
    _is << "/\n";
    _quiz.quiz();
  }

  std::stringstream _os;
  std::stringstream _es;
  std::stringstream _is;
  KanjiQuiz _quiz;
};

TEST_F(KanjiQuizTest, GroupsLoaded) {
  // Groups are actually loaded by KanjiGroupData which is a base class of KanjiQuiz
  EXPECT_FALSE(_quiz.meaningGroupList().empty());
  EXPECT_FALSE(_quiz.patternGroupList().empty());
}

TEST_F(KanjiQuizTest, ListQuiz) {
  gradeListQuiz();
  runQuiz();
  std::string line, lastLine;
  while (std::getline(_os, line))
    lastLine = line;
  // test the line sent to _os
  EXPECT_EQ(lastLine, "Final score: 0/0");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(KanjiQuizTest, SkipListQuestions) {
  for (int i = 2; i < 4; ++i) {
    gradeListQuiz();
    for (int j = 0; j < i; ++j)
      skip();
    runQuiz();
    // make sure _os is in expected 'good' state
    EXPECT_TRUE(_os.good());
    EXPECT_FALSE(_os.eof() || _os.fail() || _os.bad());
    std::string line, lastLine;
    while (std::getline(_os, line))
      lastLine = line;
    // make sure _os is in expected 'eof' state
    EXPECT_TRUE(_os.eof() && _os.fail());
    EXPECT_FALSE(_os.good() || _os.bad());

    auto expected = "Final score: 0/" + std::to_string(i) + ", skipped: " + std::to_string(i);
    EXPECT_EQ(lastLine, expected);
  }
}

TEST_F(KanjiQuizTest, ToggleListMeanings) {
  gradeListQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  runQuiz();
  std::string line;
  bool meaningsOn = false;
  int found = 0;
  std::string expected("Question 1/80.  Kanji:  一  (Rad 一, Strokes 1, Level N5, Freq 2)");
  while (std::getline(_os, line)) {
    if (line.starts_with("Question")) {
      ++found;
      EXPECT_EQ(line, expected + (meaningsOn ? " : one" : ""));
      meaningsOn = !meaningsOn;
    }
  }
  // We want to find the Question string 3 times, i.e., once without meanings, then again with a meaning
  // when meanings are toggled on and then again without a meaning when meanings are toggled off.
  EXPECT_EQ(found, 3);
}

TEST_F(KanjiQuizTest, GroupQuiz) {
  meaningGroupQuiz();
  runQuiz();
  std::string line, lastLine;
  while (std::getline(_os, line))
    lastLine = line;
  // test the line sent to _os
  EXPECT_EQ(lastLine, "Final score: 0/0");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(KanjiQuizTest, SkipGroupQuestions) {
  for (int i = 2; i < 4; ++i) {
    meaningGroupQuiz();
    for (int j = 0; j < i; ++j)
      skip();
    runQuiz();
    std::string line, lastLine;
    while (std::getline(_os, line))
      lastLine = line;
    auto expected = "Final score: 0/" + std::to_string(i) + ", skipped: " + std::to_string(i);
    EXPECT_EQ(lastLine, expected);
  }
}

TEST_F(KanjiQuizTest, ToggleGroupMeanings) {
  meaningGroupQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  runQuiz();
  bool meaningsOn = false;
  int found = 0;
  std::string line, expected("リュウ、たつ");
  std::string expectedWithMeaning = expected + " : dragon";
  while (std::getline(_os, line)) {
    if (line.starts_with("  Entry") && line.ends_with(meaningsOn ? expectedWithMeaning : expected)) {
      ++found;
      meaningsOn = !meaningsOn;
    }
  }
  // We want to find the Question string 3 times, i.e., once without meanings, then again with a meaning
  // when meanings are toggled on and then again without a meaning when meanings are toggled off.
  EXPECT_EQ(found, 3);
}

} // namespace kanji
