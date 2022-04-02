#include <gtest/gtest.h>
#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/GroupQuiz.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

class GroupQuizTest : public ::testing::Test {
protected:
  static void SetUpTestCase() {
    _data = std::make_shared<KanjiData>(Args{}, _os, _es);
    _groupData = std::make_shared<GroupData>(_data);
    _jukugoData = std::make_shared<JukugoData>(_data);
  }

  // Contructs Quiz using the real data files
  GroupQuizTest() : _quiz{{}, _data, _groupData, _jukugoData, &_is} {}

  void meaningQuiz(char listOrder = 'b') {
    // 't' for 'test' mode (instead of review mode)
    // 'b' for Beginning of list (instead of End or Random)
    // 'm' for Meaning Group Quiz
    // '1' for including only Jōyō kanji
    _is << "t\n" << listOrder << "\nm\n1\n";
  }

  void edit() { _is << "*\n"; } // '*' is the option to edit an answer

  void skip() { _is << ".\n"; } // '.' is the option to skip a question

  void toggleMeanings() {
    _is << "-\n"; // '-' is the option to toggle meanings
  }

  void startQuiz(QuizLauncher::OptChar quizType = {},
      QuizLauncher::OptChar questionList = {}) {
    _os.str(EmptyString);
    _es.str(EmptyString);
    // clear eofbit and failbit for output streams in case quiz is run again
    _os.clear();
    _es.clear();
    // final input needs to be '/' to 'quit' the quiz, otherwise test code will
    // hang while quiz is waiting for more input.
    _is << "/\n";
    _quiz.start(quizType, questionList);
  }

  void getFirstQuestion(std::string& line, QuizLauncher::OptChar quizType = {},
      QuizLauncher::OptChar questionList = {}) {
    startQuiz(quizType, questionList);
    while (std::getline(_os, line))
      if (line.starts_with("Question 1/")) break;
    if (_os.eof()) FAIL() << "couldn't find first Question";
  }

  std::stringstream _is;
  QuizLauncher _quiz;

  inline static std::stringstream _os, _es;
  inline static DataPtr _data;
  inline static GroupDataPtr _groupData;
  inline static JukugoDataPtr _jukugoData;
};

} // namespace

TEST_F(GroupQuizTest, ListOrders) {
  for (std::string lastLine; const auto i : {'b', 'e', 'r'}) {
    meaningQuiz(i);
    startQuiz();
    for (std::string line; std::getline(_os, line);) lastLine = line;
    // test the line sent to _os
    EXPECT_EQ(lastLine, "Final score: 0/0");
    // should be nothing sent to _es (for errors)
    EXPECT_FALSE(std::getline(_es, lastLine));
  }
}

TEST_F(GroupQuizTest, GroupKanjiTypes) {
  const auto f{[](int x = 0) {
    return (x ? "showing " + std::to_string(x) + " out of " : EmptyString) +
           "37 members";
  }};
  for (auto i : {std::pair{'1', f(28)}, std::pair{'2', f(31)},
           std::pair{'3', f(32)}, std::pair{'4', f()}}) {
    // t=test mode, b=beginning of list, and .=skip to the next question (totals
    // in this test are for question 2 since it contains Kanji of all 4 types)
    _is << "t\nb\n.\n";
    startQuiz('m', i.first);
    EXPECT_NE(_os.str().find(", " + i.second), std::string::npos) << i.second;
  }
}

TEST_F(GroupQuizTest, QuizWithEmptyList) {
  // should never happen with proper '-groups.txt' files
  const auto f{[this] {
    GroupQuiz{_quiz, {}, {}, {}, GroupQuiz::MemberType::All};
  }};
  EXPECT_THROW(call(f, "empty group list"), std::domain_error);
}

TEST_F(GroupQuizTest, SkipQuestions) {
  for (size_t i{2}; i < 4; ++i) {
    meaningQuiz();
    for (size_t j{}; j < i; ++j) skip();
    startQuiz();
    std::string line, lastLine;
    while (std::getline(_os, line)) lastLine = line;
    const auto skipped{std::to_string(i)};
    EXPECT_EQ(lastLine, "Final score: 0/" + skipped + ", skipped: " + skipped);
  }
}

