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
  void start(const List&, int numberOfChoicesPerQuestion, char quizStyle);

  const int _infoFields;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_LIST_QUIZ_H
