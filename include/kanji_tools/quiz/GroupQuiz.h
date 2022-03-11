#pragma once

#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

class GroupQuiz : public Quiz {
public:
  // 'MemberType' is used to determine which members of a group should be
  // included in a quiz:
  // - Jouyou: include if member is one of the standard 2,136 Jōyō kanji
  // - JLPT: include if member is Jōyō or JLPT (N5 - N2 are all Jōyō, but N1
  //   also contains 251 Jinmeiyō kanji)
  // - Frequency: include if member is Jōyō or JLPT or in the Top 2501 Frequency
  //   list (adds another 294 kanji)
  // - All: include all members (as long as they have readings)
  enum MemberType { Jouyou, JLPT, Frequency, All };

  GroupQuiz(const QuizLauncher&, size_t question, bool showMeanings,
            const GroupData::List&, MemberType);
private:
  // 'GroupEntryWidth' is the width required for 'qualified name', 'pinyin' and
  // 'other group name'
  enum Values { PinyinWidth = 12, GroupEntryWidth = 22 };

  // 'getGroupType' returns the type of first group in the given list (the list
  // should all have the same type)
  [[nodiscard]] static GroupType getGroupType(const GroupData::List&);

  // 'includeMember' returns true if a member can be included in group quiz
  // question. The member must have a reading as well as meet the criteria of
  // the given MemberType.
  [[nodiscard]] static bool includeMember(const Entry&, MemberType);

  // 'addPinyin' adds optional pinyin for 'kanji' to 's' padded to 'PinyinWidth'
  static void addPinyin(const Entry& kanji, std::string& s);

  // 'addOtherGroupName' is used in review mode to show other groups that 'name'
  // may belong to. 'z:y' is optionally added to 's' where 'x' is either 'm' or
  // 'p' (based on opposite of _groupType) and 'y' is the number of the other
  // group that contains 'name'. For example, while reviewing 'meaning groups'
  // values like 'p:123' will be displayed if 'name' is a member of 'pattern
  // group' number 123.
  void addOtherGroupName(const std::string& name, std::string& s) const;

  void start(const GroupData::List&, MemberType);

  // 'printAssignedAnswers' prints all currently assigned choices on one line in
  // the form: 1->a, 2->c, ...
  void printAssignedAnswers() const;

  // 'printAssignedAnswer' prints ' x->' if 'choice' is assigned to entry 'x',
  // otherwise prints 4 spaces
  std::ostream& printAssignedAnswer(char choice) const;

  void showGroup(const List& questions, const List& readings, Choices&,
                 bool repeatQuestion) const;
  [[nodiscard]] bool getAnswers(size_t totalQuestions, Choices&,
                                bool& skipGroup, bool& stopQuiz);
  [[nodiscard]] bool getAnswer(Choices&, bool& skipGroup, bool& refresh);
  void editAnswer(Choices&);
  [[nodiscard]] size_t getAnswerToEdit() const;
  void checkAnswers(const List& questions, const List& readings,
                    const std::string& name);

  // '_answers' holds answers for the current question, i.e., the reading
  // selected for each group member
  std::vector<char> _answers;

  const GroupType _groupType;
};

} // namespace kanji_tools
