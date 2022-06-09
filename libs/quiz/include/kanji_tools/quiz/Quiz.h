#pragma once

#include <kanji_tools/quiz/QuizLauncher.h>

namespace kanji_tools { /// \quiz_group{Quiz}
/// Quiz class

/// base class of ListQuiz and GroupQuiz holding common data including current
/// question number, correct answer count and list of mistakes \quiz{Quiz}
class Quiz {
public:
  using List = KanjiData::List;

  Quiz(const Quiz&) = delete;           ///< deleted copy ctor
  auto operator=(const Quiz&) = delete; ///< deleted operator=

  /// dtor prints the final score when in 'quiz' mode
  virtual ~Quiz();

  /// creates QuizLauncher and it dependencies, this will start an instance of
  /// either ListQuiz or GroupQuiz depending on command line args.
  static void run(const Args&, std::ostream& = std::cout);
protected:
  using Choices = QuizLauncher::Choices;
  using OptChar = QuizLauncher::OptChar;
  using Question = QuizLauncher::Question;
  using QuestionOrder = QuizLauncher::QuestionOrder;

  /// options used in for quiz questions \details These are all Ascii symbols
  /// that come before letters and numbers so that Choice::get() method displays
  /// them at the beginning of the list (assuming the other choices are just
  /// letters and/or numbers). @{
  static constexpr auto MeaningsOption{'-'}, PrevOption{','}, SkipOption{'.'};
  ///@}

  Quiz(const QuizLauncher&, Question, bool showMeanings);

  /// the following methods are shortcuts for calling #_launcher methods @{
  [[nodiscard]] auto& launcher() const { return _launcher; }
  [[nodiscard]] auto& choice() const { return _launcher.choice(); }
  [[nodiscard]] auto isQuit(char c) const { return _launcher.isQuit(c); }
  [[nodiscard]] auto isQuizMode() const { return _launcher.isQuizMode(); }
  [[nodiscard]] std::ostream& log(bool heading = false) const;
  [[nodiscard]] auto& out() const { return _launcher.out(); }
  [[nodiscard]] char get(
      const String&, const Choices&, OptChar = {}, bool = true) const;
  ///@}

  void printMeaning(const Kanji&, bool useNewLine = false) const;

  void correctMessage(); ///< increments #_correctAnswers

  std::ostream& incorrectMessage(const String& name);
  std::ostream& beginQuizMessage(size_t totalQuestions);
  std::ostream& beginQuestionMessage(size_t totalQuestions) const; // NOLINT
  [[nodiscard]] bool showMeanings() const;

  /// return Choices populated with the common values for a quiz question like
  /// skip and quit. It will also populate 'hide/show meanings' option based on
  /// the current value of #_showMeanings
  [[nodiscard]] Choices getDefaultChoices(size_t totalQuestions) const;

  /// display of English 'meanings' can be toggled on and off
  void toggleMeanings(Choices&);

  [[nodiscard]] auto& currentQuestion() { return _currentQuestion; }
private:
  const QuizLauncher& _launcher;
  Question _currentQuestion;
  bool _showMeanings;
  Question _correctAnswers{0};
  KanjiListFile::StringList _mistakes;
};

/// \end_group
} // namespace kanji_tools
