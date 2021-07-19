#ifndef KANJI_QUIZ_H
#define KANJI_QUIZ_H

#include <kanji/Choice.h>
#include <kanji/GroupData.h>

namespace kanji {

class Quiz {
public:
  Quiz(const GroupData& groupData, std::istream& in = std::cin)
    : _groupData(groupData), _question(0), _score(0), _showMeanings(false), _choice(groupData.out(), in) {}
  // 'quiz' is the top level method for choosing quiz type (List or Group based)
  void quiz() const;
private:
  const Data& data() const { return _groupData.data(); }
  std::ostream& out() const { return data().out(); }
  std::ostream& log(bool heading = false) const { return data().log(heading); }

  enum class ListOrder { FromBeginning, FromEnd, Random };
  ListOrder getListOrder() const;
  void finalScore() const;
  void reset() const;
  using Choices = Choice::Choices;
  using Entry = Data::Entry;
  using List = Data::List;
  // 'getDefaultChoices' returns a Choices structure populated with just the common values
  // for a quiz question like skip and quit. It will also populate 'hide/show meanings' option
  // based on the current value of '_showMeanings'.
  Choices getDefaultChoices() const;
  void toggleMeanings(Choices&) const;   // display of English 'meanings' can be toggled on and off
  void printMeaning(const Entry&) const; // print meaning if _showMeanings is true and meaning exists

  // 'listQuiz' starts a 'list based quiz'. 'infoFields' controls which fields are shown in a 'kanji
  // to reading' quiz (see Kanji.h for more details on 'InfoFields').
  void listQuiz(ListOrder listOrder, const List&, int infoFields) const;

  void prepareGroupQuiz(ListOrder listOrder, const GroupData::List&) const;
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
  void groupQuiz(const GroupData::List&, MemberType) const;

  void showGroup(const List& questions, const List& readings, Choices&, bool repeatQuestion) const;
  using Answers = std::vector<char>;
  bool getAnswers(Answers&, int totalQuestions, Choices&, bool& skipGroup, bool& stopQuiz) const;
  bool getAnswer(Answers&, Choices&, bool& skipGroup, bool& meanings) const;
  void editAnswer(Answers&, Choices&) const;
  void checkAnswers(const Answers&, const List& questions, const List& readings, const std::string& name) const;

  // used to track progress in quiz:
  mutable int _question;
  mutable int _score;
  mutable FileList::List _mistakes;
  mutable bool _showMeanings;

  const Choice _choice;
  const GroupData& _groupData;
};

} // namespace kanji

#endif // KANJI_QUIZ_H
