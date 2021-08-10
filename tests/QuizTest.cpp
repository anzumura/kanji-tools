#include <gtest/gtest.h>

#include <kanji/Kanji.h>
#include <kanji/KanjiData.h>
#include <kanji/Quiz.h>

#include <sstream>

namespace kanji {

class QuizTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "-data";
    static const char* arg2 = "../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }
  // Contructs Quiz using the real data files
  QuizTest() : _data(std::make_shared<KanjiData>(3, argv(), _os, _es)), _groupData(_data), _quiz(_groupData, &_is) {}

  // Populate '_is' as input for '_quiz'
  void gradeListQuiz() {
    // 'g' for Grade List Quiz
    // '1' for Grade 1
    // 'b' for Beginning of list (instead of End or Random)
    // '4' for 4 choices
    // 'k' for kanji to reading quiz
    _is << "g\n1\nb\n4\nk\n";
  }

  std::string listQuizFirstQuestion(char quizType, char questionList) {
    _is << quizType << '\n' << questionList << "\nb\n4\nk\n";
    runQuiz();
    std::string line;
    getFirstQuestion(line);
    return line.substr(9);
  }

  void meaningGroupQuiz() {
    // 'm' for Meaning Group Quiz
    // 'b' for Beginning of list (instead of End or Random)
    // '1' for including only Jōyō kanji
    _is << "m\nb\n1\n";
  }
  void edit() { _is << "*\n"; }           // '*' is the option to edit an answer
  void skip() { _is << ".\n"; }           // '.' is the option to skip a question
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

  void getFirstQuestion(std::string& line) {
    while (std::getline(_os, line))
      if (line.starts_with("Question 1/")) break;
    if (_os.eof()) FAIL() << "couldn't find first Question";
  }

  std::stringstream _os;
  std::stringstream _es;
  std::stringstream _is;
  const DataPtr _data;
  const GroupData _groupData;
  const Quiz _quiz;
};

TEST_F(QuizTest, ListQuiz) {
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

TEST_F(QuizTest, FrequencyLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('f', x); };
  EXPECT_EQ(f('1'), "1/500.  Kanji:  日  (Rad 日, Strokes 4, Grade G1, Level N5, Kyu K10)");
  EXPECT_EQ(f('2'), "1/500.  Kanji:  良  (Rad 艮, Strokes 7, Grade G4, Level N3, Kyu K7)");
  EXPECT_EQ(f('3'), "1/500.  Kanji:  贈  (Rad 貝, Strokes 18, Grade S, Level N2, Old 贈, Kyu K4)");
  EXPECT_EQ(f('4'), "1/500.  Kanji:  添  (Rad 水, Strokes 11, Grade S, Level N1, Kyu K4)");
  EXPECT_EQ(f('5'), "1/501.  Kanji:  炒  (Rad 火, Strokes 8, Kyu K1)");
}

TEST_F(QuizTest, GradeLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('g', x); };
  EXPECT_EQ(f('1'), "1/80.  Kanji:  一  (Rad 一, Strokes 1, Level N5, Freq 2, Kyu K10)");
  EXPECT_EQ(f('2'), "1/160.  Kanji:  引  (Rad 弓, Strokes 4, Level N4, Freq 218, Kyu K9)");
  EXPECT_EQ(f('3'), "1/200.  Kanji:  悪  (Rad 心, Strokes 11, Level N4, Freq 530, Old 惡, Kyu K8)");
  EXPECT_EQ(f('4'), "1/200.  Kanji:  愛  (Rad 心, Strokes 13, Level N3, Freq 640, Kyu K7)");
  EXPECT_EQ(f('5'), "1/185.  Kanji:  圧  (Rad 土, Strokes 5, Level N2, Freq 718, Old 壓, Kyu K6)");
  EXPECT_EQ(f('6'), "1/181.  Kanji:  異  (Rad 田, Strokes 11, Level N2, Freq 631, Kyu K5)");
  EXPECT_EQ(f('s'), "1/1130.  Kanji:  亜  (Rad 二, Strokes 7, Level N1, Freq 1509, Old 亞, Kyu KJ2)");
}

