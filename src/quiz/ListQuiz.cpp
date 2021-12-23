#include <kanji_tools/quiz/ListQuiz.h>

#include <random>

namespace kanji_tools {

namespace {

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

} // namespace

ListQuiz::ListQuiz(const QuizLauncher& launcher, int question, bool showMeanings, const List& list, int infoFields,
                   int choiceCount, QuizStyle quizStyle)
  : Quiz(launcher, question, showMeanings), _infoFields(infoFields), _choiceCount(choiceCount), _quizStyle(quizStyle) {
  List questions;
  for (auto& i : list)
    if (i->hasReading()) questions.push_back(i);
  if (_launcher.questionOrder() == QuizLauncher::QuestionOrder::FromEnd)
    std::reverse(questions.begin(), questions.end());
  else if (_launcher.questionOrder() == QuizLauncher::QuestionOrder::Random)
    std::shuffle(questions.begin(), questions.end(), RandomGen);

  beginQuizMessage(questions.size()) << "kanji";
  if (questions.size() < list.size())
    out() << " (original list had " << list.size() << ", but not all entries have readings)";
  out() << "\n>>>\n";

  if (isKanjiToReading()) _launcher.printLegend(infoFields);
  start(questions);
}

void ListQuiz::start(const List& questions) {
  static const std::string reviewPrompt("  Select"), quizPrompt("  Select correct ");
  const std::string prompt(isTestMode() ? quizPrompt + (isKanjiToReading() ? "reading" : "kanji")
                                        : reviewPrompt);

  bool stopQuiz = false;
  for (Choices choices; !stopQuiz && _question < questions.size(); ++_question) {
    const Entry& i = questions[_question];
    Answers answers;
    const int correctChoice = populateAnswers(i, answers, questions);
    do {
      beginQuestionMessage(questions.size());
      printQuestion(i);
      choices = getDefaultChoices(questions.size());
      printChoices(i, choices, questions, answers);
    } while (!getAnswer(prompt, choices, stopQuiz, correctChoice, i->name()));
  }
  // when quitting don't count the current question in the final score
  if (stopQuiz) --_question;
}

int ListQuiz::populateAnswers(const Entry& kanji, Answers& answers, const List& questions) const {
  std::uniform_int_distribution<> randomReading(0, questions.size() - 1);
  std::uniform_int_distribution<> randomCorrect(1, _choiceCount);

  const int correctChoice = randomCorrect(RandomGen);
  // 'sameReading' set is used to prevent more than one choice having the exact same reading
  DataFile::Set sameReading = {kanji->reading()};
  answers[correctChoice] = _question;
  for (int i = 1; i <= _choiceCount; ++i)
    if (i != correctChoice) do {
        if (int choice = randomReading(RandomGen); sameReading.insert(questions[choice]->reading()).second) {
          answers[i] = choice;
          break;
        }
      } while (true);
  return correctChoice;
}

void ListQuiz::printQuestion(const Entry& kanji) const {
  if (isKanjiToReading()) {
    out() << kanji->name();
    auto info = kanji->info(_infoFields);
    if (!info.empty()) out() << "  " << info;
    if (!isTestMode()) _launcher.printExtraTypeInfo(kanji);
  } else
    out() << "Reading:  " << kanji->reading();
  printMeaning(kanji, !isTestMode());
}

void ListQuiz::printChoices(const Entry& kanji, Choices& choices, const List& questions, const Answers& answers) const {
  if (isTestMode()) {
    for (int i = 0; i < _choiceCount; ++i)
      choices['1' + i] = "";
    for (auto& i : answers)
      out() << "    " << i.first << ".  "
            << (isKanjiToReading() ? questions[i.second]->reading() : questions[i.second]->name())
            << '\n';
  } else
    _launcher.printReviewDetails(kanji);
}

bool ListQuiz::getAnswer(const std::string& prompt, Choices& choices, bool& stopQuiz, int correctChoice,
                         const std::string& name) {
  const char answer = get(prompt, choices);
  if (answer == MeaningsOption) {
    toggleMeanings(choices);
    return false;
  }
  if (isQuit(answer))
    stopQuiz = true;
  else if (answer == PrevOption)
    _question -= 2;
  else if (answer - '0' == correctChoice)
    correctMessage();
  else if (answer != SkipOption)
    incorrectMessage(name) << "  (correct answer is " << correctChoice << ")\n";
  return true;
}

} // namespace kanji_tools
