#include <gtest/gtest.h>
#include <kt_kanji/TextKanjiData.h>
#include <kt_quiz/ListQuiz.h>
#include <kt_tests/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

class ListQuizTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    _data = std::make_shared<TextKanjiData>(Args{}, _os, _es);
    _groupData = std::make_shared<GroupData>(_data);
    _jukugoData = std::make_shared<JukugoData>(_data);
  }

  // Constructs Quiz using the real data files
  ListQuizTest() : _quiz{{}, _data, _groupData, _jukugoData, &_is} {}

  // Populate '_is' as input for '_quiz'
  void gradeQuiz(char listOrder = 'b') {
    // 't' for 'test' mode (instead of review mode)
    // 'b' for Beginning of list (instead of End or Random)
    // 'g' for List Quiz
    // '1' for 1
    // '4' for 4 choices
    // 'k' for kanji to reading quiz
    _is << "t\n" << listOrder << "\ng\n1\n4\nk\n";
  }

  [[nodiscard]] String firstQuestion(
      char quizType, char questionList, bool checkDefault = false) {
    String line, otherLine;
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
    static constexpr auto RemoveQuestionText{9};
    return line.substr(RemoveQuestionText);
  }

  void skip() { _is << ".\n"; } // '.' is the option to skip a question

  void toggleMeanings() {
    _is << "-\n"; // '-' is the option to toggle meanings
  }

  void startQuiz(QuizLauncher::OptChar quizType = {},
      QuizLauncher::OptChar questionList = {}, bool randomizeAnswers = true) {
    _os.str({});
    _es.str({});
    // clear eofbit and failbit for output streams in case quiz is run again
    _os.clear();
    _es.clear();
    // final input needs to be '/' to 'quit' the quiz, otherwise test code will
    // hang while quiz is waiting for more input.
    _is << "/\n";
    _quiz.start(quizType, questionList, {}, false, randomizeAnswers);
  }

  void getFirstQuestion(String& line, QuizLauncher::OptChar quizType = {},
      QuizLauncher::OptChar questionList = {}) {
    startQuiz(quizType, questionList);
    while (std::getline(_os, line))
      if (line.starts_with("Question 1/")) break;
    if (_os.eof()) FAIL() << "couldn't find first Question";
  }

  inline static std::stringstream _os, _es;
  inline static KanjiDataPtr _data;
  inline static GroupDataPtr _groupData;
  inline static JukugoDataPtr _jukugoData;

  auto& is() { return _is; }
  auto& quiz() { return _quiz; }

private:
  std::stringstream _is;
  QuizLauncher _quiz;
};

} // namespace

TEST_F(ListQuizTest, ListOrders) {
  for (String lastLine; const auto i : {'b', 'e', 'r'}) {
    gradeQuiz(i);
    startQuiz();
    for (String line; std::getline(_os, line);) lastLine = line;
    // test the last (non-eof) line sent to _os
    EXPECT_EQ(lastLine, "Final score: 0/0");
    // should be nothing sent to _es (for errors)
    EXPECT_FALSE(std::getline(_es, lastLine));
  }
}

TEST_F(ListQuizTest, MissingReading) {
  // Make a list containing a Kanji without a Japanese reading for this test.
  // This should never happen for any of the current quiz types since they only
  // include standard Kanji with readings.
  const String noReading("㐄");
  const auto i{_data->findByName(noReading)};
  ASSERT_TRUE(i);
  ASSERT_FALSE(i->hasReading());
  KanjiData::List questionList{i};
  const auto f{[&questionList, this] {
    ListQuiz{quiz(), {}, {}, questionList, Kanji::Info::All, 1,
        ListQuiz::QuizStyle::KanjiToReading};
  }};
  EXPECT_THROW(call(f, noReading + " has no reading"), DomainError);
}

TEST_F(ListQuizTest, QuizDefaults) {
  constexpr auto First8Chars{8};
  const auto run{[this](String& out) {
    startQuiz();
    String line;
    // collect all lines after ">>>" (the start of the quiz), but don't add
    // the readings for the choices since they are randomly selected (instead
    // just get the first 8 chars, i.e., the "    #.  " part)
    while (std::getline(_os, line))
      if (!out.empty() || line.starts_with(">>>"))
        out += line.starts_with("    ") ? line.substr(0, First8Chars) : line;
  }};
  String all, allWithDefaults;
  gradeQuiz();
  run(all);
  ASSERT_FALSE(all.empty());
  // run again using defaults for the following and expect the same results:
  // - program mode: 't' (test)
  // - quiz type: 'g' (grade)
  // - list quiz answers: '4'
  // - list quiz style: 'k' (kanji to reading)
  // still need to specify '1' (for grade) and 'b' (for beginning of list)
  // since these aren't defaults
  is() << "\nb\n\n1\n\n\n";
  run(allWithDefaults);
  EXPECT_EQ(all, allWithDefaults);
}

