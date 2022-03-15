#pragma once

#include <kanji_tools/quiz/QuizLauncher.h>

namespace kanji_tools {

// 'Quiz' is the base class of 'ListQuiz' and 'GroupQuiz'. It holds common data
// including current question number, correct answer count and list of mistakes.
class Quiz {
public:
  using List = QuizLauncher::List;
protected:
  using Choices = QuizLauncher::Choices;
  using Entry = QuizLauncher::Entry;
  using OptChar = QuizLauncher::OptChar;

  // Below are some options used in for quiz questions. These are all ascii
  // symbols that come before letters and numbers so that 'Choice::get' method
  // displays them at the beginning of the list (assuming the other choices are
  // just letters and/or numbers).
  static constexpr auto MeaningsOption{'-'}, PrevOption{','}, SkipOption{'.'};

  Quiz(const QuizLauncher& launcher, u_int16_t question, bool showMeanings)
      : _launcher(launcher), _question(question), _correctAnswers(0),
        _showMeanings(showMeanings) {}

  Quiz(const Quiz&) = delete;
  Quiz& operator=(const Quiz&) = delete;

  // destructor prints the final score when in test mode
  virtual ~Quiz();

  // the following methods are shotcuts for calling '_launcher' methods
  [[nodiscard]] auto& choice() const { return _launcher.choice(); }
  [[nodiscard]] auto get(const std::string& msg, const Choices& choices,
                         OptChar def = {}, bool useQuit = true) const {
    return choice().get(msg, useQuit, choices, def);
  }
  [[nodiscard]] auto isQuit(char c) const { return _launcher.isQuit(c); }
  [[nodiscard]] auto isTestMode() const { return _launcher.isTestMode(); }
  [[nodiscard]] auto& log(bool heading = false) const {
    return _launcher.log(heading);
  }
  [[nodiscard]] auto& out() const { return _launcher.out(); }
  void printMeaning(const Entry& kanji, bool useNewLine = false) const {
    _launcher.printMeaning(kanji, useNewLine, _showMeanings);
  }

  void correctMessage(); // increments '_correctAnswers'
  std::ostream& incorrectMessage(const std::string& name);
  std::ostream& beginQuizMessage(size_t totalQuestions);
  std::ostream& beginQuestionMessage(size_t totalQuestions) const;
  [[nodiscard]] auto showMeanings() const { return _showMeanings; }

  // 'getDefaultChoices' returns a Choices structure populated with just the
  // common values for a quiz question like skip and quit. It will also populate
  // 'hide/show meanings' option based on the current value of '_showMeanings'.
  [[nodiscard]] Choices getDefaultChoices(size_t totalQuestions) const;

  // display of English 'meanings' can be toggled on and off
  void toggleMeanings(Choices&);

  const QuizLauncher& _launcher;
  u_int16_t _question;
private:
  u_int16_t _correctAnswers;
  DataFile::List _mistakes;
  bool _showMeanings;
};

} // namespace kanji_tools
