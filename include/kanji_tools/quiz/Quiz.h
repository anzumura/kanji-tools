#ifndef KANJI_TOOLS_QUIZ_QUIZ_H
#define KANJI_TOOLS_QUIZ_QUIZ_H

#include <kanji_tools/quiz/GroupData.h>
#include <kanji_tools/quiz/JukugoData.h>
#include <kanji_tools/utils/Choice.h>

namespace kanji_tools {

class Quiz {
public:
  using OptChar = std::optional<char>;

  // An istream 'in' can be provided for testing purposes (instead of reading std::cin) and
  // if given, 'start' must be explicitly called to start a quiz.
  Quiz(int argc, const char** argv, DataPtr, std::istream* in = 0);

  // 'start' is the top level method for starting a quiz or doing a review (List or Group based).
  // 'quizType' can be 'f', 'g', 'k', 'l', 'm' or 'p' for the type of quiz/review and 'questionList'
  // can also be provided (values depend on quiz type - see Quiz.cpp 'HelpMessage' for details).
  void start(OptChar quizType, OptChar questionList);
private:
  enum Values { JukugoPerLine = 3, MaxJukugoLength = 30 };

  const Data& data() const { return _groupData.data(); }
  std::ostream& out() const { return data().out(); }
  std::ostream& log(bool heading = false) const { return data().log(heading); }

  enum class ListOrder { FromBeginning, FromEnd, Random, NotAssigned };
  enum class Mode { Review, Test, NotAssigned };

  void parseModeArg(const std::string& arg);
  void parseKanjiArg(const std::string& arg) const;

  // 'printDetails' prints info about a kanji provided on the command line (instead of running a quiz)
  void printDetails(const Data::List&, const std::string& name, const std::string& arg) const;
  void printDetails(const std::string&, bool showLegend = true) const;

  bool getListOrder();
  void reset();

  std::ostream& beginQuizMessage(int totalQuestions); // can modify '_questions'
  std::ostream& beginQuestionMessage(int totalQuestions) const;
  bool isTestMode() const { return _mode == Mode::Test; }
  void finalScore() const;

  using Choices = Choice::Choices;
  using Entry = Data::Entry;
  using List = Data::List;

  // 'getDefaultChoices' returns a Choices structure populated with just the common values
  // for a quiz question like skip and quit. It will also populate 'hide/show meanings' option
  // based on the current value of '_showMeanings'.
  Choices getDefaultChoices(int totalQuestions) const;

  // display of English 'meanings' can be toggled on and off
  void toggleMeanings(Choices&);

  // print meaning if _showMeanings is true and meaning exists
  void printMeaning(const Entry&, bool useNewLine = false) const;

  void printLegend(int infoFields = Kanji::AllFields) const;
  void printExtraTypeInfo(const Entry&) const;
  void printReviewDetails(const Entry&) const;
  void printJukugoList(const std::string& name, const JukugoData::List&) const;

  // 'listQuiz' starts a 'list based quiz'. 'infoFields' controls which fields are shown in a 'kanji
  // to reading' quiz (see Kanji.h for more details on 'InfoFields').
  void listQuiz(const List&, int infoFields);

  template<typename T>
  void prepareGroupQuiz(const GroupData::List&, const T& otherMap, char otherGroup, OptChar questionList);

  // 'MemberType' if used to determine which members of a group should be included in a quiz:
  // - Jouyou: include if member is a Jouyou type
  // - JLPT: include if member is Jouyou or JLPT (there are 251 non-Jouyou kanji in JLPT)
  // - Frequency: include if the member is Jouyou or JLPT or in the Top Frequency
  // - All: include all members (as long as they have readings)
  enum MemberType { Jouyou = 0, JLPT, Frequency, All };

  // 'includeMember' returns true if a member can be included in group quiz question. The member must
  // have a reading as well as meet the criteria of the given MemberType.
  static bool includeMember(const Entry&, MemberType);

  // 'groupQuiz' starts a Group Quiz (callled by 'prepareGroupQuiz')
  template<typename T> void groupQuiz(const GroupData::List&, MemberType, const T& otherMap, char otherGroup);

  using Answers = std::vector<char>;
  template<typename T>
  void showGroup(const List& questions, const Answers&, const List& readings, Choices&, bool repeatQuestion,
                 const T& otherMap, char otherGroup) const;
  bool getAnswers(Answers&, int totalQuestions, Choices&, bool& skipGroup, bool& stopQuiz);
  bool getAnswer(Answers&, Choices&, bool& skipGroup, bool& refresh);
  void editAnswer(Answers&, Choices&);
  void checkAnswers(const Answers&, const List& questions, const List& readings, const std::string& name);

  // '_listOrder' and '_mode' can be set via the command line, otherwise the user is prompted at the start
  ListOrder _listOrder;
  Mode _mode;

  // the following attributes are updated throughout the quiz
  Choice _choice;
  DataFile::List _mistakes;
  int _question;
  int _score;
  bool _showMeanings;

  const GroupData _groupData;
  const JukugoData _jukugoData;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_QUIZ_H