TEST_F(ListQuizTest, QuizReview) {
  is() << "r\nb\ng\n1\n";
  toggleMeanings();
  startQuiz();
  String line, lastLine;
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
  EXPECT_FALSE(std::getline(is(), line));
}

TEST_F(ListQuizTest, ReviewNextPrev) {
  // move forward twice (.) and then back twice (,)
  is() << "r\nb\n.\n.\n,\n,\n";
  startQuiz('g', '2');
  size_t found{};
  String line;
  const auto f{[&line, &found](auto question) {
    if (line.starts_with(std::to_string(question) + "/")) ++found;
  }};
  while (std::getline(_os, line)) switch (found) {
    case 0: f(1); break;
    case 1: f(2); break;
    case 2: f(3); break;
    case 3: f(2); break;
    case 4: f(1); break;
    default: break;
    }
  // expect to find question 1 then 2 then 3 then 2 then 1
  EXPECT_EQ(found, 5);
}

TEST_F(ListQuizTest, ReadingQuiz) {
  is() << "t\nb\ng\n1\n4\nr\n";
  String line;
  getFirstQuestion(line);
  EXPECT_EQ(line, "Question 1/80:  Reading:  イチ、イツ、ひと、ひと-つ");
}

TEST_F(ListQuizTest, CorrectResponse) {
  is() << "t\nb\n4\nr\n1\n";
  startQuiz('g', '1', false);
  auto found{false};
  String lastLine;
  for (String line; std::getline(_os, line); lastLine = line)
    if (line.ends_with("Correct! (1/1)")) found = true;
  EXPECT_TRUE(found);
  EXPECT_EQ(lastLine, "Final score: 1/1 - Perfect!");
}

TEST_F(ListQuizTest, IncorrectResponse) {
  is() << "t\nb\n4\nr\n2\n";
  startQuiz('g', '1', false);
  auto found{false};
  String lastLine;
  for (String line; std::getline(_os, line); lastLine = line)
    if (line.ends_with("Incorrect (correct answer is 1)")) found = true;
  EXPECT_TRUE(found);
  EXPECT_EQ(lastLine, "Final score: 0/1 - mistakes: 一");
}

TEST_F(ListQuizTest, FrequencyLists) {
  const auto f{[this](char x) { return firstQuestion('f', x); }};
  EXPECT_EQ(f('0'), "1/250:  日  Rad 日(72), Strokes 4, rì, G1, N5, K10");
  EXPECT_EQ(f('1'), "1/250:  式  Rad 弋(56), Strokes 6, shì, G3, N3, K8");
  EXPECT_EQ(f('2'), "1/250:  良  Rad 艮(138), Strokes 7, liáng, G4, N3, K7");
  EXPECT_EQ(
      f('3'), "1/250:  闘  Rad 鬥(191), Strokes 18, dòu, S, N1, Old 鬭, K4");
  EXPECT_EQ(
      f('4'), "1/250:  贈  Rad 貝(154), Strokes 18, zèng, S, N2, Old 贈, K4");
  EXPECT_EQ(f('5'), "1/250:  彩  Rad 彡(59), Strokes 11, cǎi, S, N1, K4");
  EXPECT_EQ(f('6'), "1/250:  添  Rad 水(85), Strokes 11, tiān, S, N1, K4");
  EXPECT_EQ(f('7'), "1/250:  釧  Rad 金(167), Strokes 11, chuàn, KJ1");
  EXPECT_EQ(f('8'), "1/250:  炒  Rad 火(86), Strokes 8, chǎo, K1");
  EXPECT_EQ(f('9'), "1/251:  蒋  Rad 艸(140), Strokes 13, jiǎng, Old 蔣, KJ1");
}

