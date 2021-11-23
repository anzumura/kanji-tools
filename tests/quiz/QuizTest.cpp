#include <gtest/gtest.h>

#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/Quiz.h>

#include <sstream>

namespace kanji_tools {

class QuizTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "-data";
    static const char* arg2 = "../../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }
  // Contructs Quiz using the real data files
  QuizTest() : _data(std::make_shared<KanjiData>(3, argv(), _os, _es)), _quiz(3, argv(), _data, &_is) {}

  // Populate '_is' as input for '_quiz'
  void gradeListQuiz() {
    // 't' for 'test' mode (instead of review mode)
    // 'g' for Grade List Quiz
    // '1' for Grade 1
    // 'b' for Beginning of list (instead of End or Random)
    // '4' for 4 choices
    // 'k' for kanji to reading quiz
    _is << "t\ng\n1\nb\n4\nk\n";
  }

  std::string listQuizFirstQuestion(char quizType, char questionList) {
    _is << "t\n" << quizType << '\n' << questionList << "\nb\n4\nk\n";
    std::string line;
    getFirstQuestion(line);
    return line.substr(9);
  }

  void meaningGroupQuiz() {
    // 't' for 'test' mode (instead of review mode)
    // 'm' for Meaning Group Quiz
    // 'b' for Beginning of list (instead of End or Random)
    // '1' for including only Jōyō kanji
    _is << "t\nm\nb\n1\n";
  }
  void edit() { _is << "*\n"; }           // '*' is the option to edit an answer
  void skip() { _is << ".\n"; }           // '.' is the option to skip a question
  void toggleMeanings() { _is << "-\n"; } // '-' is the option to toggle meanings
  void startQuiz() {
    // clear eofbit and failbit for output streams in case quiz is run more than once during a test
    _os.clear();
    _es.clear();
    // final input needs to be '/' to 'quit' the quiz, otherwise test code will hang while quiz
    // is waiting for more input.
    _is << "/\n";
    _quiz.start();
  }

  void getFirstQuestion(std::string& line) {
    startQuiz();
    while (std::getline(_os, line))
      if (line.starts_with("Question 1/")) break;
    if (_os.eof()) FAIL() << "couldn't find first Question";
  }

  std::stringstream _os;
  std::stringstream _es;
  std::stringstream _is;
  const DataPtr _data;
  const Quiz _quiz;
};

TEST_F(QuizTest, ListQuiz) {
  gradeListQuiz();
  startQuiz();
  std::string line, lastLine;
  while (std::getline(_os, line))
    lastLine = line;
  // test the last (non-eof) line sent to _os
  EXPECT_EQ(lastLine, "Final score: 0/0");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(QuizTest, ListQuizReview) {
  _is << "r\ng\n1\nb\n";
  toggleMeanings();
  startQuiz();
  std::string line, lastLine;
  int kanjiCount = 0;
  int meaningCount = 0;
  while (std::getline(_os, line)) {
    if (line == "1/80:  一  Rad 一(1), Strokes 1, yī, N5, Frq 2, Kyu K10, Jouyou")
      ++kanjiCount;
    else if (line == "    Meaning: one")
      ++meaningCount;
    else
      lastLine = line;
  }
  EXPECT_EQ(kanjiCount, 2);   // once before toggling meanings on and once after
  EXPECT_EQ(meaningCount, 1); // in reviewMode meanings are shown on a separate line
  // test the last (non-eof) line sent to _os
  EXPECT_EQ(lastLine, "  Select (-=hide meanings, .=next, /=quit): ");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(QuizTest, FrequencyLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('f', x); };
  EXPECT_EQ(f('1'), "1/500:  日  Rad 日(72), Strokes 4, rì, Grade G1, N5, Kyu K10");
  EXPECT_EQ(f('2'), "1/500:  良  Rad 艮(138), Strokes 7, liáng, Grade G4, N3, Kyu K7");
  EXPECT_EQ(f('3'), "1/500:  贈  Rad 貝(154), Strokes 18, zèng, Grade S, N2, Old 贈, Kyu K4");
  EXPECT_EQ(f('4'), "1/500:  添  Rad 水(85), Strokes 11, tiān, Grade S, N1, Kyu K4");
  EXPECT_EQ(f('5'), "1/501:  炒  Rad 火(86), Strokes 8, chǎo, Kyu K1");
}

TEST_F(QuizTest, GradeLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('g', x); };
  EXPECT_EQ(f('1'), "1/80:  一  Rad 一(1), Strokes 1, yī, N5, Frq 2, Kyu K10");
  EXPECT_EQ(f('2'), "1/160:  引  Rad 弓(57), Strokes 4, yǐn, N4, Frq 218, Kyu K9");
  EXPECT_EQ(f('3'), "1/200:  悪  Rad 心(61), Strokes 11, è, N4, Frq 530, Old 惡, Kyu K8");
  EXPECT_EQ(f('4'), "1/200:  愛  Rad 心(61), Strokes 13, ài, N3, Frq 640, Kyu K7");
  EXPECT_EQ(f('5'), "1/185:  圧  Rad 土(32), Strokes 5, yā, N2, Frq 718, Old 壓, Kyu K6");
  EXPECT_EQ(f('6'), "1/181:  異  Rad 田(102), Strokes 11, yì, N2, Frq 631, Kyu K5");
  EXPECT_EQ(f('s'), "1/1130:  亜  Rad 二(7), Strokes 7, yà, N1, Frq 1509, Old 亞, Kyu KJ2");
}

