#include <kanji_tools/quiz/Quiz.h>

namespace kanji_tools {

namespace {

constexpr auto ShowMeanings = "show meanings";
constexpr auto HideMeanings = "hide meanings";

} // namespace

Quiz::~Quiz() {
  if (isTestMode()) {
    out() << "\nFinal score: " << _correctAnswers << '/' << _question;
    if (!_question)
      out() << '\n';
    else if (_correctAnswers == _question)
      out() << " - Perfect!\n";
    else {
      if (int skipped = _question - _correctAnswers - _mistakes.size(); skipped) out() << ", skipped: " << skipped;
      if (!_mistakes.empty()) {
        out() << " - mistakes:";
        for (auto& i : _mistakes) out() << ' ' << i;
      }
      out() << '\n';
    }
  }
}

std::ostream& Quiz::beginQuizMessage(size_t totalQuestions) {
  // _question can be set to non-zero from command line -r or -t options, report an error if it's out
  // of range, otherwise subtract one since 'question' list index starts at zero.
  if (_question) {
    if (_question > totalQuestions)
      Data::usage("entry num '" + std::to_string(_question) +
                  "' is larger than total questions: " + std::to_string(totalQuestions));
    --_question;
  }
  return log(true) << "Starting " << (isTestMode() ? "quiz" : "review") << " for " << totalQuestions << ' ';
}

std::ostream& Quiz::beginQuestionMessage(size_t totalQuestions) const {
  return out() << (isTestMode() ? "\nQuestion " : "\n") << _question + 1 << '/' << totalQuestions << ":  ";
}

void Quiz::correctMessage() { out() << "  Correct! (" << ++_correctAnswers << '/' << _question + 1 << ")\n"; }

std::ostream& Quiz::incorrectMessage(const std::string& name) {
  _mistakes.push_back(name);
  return out() << "  Incorrect";
}

Choice::Choices Quiz::getDefaultChoices(size_t totalQuestions) const {
  Choice::Choices c = {{MeaningsOption, _showMeanings ? HideMeanings : ShowMeanings},
                       {SkipOption,
                        _question + 1 == totalQuestions ? "finish"
                          : !isTestMode()               ? "next"
                                                        : "skip"}};
  if (!isTestMode() && _question) c[PrevOption] = "prev";
  return c;
}

void Quiz::toggleMeanings(Choices& choices) {
  _showMeanings = !_showMeanings;
  choices[MeaningsOption] = _showMeanings ? HideMeanings : ShowMeanings;
}

} // namespace kanji_tools