TEST_F(ListQuizTest, GradeLists) {
  const auto f{[this](char x, bool checkDefault = false) {
    return firstQuestion('g', x, checkDefault);
  }};
  EXPECT_EQ(f('1'), "1/80:  一  Rad 一(1), Strokes 1, yī, N5, Frq 2, K10");
  EXPECT_EQ(f('2'), "1/160:  引  Rad 弓(57), Strokes 4, yǐn, N4, Frq 218, K9");
  EXPECT_EQ(
      f('3'), "1/200:  悪  Rad 心(61), Strokes 11, è, N4, Frq 530, Old 惡, K8");
  EXPECT_EQ(f('4'), "1/200:  愛  Rad 心(61), Strokes 13, ài, N3, Frq 640, K7");
  EXPECT_EQ(
      f('5'), "1/185:  圧  Rad 土(32), Strokes 5, yā, N2, Frq 718, Old 壓, K6");
  EXPECT_EQ(
      f('6', true), "1/181:  異  Rad 田(102), Strokes 11, yì, N2, Frq 631, K5");
  EXPECT_EQ(f('s'),
      "1/1130:  亜  Rad 二(7), Strokes 7, yà, N1, Frq 1509, Old 亞, KJ2");
}

TEST_F(ListQuizTest, KyuLists) {
  const auto f{[this](char x, bool checkDefault = false) {
    return firstQuestion('k', x, checkDefault);
  }};
  EXPECT_EQ(f('a'), "1/80:  一  Rad 一(1), Strokes 1, yī, G1, N5, Frq 2");
  EXPECT_EQ(f('9'), "1/160:  引  Rad 弓(57), Strokes 4, yǐn, G2, N4, Frq 218");
  EXPECT_EQ(
      f('8'), "1/200:  悪  Rad 心(61), Strokes 11, è, G3, N4, Frq 530, Old 惡");
  EXPECT_EQ(f('7'), "1/202:  愛  Rad 心(61), Strokes 13, ài, G4, N3, Frq 640");
  EXPECT_EQ(
      f('6'), "1/193:  圧  Rad 土(32), Strokes 5, yā, G5, N2, Frq 718, Old 壓");
  EXPECT_EQ(f('5'), "1/191:  異  Rad 田(102), Strokes 11, yì, G6, N2, Frq 631");
  EXPECT_EQ(f('4'), "1/313:  握  Rad 手(64), Strokes 12, wò, S, N1, Frq 1003");
  EXPECT_EQ(f('3'), "1/284:  哀  Rad 口(30), Strokes 9, āi, S, N1, Frq 1715");
  EXPECT_EQ(
      f('c'), "1/328:  亜  Rad 二(7), Strokes 7, yà, S, N1, Frq 1509, Old 亞");
  EXPECT_EQ(
      f('2', true), "1/188:  挨  Rad 手(64), Strokes 10, āi, S, Frq 2258");
  EXPECT_EQ(f('b'), "1/940:  唖  Rad 口(30), Strokes 10, yǎ");
  EXPECT_EQ(f('1'), "1/2780:  芦  Rad 艸(140), Strokes 7, lú, Frq 1733");
}

TEST_F(ListQuizTest, LevelLists) {
  const auto f{[this](char x) { return firstQuestion('l', x); }};
  EXPECT_EQ(f('5'), "1/103:  一  Rad 一(1), Strokes 1, yī, G1, Frq 2, K10");
  EXPECT_EQ(f('4'), "1/181:  不  Rad 一(1), Strokes 4, bù, G4, Frq 101, K7");
  EXPECT_EQ(f('3'), "1/361:  丁  Rad 一(1), Strokes 2, dīng, G3, Frq 1312, K8");
  EXPECT_EQ(
      f('2'), "1/415:  腕  Rad 肉(130), Strokes 12, wàn, S, Frq 1163, K4");
  EXPECT_EQ(
      f('1'), "1/1162:  統  Rad 糸(120), Strokes 12, tǒng, G5, Frq 125, K6");
}

TEST_F(ListQuizTest, SkipQuestions) {
  for (size_t i{2}; i < 4; ++i) {
    gradeQuiz();
    for (size_t j{}; j < i; ++j) skip();
    startQuiz();
    // make sure _os is in expected 'good' state
    EXPECT_TRUE(_os.good());
    EXPECT_FALSE(_os.eof() || _os.fail() || _os.bad());
    String line, lastLine;
    while (std::getline(_os, line)) lastLine = line;
    // make sure _os is in expected 'eof' state
    EXPECT_TRUE(_os.eof() && _os.fail());
    EXPECT_FALSE(_os.good() || _os.bad());
    const auto skipped(std::to_string(i));
    EXPECT_EQ(
        lastLine, "Final score: 0/" + skipped += (", skipped: " + skipped));
  }
}

TEST_F(ListQuizTest, ToggleMeanings) {
  gradeQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  startQuiz();
  String line;
  auto meaningsOn{false};
  size_t found{};
  const String expected{
      "Question 1/80:  一  Rad 一(1), Strokes 1, yī, N5, Frq 2, K10"};
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

} // namespace kanji_tools
