#include <kanji/Group.h>
#include <kanji/KanjiQuiz.h>

#include <random>

namespace kanji {

static std::random_device RandomDevice;
static std::mt19937 RandomGen(RandomDevice());

KanjiQuiz::KanjiQuiz(int argc, const char** argv) : KanjiData(argc, argv) {
  quiz();
}

char KanjiQuiz::getChoice(const std::string& msg, const Choices& choices, std::optional<char> def) {
  std::string line, prompt(msg + " (");
  std::optional<char> range = std::nullopt;
  for (const auto& i : choices)
    if (i.second.empty()) {
      if (!range.has_value()) {
        prompt += i.first;
        prompt += '-';
      }
      range = i.first;
    } else {
      if (range.has_value()) {
        prompt += *range;
        range = std::nullopt;
      }
      if (i.first != choices.begin()->first) prompt += ", ";
      prompt += i.first;
      prompt += "=" + i.second;
    }
  if (range.has_value()) prompt += *range;
  if (def.has_value()) {
    assert(choices.find(*def) != choices.end());
    prompt += std::string(") default '");
    prompt += *def;
    prompt += "': ";
  } else
    prompt += "): ";
  do {
    std::cout << prompt;
    std::getline(std::cin, line);
    if (line.empty() && def.has_value()) return *def;
  } while (line.length() != 1 || choices.find(line[0]) == choices.end());
  return line[0];
}

KanjiQuiz::ListOrder KanjiQuiz::getListOrder() {
  switch (getChoice("List order", {{'b', "from beginning"}, {'e', "from end"}, {'r', "random"}}, 'r')) {
  case 'b': return ListOrder::FromBeginning;
  case 'e': return ListOrder::FromEnd;
  default: return ListOrder::Random;
  }
}

void KanjiQuiz::quiz() const {
  char c =
    getChoice("Quiz type", {{'f', "freq."}, {'g', "grade"}, {'l', "level"}, {'m', "meanings"}, {'p', "patterns"}}, 'g');
  if (c == 'f') {
    c = getChoice("Choose list",
                  {{'1', "1-500"}, {'2', "501-1000"}, {'3', "1001-1500"}, {'4', "1501-2000"}, {'5', "2001-2501"}});
    quiz(getListOrder(), frequencyList(c - '1'), false, true, true);
  } else if (c == 'g') {
    c = getChoice("Choose grade",
                  {{'1', ""}, {'2', ""}, {'3', ""}, {'4', ""}, {'5', ""}, {'6', ""}, {'s', "Secondary School"}});
    quiz(getListOrder(), gradeList(AllGrades[c == 's' ? 6 : c - '1']), true, false, true);
  } else if (c == 'l') {
    c = getChoice("Choose level", {{'1', "N5"}, {'2', "N4"}, {'3', "N3"}, {'4', "N2"}, {'5', "N1"}});
    quiz(getListOrder(), levelList(AllLevels[c - '1']), true, true, false);
  } else if (c == 'm')
    quiz(getListOrder(), _meaningGroupList);
  else
    quiz(getListOrder(), _patternGroupList);
}

void KanjiQuiz::quiz(ListOrder listOrder, const List& list, bool printFrequency, bool printGrade,
                     bool printLevel) const {
  Choices numberOfChoices;
  for (int i = 2; i < 10; ++i)
    numberOfChoices['0' + i] = "";
  const int choices = getChoice("Number of choices", numberOfChoices, '4') - '0';
  numberOfChoices = {{'q', "quit"}};
  for (int i = 0; i < choices; ++i)
    numberOfChoices['1' + i] = "";
  const char quizStyle = getChoice("Quiz style", {{'k', "kanji to reading"}, {'r', "reading to kanji"}}, 'k');
  const std::string prompt = std::string("  Select correct ") + (quizStyle == 'k' ? "reading" : "kanji");

  List questions;
  FileList::List mistakes;
  for (auto& i : list)
    if (i->hasReading()) questions.push_back(i);
  if (listOrder == ListOrder::FromEnd)
    std::reverse(questions.begin(), questions.end());
  else if (listOrder == ListOrder::Random)
    std::shuffle(questions.begin(), questions.end(), RandomGen);

  std::cout << ">>> Starting quiz for " << questions.size() << " kanji";
  if (questions.size() < list.size())
    std::cout << " (original list had " << list.size() << ", but not all entries have readings yet)";
  std::cout << '\n';
  std::uniform_int_distribution<> randomReading(0, questions.size() - 1);
  std::uniform_int_distribution<> randomCorrect(1, choices);
  int question = 0, score = 0;
  for (auto& i : questions) {
    const int correctChoice = randomCorrect(RandomGen);
    // 'sameReading' set is used to prevent more than one choice having the exact same reading
    std::set<std::string> sameReading = {i->reading()};
    std::map<int, int> answers = {{correctChoice, question}};
    for (int j = 1; j <= choices; ++j) {
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
    std::cout << "\nQuestion " << ++question << '/' << questions.size() << ".  ";
    if (quizStyle == 'k') {
      std::cout << "Kanji: '" << i->name() << "'";
      if (printFrequency && i->frequency()) std::cout << ", Frequency: " << i->frequency();
      if (printGrade && i->grade() != Grades::None) std::cout << ", Grade: " << i->grade();
      if (printLevel && i->level() != Levels::None) std::cout << ", Level: " << i->level();
      if (i->hasMeaning()) std::cout << ", Meaning: '" << i->meaning() << "'";
    } else
      std::cout << "Reading: '" << i->reading() << "'";
    std::cout << '\n';
    for (auto& j : answers)
      std::cout << "    " << j.first << ".  "
                << (quizStyle == 'k' ? questions[j.second]->reading() : questions[j.second]->name()) << '\n';
    const char answer = getChoice(prompt, numberOfChoices);
    if (answer == 'q') {
      // when quitting don't count the current question in the final score
      --question;
      break;
    }
    if (answer - '0' == correctChoice)
      std::cout << "  Correct! (" << ++score << '/' << question << ")\n";
    else {
      std::cout << "  The correct answer is " << correctChoice << '\n';
      mistakes.push_back(i->name());
    }
  }
  finalScore(question, score, mistakes);
}

bool KanjiQuiz::includeMember(const Entry& k, MemberType type) {
  return k->hasReading() && (k->is(Types::Jouyou) || type && k->hasLevel() || type > 1 && k->frequency() || type > 2);
}

void KanjiQuiz::quiz(ListOrder listOrder, const GroupList& list) const {
  const MemberType type = static_cast<MemberType>(
    getChoice("Kanji type", {{'1', "Jōyō"}, {'2', "1+JLPT"}, {'3', "2+Freq."}, {'4', "all"}}, '2') - '1');
  if (listOrder == ListOrder::FromBeginning && type == All)
    quiz(list, type);
  else {
    GroupList newList;
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
    quiz(newList, type);
  }
}

void KanjiQuiz::quiz(const GroupList& list, MemberType type) const {
  int question = 0, score = 0;
  FileList::List mistakes;
  for (auto& i : list) {
    List questions;
    FileList::List readings;
    for (auto& j : i->members())
      if (includeMember(j, type)) {
        questions.push_back(j);
        readings.push_back(j->reading());
      }
    if (!question) std::cout << ">>> Starting quiz for " << list.size() << ' ' << i->type() << " groups\n";
    std::cout << "\nQuestion " << ++question << '/' << list.size() << ".  ";
    std::cout << (i->peers() ? "peers of entry: " : "name: ") << i->name() << ", showing ";
    if (questions.size() == i->members().size())
      std::cout << "all " << questions.size();
    else
      std::cout << questions.size() << " out of " << i->members().size();
    std::cout << " members\n";

    std::shuffle(readings.begin(), readings.end(), RandomGen);
    int count = 0;
    std::map<char, std::string> choices = {{'~', "quit"}};
    for (auto& j : questions) {
      const char choice = (count < 26 ? 'a' + count : 'A' + count);
      std::cout << "  Entry: " << std::setw(3) << count + 1 << "  '" << j->name() << "'\t\tReading:  " << choice
                << "  '" << readings[count++] << "'\n";
      choices[choice] = "";
    }
    std::cout << '\n';
    count = 0;
    std::vector<char> answers;
    for (auto& j : questions) {
      std::string space = (count < 9 ? " " : "");
      const char answer = getChoice("  Select reading for Entry: " + space + std::to_string(++count), choices);
      if (answer == '~') break;
      choices.erase(answer);
      answers.push_back(answer);
    }
    // break out of top quiz loop if use quit in the middle of providing answers
    if (answers.size() < count) {
      --question;
      break;
    }
    count = 0;
    for (auto j : answers) {
      if (questions[count]->reading() != readings[(j <= 'z' ? j - 'a' : j - 'A')]) break;
      ++count;
    }
    if (count == answers.size())
      std::cout << "  Correct! (" << ++score << '/' << question << ")\n";
    else {
      std::cout << "  Incorrect!\n";
      mistakes.push_back(i->name());
    }
  }
  finalScore(question, score, mistakes);
}

void KanjiQuiz::finalScore(int questionsAnswered, int score, const FileList::List& mistakes) {
  std::cout << "\nFinal score: " << score << '/' << questionsAnswered;
  if (!questionsAnswered)
    std::cout << '\n';
  else if (score == questionsAnswered)
    std::cout << " - Perfect!\n";
  else if (!mistakes.empty()) {
    std::cout << " - mistakes:";
    for (auto& i : mistakes)
      std::cout << ' ' << i;
    std::cout << '\n';
  }
}

} // namespace kanji
