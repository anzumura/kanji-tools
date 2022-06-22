#pragma once

#include <kt_kana/Choice.h>
#include <kt_quiz/GroupData.h>
#include <kt_quiz/JukugoData.h>

namespace kanji_tools { /// \quiz_group{QuizLauncher}
/// QuizLauncher class

/// starts a quiz (or review) or prints info about a Kanji \quiz{QuizLauncher}
class QuizLauncher final {
public:
  using Choices = Choice::Choices;
  using List = KanjiData::List;
  using OptChar = Choice::OptChar;
  using Question = uint16_t;

  static constexpr char QuitOption{'/'};

  /// `in` can be provided for testing instead of using `std::cin` and if given,
  /// the start() method must be explicitly called to start the quiz/review.
  QuizLauncher(const Args&, const KanjiDataPtr&, const GroupDataPtr&,
      const JukugoDataPtr&, std::istream* in = {});

  QuizLauncher(const QuizLauncher&) = delete; ///< deleted copy ctor

  /// starts a (list or group based) quiz/review
  /// \param quizType if provided, value must be one of 'f', 'g', 'k', 'l', 'm'
  ///     or 'p' (see QuizLauncher.cpp for details)
  /// \param qList question list sub-type, valid values depend on `quizType`
  /// \param question where to start from, `0` means prompt the user
  /// \param showMeanings true indicates English meanings should be shown
  /// \param randomizeAnswers should only be set to false in test code
  void start(OptChar quizType, OptChar qList, Question question = 0,
      bool showMeanings = false, bool randomizeAnswers = true);

  [[nodiscard]] std::ostream& log(bool heading = false) const;
  [[nodiscard]] auto& out() const { return data().out(); }

  /// main program mode
  enum class ProgramMode {
    Review,     ///< review lists or groups including showing more detailed info
    Quiz,       ///< quiz mode (the default command-line option)
    NotAssigned ///< prompt user to get the mode
  };

  /// question order
  enum class QuestionOrder {
    FromBeginning, ///< start at the first question and move forward
    FromEnd,       ///< start at the last question and move backward
    Random,        ///< proceed through the questions in random order
    NotAssigned    ///< prompt user to get the order
  };

  [[nodiscard]] bool isQuizMode() const;
  [[nodiscard]] auto questionOrder() const { return _questionOrder; }
  [[nodiscard]] auto& choice() const { return _choice; }
  [[nodiscard]] auto isQuit(char c) const { return _choice.isQuit(c); }
  [[nodiscard]] auto& groupData() const { return _groupData; }
  [[nodiscard]] auto randomizeAnswers() const { return _randomizeAnswers; }

  void printExtraTypeInfo(const Kanji&) const;
  void printLegend(Kanji::Info fields = Kanji::Info::All) const;
  void printMeaning(
      const Kanji&, bool useNewLine = false, bool showMeaning = true) const;
  void printReviewDetails(const Kanji&) const;

private:
  static constexpr uint16_t JukugoPerLine{3}, MaxJukugoSize{30};

  [[nodiscard]] KanjiDataRef data() const { return _groupData->data(); }

  void startListQuiz(Question question, bool showMeanings,
      Kanji::Info excludeField, const List&) const;
  void startGroupQuiz(Question question, bool showMeanings, OptChar qList,
      const GroupData::List& list) const;

  [[nodiscard]] static Kanji::NelsonId getId(
      const String& msg, const String& arg);

  /// called by ctor for 'quiz type' and 'program mode' args
  /// \param[out] question set if `arg` is a 'program mode' ('-t' or '-r')
  /// \param[out] quizType set if `arg` is a 'quiz type' (-f, -g, etc..)
  /// \param arg command-line arg to process
  /// \return optional 'question list' depending on `arg` - see setQuizType()
  /// \throw DomainError if `arg` isn't a valid option
  [[nodiscard]] OptChar processArg(
      Question& question, OptChar& quizType, const String& arg);

  /// called by processArg() if `arg` starts with -f, -g, -l, -k, -m or -p
  /// \param[out] quizType set to the second char of `arg` so 'f', 'g'. etc..
  /// \param arg the command like arg to process
  /// \return optional 'question list', for example if `arg` is "-g6" then '6'
  ///     is returned, but if `arg` is "-g" then `std::nullopt` is returned
  /// \throw DomainError if called multiple times or if `arg` format is invalid
  [[nodiscard]] static OptChar setQuizType(OptChar& quizType, const String& arg,
      const Choices&, const std::optional<Choice::Range>& = {});

  /// called for '-r' and '-t' args and sets #_programMode. It can also set
  /// #_questionOrder if `arg` is followed by a number (like "-t30")
  /// \return question to start from or `0` which means 'not specified'
  /// \throw DomainError if `arg` format is invalid (like "-tABC")
  [[nodiscard]] Question processProgramModeArg(const String& arg);

  /// called when a Kanji `arg` is passed to the program (see QuizLauncher.cpp)
  void processKanjiArg(const String& arg) const;

  /// prints details about a list of Kanji (instead of running a quiz/review)
  void printDetails(
      const KanjiData::List&, const String& name, const String& arg) const;

  /// print details about `arg` (should be a single Kanji name)
  void printDetails(const String& arg, bool showLegend = true) const;

  [[nodiscard]] bool getQuestionOrder();

  void printJukugo(const Kanji&) const;
  void printJukugoList(const String& name, const JukugoData::List&) const;

  [[nodiscard]] char chooseQuizType(OptChar) const;

  [[nodiscard]] char chooseFreq(OptChar) const;
  [[nodiscard]] char chooseGrade(OptChar) const;
  [[nodiscard]] char chooseKyu(OptChar) const;
  [[nodiscard]] char chooseLevel(OptChar) const;

  [[nodiscard]] const KanjiData::List& getKyuList(char) const;

  /// can be set via params to start(), otherwise obtained via reading input @{
  ProgramMode _programMode{ProgramMode::NotAssigned};
  QuestionOrder _questionOrder{QuestionOrder::NotAssigned};
  ///@}

  /// set via a parameter to start()
  bool _randomizeAnswers{true};

  const Choice _choice;
  const GroupDataPtr _groupData;
  const JukugoDataPtr _jukugoData;
};

/// \end_group
} // namespace kanji_tools
