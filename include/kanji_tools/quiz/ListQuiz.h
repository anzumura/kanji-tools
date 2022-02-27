#pragma once

#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

class ListQuiz : public Quiz {
public:
  enum class QuizStyle { KanjiToReading, ReadingToKanji };
  static constexpr auto toQuizStyle(char c) {
    return c == 'k' ? QuizStyle::KanjiToReading : QuizStyle::ReadingToKanji;
  }

  // 'fields' controls which fields are shown in a 'kanji to reading' quiz (see
  // Kanji.h for more details on 'KanjiInfo') and 'choiceCount' specifies the
  // number of choices per question (2 to 9).
  ListQuiz(const QuizLauncher&, int question, bool showMeanings, const List&,
           KanjiInfo fields, int choiceCount, QuizStyle);
private:
  void start(const List&);

  [[nodiscard]] auto isKanjiToReading() const {
    return _quizStyle == QuizStyle::KanjiToReading;
  }

  // populates '_answers' and returns the position corresponding to the current
  // question, i.e., the correct answer for 'kanji'.
  [[nodiscard]] int populateAnswers(const Entry& kanji, const List& questions);

  void printQuestion(const Entry& kanji) const;
  void printChoices(const Entry& kanji, const List& questions) const;

  // 'getAnswer' prompts for an answer and processes the result. This method
  // only returns 'false' if the question should be repeated (for toggling
  // meanings), otherwise it returns 'true'.
  [[nodiscard]] bool getAnswer(Choices& choices, bool& stopQuiz,
                               int correctChoice, const std::string& name);

  // '_answers' contains '_choiceCount' entries and is repopulated for each
  // question (one index has the correct answer and others are randomly chosen
  // from the full question list).
  std::vector<int> _answers;

  const KanjiInfo _infoFields;
  const int _choiceCount;
  const QuizStyle _quizStyle;
  const std::string _prompt;
  const char _choiceEnd; // '0' + _choiceCount
};

} // namespace kanji_tools
