#include <kanji_tools/quiz/ListQuiz.h>

#include <random>

namespace kanji_tools {

namespace {

std::mt19937 RandomGen(std::random_device{}());

const String Prompt{"  Select"};

constexpr auto ChoiceStart{'1'};

} // namespace

ListQuiz::QuizStyle ListQuiz::toQuizStyle(char c) {
  return c == 'k' ? QuizStyle::KanjiToReading : QuizStyle::ReadingToKanji;
}

ListQuiz::ListQuiz(const QuizLauncher& launcher, Question question,
    bool showMeanings, const List& list, // LCOV_EXCL_LINE
    Kanji::Info fields, ChoiceCount choiceCount, QuizStyle quizStyle)
    : Quiz{launcher, question, showMeanings},
      _answers(choiceCount), _infoFields{fields}, _choiceCount{choiceCount},
      _quizStyle{quizStyle}, _prompt{getPrompt()}, _choiceEnd{toChar(
                                                       '0' + _choiceCount)} {
  assert(_answers.size() == _choiceCount); // need () ctor
  List questions;
  for (auto& i : list) {
    if (!i->hasReading())
      // should never happen for any of the existing list quiz types
      throw DomainError{i->name() + " has no reading"};
    questions.emplace_back(i);
  }
  if (launcher.questionOrder() == QuestionOrder::FromEnd)
    std::reverse(questions.begin(), questions.end());
  else if (launcher.questionOrder() == QuestionOrder::Random)
    std::shuffle(questions.begin(), questions.end(), RandomGen);

  beginQuizMessage(questions.size()) << "kanji\n>>>\n";

  if (isKanjiToReading()) launcher.printLegend(fields);
  start(questions);
}

void ListQuiz::start(const List& questions) {
  auto stopQuiz{false};
  for (; !stopQuiz && currentQuestion() < questions.size();
       ++currentQuestion()) {
    const Kanji& i{*questions[currentQuestion()]};
    auto choices{getDefaultChoices(questions.size())};
    const auto correct{populateAnswers(i, questions)};
    do {
      beginQuestionMessage(questions.size());
      printQuestion(i);
      printChoices(i, questions);
    } while (!getAnswer(choices, stopQuiz, correct, i.name()));
  }
  // when quitting don't count the current question in the final score
  if (stopQuiz) --currentQuestion();
}

const String& ListQuiz::getPrompt() const {
  static const String Reading{Prompt + " correct reading"},
      Kanji(Prompt + " correct kanji");
  return isTestMode() ? isKanjiToReading() ? Reading : Kanji : Prompt;
}

bool ListQuiz::isKanjiToReading() const {
  return _quizStyle == QuizStyle::KanjiToReading;
}

ListQuiz::ChoiceCount ListQuiz::populateAnswers(
    const Kanji& kanji, const List& questions) {
  std::uniform_int_distribution<Question> randomReading(
      0, static_cast<Question>(questions.size()) - 1);
  std::uniform_int_distribution<ChoiceCount> randomCorrect(0, _choiceCount - 1);

  const auto correct{
      launcher().randomizeAnswers() ? randomCorrect(RandomGen) : ChoiceCount{}};
  // 'sameReading' prevents more than one choice having the same reading
  KanjiListFile::StringSet sameReading{kanji.reading()};
  _answers[correct] = currentQuestion();
  for (ChoiceCount i{}; i < _choiceCount; ++i)
    while (i != correct)
      if (const auto choice{randomReading(RandomGen)};
          sameReading.insert(questions[choice]->reading()).second) {
        _answers[i] = choice;
        break;
      }
  return correct;
}

void ListQuiz::printQuestion(const Kanji& kanji) const {
  if (isKanjiToReading()) {
    out() << kanji.name();
    if (const auto info{kanji.info(_infoFields)}; !info.empty())
      out() << "  " << info;
    if (!isTestMode()) launcher().printExtraTypeInfo(kanji);
  } else
    out() << "Reading:  " << kanji.reading();
  printMeaning(kanji, !isTestMode());
}

void ListQuiz::printChoices(const Kanji& kanji, const List& questions) const {
  if (isTestMode())
    for (ChoiceCount i{}; i < _choiceCount; ++i)
      out() << "    " << i + 1 << ".  "
            << (isKanjiToReading() ? questions[_answers[i]]->reading()
                                   : questions[_answers[i]]->name())
            << '\n';
  else
    launcher().printReviewDetails(kanji);
}

bool ListQuiz::getAnswer(Choices& choices, bool& stopQuiz, ChoiceCount correct,
    const String& kanjiName) {
  const auto answer{
      isTestMode() ? choice().get({ChoiceStart, _choiceEnd}, _prompt, choices)
                   : get(_prompt, choices)};
  if (answer == MeaningsOption) {
    toggleMeanings(choices);
    return false;
  }
  if (isQuit(answer))
    stopQuiz = true;
  else if (answer == PrevOption)
    currentQuestion() -= 2;
  else if (const auto c{toChar(ChoiceStart + correct)}; c == answer)
    correctMessage();
  else if (answer != SkipOption)
    incorrectMessage(kanjiName) << " (correct answer is " << c << ")\n";
  return true;
}

} // namespace kanji_tools
