#ifndef KANJI_TOOLS_QUIZ_LIST_QUIZ_H
#define KANJI_TOOLS_QUIZ_LIST_QUIZ_H

#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

class ListQuiz : public Quiz {
public:
  // 'infoFields' controls which fields are shown in a 'kanji to reading' quiz (see Kanji.h for more
  // details on 'InfoFields').
  ListQuiz(const QuizLauncher&, int question, bool showMeanings, const List&, int infoFields);
private:
  void start(const List&, int choiceCount, char quizStyle);

  using Answers = std::map<int, int>;

  int populateAnswers(const Entry&, Answers&, const List& questions, int choiceCount) const;
  void printQuestion(const Entry&, char quizStyle) const;
  void printChoices(const Entry&, Choices&, int choiceCount, char quizStyle, const List& questions,
                    const Answers&) const;

  // 'getAnswer' prompts user for an answer and processes the result. This method only returns 'false' if the
  // question should be repeated (for toggling meanings), otherwise it returns 'true'.
  bool getAnswer(const std::string& prompt, Choices&, bool& stopQuiz, int correctChoice, const std::string& name);

  const int _infoFields;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_LIST_QUIZ_H
