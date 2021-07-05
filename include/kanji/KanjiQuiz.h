#ifndef KANJI_KANJI_QUIZ
#define KANJI_KANJI_QUIZ

#include <kanji/KanjiData.h>

namespace kanji {

class KanjiQuiz : public KanjiData {
public:
  KanjiQuiz(int argc, const char** argv);
private:
  enum class ListOrder { FromBeginning, FromEnd, Random };
  static ListOrder getListOrder();
  // 'Choices' should map 'char' choices to a description of the choice
  using Choices = std::map<char, std::string>;
  // 'getChoice' will prompt the use to enter one of the choices in the 'choices' structure.
  // If an optional default choice is provided it must correspond to an entry in 'choices'.
  static char getChoice(const std::string& msg, const Choices& choices) { return getChoice(msg, choices, {}); }
  static char getChoice(const std::string& msg, const Choices& choices, std::optional<char> def);
  void quiz() const;

  // List type quiz
  void quiz(ListOrder listOrder, const List&, bool printFrequency, bool printGrade, bool printLevel) const;

  // Group type quiz
  void quiz(ListOrder listOrder, const GroupList&) const;
  // 'MemberType' if used to determine which members of a group should be included in a quiz:
  // - Jouyou: include if member is a Jouyou type
  // - JLPT: include if member is Jouyou or JLPT (there are 251 non-Jouyou kanji in JLPT)
  // - Frequency: include if the member is Jouyou or JLPT or in the Top Frequency
  // - All: include all members (as long as they have readings)
  enum MemberType { Jouyou = 0, JLPT, Frequency, All };
  static bool includeMember(const Entry&, MemberType);
  void quiz(const GroupList&, MemberType) const;

  static void finalScore(int questionsAnswered, int score, const FileList::List& mistakes);
};

} // namespace kanji

#endif // KANJI_KANJI_QUIZ
