#ifndef KANJI_TOOLS_QUIZ_QUIZ_LAUNCHER_H
#define KANJI_TOOLS_QUIZ_QUIZ_LAUNCHER_H

#include <kanji_tools/quiz/GroupData.h>
#include <kanji_tools/quiz/JukugoData.h>
#include <kanji_tools/utils/Choice.h>

namespace kanji_tools {

// 'QuizLauncher' is a class that will either start a quiz/review or print info about a kanji based on
// command line args passed in to the constructor.
class QuizLauncher {
public:
  using Choices = Choice::Choices;
  using OptChar = Choice::OptChar;
  using Entry = Data::Entry;
  using List = Data::List;

  static constexpr char QuitOption = '/';

  // An istream 'in' can be provided for testing purposes (instead of reading std::cin) and
  // if given, 'start' must be explicitly called to start a quiz.
  QuizLauncher(int argc, const char** argv, DataPtr, std::istream* in = 0);

  // 'start' is the top level method for starting a quiz or doing a review (List or Group based).
  // 'quizType' can be 'f', 'g', 'k', 'l', 'm' or 'p' for the type of quiz/review and 'questionList'
  // can also be provided (values depend on quiz type - see Quiz.cpp 'HelpMessage' for details).
  void start(OptChar quizType, OptChar questionList, int question = 0, bool showMeanings = false);

  auto& log(bool heading = false) const { return data().log(heading); }
  auto& out() const { return data().out(); }

  enum class ProgramMode { Review, Test, NotAssigned };
  enum class QuestionOrder { FromBeginning, FromEnd, Random, NotAssigned };

  auto isTestMode() const { return _programMode == ProgramMode::Test; }
  auto questionOrder() const { return _questionOrder; }
  auto& choice() const { return _choice; }
  auto isQuit(char c) const { return _choice.isQuit(c); }
  auto& groupData() const { return _groupData; }

  void printExtraTypeInfo(const Entry&) const;
  void printLegend(int infoFields = Kanji::AllFields) const;
  void printMeaning(const Entry&, bool useNewLine = false, bool showMeaning = true) const;
  void printReviewDetails(const Entry&) const;
private:
  enum Values { JukugoPerLine = 3, MaxJukugoLength = 30 };

  const Data& data() const { return _groupData.data(); }

  void startListQuiz(int question, bool showMeanings, int excludeField, const List&) const;
  void startGroupQuiz(int question, bool showMeanings, OptChar questionList, const GroupData::List& list) const;

  // 'processProgramModeArg' is called for '-r' and '-t' args and sets '_programMode'. It can also
  // set '_questionOrder' depending on the value of 'arg' and returns the question to start from.
  int processProgramModeArg(const std::string& arg);

  // 'processKanjiArg' is called when a kanji arg is passed to the program (see 'HelpMessage' in QuizLauncher.cpp)
  void processKanjiArg(const std::string& arg) const;

  // 'printDetails' prints info about a kanji provided on the command line (instead of running a quiz)
  void printDetails(const Data::List&, const std::string& name, const std::string& arg) const;
  void printDetails(const std::string&, bool showLegend = true) const;

  bool getQuestionOrder();

  void printJukugoList(const std::string& name, const JukugoData::List&) const;

  // '_programMode' and '_questionOrder' can be set via the command line, otherwise they are obtained interactively
  ProgramMode _programMode;
  QuestionOrder _questionOrder;

  const Choice _choice;
  const GroupData _groupData;
  const JukugoData _jukugoData;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_QUIZ_LAUNCHER_H
