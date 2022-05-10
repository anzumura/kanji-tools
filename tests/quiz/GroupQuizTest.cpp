#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/GroupQuiz.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

class GroupQuizTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    _data = std::make_shared<KanjiData>(Args{}, _os, _es);
    _groupData = std::make_shared<GroupData>(_data);
    _jukugoData = std::make_shared<JukugoData>(_data);
  }

  // Constructs Quiz using the real data files
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
      QuizLauncher::OptChar questionList = {}, bool meanings = false,
      bool randomizeAnswers = true) {
    _os.str({});
    _es.str({});
    // clear eofbit and failbit for output streams in case quiz is run again
    _os.clear();
    _es.clear();
    // final input needs to be '/' to 'quit' the quiz, otherwise test code will
    // hang while quiz is waiting for more input.
    _is << "/\n";
    _quiz.start(quizType, questionList, {}, meanings, randomizeAnswers);
  }

  void getFirstQuestion(String& line, QuizLauncher::OptChar quizType = {},
      QuizLauncher::OptChar questionList = {}) {
    startQuiz(quizType, questionList);
    while (std::getline(_os, line))
      if (line.starts_with("Question 1/")) break;
    if (_os.eof()) FAIL() << "couldn't find first Question";
  }

  inline static std::stringstream _os, _es;
  inline static DataPtr _data;
  inline static GroupDataPtr _groupData;
  inline static JukugoDataPtr _jukugoData;

  [[nodiscard]] auto& is() { return _is; }
  [[nodiscard]] auto& quiz() { return _quiz; }
private:
  std::stringstream _is;
  QuizLauncher _quiz;
};

} // namespace

TEST_F(GroupQuizTest, ListOrders) {
  for (String lastLine; const auto i : {'b', 'e', 'r'}) {
    meaningQuiz(i);
    startQuiz();
    for (String line; std::getline(_os, line);) lastLine = line;
    // test the line sent to _os
    EXPECT_EQ(lastLine, "Final score: 0/0");
    // should be nothing sent to _es (for errors)
    EXPECT_FALSE(std::getline(_es, lastLine));
  }
}

TEST_F(GroupQuizTest, GroupKanjiTypes) {
  const auto f{[](int x = 0) {
    String msg{"37 members"};
    if (x) msg = ("showing " + std::to_string(x) += " out of ") += msg;
    return msg;
  }};
  for (auto i : {std::pair{'1', f(28)}, std::pair{'2', f(31)},
           std::pair{'3', f(32)}, std::pair{'4', f()}}) {
    // t=test mode, b=beginning of list, and .=skip to the next question (totals
    // in this test are for question 2 since it contains Kanji of all 4 types)
    is() << "t\nb\n.\n";
    startQuiz('m', i.first);
    EXPECT_NE(_os.str().find(", " + i.second), String::npos) << i.second;
  }
}

TEST_F(GroupQuizTest, CorrectResponse) {
  for (const auto meanings : {false, true}) {
    is() << "t\nb\n1\na\nb\n";
    startQuiz('p', '1', meanings, false);
    auto found{false};
    String lastLine;
    for (String line; std::getline(_os, line); lastLine = line)
      if (line.ends_with("Correct! (1/1)")) found = true;
    EXPECT_TRUE(found);
    EXPECT_EQ(lastLine, "Final score: 1/1 - Perfect!");
  }
}

TEST_F(GroupQuizTest, IncorrectResponse) {
  for (const auto meanings : {false, true}) {
    is() << "t\nb\n1\nb\na\n";
    startQuiz('p', '1', meanings, false);
    auto found{false};
    String lastLine;
    for (String line; std::getline(_os, line); lastLine = line)
      if (line.ends_with("Incorrect (got 0 right out of 2)")) found = true;
    EXPECT_TRUE(found);
    EXPECT_EQ(lastLine, "Final score: 0/1 - mistakes: 亜：ア、アク");
  }
}

TEST_F(GroupQuizTest, QuizWithEmptyList) {
  // should never happen with proper '-groups.txt' files
  const auto f{[this] {
    GroupQuiz{quiz(), {}, {}, {}, GroupQuiz::MemberType::All};
  }};
  EXPECT_THROW(call(f, "empty group list"), std::domain_error);
}

TEST_F(GroupQuizTest, SkipQuestions) {
  for (size_t i{2}; i < 4; ++i) {
    meaningQuiz();
    for (size_t j{}; j < i; ++j) skip();
    startQuiz();
    String line, lastLine;
    while (std::getline(_os, line)) lastLine = line;
    const auto skipped{std::to_string(i)};
    EXPECT_EQ(lastLine, "Final score: 0/" + skipped += ", skipped: " + skipped);
  }
}

