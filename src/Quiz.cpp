#include <kanji/Group.h>
#include <kanji/Quiz.h>

#include <random>

namespace kanji {

namespace {

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

// Below are some options used in for quiz questions. These are all ascii symbols that come
// before letters and numbers so that 'Choice::get' method displays them at the beginning
// of the list (assuming the other choices are just letters and/or numbers).
constexpr char EditOption = '*';
constexpr char MeaningsOption = '-';
constexpr char SkipOption = '.';
constexpr char QuitOption = '/';

constexpr auto ShowMeanings = "show meanings";
constexpr auto HideMeanings = "hide meanings";

} // namespace

// Top level 'quiz' function

void Quiz::quiz() const {
  reset();
  char c = _choice.get("Quiz type",
                       {{'f', "freq."}, {'g', "grade"}, {'l', "level"}, {'m', "meanings"}, {'p', "patterns"}}, 'g');
  if (c == 'f') {
    c = _choice.get("Choose list",
                    {{'1', "1-500"}, {'2', "501-1000"}, {'3', "1001-1500"}, {'4', "1501-2000"}, {'5', "2001-2501"}});
    // suppress printing 'Freq' since this would work against showing the list in a random order.
    listQuiz(getListOrder(), data().frequencyList(c - '1'), Kanji::AllFields ^ Kanji::FreqField);
  } else if (c == 'g') {
    c = _choice.get("Choose grade",
                    {{'1', ""}, {'2', ""}, {'3', ""}, {'4', ""}, {'5', ""}, {'6', ""}, {'s', "Secondary School"}}, 's');
    // suppress printing 'Grade' since it's the same for every kanji in the list
    listQuiz(getListOrder(), data().gradeList(AllGrades[c == 's' ? 6 : c - '1']), Kanji::AllFields ^ Kanji::GradeField);
  } else if (c == 'l') {
    c = _choice.get("Choose level", {{'1', "N5"}, {'2', "N4"}, {'3', "N3"}, {'4', "N2"}, {'5', "N1"}});
    // suppress printing 'Level' since it's the same for every kanji in the list
    listQuiz(getListOrder(), data().levelList(AllLevels[c - '1']), Kanji::AllFields ^ Kanji::LevelField);
  } else if (c == 'm')
    prepareGroupQuiz(getListOrder(), _groupData.meaningGroups());
  else
    prepareGroupQuiz(getListOrder(), _groupData.patternGroups());
  finalScore();
}

// Helper functions for both List and Group quizzes

Quiz::ListOrder Quiz::getListOrder() const {
  switch (_choice.get("List order", {{'b', "from beginning"}, {'e', "from end"}, {'r', "random"}}, 'r')) {
  case 'b': return ListOrder::FromBeginning;
  case 'e': return ListOrder::FromEnd;
  default: return ListOrder::Random;
  }
}

void Quiz::finalScore() const {
  out() << "\nFinal score: " << _score << '/' << _question;
  if (!_question)
    out() << '\n';
  else if (_score == _question)
    out() << " - Perfect!\n";
  else {
    int skipped = _question - _score - _mistakes.size();
    if (skipped) out() << ", skipped: " << skipped;
    if (!_mistakes.empty()) {
      out() << " - mistakes:";
      for (auto& i : _mistakes)
        out() << ' ' << i;
    }
    out() << '\n';
  }
}

void Quiz::reset() const {
  _question = 0;
  _score = 0;
  _mistakes.clear();
  _showMeanings = false;
}

Choice::Choices Quiz::getDefaultChoices() const {
  return {{MeaningsOption, _showMeanings ? HideMeanings : ShowMeanings}, {SkipOption, "skip"}, {QuitOption, "quit"}};
}

void Quiz::toggleMeanings(Choices& choices) const {
  _showMeanings = !_showMeanings;
  choices[MeaningsOption] = _showMeanings ? HideMeanings : ShowMeanings;
}

void Quiz::printMeaning(const Entry& k) const {
  if (_showMeanings && k->hasMeaning()) out() << " : " << k->meaning();
  out() << '\n';
}

// List Based Quiz

void Quiz::listQuiz(ListOrder listOrder, const List& list, int infoFields) const {
  Choices choices;
  for (int i = 2; i < 10; ++i)
    choices['0' + i] = "";
  const int numberOfChoicesPerQuestion = _choice.get("Number of choices", choices, '4') - '0';
  choices = getDefaultChoices();
  for (int i = 0; i < numberOfChoicesPerQuestion; ++i)
    choices['1' + i] = "";
  const char quizStyle = _choice.get("Quiz style", {{'k', "kanji to reading"}, {'r', "reading to kanji"}}, 'k');
  const std::string prompt = std::string("  Select correct ") + (quizStyle == 'k' ? "reading" : "kanji");

  List questions;
  for (auto& i : list)
    if (i->hasReading()) questions.push_back(i);
  if (listOrder == ListOrder::FromEnd)
    std::reverse(questions.begin(), questions.end());
  else if (listOrder == ListOrder::Random)
    std::shuffle(questions.begin(), questions.end(), RandomGen);

  log(true) << "Starting quiz for " << questions.size() << " kanji";
  if (questions.size() < list.size())
    out() << " (original list had " << list.size() << ", but not all entries have readings yet)";
  out() << '\n';
  std::uniform_int_distribution<> randomReading(0, questions.size() - 1);
  std::uniform_int_distribution<> randomCorrect(1, numberOfChoicesPerQuestion);
  for (auto& i : questions) {
    const int correctChoice = randomCorrect(RandomGen);
    // 'sameReading' set is used to prevent more than one choice having the exact same reading
    FileList::Set sameReading = {i->reading()};
    std::map<int, int> answers = {{correctChoice, _question}};
    for (int j = 1; j <= numberOfChoicesPerQuestion; ++j) {
      if (j != correctChoice) {
        do {
          const int choice = randomReading(RandomGen);
          if (sameReading.insert(questions[choice]->reading()).second) {
            answers[j] = choice;
            break;
          }
        } while (true);
      }
    }
    ++_question;
    bool stopQuiz = false;
    do {
      out() << "\nQuestion " << _question << '/' << questions.size() << ".  ";
      if (quizStyle == 'k') {
        out() << "Kanji:  " << i->name();
        auto info = i->info(infoFields);
        if (!info.empty()) out() << "  (" << info << ")";
      } else
        out() << "Reading: " << i->reading();
      printMeaning(i);
      for (auto& j : answers)
        out() << "    " << j.first << ".  "
              << (quizStyle == 'k' ? questions[j.second]->reading() : questions[j.second]->name()) << '\n';
      const char answer = _choice.get(prompt, choices);
      if (answer == SkipOption) break;
      if (answer == QuitOption)
        stopQuiz = true;
      else if (answer == MeaningsOption)
        toggleMeanings(choices);
      else {
        if (answer - '0' == correctChoice)
          out() << "  Correct! (" << ++_score << '/' << _question << ")\n";
        else {
          out() << "  The correct answer is " << correctChoice << '\n';
          _mistakes.push_back(i->name());
        }
        break;
      }
    } while (!stopQuiz);
    if (stopQuiz) {
      // when quitting don't count the current question in the final score
      --_question;
      break;
    }
  }
}

// Group Based Quiz

bool Quiz::includeMember(const Entry& k, MemberType type) {
  return k->hasReading() && (k->is(Types::Jouyou) || type && k->hasLevel() || type > 1 && k->frequency() || type > 2);
}

void Quiz::prepareGroupQuiz(ListOrder listOrder, const GroupData::List& list) const {
  const MemberType type = static_cast<MemberType>(
    _choice.get("Kanji type", {{'1', "Jōyō"}, {'2', "1+JLPT"}, {'3', "2+Freq."}, {'4', "all"}}, '2') - '1');
  if (listOrder == ListOrder::FromBeginning && type == All)
    groupQuiz(list, type);
  else {
    GroupData::List newList;
    for (const auto& i : list) {
      int memberCount = 0;
      for (auto& j : i->members())
        if (includeMember(j, type)) ++memberCount;
      // only include groups that have 2 or more members after applying the 'include member' filter
      if (memberCount > 1) newList.push_back(i);
    }
    if (listOrder == ListOrder::FromEnd)
      std::reverse(newList.begin(), newList.end());
    else if (listOrder == ListOrder::Random)
      std::shuffle(newList.begin(), newList.end(), RandomGen);
    groupQuiz(newList, type);
  }
}

void Quiz::groupQuiz(const GroupData::List& list, MemberType type) const {
  for (auto& i : list) {
    List questions, readings;
    for (auto& j : i->members())
      if (includeMember(j, type)) {
        questions.push_back(j);
        readings.push_back(j);
      }
    std::shuffle(questions.begin(), questions.end(), RandomGen);
    std::shuffle(readings.begin(), readings.end(), RandomGen);
    if (!_question++) {
      log(true) << "Starting quiz for " << list.size() << ' ' << i->type() << " groups\n";
      if (type) log() << "  Note: " << KanjiLegend << '\n';
    }
    Answers answers;
    Choices choices = getDefaultChoices();
    bool repeatQuestion = false, skipGroup = false, stopQuiz = false;
    do {
      out() << "\nQuestion " << _question << '/' << list.size() << ".  ";
      out() << (i->peers() ? "peers of entry: " : "name: ") << i->name() << ", showing ";
      if (questions.size() == i->members().size())
        out() << "all " << questions.size();
      else
        out() << questions.size() << " out of " << i->members().size();
      out() << " members\n";
      showGroup(questions, readings, choices, repeatQuestion);
      if (getAnswers(answers, questions.size(), choices, skipGroup, stopQuiz)) {
        checkAnswers(answers, questions, readings, i->name());
        break;
      }
      repeatQuestion = true;
    } while (!stopQuiz && !skipGroup);
    if (stopQuiz) {
      --_question;
      break;
    }
  }
}

void Quiz::showGroup(const List& questions, const List& readings, Choices& choices, bool repeatQuestion) const {
  int count = 0;
  for (auto& i : questions) {
    const char choice = (count < 26 ? 'a' + count : 'A' + (count - 26));
    out() << "  Entry: " << std::setw(3) << count + 1 << "  " << i->qualifiedName() << "\t\t" << choice << ":  "
          << readings[count]->reading();
    printMeaning(readings[count]);
    if (!repeatQuestion) choices[choice] = "";
    ++count;
  }
  out() << '\n';
}

bool Quiz::getAnswers(Answers& answers, int totalQuestions, Choices& choices, bool& skipGroup, bool& stopQuiz) const {
  for (int i = answers.size(); i < totalQuestions; ++i) {
    bool meanings = false;
    if (!getAnswer(answers, choices, skipGroup, meanings)) {
      if (meanings)
        toggleMeanings(choices);
      else if (!skipGroup)
        // set 'stopQuiz' to break out of top quiz loop if user quit in the middle of providing answers
        stopQuiz = true;
      return false;
    }
  }
  return true;
}

bool Quiz::getAnswer(Answers& answers, Choices& choices, bool& skipGroup, bool& meanings) const {
  const std::string space = (answers.size() < 9 ? " " : "");
  do {
    if (!answers.empty()) {
      out() << "   ";
      for (int k = 0; k < answers.size(); ++k)
        out() << ' ' << k + 1 << "->" << answers[k];
      out() << '\n';
    }
    const char answer =
      _choice.get("  Select reading for Entry: " + space + std::to_string(answers.size() + 1), choices);
    if (answer == QuitOption) break;
    if (answer == MeaningsOption)
      meanings = true;
    else if (answer == SkipOption)
      skipGroup = true;
    else if (answer == EditOption)
      editAnswer(answers, choices);
    else {
      answers.push_back(answer);
      choices.erase(answer);
      if (answers.size() == 1) choices[EditOption] = "edit";
      return true;
    }
  } while (!skipGroup && !meanings);
  return false;
}

void Quiz::editAnswer(Answers& answers, Choices& choices) const {
  auto getEntry = [this, &answers]() {
    std::map<char, std::string> answersToEdit;
    for (auto k : answers)
      answersToEdit[k] = "";
    const auto index = std::find(answers.begin(), answers.end(), _choice.get("    Answer to edit: ", answersToEdit));
    assert(index != answers.end());
    return std::distance(answers.begin(), index);
  };
  const int entry = answers.size() == 1 ? 0 : getEntry();
  // put the answer back as a choice
  choices[answers[entry]] = "";
  auto newChoices = choices;
  newChoices.erase(EditOption);
  newChoices.erase(MeaningsOption);
  newChoices.erase(SkipOption);
  newChoices.erase(QuitOption);
  const char answer =
    _choice.get("    New reading for Entry: " + std::to_string(entry + 1), newChoices, answers[entry]);
  answers[entry] = answer;
  choices.erase(answer);
}

void Quiz::checkAnswers(const Answers& answers, const List& questions, const List& readings,
                        const std::string& name) const {
  int count = 0;
  for (auto i : answers) {
    int index = (i <= 'z' ? i - 'a' : i - 'A' + 26);
    // Only match on readings (and meanings if '_showMeanings' is true) instead of making
    // sure the kanji is exactly the same since many kanjis have identical readings
    // especially in the 'patterns' groups (and the user has no way to distinguish).
    if (questions[count]->reading() == readings[index]->reading() &&
        (!_showMeanings || questions[count]->meaning() == readings[index]->meaning()))
      ++count;
  }
  if (count == answers.size())
    out() << "  Correct! (" << ++_score << '/' << _question << ")\n";
  else {
    out() << "  Incorrect (got " << count << " right out of " << answers.size() << ")\n";
    _mistakes.push_back(name);
  }
}

} // namespace kanji