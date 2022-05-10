#pragma once

#include <kanji_tools/kana/Choice.h>
#include <kanji_tools/quiz/GroupData.h>
#include <kanji_tools/quiz/JukugoData.h>

namespace kanji_tools {

// 'QuizLauncher' is a class that will either start a quiz/review or print info
// about a kanji based on command line args passed in to the constructor.
class QuizLauncher {
public:
  using Choices = Choice::Choices;
  using OptChar = Choice::OptChar;
  using KanjiList = Data::KanjiList;
  using Question = uint16_t;

  static constexpr char QuitOption{'/'};

  // An istream 'in' can be provided for testing purposes (instead of reading
  // std::cin) and if given, 'start' must be explicitly called to start a quiz.
  QuizLauncher(const Args&, const DataPtr&, const GroupDataPtr&,
      const JukugoDataPtr&, std::istream* in = {});

  QuizLauncher(const QuizLauncher&) = delete;

  // 'start' is the top level method for starting a quiz or doing a review (List
  // or Group based). 'quizType' can be 'f', 'g', 'k', 'l', 'm' or 'p' for the
  // type of quiz/review and 'questionList' can also be provided (values depend
  // on quiz type - see Quiz.cpp 'HelpMessage' for details).
  void start(OptChar quizType, OptChar qList, Question question = 0,
      bool showMeanings = false, bool randomizeAnswers = true);

  [[nodiscard]] std::ostream& log(bool heading = false) const;
  [[nodiscard]] auto& out() const { return data().out(); }

  enum class ProgramMode { Review, Test, NotAssigned };
  enum class QuestionOrder { FromBeginning, FromEnd, Random, NotAssigned };

  [[nodiscard]] bool isTestMode() const;
  [[nodiscard]] auto questionOrder() const { return _questionOrder; }
  [[nodiscard]] auto& choice() const { return _choice; }
  [[nodiscard]] auto isQuit(char c) const { return _choice.isQuit(c); }
  [[nodiscard]] auto& groupData() const { return _groupData; }
  [[nodiscard]] auto randomizeAnswers() const { return _randomizeAnswers; }

  void printExtraTypeInfo(const Kanji&) const;
  void printLegend(KanjiInfo fields = KanjiInfo::All) const;
  void printMeaning(
      const Kanji&, bool useNewLine = false, bool showMeaning = true) const;
  void printReviewDetails(const Kanji&) const;
private:
  static constexpr uint16_t JukugoPerLine{3}, MaxJukugoSize{30};

  [[nodiscard]] DataRef data() const { return _groupData->data(); }

  void startListQuiz(Question question, bool showMeanings,
      KanjiInfo excludeField, const KanjiList&) const;
  void startGroupQuiz(Question question, bool showMeanings, OptChar qList,
      const GroupData::List& list) const;

  [[nodiscard]] static Kanji::NelsonId getId(
      const String& msg, const String& arg);

  // 'setQuizType' is called for -f, -g, -l, -k, -m and -p args (so ok to assume
  // size is at least 2) and sets 'quizType'. It also returns optional 'question
  // list' that may be part of the arg, i.e., '-g6' arg represents 'Grade' '6'.
  [[nodiscard]] static OptChar setQuizType(OptChar& quizType, const String& arg,
      const Choices&, const std::optional<Choice::Range>& = {});

  // top level function for processing args (program mode and quiz types)
  [[nodiscard]] OptChar processArg(Question&, OptChar&, const String& arg);

  // 'processProgramModeArg' is called for '-r' and '-t' args and sets
  // '_programMode'. It can also set '_questionOrder' depending on the value
  // of 'arg' and returns the question to start from.
  [[nodiscard]] Question processProgramModeArg(const String& arg);

  // 'processKanjiArg' is called when a kanji arg is passed to the program
  // (see 'HelpMessage' in QuizLauncher.cpp)
  void processKanjiArg(const String& arg) const;

  // 'printDetails' prints info about a kanji provided on the command line
  // (instead of running a quiz)
  void printDetails(
      const Data::KanjiList&, const String& name, const String& arg) const;
  void printDetails(const String&, bool showLegend = true) const;

  [[nodiscard]] bool getQuestionOrder();

  void printJukugo(const Kanji&) const;
  void printJukugoList(const String& name, const JukugoData::List&) const;

  [[nodiscard]] char chooseQuizType(OptChar) const;

  [[nodiscard]] char chooseFreq(OptChar) const;
  [[nodiscard]] char chooseGrade(OptChar) const;
  [[nodiscard]] char chooseKyu(OptChar) const;
  [[nodiscard]] char chooseLevel(OptChar) const;

  [[nodiscard]] const Data::KanjiList& getKyuList(char) const;

  // '_programMode' and '_questionOrder' can be set via the command line,
  // otherwise they are obtained interactively
  ProgramMode _programMode;
  QuestionOrder _questionOrder;
  bool _randomizeAnswers;

  const Choice _choice;
  const GroupDataPtr _groupData;
  const JukugoDataPtr _jukugoData;
};

} // namespace kanji_tools
