#pragma once

#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

class ListQuiz : public Quiz {
public:
  using ChoiceCount = uint16_t;

  enum class QuizStyle { KanjiToReading, ReadingToKanji };
  static QuizStyle toQuizStyle(char);

  // 'fields' controls which fields are shown in a 'kanji to reading' quiz (see
  // Kanji.h for more details on 'KanjiInfo') and 'choiceCount' specifies the
  // number of choices per question (2 to 9).
  ListQuiz(const QuizLauncher&, Question, bool showMeanings, const KanjiList&,
      Kanji::Info, ChoiceCount, QuizStyle);
private:
  void start(const KanjiList&);

  [[nodiscard]] const String& getPrompt() const;
  [[nodiscard]] bool isKanjiToReading() const;

  // populates '_answers' and returns the position corresponding to the current
  // question, i.e., the correct answer for the given Kanji.
  [[nodiscard]] ChoiceCount populateAnswers(
      const Kanji&, const KanjiList& questions);

  void printQuestion(const Kanji&) const;
  void printChoices(const Kanji&, const KanjiList& questions) const;

  // 'getAnswer' prompts for an answer and processes the result. This method
  // only returns 'false' if the question should be repeated (for toggling
  // meanings), otherwise it returns 'true'.
  [[nodiscard]] bool getAnswer(Choices& choices, bool& stopQuiz,
      ChoiceCount correct, const String& name);

  // '_answers' contains '_choiceCount' entries and is repopulated for each
  // question (one index has the correct answer and others are randomly chosen
  // from the full question list).
  std::vector<Question> _answers;

  const Kanji::Info _infoFields;
  const ChoiceCount _choiceCount;
  const QuizStyle _quizStyle;
  const String _prompt;
  const char _choiceEnd; // '0' + _choiceCount
};

} // namespace kanji_tools
