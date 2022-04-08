#include <kanji_tools/quiz/ListQuiz.h>

#include <random>

namespace kanji_tools {

namespace {

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

const std::string Prompt{"  Select"};
const std::string QuizPrompt{Prompt + " correct "};

constexpr auto ChoiceStart{'1'};

} // namespace

ListQuiz::QuizStyle ListQuiz::toQuizStyle(char c) {
  return c == 'k' ? QuizStyle::KanjiToReading : QuizStyle::ReadingToKanji;
}

ListQuiz::ListQuiz(const QuizLauncher& launcher, Question question,
    bool showMeanings, const List& list, KanjiInfo fields,
    ChoiceCount choiceCount, QuizStyle quizStyle)
    : Quiz{launcher, question, showMeanings},
      _answers(choiceCount), _infoFields{fields}, _choiceCount{choiceCount},
      _quizStyle{quizStyle}, _prompt{isTestMode()
                                         ? QuizPrompt + (isKanjiToReading()
                                                                ? "reading"
                                                                : "kanji")
                                         : Prompt},
      _choiceEnd{static_cast<char>('0' + _choiceCount)} {
  assert(_answers.size() == _choiceCount); // need () ctor
  List questions;
  for (auto& i : list) {
    if (!i->hasReading())
      // should never happen for any of the existing list quiz types
      throw std::domain_error{i->name() + " has no reading"};
    questions.emplace_back(i);
  }
  if (_launcher.questionOrder() == QuestionOrder::FromEnd)
    std::reverse(questions.begin(), questions.end());
  else if (_launcher.questionOrder() == QuestionOrder::Random)
    std::shuffle(questions.begin(), questions.end(), RandomGen);

  beginQuizMessage(questions.size()) << "kanji\n>>>\n";

  if (isKanjiToReading()) _launcher.printLegend(fields);
  start(questions);
}

void ListQuiz::start(const List& questions) {
  auto stopQuiz{false};
  for (; !stopQuiz && _question < questions.size(); ++_question) {
    const Entry& i{questions[_question]};
    auto choices{getDefaultChoices(questions.size())};
    const auto correct{populateAnswers(i, questions)};
    do {
      beginQuestionMessage(questions.size());
      printQuestion(i);
      printChoices(i, questions);
    } while (!getAnswer(choices, stopQuiz, correct, i->name()));
  }
  // when quitting don't count the current question in the final score
  if (stopQuiz) --_question;
}

bool ListQuiz::isKanjiToReading() const {
  return _quizStyle == QuizStyle::KanjiToReading;
}

ListQuiz::ChoiceCount ListQuiz::populateAnswers(
    const Entry& kanji, const List& questions) {
  std::uniform_int_distribution<Question> randomReading(
      0, static_cast<Question>(questions.size()) - 1);
  std::uniform_int_distribution<ChoiceCount> randomCorrect(0, _choiceCount - 1);

  const auto correct{
      _launcher.randomizeAnswers() ? randomCorrect(RandomGen) : ChoiceCount{}};
  // 'sameReading' prevents more than one choice having the same reading
  DataFile::Set sameReading{kanji->reading()};
  _answers[correct] = _question;
  for (ChoiceCount i{}; i < _choiceCount; ++i)
    if (i != correct) do {
        if (const auto choice{randomReading(RandomGen)};
            sameReading.insert(questions[choice]->reading()).second) {
          _answers[i] = choice;
          break;
        }
      } while (true);
  return correct;
}

void ListQuiz::printQuestion(const Entry& kanji) const {
  if (isKanjiToReading()) {
    out() << kanji->name();
    if (const auto info{kanji->info(_infoFields)}; !info.empty())
      out() << "  " << info;
    if (!isTestMode()) _launcher.printExtraTypeInfo(kanji);
  } else
    out() << "Reading:  " << kanji->reading();
  printMeaning(kanji, !isTestMode());
}

void ListQuiz::printChoices(const Entry& kanji, const List& questions) const {
  if (isTestMode())
    for (ChoiceCount i{}; i < _choiceCount; ++i)
      out() << "    " << i + 1 << ".  "
            << (isKanjiToReading() ? questions[_answers[i]]->reading()
                                   : questions[_answers[i]]->name())
            << '\n';
  else
    _launcher.printReviewDetails(kanji);
}

bool ListQuiz::getAnswer(Choices& choices, bool& stopQuiz, ChoiceCount correct,
    const std::string& name) {
  const auto answer{
      isTestMode() ? choice().get(_prompt, {ChoiceStart, _choiceEnd}, choices)
                   : get(_prompt, choices)};
  if (answer == MeaningsOption) {
    toggleMeanings(choices);
    return false;
  }
  if (isQuit(answer))
    stopQuiz = true;
  else if (answer == PrevOption)
    _question -= 2;
  else if (const auto c{static_cast<char>(ChoiceStart + correct)}; c == answer)
    correctMessage();
  else if (answer != SkipOption)
    incorrectMessage(name) << " (correct answer is " << c << ")\n";
  return true;
}

} // namespace kanji_tools
