#ifndef KANJI_TOOLS_QUIZ_QUIZ_H
#define KANJI_TOOLS_QUIZ_QUIZ_H

#include <kanji_tools/quiz/QuizLauncher.h>

namespace kanji_tools {

// 'Quiz' is the base class for 'ListQuiz' and 'GroupQuiz'. It holds common data for a quiz including
// the current question number, correct answer count and list of mistakes.
class Quiz {
protected:
  using Choices = QuizLauncher::Choices;
  using Entry = QuizLauncher::Entry;
  using List = QuizLauncher::List;
  using OptChar = QuizLauncher::OptChar;

  // Below are some options used in for quiz questions. These are all ascii symbols that come
  // before letters and numbers so that 'Choice::get' method displays them at the beginning
  // of the list (assuming the other choices are just letters and/or numbers).
  static constexpr auto MeaningsOption = '-';
  static constexpr auto PrevOption = ',';
  static constexpr auto SkipOption = '.';

  Quiz(const QuizLauncher& launcher, int question, bool showMeanings)
    : _launcher(launcher), _question(question), _correctAnswers(0), _showMeanings(showMeanings) {}

  // destructor prints the final score when in test mode
  virtual ~Quiz();

  // the following methods are shotcuts for calling '_launcher' methods
  auto& choice() const { return _launcher.choice(); }
  auto get(const std::string& msg, const Choices& choices, OptChar def = std::nullopt, bool useQuit = true) const {
    return choice().get(msg, useQuit, choices, def);
  }
  auto isQuit(char c) const { return _launcher.isQuit(c); }
  auto isTestMode() const { return _launcher.isTestMode(); }
  auto& log(bool heading = false) const { return _launcher.log(heading); }
  auto& out() const { return _launcher.out(); }
  void printMeaning(const Entry& kanji, bool useNewLine = false) const {
    _launcher.printMeaning(kanji, useNewLine, _showMeanings);
  }

  void correctMessage();                                           // increments '_correctAnswers'
  std::ostream& incorrectMessage(const std::string& name);         // adds 'name' to '_mistakes'
  std::ostream& beginQuizMessage(size_t totalQuestions);           // can modify '_question'
  std::ostream& beginQuestionMessage(size_t totalQuestions) const; // 'const' since can be called >1 times per question
  auto showMeanings() const { return _showMeanings; }

  // 'getDefaultChoices' returns a Choices structure populated with just the common values
  // for a quiz question like skip and quit. It will also populate 'hide/show meanings' option
  // based on the current value of '_showMeanings'.
  Choices getDefaultChoices(size_t totalQuestions) const;

  // display of English 'meanings' can be toggled on and off
  void toggleMeanings(Choices&);

  const QuizLauncher& _launcher;
  size_t _question;
private:
  size_t _correctAnswers;
  DataFile::List _mistakes;
  bool _showMeanings;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_QUIZ_H
