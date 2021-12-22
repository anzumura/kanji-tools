#ifndef KANJI_TOOLS_QUIZ_GROUP_QUIZ_H
#define KANJI_TOOLS_QUIZ_GROUP_QUIZ_H

#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

class GroupQuiz : public Quiz {
public:
  // 'MemberType' is used to determine which members of a group should be included in a quiz:
  // - Jouyou: include if member is one of the standard 2,136 Jōyō kanji
  // - JLPT: include if member is Jōyō or JLPT (N5 - N2 are all Jōyō, but N1 also contains 251 Jinmeiyō kanji)
  // - Frequency: include if member is Jōyō or JLPT or in the Top 2501 Frequency list (adds another 294 kanji)
  // - All: include all members (as long as they have readings)
  enum MemberType { Jouyou = 0, JLPT, Frequency, All };

  GroupQuiz(const QuizLauncher&, int question, bool showMeanings, const GroupData::List&, char otherGroup, MemberType);
private:
  // 'GroupEntryWidth' is the width required for 'qualified name', 'pinyin' and 'other group name'
  enum Values { PinyinWidth = 12, GroupEntryWidth = 22 };

  // 'includeMember' returns true if a member can be included in group quiz question. The member must
  // have a reading as well as meet the criteria of the given MemberType.
  static bool includeMember(const Entry&, MemberType);

  // 'addPinyin' adds optional pinyin for 'kanji' to 's' padded to 'PinyinWidth'
  static void addPinyin(const Entry& kanji, std::string& s);

  // 'addOtherGroupName' is used in review mode to show other groups that 'name' may belong to. 'z:y' is
  // optionally added to 's' where 'x' is the value of '_otherGroup' (so either 'm' or 'p') and 'y' is
  // the number of the other group that contains 'name'. For example, while reviewing 'meaning groups'
  // values like 'p:123' will be displayed if 'name' is a member of 'pattern group' number 123.
  void addOtherGroupName(const std::string& name, std::string& s) const;

  void start(const GroupData::List&, MemberType);

  using Answers = std::vector<char>;

  // 'printAssignedAnswers' prints all currently assigned choices on one line in the form: 1->a, 2->c, ...
  void printAssignedAnswers(const Answers&) const;

  // 'printAssignedAnswer' prints ' x->' if 'choice' is assigned to entry 'x', otherwise prints 4 spaces
  std::ostream& printAssignedAnswer(const Answers&, char choice) const;

  void showGroup(const List& questions, const Answers&, const List& readings, Choices&, bool repeatQuestion) const;
  bool getAnswers(Answers&, int totalQuestions, Choices&, bool& skipGroup, bool& stopQuiz);
  bool getAnswer(Answers&, Choices&, bool& skipGroup, bool& refresh);
  void editAnswer(Answers&, Choices&);
  int getAnswerToEdit(const Answers&) const;
  void checkAnswers(const Answers&, const List& questions, const List& readings, const std::string& name);

  const char _otherGroup;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_GROUP_QUIZ_H