TEST_F(GroupQuizTest, ToggleMeanings) {
  meaningQuiz();
  toggleMeanings(); // turn meanings on
  toggleMeanings(); // turn meanings off
  startQuiz();
  auto meaningsOn{false};
  size_t found{};
  const String expected{"みなみ"};
  const auto expectedWithMeaning{expected + " : south"};
  for (String line; std::getline(_os, line);) {
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
  is() << "a\n"; // provide an answer for the first group entry
  edit();
  is() << "b\n"; // change the answer from 'a' to 'b'
  startQuiz();
  size_t found{};
  for (String line; std::getline(_os, line);) {
    if (!found) {
      if (line.ends_with("1->a")) ++found;
    } else if (line.ends_with("1->b"))
      ++found;
  }
  EXPECT_EQ(found, 2);
}

TEST_F(GroupQuizTest, EditAfterMultipleAnswers) {
  meaningQuiz();
  is() << "a\nb\n"; // entry 1 maps to 'a' and 2 maps to 'b'
  edit();
  is() << "a\n"; // pick the answer to change (so 1->a)
  is() << "c\n"; // set new value (should now be 1->c and 2 still maps to 'b')
  startQuiz();
  size_t found{};
  for (String line; std::getline(_os, line);) {
    if (!found) {
      if (line.ends_with("1->a 2->b")) ++found; // before edit
    } else if (line.ends_with("1->c 2->b"))     // after edit
      ++found;
  }
  EXPECT_EQ(found, 2);
}

TEST_F(GroupQuizTest, RefreshAfterAnswer) {
  meaningQuiz();
  is() << "a\n"; // provide an answer for the first group entry
  is() << "'\n"; // refresh - will update the screen with '1->a:'
  startQuiz();
  size_t found{};
  for (String line; std::getline(_os, line);)
    if (line.starts_with("   1:  ") &&
        (!found || line.find("1->a:") != String::npos))
      ++found;
  EXPECT_EQ(found, 2);
}

TEST_F(GroupQuizTest, PatternGroupBuckets) {
  constexpr auto RemoveQuestionText{9};
  const auto f{[this](char x) {
    is() << "t\nb\np\n4\n" << x << "\n";
    String line;
    getFirstQuestion(line);
    return line.substr(RemoveQuestionText);
  }};
  EXPECT_EQ(f('1'), "1/85:  [阿：ア], 3 members");
  EXPECT_EQ(f('2'), "1/269:  [華：カ], 5 members");
  EXPECT_EQ(f('3'), "1/286:  [差：サ], 9 members");
  EXPECT_EQ(f('4'), "1/143:  [朶：タ], 2 members");
  EXPECT_EQ(f('5'), "1/144:  [巴：ハ、ヒ], 8 members");
  EXPECT_EQ(f('6'), "1/111:  [耶：ヤ], 4 members");
}

TEST_F(GroupQuizTest, LoopOverAllPatternsInABucket) {
  constexpr auto FirstBucketGroups{85};
  // specify enough '.'s to loop through all the groups in the first bucket to
  // complete the quiz (and test that '/' from 'startQuiz' is still on _is).
  is() << "r\nb\n1\n";
  for (auto _{FirstBucketGroups}; _--;) is() << ".\n";
  startQuiz('p', '4');
  String leftOverInput;
  std::getline(is(), leftOverInput);
  EXPECT_EQ(leftOverInput, "/");
}

TEST_F(GroupQuizTest, QuizDefaults) {
  is() << "t\nb\np\n2\n1\n";
  String line, lineWithDefaults;
  getFirstQuestion(line);
  EXPECT_EQ(
      line.substr(9), "1/37:  [亜：ア、アク], showing 2 out of 3 members");
  // check default 'member filter' is '2' and the default 'bucket' is '1'
  is() << "t\nb\np\n\n\n";
  getFirstQuestion(lineWithDefaults);
  EXPECT_EQ(line, lineWithDefaults);
}

TEST_F(GroupQuizTest, QuizReview) {
  for (auto& i :
      {std::pair{'p', "1:  華.  (huá)     m:24        :  カ、（ケ）、はな"},
          std::pair{'m', "1:  北.  (běi)     p:897       :  ホク、きた"}}) {
    is() << "r\nb\n";
    if (i.first == 'p') is() << "2\n"; // choose 'カ' pattern group bucket
    startQuiz(i.first, '4');
    for (String line; std::getline(_os, line);)
      if (line.ends_with(i.second)) break;
    EXPECT_FALSE(_os.eof()) << "line not found: " << i.second;
  }
}

TEST_F(GroupQuizTest, ReviewNextPrev) {
  // move forward twice (.) and then back twice (,)
  is() << "r\nb\n.\n.\n,\n,\n";
  startQuiz('m', '4');
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

} // namespace kanji_tools