TEST_F(GroupQuizTest, ToggleMeanings) {
  meaningQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  startQuiz();
  auto meaningsOn{false};
  size_t found{};
  const std::string expected{"みなみ"};
  const auto expectedWithMeaning{expected + " : south"};
  for (std::string line; std::getline(_os, line);) {
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

TEST_F(GroupQuizTest, EditAfterOneAnswer) {
  meaningQuiz();
  _is << "a\n"; // provide an answer for the first group entry
  edit();
  _is << "b\n"; // change the answer from 'a' to 'b'
  startQuiz();
  size_t found{};
  for (std::string line; std::getline(_os, line);) {
    if (!found) {
      if (line.ends_with("1->a")) ++found;
    } else if (line.ends_with("1->b"))
      ++found;
  }
  EXPECT_EQ(found, 2);
}

TEST_F(GroupQuizTest, EditAfterMultipleAnswers) {
  meaningQuiz();
  _is << "a\nb\n"; // entry 1 maps to 'a' and 2 maps to 'b'
  edit();
  _is << "a\n"; // pick the answer to change (so 1->a)
  _is << "c\n"; // set new value (should now be 1->c and 2 still maps to 'b')
  startQuiz();
  size_t found{};
  for (std::string line; std::getline(_os, line);) {
    if (!found) {
      if (line.ends_with("1->a 2->b")) ++found; // before edit
    } else if (line.ends_with("1->c 2->b"))     // after edit
      ++found;
  }
  EXPECT_EQ(found, 2);
}

TEST_F(GroupQuizTest, RefreshAfterAnswer) {
  meaningQuiz();
  _is << "a\n"; // provide an answer for the first group entry
  _is << "'\n"; // refresh - will update the screen with '1->a:'
  startQuiz();
  size_t found{};
  for (std::string line; std::getline(_os, line);) {
    if (line.starts_with("   1:  ")) {
      if (!found)
        ++found;
      else if (line.find("1->a:") != std::string::npos)
        ++found;
    }
  }
  EXPECT_EQ(found, 2);
}

TEST_F(GroupQuizTest, PatternGroupBuckets) {
  const auto f{[this](char x) {
    _is << "t\nb\np\n4\n" << x << "\n";
    std::string line;
    getFirstQuestion(line);
    return line.substr(9);
  }};
  EXPECT_EQ(f('1'), "1/85:  [阿：ア], 3 members");
  EXPECT_EQ(f('2'), "1/269:  [華：カ], 5 members");
  EXPECT_EQ(f('3'), "1/286:  [差：サ], 9 members");
  EXPECT_EQ(f('4'), "1/143:  [朶：タ], 2 members");
  EXPECT_EQ(f('5'), "1/144:  [巴：ハ、ヒ], 8 members");
  EXPECT_EQ(f('6'), "1/111:  [耶：ヤ], 4 members");
}

TEST_F(GroupQuizTest, QuizDefaults) {
  _is << "t\nb\np\n2\n1\n";
  std::string line, lineWithDefaults;
  getFirstQuestion(line);
  EXPECT_EQ(
      line.substr(9), "1/37:  [亜：ア、アク], showing 2 out of 3 members");
  // check default 'member filter' is '2' and the default 'bucket' is '1'
  _is << "t\nb\np\n\n\n";
  getFirstQuestion(lineWithDefaults);
  EXPECT_EQ(line, lineWithDefaults);
}

TEST_F(GroupQuizTest, QuizReview) {
  for (auto& i :
      {std::pair{'p', "1:  華.  (huá)     m:24        :  カ、（ケ）、はな"},
          std::pair{'m', "1:  北.  (běi)     p:897       :  ホク、きた"}}) {
    _is << "r\nb\n";
    if (i.first == 'p') _is << "2\n"; // choose 'カ' pattern group bucket
    startQuiz(i.first, '4');
    for (std::string line; std::getline(_os, line);)
      if (line.ends_with(i.second)) break;
    EXPECT_FALSE(_os.eof()) << "line not found: " << i.second;
  }
}

TEST_F(GroupQuizTest, ReviewNextPrev) {
  // move forward twice (.) and then back twice (,)
  _is << "r\nb\n.\n.\n,\n,\n";
  startQuiz('m', '4');
  size_t found{};
  std::string line;
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

} // namespace kanji_tools
