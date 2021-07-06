#ifndef KANJI_KANJI_QUIZ
#define KANJI_KANJI_QUIZ

#include <kanji/KanjiData.h>

namespace kanji {

// forward declares
class Group;
enum class GroupType;

class KanjiQuiz : public KanjiData {
public:
  KanjiQuiz(int argc, const char** argv);
private:
  using GroupEntry = std::shared_ptr<Group>;
  using GroupMap = std::map<std::string, GroupEntry>;
  using GroupList = std::vector<GroupEntry>;
  static bool checkInsert(const std::string&, GroupMap&, const GroupEntry&);
  // 'loadGroups' loads from '-groups.txt' files
  void loadGroup(const std::filesystem::path&, GroupMap&, GroupList&, GroupType);
  void printGroups(const GroupMap&, const GroupList&) const;

  // 'quiz' is the top level method for choosing quiz type (List or Group based)
  void quiz() const;

  enum class ListOrder { FromBeginning, FromEnd, Random };
  static ListOrder getListOrder();
  // 'Choices' should map 'char' choices to a description of the choice
  using Choices = std::map<char, std::string>;
  using Answers = std::vector<char>;
  // 'getChoice' will prompt the use to enter one of the choices in the 'choices' structure.
  // If an optional default choice is provided it must correspond to an entry in 'choices'.
  static char getChoice(const std::string& msg, const Choices& choices) { return getChoice(msg, choices, {}); }
  static char getChoice(const std::string& msg, const Choices& choices, std::optional<char> def);
  static void addChoices(std::string& prompt, const Choices& choices);
  static void finalScore(int questionsAnswered, int score, const FileList::List& mistakes);

  // List type quiz
  static void quiz(ListOrder listOrder, const List&, bool printFrequency, bool printGrade, bool printLevel);

  // Group type quiz
  static void quiz(ListOrder listOrder, const GroupList&);
  // 'MemberType' if used to determine which members of a group should be included in a quiz:
  // - Jouyou: include if member is a Jouyou type
  // - JLPT: include if member is Jouyou or JLPT (there are 251 non-Jouyou kanji in JLPT)
  // - Frequency: include if the member is Jouyou or JLPT or in the Top Frequency
  // - All: include all members (as long as they have readings)
  enum MemberType { Jouyou = 0, JLPT, Frequency, All };
  static bool includeMember(const Entry&, MemberType);
  static void quiz(const GroupList&, MemberType);
  static bool getAnswer(Answers&, Choices&, bool& skipGroup);
  static void editAnswer(Answers&, Choices&);

  // '_meaningGroups' and '_meaningGroupList' are populated from 'meaning-groups.txt' and
  // '_patternGroups' and '_patternGroupList' are populated from 'pattern-groups.txt. The
  // maps have an entry for each kanji to its group so currently a kanji can't be in more
  // than one group per group type.
  GroupMap _meaningGroups;
  GroupMap _patternGroups;
  GroupList _meaningGroupList;
  GroupList _patternGroupList;
};

} // namespace kanji

#endif // KANJI_KANJI_QUIZ
