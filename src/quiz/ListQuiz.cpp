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
  assert(_answers.size() == _choiceCount);
  List questions;
  for (auto& i : list)
    if (i->hasReading()) questions.push_back(i);
  if (_launcher.questionOrder() == QuestionOrder::FromEnd)
    std::reverse(questions.begin(), questions.end());
  else if (_launcher.questionOrder() == QuestionOrder::Random)
    std::shuffle(questions.begin(), questions.end(), RandomGen);

  beginQuizMessage(questions.size()) << "kanji";
  if (questions.size() < list.size())
    out() << " (original list had " << list.size()
          << ", but not all entries have readings)";
  out() << "\n>>>\n";

  if (isKanjiToReading()) _launcher.printLegend(fields);
  start(questions);
}

void ListQuiz::start(const List& questions) {
  auto stopQuiz{false};
  for (; !stopQuiz && _question < questions.size(); ++_question) {
    const Entry& i{questions[_question]};
    auto choices{getDefaultChoices(questions.size())};
    const auto correctChoice{populateAnswers(i, questions)};
    do {
      beginQuestionMessage(questions.size());
      printQuestion(i);
      printChoices(i, questions);
    } while (!getAnswer(choices, stopQuiz, correctChoice, i->name()));
  }
  // when quitting don't count the current question in the final score
  if (stopQuiz) --_question;
}

ListQuiz::ChoiceCount ListQuiz::populateAnswers(
    const Entry& kanji, const List& questions) {
  std::uniform_int_distribution<Question> randomReading(
      0, static_cast<Question>(questions.size()) - 1);
  std::uniform_int_distribution<ChoiceCount> randomCorrect(0, _choiceCount - 1);

  const auto correctChoice{randomCorrect(RandomGen)};
  // 'sameReading' prevents more than one choice having the same reading
  DataFile::Set sameReading{kanji->reading()};
  _answers[correctChoice] = _question;
  for (ChoiceCount i{}; i < _choiceCount; ++i)
    if (i != correctChoice) do {
        if (const auto choice{randomReading(RandomGen)};
            sameReading.insert(questions[choice]->reading()).second) {
          _answers[i] = choice;
          break;
        }
      } while (true);
  return correctChoice;
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

bool ListQuiz::getAnswer(Choices& choices, bool& stopQuiz,
    ChoiceCount correctChoice, const std::string& name) {
  const auto answer{
      isTestMode() ? choice().get(_prompt, ChoiceStart, _choiceEnd, choices)
                   : get(_prompt, choices)};
  if (answer == MeaningsOption) {
    toggleMeanings(choices);
    return false;
  }
  if (isQuit(answer))
    stopQuiz = true;
  else if (answer == PrevOption)
    _question -= 2;
  else if (static_cast<size_t>(answer - ChoiceStart) == correctChoice)
    correctMessage();
  else if (answer != SkipOption)
    incorrectMessage(name) << "  (correct answer is " << correctChoice << ")\n";
  return true;
}

} // namespace kanji_tools
