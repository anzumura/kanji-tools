#ifndef KANJI_TOOLS_QUIZ_LIST_QUIZ_H
#define KANJI_TOOLS_QUIZ_LIST_QUIZ_H

#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

class ListQuiz : public Quiz {
public:
  enum class QuizStyle { KanjiToReading, ReadingToKanji };
  static QuizStyle toQuizStyle(char c) { return c == 'k' ? QuizStyle::KanjiToReading : QuizStyle::ReadingToKanji; }

  // 'infoFields' controls which fields are shown in a 'kanji to reading' quiz (see Kanji.h for more
  // details on 'InfoFields') and 'choiceCount' specifies the number of choices per question (2 to 9).
  ListQuiz(const QuizLauncher&, int question, bool showMeanings, const List&, int infoFields, int choiceCount,
           QuizStyle);
private:
  void start(const List&);

  bool isKanjiToReading() const { return _quizStyle == QuizStyle::KanjiToReading; }

  using Answers = std::map<int, int>;

  int populateAnswers(const Entry&, Answers&, const List& questions) const;
  void printQuestion(const Entry&) const;
  void printChoices(const Entry&, Choices&, const List& questions, const Answers&) const;

  // 'getAnswer' prompts user for an answer and processes the result. This method only returns 'false' if the
  // question should be repeated (for toggling meanings), otherwise it returns 'true'.
  bool getAnswer(const std::string& prompt, Choices&, bool& stopQuiz, int correctChoice, const std::string& name);

  const int _infoFields;
  const int _choiceCount;
  const QuizStyle _quizStyle;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_LIST_QUIZ_H
