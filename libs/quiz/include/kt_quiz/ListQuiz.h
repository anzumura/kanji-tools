#pragma once

#include <kt_quiz/Quiz.h>

namespace kanji_tools { /// \quiz_group{ListQuiz}
/// ListQuiz class

/// class for creating and running a 'list quiz' \quiz{ListQuiz}
///
/// Currently there are four types of list quizzes: 'frequency', 'grade', 'kyu',
/// and 'level'. The user must choose a subset within each of these lists such
/// as 'l2' for a (JLPT) 'N2' quiz. This still results in some lists having over
/// 1,000 entries like 'gs' ('grade' Secondary School) or 'k1' (top Kentei 'kyu'
/// Kanji list). Random order can help (and stop in the middle) or use command
/// line options to start at a specific question like '-gs -t200' (start 'gs' at
/// question 200) - see README.md or QuizLauncher.cpp for more details.
class ListQuiz : public Quiz {
public:
  using ChoiceCount = uint16_t;

  /// style of quiz to run (doesn't affect 'review mode')
  enum class QuizStyle {
    KanjiToReading, ///< user must choose the correct 'reading' for a Kanji
    ReadingToKanji  ///< use must choose the correct Kanji for a 'reading'
  };

  /// if `c` is 'k' return 'KanjiToReading', otherwise return 'ReadingToKanji'
  static QuizStyle toQuizStyle(char c);

  /// create a ListQuiz
  /// \param launcher reference to QuizLauncher (holds common data for quizzes)
  /// \param question where to start from, `0` means prompt the user
  /// \param showMeanings true indicates English meanings should be shown
  /// \param list questions for the quiz (or review)
  /// \param fields controls which 'fields' are shown in 'Kanji to reading' quiz
  /// \param choiceCount number of choices per question (2 to 9)
  /// \param quizStyle which style of quiz to run (see #QuizStyle)
  ListQuiz(const QuizLauncher& launcher, Question question, bool showMeanings,
      const List& list, Kanji::Info fields, ChoiceCount choiceCount,
      QuizStyle quizStyle);
private:
  void start(const List&);

  [[nodiscard]] const String& getPrompt() const;
  [[nodiscard]] bool isKanjiToReading() const;

  /// populate #_answers and returns the position corresponding to the current
  /// question, i.e., the correct answer for the given Kanji.
  [[nodiscard]] ChoiceCount populateAnswers(
      const Kanji&, const List& questions);

  void printQuestion(const Kanji&) const;
  void printChoices(const Kanji&, const List& questions) const;

  /// prompt for an answer and processes results, return false if the question
  /// should be repeated (for toggling meanings), otherwise return true
  [[nodiscard]] bool getAnswer(Choices& choices, bool& stopQuiz,
      ChoiceCount correct, const String& name);

  /// one entry has the correct answer and others are randomly chosen from the
  /// full question list (repopulated for each question)
  std::vector<Question> _answers;

  const Kanji::Info _infoFields;
  const ChoiceCount _choiceCount;
  const QuizStyle _quizStyle;
  const String _prompt;
  const char _choiceEnd; ///< `0` + #_choiceCount
};

/// \end_group
} // namespace kanji_tools
