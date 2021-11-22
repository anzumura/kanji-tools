#ifndef KANJI_TOOLS_QUIZ_QUIZ_H
#define KANJI_TOOLS_QUIZ_QUIZ_H

#include <kanji_tools/quiz/GroupData.h>
#include <kanji_tools/quiz/JukugoData.h>
#include <kanji_tools/utils/Choice.h>

namespace kanji_tools {

class Quiz {
public:
  // An istream 'in' can be provided for testing purposes (instead of reading std::cin) and
  // if given, 'start' must be explicitly called to start a quiz.
  Quiz(int argc, const char** argv, DataPtr, std::istream* in = 0);
  // 'start' is the top level method for starting a quiz or doing a review (List or Group based)
  void start() const;
private:
  enum Values { JukugoPerLine = 3, MaxJukugoLength = 30 };
  // 'printDetails' prints info about a kanji provided on the command line (instead of running a quiz)
  void printDetails(const std::string&) const;

  const Data& data() const { return _groupData.data(); }
  std::ostream& out() const { return data().out(); }
  std::ostream& log(bool heading = false) const { return data().log(heading); }

  enum class ListOrder { FromBeginning, FromEnd, Random, Quit };
  ListOrder getListOrder() const;
  void finalScore() const;
  void reset() const;
  using Choices = Choice::Choices;
  using Entry = Data::Entry;
  using List = Data::List;
  // 'getDefaultChoices' returns a Choices structure populated with just the common values
  // for a quiz question like skip and quit. It will also populate 'hide/show meanings' option
  // based on the current value of '_showMeanings'.
  Choices getDefaultChoices(int totalQuestions) const;
  // display of English 'meanings' can be toggled on and off
  void toggleMeanings(Choices&) const;
  // print meaning if _showMeanings is true and meaning exists
  void printMeaning(const Entry&, bool useNewLine = false) const;

  void printReviewDetails(const Entry&) const;

  // 'listQuiz' starts a 'list based quiz'. 'infoFields' controls which fields are shown in a 'kanji
  // to reading' quiz (see Kanji.h for more details on 'InfoFields').
  void listQuiz(ListOrder listOrder, const List&, int infoFields) const;

  void prepareGroupQuiz(ListOrder, const GroupData::List&, const GroupData::Map& otherMap, char otherGroup) const;
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
  void groupQuiz(const GroupData::List&, MemberType, const GroupData::Map& otherMap, char otherGroup) const;

  void showGroup(const List& questions, const List& readings, Choices&, bool repeatQuestion,
                 const GroupData::Map& otherMap, char otherGroup) const;
  using Answers = std::vector<char>;
  bool getAnswers(Answers&, int totalQuestions, Choices&, bool& skipGroup, bool& stopQuiz) const;
  bool getAnswer(Answers&, Choices&, bool& skipGroup, bool& meanings) const;
  void editAnswer(Answers&, Choices&) const;
  void checkAnswers(const Answers&, const List& questions, const List& readings, const std::string& name) const;

  // used to track progress in quiz:
  mutable int _question;
  mutable int _score;
  mutable DataFile::List _mistakes;
  mutable bool _showMeanings;
  mutable Choice _choice;
  mutable bool _reviewMode;

  const GroupData _groupData;
  const JukugoData _jukugoData;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_QUIZ_H
