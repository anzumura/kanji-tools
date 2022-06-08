#pragma once

#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools { /// \quiz_group{GroupQuiz}
/// GroupQuiz class

/// class for creating and running a 'group quiz' \quiz{GroupQuiz}
///
/// Currently there are two types of group quizzes: 'meaning' and 'pattern'. The
/// quiz consists of one question per group that has two or more members (after
/// any filtering is done based on #MemberType). Each question shows the Kanji
/// members of the group on the left and a list of numbered 'readings' on the
/// right and the goal is to match the Kanji with their correct readings.
///
/// Since there are many 'pattern' groups, the user must choose a subset based
/// on an 'On reading' range (like 'ア', 'カ', etc..). Later this should also be
/// done for 'meaning' groups, but perhaps by categories instead (like '時間',
/// '地理', etc..). A 'curses' interface would also be nice instead of requiring
/// the user to choose a 'refresh' option to see the mappings chosen so far.
class GroupQuiz : public Quiz {
public:
  /// used to determine which members of a group should be included in a quiz
  enum class MemberType {
    Jouyou,    ///< standard 2,136 Jōyō kanji
    JLPT,      ///< is Jouyou or has a JLPT level (adds 251 JinmeiKanji)
    Frequency, ///< is Jouyou, has a JLPT level or has a 'frequency' (adds 294)
    All        ///< include any Kanji as long as it has at least one 'reading'
  };

  /// create a GroupQuiz
  /// \param launcher reference to QuizLauncher (holds common data for quizzes)
  /// \param question where to start from, `0` means prompt the user
  /// \param showMeanings true indicates English meanings should be shown
  /// \param list questions for the quiz (or review)
  /// \param memberType controls which Kanji to include from the groups
  /// \throw DomainError if `list` is empty or `question` is > `list.size()`
  GroupQuiz(const QuizLauncher& launcher, Question question, bool showMeanings,
      const GroupData::List& list, MemberType memberType);
private:
  using Bucket = std::optional<size_t>;

  static constexpr auto PinyinWidth{12}, ///< width of 'pinyin'
      GroupEntryWidth{22}; ///< width of left side of group detail display

  /// return type of first group `list` (they all have the same type by design)
  [[nodiscard]] static GroupType getGroupType(const GroupData::List& list);

  /// adds optional pinyin for `kanji` to `s` padded to #PinyinWidth
  static void addPinyin(const Kanji& kanji, String& s);

  /// return true if a member can be included in group quiz question. The member
  /// must have a reading as well as meet the criteria '_memberType'
  [[nodiscard]] bool includeMember(const Kanji&) const;

  [[nodiscard]] GroupData::List prepareList(
      const GroupData::List&, Bucket = {}) const;

  /// used in review mode to show other groups that `name` may belong to
  /// \details 'z:y' is optionally added to `s` where 'x' is either 'm' or 'p'
  /// ('meaning' or 'pattern') and 'y' is the other group number containing
  /// `name`. For example, while reviewing 'meaning groups', values like 'p:123'
  /// will be displayed if `name` is a member of 'pattern group' number 123.
  void addOtherGroupName(const String& name, String& s) const;

  void start(const GroupData::List&);

  /// prints all currently assigned choices on one line like: 1->a, 2->c, ...
  void printAssignedAnswers() const;

  /// prints ' x->' if `choice` is assigned, otherwise prints 4 spaces
  [[nodiscard]] std::ostream& printAssignedAnswer(char choice) const;

  void showGroup(const List&, const List&, Choices&, bool repeatQuestion) const;

  [[nodiscard]] bool getAnswers(
      size_t totalQuestions, Choices&, bool& skipGroup, bool& stopQuiz);
  [[nodiscard]] bool getAnswer(Choices&, bool& skipGroup, bool& refresh);

  void editAnswer(Choices&);
  [[nodiscard]] size_t getAnswerToEdit() const;

  void checkAnswers(
      const List& questions, const List& readings, const String& kanjiName);

  /// holds answers for the current question, i.e., the 'reading' selected for
  /// each group member
  std::vector<char> _answers;

  const GroupType _groupType;
  const MemberType _memberType;
};

/// \end_group
} // namespace kanji_tools