TEST_F(QuizTest, KyuLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('k', x); };
  EXPECT_EQ(f('a'), "1/80.  Kanji:  一  (Rad 一, Strokes 1, Grade G1, Level N5, Freq 2)");
  EXPECT_EQ(f('9'), "1/160.  Kanji:  引  (Rad 弓, Strokes 4, Grade G2, Level N4, Freq 218)");
  EXPECT_EQ(f('8'), "1/200.  Kanji:  悪  (Rad 心, Strokes 11, Grade G3, Level N4, Freq 530, Old 惡)");
  EXPECT_EQ(f('7'), "1/202.  Kanji:  愛  (Rad 心, Strokes 13, Grade G4, Level N3, Freq 640)");
  EXPECT_EQ(f('6'), "1/193.  Kanji:  圧  (Rad 土, Strokes 5, Grade G5, Level N2, Freq 718, Old 壓)");
  EXPECT_EQ(f('5'), "1/191.  Kanji:  異  (Rad 田, Strokes 11, Grade G6, Level N2, Freq 631)");
  EXPECT_EQ(f('4'), "1/313.  Kanji:  握  (Rad 手, Strokes 12, Grade S, Level N1, Freq 1003)");
  EXPECT_EQ(f('3'), "1/284.  Kanji:  哀  (Rad 口, Strokes 9, Grade S, Level N1, Freq 1715)");
  EXPECT_EQ(f('c'), "1/328.  Kanji:  亜  (Rad 二, Strokes 7, Grade S, Level N1, Freq 1509, Old 亞)");
  EXPECT_EQ(f('2'), "1/188.  Kanji:  挨  (Rad 手, Strokes 10, Grade S, Freq 2258)");
  EXPECT_EQ(f('b'), "1/940.  Kanji:  唖  (Rad 口, Strokes 10)");
  EXPECT_EQ(f('1'), "1/2780.  Kanji:  芦  (Rad 艸, Strokes 7, Freq 1733)");
}

TEST_F(QuizTest, LevelLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('l', x); };
  EXPECT_EQ(f('5'), "1/103.  Kanji:  一  (Rad 一, Strokes 1, Grade G1, Freq 2, Kyu K10)");
  EXPECT_EQ(f('4'), "1/181.  Kanji:  不  (Rad 一, Strokes 4, Grade G4, Freq 101, Kyu K7)");
  EXPECT_EQ(f('3'), "1/361.  Kanji:  丁  (Rad 一, Strokes 2, Grade G3, Freq 1312, Kyu K8)");
  EXPECT_EQ(f('2'), "1/415.  Kanji:  腕  (Rad 肉, Strokes 12, Grade S, Freq 1163, Kyu K4)");
  EXPECT_EQ(f('1'), "1/1162.  Kanji:  統  (Rad 糸, Strokes 12, Grade G5, Freq 125, Kyu K6)");
}

TEST_F(QuizTest, SkipListQuestions) {
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

TEST_F(QuizTest, ToggleListMeanings) {
  gradeListQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  runQuiz();
  std::string line;
  bool meaningsOn = false;
  int found = 0;
  std::string expected("Question 1/80.  Kanji:  一  (Rad 一, Strokes 1, Level N5, Freq 2, Kyu K10)");
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

TEST_F(QuizTest, SkipGroupQuestions) {
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

TEST_F(QuizTest, ToggleGroupMeanings) {
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

TEST_F(QuizTest, EditAfterOneAnswer) {
  meaningGroupQuiz();
  _is << "a\n"; // provide an answer for the first group entry
  edit();
  _is << "b\n"; // change the answer from 'a' to 'b'
  runQuiz();
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
  runQuiz();
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

} // namespace kanji
