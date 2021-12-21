#ifndef KANJI_TOOLS_QUIZ_QUIZ_H
#define KANJI_TOOLS_QUIZ_QUIZ_H

#include <kanji_tools/quiz/QuizLauncher.h>

namespace kanji_tools {

// 'Quiz' is the base class for 'ListQuiz' and 'GroupQuiz'. It holds common data for a quiz including the
// current question number, correct response count and list of mistakes.
class Quiz {
protected:
  using Choices = QuizLauncher::Choices;
  using Entry = QuizLauncher::Entry;
  using List = QuizLauncher::List;
  using OptChar = QuizLauncher::OptChar;

  Quiz(const QuizLauncher& launcher, int question, bool showMeanings)
    : _launcher(launcher), _question(question), _correctAnswers(0), _showMeanings(showMeanings) {}

  // destructor prints the final score when in test mode
  virtual ~Quiz();

  // the following methods are shotcuts for calling '_launcher' methods
  char getChoice(const std::string& msg, const Choices& choices, OptChar def = std::nullopt) {
    return _launcher.choice().get(msg, choices, def);
  }
  bool isTestMode() const { return _launcher.isTestMode(); }
  std::ostream& log(bool heading = false) const { return _launcher.log(heading); }
  std::ostream& out() const { return _launcher.out(); }
  void printMeaning(const Entry& kanji, bool useNewLine = false) const {
    _launcher.printMeaning(kanji, useNewLine, _showMeanings);
  }

  // Below are some options used in for quiz questions. These are all ascii symbols that come
  // before letters and numbers so that 'Choice::get' method displays them at the beginning
  // of the list (assuming the other choices are just letters and/or numbers).
  static constexpr char MeaningsOption = '-';
  static constexpr char PrevOption = ',';
  static constexpr char SkipOption = '.';

  std::ostream& beginQuizMessage(int totalQuestions); // can modify '_questions'
  std::ostream& beginQuestionMessage(int totalQuestions) const;
  void correctAnswerMessage(); // increments '_correctAnswers'

  // 'getDefaultChoices' returns a Choices structure populated with just the common values
  // for a quiz question like skip and quit. It will also populate 'hide/show meanings' option
  // based on the current value of '_showMeanings'.
  Choices getDefaultChoices(int totalQuestions) const;

  // display of English 'meanings' can be toggled on and off
  void toggleMeanings(Choices&);

  const QuizLauncher& _launcher;
  DataFile::List _mistakes;
  int _question;
  int _correctAnswers;
  bool _showMeanings;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_QUIZ_H
