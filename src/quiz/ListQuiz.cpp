#include <kanji_tools/quiz/ListQuiz.h>

#include <random>

namespace kanji_tools {

namespace {

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

const Choice::Choices ListQuizStyleChoices({{'k', "kanji to reading"}, {'r', "reading to kanji"}});

constexpr char DefaultListQuizAnswers = '4', DefaultListQuizStyle = 'k';

} // namespace

ListQuiz::ListQuiz(const QuizLauncher& launcher, int question, bool showMeanings, const List& list, int infoFields)
  : Quiz(launcher, question, showMeanings), _infoFields(infoFields) {
  int numberOfChoicesPerQuestion = 1;
  char quizStyle = DefaultListQuizStyle;
  if (Choices choices; isTestMode()) {
    // in quiz mode, numberOfChoicesPerQuestion should be a value from 2 to 9
    for (int i = 2; i < 10; ++i)
      choices['0' + i] = "";
    const char c = getChoice("Number of choices", choices, DefaultListQuizAnswers);
    if (isQuit(c)) return;
    numberOfChoicesPerQuestion = c - '0';
    quizStyle = getChoice("Quiz style", ListQuizStyleChoices, quizStyle);
    if (isQuit(quizStyle)) return;
  }

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

  if (quizStyle == 'k') _launcher.printLegend(infoFields);
  start(questions, numberOfChoicesPerQuestion, quizStyle);
}

void ListQuiz::start(const List& questions, int numberOfChoicesPerQuestion, char quizStyle) {
  static const std::string reviewPrompt("  Select"), quizPrompt("  Select correct ");
  const std::string prompt(isTestMode() ? quizPrompt + (quizStyle == 'k' ? "reading" : "kanji") : reviewPrompt);

  std::uniform_int_distribution<> randomReading(0, questions.size() - 1);
  std::uniform_int_distribution<> randomCorrect(1, numberOfChoicesPerQuestion);

  bool stopQuiz = false;
  for (Choices choices; !stopQuiz && _question < questions.size(); ++_question) {
    const int correctChoice = randomCorrect(RandomGen);
    const Data::Entry& i = questions[_question];
    // 'sameReading' set is used to prevent more than one choice having the exact same reading
    DataFile::Set sameReading = {i->reading()};
    std::map<int, int> answers = {{correctChoice, _question}};
    for (int j = 1; j <= numberOfChoicesPerQuestion; ++j)
      if (j != correctChoice) do {
          if (int choice = randomReading(RandomGen); sameReading.insert(questions[choice]->reading()).second) {
            answers[j] = choice;
            break;
          }
        } while (true);
    do {
      beginQuestionMessage(questions.size());
      if (quizStyle == 'k') {
        out() << i->name();
        auto info = i->info(_infoFields);
        if (!info.empty()) out() << "  " << info;
        if (!isTestMode()) _launcher.printExtraTypeInfo(i);
      } else
        out() << "Reading:  " << i->reading();
      printMeaning(i, !isTestMode());
      choices = getDefaultChoices(questions.size());
      if (isTestMode()) {
        for (int i = 0; i < numberOfChoicesPerQuestion; ++i)
          choices['1' + i] = "";
        for (auto& j : answers)
          out() << "    " << j.first << ".  "
                << (quizStyle == 'k' ? questions[j.second]->reading() : questions[j.second]->name()) << '\n';
      } else
        _launcher.printReviewDetails(i);
      const char answer = getChoice(prompt, choices);
      if (isQuit(answer))
        stopQuiz = true;
      else if (answer == MeaningsOption)
        toggleMeanings(choices);
      else {
        if (answer == PrevOption)
          _question -= 2;
        else if (answer - '0' == correctChoice)
          correctMessage();
        else if (answer != SkipOption)
          incorrectMessage(i->name()) << "  (correct answer is " << correctChoice << ")\n";
        break;
      }
    } while (!stopQuiz);
  }
  // when quitting don't count the current question in the final score
  if (stopQuiz) --_question;
}

} // namespace kanji_tools
