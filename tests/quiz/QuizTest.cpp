#include <gtest/gtest.h>
#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/QuizLauncher.h>

#include <sstream>

namespace kanji_tools {

class QuizTest : public ::testing::Test {
protected:
  [[nodiscard]] static auto argv() {
    static auto arg0 = "testMain", arg1 = "-data", arg2 = "../../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }

  static void SetUpTestCase() {
    _data = std::make_shared<KanjiData>(3, argv(), _os, _es);
    _groupData = std::make_shared<GroupData>(_data);
    _jukugoData = std::make_shared<JukugoData>(_data);
  }

  // Contructs Quiz using the real data files
  QuizTest() : _quiz(3, argv(), _data, _groupData, _jukugoData, &_is) {}

  // Populate '_is' as input for '_quiz'
  void gradeListQuiz() {
    // 't' for 'test' mode (instead of review mode)
    // 'b' for Beginning of list (instead of End or Random)
    // 'g' for List Quiz
    // '1' for 1
    // '4' for 4 choices
    // 'k' for kanji to reading quiz
    _is << "t\nb\ng\n1\n4\nk\n";
  }

  [[nodiscard]] std::string listQuizFirstQuestion(char quizType,
                                                  char questionList,
                                                  bool checkDefault = false) {
    std::string line, otherLine;
    // run with quizType and questionList coming from stdin
    _is << "t\nb\n" << quizType << '\n' << questionList << "\n4\nk\n";
    getFirstQuestion(line);
    if (checkDefault) {
      // run again with '\n' for questionList to check if it's the default
      _is << "t\nb\n" << quizType << "\n\n4\nk\n";
      getFirstQuestion(otherLine);
      EXPECT_EQ(line, otherLine);
    }
    // run explicitly passing in quizType and questionList (so not from stdin)
    _is << "t\nb\n4\nk\n";
    getFirstQuestion(otherLine, quizType, questionList);
    EXPECT_EQ(line, otherLine);
    return line.substr(9);
  }

  void meaningGroupQuiz() {
    // 't' for 'test' mode (instead of review mode)
    // 'b' for Beginning of list (instead of End or Random)
    // 'm' for Meaning Group Quiz
    // '1' for including only Jōyō kanji
    _is << "t\nb\nm\n1\n";
  }
  void edit() { _is << "*\n"; } // '*' is the option to edit an answer
  void skip() { _is << ".\n"; } // '.' is the option to skip a question
  void toggleMeanings() {
    _is << "-\n";
  } // '-' is the option to toggle meanings
  void startQuiz(QuizLauncher::OptChar quizType = std::nullopt,
                 QuizLauncher::OptChar questionList = std::nullopt) {
    // clear eofbit and failbit for output streams in case quiz is run again
    _os.clear();
    _es.clear();
    // final input needs to be '/' to 'quit' the quiz, otherwise test code will
    // hang while quiz is waiting for more input.
    _is << "/\n";
    _quiz.start(quizType, questionList);
  }

  void getFirstQuestion(std::string& line,
                        QuizLauncher::OptChar quizType = std::nullopt,
                        QuizLauncher::OptChar questionList = std::nullopt) {
    startQuiz(quizType, questionList);
    while (std::getline(_os, line))
      if (line.starts_with("Question 1/")) break;
    if (_os.eof()) FAIL() << "couldn't find first Question";
  }

  std::stringstream _is;
  QuizLauncher _quiz;

  inline static std::stringstream _os;
  inline static std::stringstream _es;
  inline static DataPtr _data;
  inline static GroupDataPtr _groupData;
  inline static JukugoDataPtr _jukugoData;
};

TEST_F(QuizTest, ListQuiz) {
  gradeListQuiz();
  startQuiz();
  std::string line, lastLine;
  while (std::getline(_os, line)) lastLine = line;
  // test the last (non-eof) line sent to _os
  EXPECT_EQ(lastLine, "Final score: 0/0");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(QuizTest, ListQuizDefaults) {
  auto run = [this](std::string& out) {
    startQuiz();
    std::string line;
    // collect all lines after ">>>" (the start of the quiz), but don't add the
    // readings for the choices since they are randomly selected (instead just
    // get the first 8 chars, i.e., the "    #.  " part)
    while (std::getline(_os, line))
      if (!out.empty() || line.starts_with(">>>"))
        out += line.starts_with("    ") ? line.substr(0, 8) : line;
  };
  std::string all, allWithDefaults;
  gradeListQuiz();
  run(all);
  ASSERT_FALSE(all.empty());
  // run again using defaults for the following and expect the same results:
  // - program mode: 't' (test)
  // - quiz type: 'g' (grade)
  // - list quiz answers: '4'
  // - list quiz style: 'k' (kanji to reading)
  // still need to specify '1' (for grade) and 'b' (for beginning of list) since
  // these aren't defaults
  _is << "\nb\n\n1\n\n\n";
  run(allWithDefaults);
  EXPECT_EQ(all, allWithDefaults);
}

TEST_F(QuizTest, ListQuizReview) {
  _is << "r\nb\ng\n1\n";
  toggleMeanings();
  startQuiz();
  std::string line, lastLine;
  auto kanjiCount{0}, meaningCount{0};
  while (std::getline(_os, line)) {
    if (line ==
        "1/80:  一  Rad 一(1), Strokes 1, yī, N5, Frq 2, K10, Jouyou (#41)")
      ++kanjiCount;
    else if (line == "    Meaning: one")
      ++meaningCount;
    else
      lastLine = line;
  }
  EXPECT_EQ(kanjiCount, 2);   // once before toggling meanings on and once after
  EXPECT_EQ(meaningCount, 1); // in reviewMode meanings are on separate line
  // test the last (non-eof) line sent to _os
  EXPECT_EQ(lastLine, "  Select (-=hide meanings, .=next, /=quit): ");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(QuizTest, FrequencyLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('f', x); };
  EXPECT_EQ(f('1'), "1/500:  日  Rad 日(72), Strokes 4, rì, G1, N5, K10");
  EXPECT_EQ(f('2'), "1/500:  良  Rad 艮(138), Strokes 7, liáng, G4, N3, K7");
  EXPECT_EQ(f('3'),
            "1/500:  贈  Rad 貝(154), Strokes 18, zèng, S, N2, Old 贈, K4");
  EXPECT_EQ(f('4'), "1/500:  添  Rad 水(85), Strokes 11, tiān, S, N1, K4");
  EXPECT_EQ(f('5'), "1/501:  炒  Rad 火(86), Strokes 8, chǎo, K1");
}

TEST_F(QuizTest, GradeLists) {
  auto f = [this](char x, bool checkDefault = false) {
    return listQuizFirstQuestion('g', x, checkDefault);
  };
  EXPECT_EQ(f('1'), "1/80:  一  Rad 一(1), Strokes 1, yī, N5, Frq 2, K10");
  EXPECT_EQ(f('2'), "1/160:  引  Rad 弓(57), Strokes 4, yǐn, N4, Frq 218, K9");
  EXPECT_EQ(f('3'),
            "1/200:  悪  Rad 心(61), Strokes 11, è, N4, Frq 530, Old 惡, K8");
  EXPECT_EQ(f('4'), "1/200:  愛  Rad 心(61), Strokes 13, ài, N3, Frq 640, K7");
  EXPECT_EQ(f('5'),
            "1/185:  圧  Rad 土(32), Strokes 5, yā, N2, Frq 718, Old 壓, K6");
  EXPECT_EQ(f('6', true),
            "1/181:  異  Rad 田(102), Strokes 11, yì, N2, Frq 631, K5");
  EXPECT_EQ(f('s'),
            "1/1130:  亜  Rad 二(7), Strokes 7, yà, N1, Frq 1509, Old 亞, KJ2");
}

TEST_F(QuizTest, KyuLists) {
  auto f = [this](char x, bool checkDefault = false) {
    return listQuizFirstQuestion('k', x, checkDefault);
  };
  EXPECT_EQ(f('a'), "1/80:  一  Rad 一(1), Strokes 1, yī, G1, N5, Frq 2");
  EXPECT_EQ(f('9'), "1/160:  引  Rad 弓(57), Strokes 4, yǐn, G2, N4, Frq 218");
  EXPECT_EQ(f('8'),
            "1/200:  悪  Rad 心(61), Strokes 11, è, G3, N4, Frq 530, Old 惡");
  EXPECT_EQ(f('7'), "1/202:  愛  Rad 心(61), Strokes 13, ài, G4, N3, Frq 640");
  EXPECT_EQ(f('6'),
            "1/193:  圧  Rad 土(32), Strokes 5, yā, G5, N2, Frq 718, Old 壓");
  EXPECT_EQ(f('5'), "1/191:  異  Rad 田(102), Strokes 11, yì, G6, N2, Frq 631");
  EXPECT_EQ(f('4'), "1/313:  握  Rad 手(64), Strokes 12, wò, S, N1, Frq 1003");
  EXPECT_EQ(f('3'), "1/284:  哀  Rad 口(30), Strokes 9, āi, S, N1, Frq 1715");
  EXPECT_EQ(f('c'),
            "1/328:  亜  Rad 二(7), Strokes 7, yà, S, N1, Frq 1509, Old 亞");
  EXPECT_EQ(f('2', true),
            "1/188:  挨  Rad 手(64), Strokes 10, āi, S, Frq 2258");
  EXPECT_EQ(f('b'), "1/940:  唖  Rad 口(30), Strokes 10, yǎ");
  EXPECT_EQ(f('1'), "1/2780:  芦  Rad 艸(140), Strokes 7, lú, Frq 1733");
}

TEST_F(QuizTest, LevelLists) {
  auto f = [this](char x) { return listQuizFirstQuestion('l', x); };
  EXPECT_EQ(f('5'), "1/103:  一  Rad 一(1), Strokes 1, yī, G1, Frq 2, K10");
  EXPECT_EQ(f('4'), "1/181:  不  Rad 一(1), Strokes 4, bù, G4, Frq 101, K7");
  EXPECT_EQ(f('3'), "1/361:  丁  Rad 一(1), Strokes 2, dīng, G3, Frq 1312, K8");
  EXPECT_EQ(f('2'),
            "1/415:  腕  Rad 肉(130), Strokes 12, wàn, S, Frq 1163, K4");
  EXPECT_EQ(f('1'),
            "1/1162:  統  Rad 糸(120), Strokes 12, tǒng, G5, Frq 125, K6");
}

TEST_F(QuizTest, SkipListQuestions) {
  for (auto i = 2; i < 4; ++i) {
    gradeListQuiz();
    for (auto j = 0; j < i; ++j) skip();
    startQuiz();
    // make sure _os is in expected 'good' state
    EXPECT_TRUE(_os.good());
    EXPECT_FALSE(_os.eof() || _os.fail() || _os.bad());
    std::string line, lastLine;
    while (std::getline(_os, line)) lastLine = line;
    // make sure _os is in expected 'eof' state
    EXPECT_TRUE(_os.eof() && _os.fail());
    EXPECT_FALSE(_os.good() || _os.bad());

    auto expected =
      "Final score: 0/" + std::to_string(i) + ", skipped: " + std::to_string(i);
    EXPECT_EQ(lastLine, expected);
  }
}

TEST_F(QuizTest, ToggleListMeanings) {
  gradeListQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  startQuiz();
  std::string line;
  auto meaningsOn = false;
  auto found = 0;
  std::string expected(
    "Question 1/80:  一  Rad 一(1), Strokes 1, yī, N5, Frq 2, K10");
  while (std::getline(_os, line)) {
    if (line.starts_with("Question")) {
      ++found;
      EXPECT_EQ(line, expected + (meaningsOn ? " : one" : ""));
      meaningsOn = !meaningsOn;
    }
  }
  // We want to find the Question string 3 times, i.e., once without meanings,
  // then again with a meaning when meanings are toggled on and then again
  // without a meaning when meanings are toggled off.
  EXPECT_EQ(found, 3);
}

TEST_F(QuizTest, GroupQuiz) {
  meaningGroupQuiz();
  startQuiz();
  std::string line, lastLine;
  while (std::getline(_os, line)) lastLine = line;
  // test the line sent to _os
  EXPECT_EQ(lastLine, "Final score: 0/0");
  // should be nothing sent to _es (for errors) and nothing left in _is
  EXPECT_FALSE(std::getline(_es, line));
  EXPECT_FALSE(std::getline(_is, line));
}

TEST_F(QuizTest, SkipGroupQuestions) {
  for (auto i = 2; i < 4; ++i) {
    meaningGroupQuiz();
    for (auto j = 0; j < i; ++j) skip();
    startQuiz();
    std::string line, lastLine;
    while (std::getline(_os, line)) lastLine = line;
    auto expected =
      "Final score: 0/" + std::to_string(i) + ", skipped: " + std::to_string(i);
    EXPECT_EQ(lastLine, expected);
  }
}

TEST_F(QuizTest, ToggleGroupMeanings) {
  meaningGroupQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  startQuiz();
  auto meaningsOn = false;
  auto found = 0;
  std::string line, expected("みなみ");
  std::string expectedWithMeaning = expected + " : south";
  while (std::getline(_os, line)) {
    if (line.erase(0, 4).starts_with(":  ") &&
        line.ends_with(meaningsOn ? expectedWithMeaning : expected)) {
      ++found;
      meaningsOn = !meaningsOn;
    }
  }
  // We want to find the Question string 3 times, i.e., once without meanings,
  // then again with a meaning when meanings are toggled on and then again
  // without a meaning when meanings are toggled off.
  EXPECT_EQ(found, 3);
}

TEST_F(QuizTest, EditAfterOneAnswer) {
  meaningGroupQuiz();
  _is << "a\n"; // provide an answer for the first group entry
  edit();
  _is << "b\n"; // change the answer from 'a' to 'b'
  startQuiz();
  auto found = 0;
  std::string line;
  while (std::getline(_os, line)) {
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
  _is << "c\n"; // set new value (should now be 1->c and 2 still maps to 'b')
  startQuiz();
  auto found = 0;
  std::string line;
  while (std::getline(_os, line)) {
    if (!found) {
      if (line.ends_with("1->a 2->b")) ++found; // before edit
    } else if (line.ends_with("1->c 2->b"))     // after edit
      ++found;
  }
  EXPECT_EQ(found, 2);
}

TEST_F(QuizTest, PatternGroupBuckets) {
  auto f = [this](char x) {
    _is << "t\nb\np\n4\n" << x << "\n";
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

TEST_F(QuizTest, GroupQuizDefaults) {
  _is << "t\nb\np\n2\n1\n";
  std::string line, lineWithDefaults;
  getFirstQuestion(line);
  EXPECT_EQ(line.substr(9),
            "1/37:  [亜：ア、アク], showing 2 out of 3 members");
  // check default 'member filter' is '2' and the default 'bucket' is '1'
  _is << "t\nb\np\n\n\n";
  getFirstQuestion(lineWithDefaults);
  EXPECT_EQ(line, lineWithDefaults);
}

} // namespace kanji_tools
