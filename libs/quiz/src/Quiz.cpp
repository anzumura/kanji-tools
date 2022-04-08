#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

namespace {

constexpr auto ShowMeanings{"show meanings"}, HideMeanings{"hide meanings"};

} // namespace

void Quiz::run(const Args& args, std::ostream& out) {
  const auto data{std::make_shared<KanjiData>(args, out)};
  QuizLauncher(args, data, std::make_shared<GroupData>(data),
      std::make_shared<JukugoData>(data));
}

Quiz::Quiz(const QuizLauncher& launcher, Question question, bool showMeanings)
    : _launcher{launcher}, _question{question}, _correctAnswers{0},
      _showMeanings{showMeanings} {}

Quiz::~Quiz() {
  if (isTestMode()) {
    out() << "\nFinal score: " << _correctAnswers << '/' << _question;
    if (!_question)
      out() << '\n';
    else if (_correctAnswers == _question)
      out() << " - Perfect!\n";
    else {
      if (const auto skipped{_question - _correctAnswers - _mistakes.size()};
          skipped)
        out() << ", skipped: " << skipped;
      if (!_mistakes.empty()) {
        out() << " - mistakes:";
        for (auto& i : _mistakes) out() << ' ' << i;
      }
      out() << '\n';
    }
  }
}

char Quiz::get(const std::string& msg, const Choices& choices, OptChar def,
    bool useQuit) const {
  return choice().get(msg, useQuit, choices, def);
}

std::ostream& Quiz::log(bool heading) const { return _launcher.log(heading); }

void Quiz::printMeaning(const Entry& kanji, bool useNewLine) const {
  _launcher.printMeaning(kanji, useNewLine, _showMeanings);
}

std::ostream& Quiz::beginQuizMessage(size_t totalQuestions) {
  // _question can be set to non-zero from command line -r or -t options, report
  // an error if it's out of range, otherwise subtract one since 'question' list
  // index starts at zero.
  if (_question) {
    if (_question > totalQuestions)
      Data::usage("entry num '" + std::to_string(_question) +
                  "' is larger than total questions: " +
                  std::to_string(totalQuestions));
    --_question;
  }
  return log(true) << "Starting " << (isTestMode() ? "quiz" : "review")
                   << " for " << totalQuestions << ' ';
}

std::ostream& Quiz::beginQuestionMessage(size_t totalQuestions) const {
  return out() << (isTestMode() ? "\nQuestion " : "\n") << _question + 1 << '/'
               << totalQuestions << ":  ";
}

bool Quiz::showMeanings() const { return _showMeanings; }

void Quiz::correctMessage() {
  out() << "  Correct! (" << ++_correctAnswers << '/' << _question + 1 << ")\n";
}

std::ostream& Quiz::incorrectMessage(const std::string& name) {
  _mistakes.emplace_back(name);
  return out() << "  Incorrect";
}

Choice::Choices Quiz::getDefaultChoices(size_t totalQuestions) const {
  Choice::Choices c{
      {MeaningsOption, _showMeanings ? HideMeanings : ShowMeanings},
      {SkipOption, _question + 1U == totalQuestions ? "finish"
                   : !isTestMode()                  ? "next"
                                                    : "skip"}};
  if (!isTestMode() && _question) c[PrevOption] = "prev";
  return c;
}

void Quiz::toggleMeanings(Choices& choices) {
  _showMeanings = !_showMeanings;
  choices[MeaningsOption] = _showMeanings ? HideMeanings : ShowMeanings;
}

} // namespace kanji_tools