TEST_F(QuizTest, KyuLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('k', x); };
  EXPECT_EQ(f('a'), "1/80:  一  Rad 一(1), Strokes 1, yī, Grade G1, N5, Frq 2");
  EXPECT_EQ(f('9'), "1/160:  引  Rad 弓(57), Strokes 4, yǐn, Grade G2, N4, Frq 218");
  EXPECT_EQ(f('8'), "1/200:  悪  Rad 心(61), Strokes 11, è, Grade G3, N4, Frq 530, Old 惡");
  EXPECT_EQ(f('7'), "1/202:  愛  Rad 心(61), Strokes 13, ài, Grade G4, N3, Frq 640");
  EXPECT_EQ(f('6'), "1/193:  圧  Rad 土(32), Strokes 5, yā, Grade G5, N2, Frq 718, Old 壓");
  EXPECT_EQ(f('5'), "1/191:  異  Rad 田(102), Strokes 11, yì, Grade G6, N2, Frq 631");
  EXPECT_EQ(f('4'), "1/313:  握  Rad 手(64), Strokes 12, wò, Grade S, N1, Frq 1003");
  EXPECT_EQ(f('3'), "1/284:  哀  Rad 口(30), Strokes 9, āi, Grade S, N1, Frq 1715");
  EXPECT_EQ(f('c'), "1/328:  亜  Rad 二(7), Strokes 7, yà, Grade S, N1, Frq 1509, Old 亞");
  EXPECT_EQ(f('2'), "1/188:  挨  Rad 手(64), Strokes 10, āi, Grade S, Frq 2258");
  EXPECT_EQ(f('b'), "1/940:  唖  Rad 口(30), Strokes 10, yǎ");
  EXPECT_EQ(f('1'), "1/2780:  芦  Rad 艸(140), Strokes 7, lú, Frq 1733");
}

TEST_F(QuizTest, LevelLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('l', x); };
  EXPECT_EQ(f('5'), "1/103:  一  Rad 一(1), Strokes 1, yī, Grade G1, Frq 2, Kyu K10");
  EXPECT_EQ(f('4'), "1/181:  不  Rad 一(1), Strokes 4, bù, Grade G4, Frq 101, Kyu K7");
  EXPECT_EQ(f('3'), "1/361:  丁  Rad 一(1), Strokes 2, dīng, Grade G3, Frq 1312, Kyu K8");
  EXPECT_EQ(f('2'), "1/415:  腕  Rad 肉(130), Strokes 12, wàn, Grade S, Frq 1163, Kyu K4");
  EXPECT_EQ(f('1'), "1/1162:  統  Rad 糸(120), Strokes 12, tǒng, Grade G5, Frq 125, Kyu K6");
}

