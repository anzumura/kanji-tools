#include <kanji_tools/kanji/RealKanjiData.h>
#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

namespace {

constexpr auto ShowMeanings{"show meanings"}, HideMeanings{"hide meanings"};

} // namespace

void Quiz::run(const Args& args, std::ostream& out) {
  const auto data{std::make_shared<RealKanjiData>(args, out)};
  QuizLauncher{args, data, std::make_shared<GroupData>(data),
      std::make_shared<JukugoData>(data)};
}

Quiz::Quiz(const QuizLauncher& launcher, Question question, bool showMeanings)
    : _launcher{launcher}, _currentQuestion{question}, _correctAnswers{0},
      _showMeanings{showMeanings} {}

Quiz::~Quiz() {
  if (isTestMode()) {
    out() << "\nFinal score: " << _correctAnswers << '/' << _currentQuestion;
    if (_currentQuestion) {
      const auto skipped{_currentQuestion - _correctAnswers - _mistakes.size()};
      if (skipped) out() << ", skipped: " << skipped;
      if (!_mistakes.empty()) {
        out() << " - mistakes:";
        for (auto& i : _mistakes) out() << ' ' << i;
      } else if (!skipped)
        out() << " - Perfect!";
    }
    out() << '\n';
  }
}

char Quiz::get(const String& msg, const Choices& choices, OptChar def,
    bool useQuit) const {
  return choice().get(msg, useQuit, choices, def);
}

std::ostream& Quiz::log(bool heading) const { return _launcher.log(heading); }

void Quiz::printMeaning(const Kanji& kanji, bool useNewLine) const {
  _launcher.printMeaning(kanji, useNewLine, _showMeanings);
}

std::ostream& Quiz::beginQuizMessage(size_t totalQuestions) {
  // _currentQuestion can be set to non-zero from command line -r or -t options,
  // report an error if it's out of range, otherwise subtract one since
  // 'question' list index starts at zero.
  if (_currentQuestion) {
    if (_currentQuestion > totalQuestions)
      KanjiData::usage("entry num '" + std::to_string(_currentQuestion) +
                       "' is larger than total questions: " +
                       std::to_string(totalQuestions));
    --_currentQuestion;
  }
  return log(true) << "Starting " << (isTestMode() ? "quiz" : "review")
                   << " for " << totalQuestions << ' ';
}

std::ostream& Quiz::beginQuestionMessage(size_t totalQuestions) const {
  return out() << (isTestMode() ? "\nQuestion " : "\n") << _currentQuestion + 1
               << '/' << totalQuestions << ":  ";
}

bool Quiz::showMeanings() const { return _showMeanings; }

void Quiz::correctMessage() {
  out() << "  Correct! (" << ++_correctAnswers << '/' << _currentQuestion + 1
        << ")\n";
}

std::ostream& Quiz::incorrectMessage(const String& name) {
  _mistakes.emplace_back(name);
  return out() << "  Incorrect";
}

Choice::Choices Quiz::getDefaultChoices(size_t totalQuestions) const {
  Choice::Choices c{
      {MeaningsOption, _showMeanings ? HideMeanings : ShowMeanings},
      {SkipOption, _currentQuestion + 1U == totalQuestions ? "finish"
                   : !isTestMode()                         ? "next"
                                                           : "skip"}};
  if (!isTestMode() && _currentQuestion) c[PrevOption] = "prev";
  return c;
}

void Quiz::toggleMeanings(Choices& choices) {
  _showMeanings = !_showMeanings;
  choices[MeaningsOption] = _showMeanings ? HideMeanings : ShowMeanings;
}

} // namespace kanji_tools