TEST_F(QuizTest, SkipListQuestions) {
  for (int i = 2; i < 4; ++i) {
    gradeListQuiz();
    for (int j = 0; j < i; ++j)
      skip();
    startQuiz();
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

TEST_F(QuizTest, ToggleListMeanings) {
  gradeListQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  startQuiz();
  std::string line;
  bool meaningsOn = false;
  int found = 0;
  std::string expected("Question 1/80:  一  Rad 一(1), Strokes 1, yī, N5, Frq 2, Kyu K10");
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

TEST_F(QuizTest, GroupQuiz) {
  meaningGroupQuiz();
  startQuiz();
  std::string line, lastLine;
  while (std::getline(_os, line))
    lastLine = line;
  // test the line sent to _os
  EXPECT_EQ(lastLine, "Final score: 0/0");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(QuizTest, SkipGroupQuestions) {
  for (int i = 2; i < 4; ++i) {
    meaningGroupQuiz();
    for (int j = 0; j < i; ++j)
      skip();
    startQuiz();
    std::string line, lastLine;
    while (std::getline(_os, line))
      lastLine = line;
    auto expected = "Final score: 0/" + std::to_string(i) + ", skipped: " + std::to_string(i);
    EXPECT_EQ(lastLine, expected);
  }
}

TEST_F(QuizTest, ToggleGroupMeanings) {
  meaningGroupQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  startQuiz();
  bool meaningsOn = false;
  int found = 0;
  std::string line, expected("みなみ");
  std::string expectedWithMeaning = expected + " : south";
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

TEST_F(QuizTest, EditAfterOneAnswer) {
  meaningGroupQuiz();
  _is << "a\n"; // provide an answer for the first group entry
  edit();
  _is << "b\n"; // change the answer from 'a' to 'b'
  startQuiz();
  int found = 0;
  std::string line;
  while (std::getline(_os, line)) {
    std::cout << line << '\n';
    if (!found) {
      if (line.ends_with("1->a")) ++found;
    } else if (line.ends_with("1->b"))
      ++found;
  }
  EXPECT_EQ(found, 2);
}

TEST_F(QuizTest, EditAfterMultipleAnswers) {
  meaningGroupQuiz();
  _is << "a\nb\n"; // entry 1 maps to 'a' and 2 maps to 'b'
  edit();
  _is << "a\n"; // pick the answer to change (so 1->a)
  _is << "c\n"; // set to a new value (should now become 1->c and 2 still maps to 'b')
  startQuiz();
  int found = 0;
  std::string line;
  while (std::getline(_os, line)) {
    std::cout << line << '\n';
    if (!found) {
      if (line.ends_with("1->a 2->b")) ++found; // before edit
    } else if (line.ends_with("1->c 2->b"))     // after edit
      ++found;
  }
  EXPECT_EQ(found, 2);
}

TEST_F(QuizTest, PatternListFilters) {
  auto f = [this](char x) {
    _is << "t\np\nb\n4\n" << x << "\n";
    std::string line;
    getFirstQuestion(line);
    return line.substr(9);
  };
  EXPECT_EQ(f('1'), "1/85:  [阿：ア], 3 members");
  EXPECT_EQ(f('2'), "1/269:  [華：カ], 5 members");
  EXPECT_EQ(f('3'), "1/286:  [差：サ], 9 members");
  EXPECT_EQ(f('4'), "1/143:  [朶：タ], 2 members");
  EXPECT_EQ(f('5'), "1/144:  [巴：ハ、ヒ], 8 members");
  EXPECT_EQ(f('6'), "1/111:  [耶：ヤ], 4 members");
}

} // namespace kanji_tools
