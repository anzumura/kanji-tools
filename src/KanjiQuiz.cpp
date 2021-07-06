#include <kanji/Group.h>
#include <kanji/KanjiQuiz.h>
#include <kanji/MBChar.h>

#include <fstream>
#include <random>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

namespace {

const fs::path MeaningGroupFile = "meaning-groups.txt";
const fs::path PatternGroupFile = "pattern-groups.txt";

std::random_device RandomDevice;
std::mt19937 RandomGen(RandomDevice());

// Options used in for quiz questions - picked ascii symbols that come before letters so that 'getChoice'
// method displays them at the beginning of the set of choices.
constexpr char EditOption = '*';
constexpr char MeaningOption = '-';
constexpr char SkipOption = '.';
constexpr char QuitOption = '/';

constexpr auto KanjiLegend = "Jōyō=no suffix, JLPT=', Freq=\", Jinmei=^, Linked Jinmei=~, Extra=+, ...=*";
constexpr auto ShowMeanings = "show meanings";
constexpr auto HideMeanings = "hide meanings";

} // namespace

KanjiQuiz::KanjiQuiz(int argc, const char** argv, std::ostream& out, std::ostream& err, std::istream& in)
  : KanjiData(argc, argv, out, err), _in(in), _question(0), _score(0), _showMeanings(false) {
  loadGroup(FileList::getFile(_dataDir, MeaningGroupFile), _meaningGroups, _meaningGroupList, GroupType::Meaning);
  loadGroup(FileList::getFile(_dataDir, PatternGroupFile), _patternGroups, _patternGroupList, GroupType::Pattern);
  if (_debug) {
    printGroups(_meaningGroups, _meaningGroupList);
    printGroups(_patternGroups, _patternGroupList);
  }
}

// functions related to loading and 'debug' printing Groups

bool KanjiQuiz::checkInsert(const std::string& name, GroupMap& groups, const GroupEntry& group) const {
  auto i = groups.insert(std::make_pair(name, group));
  if (!i.second)
    printError(name + " from Group " + std::to_string(group->number()) + " already in group " +
               i.first->second->toString());
  return i.second;
}

void KanjiQuiz::loadGroup(const std::filesystem::path& file, GroupMap& groups, GroupList& list, GroupType type) {
  int lineNumber = 1, numberCol = -1, nameCol = -1, membersCol = -1;
  auto error = [&lineNumber, &file](const std::string& s, bool printLine = true) {
    usage(s + (printLine ? " - line: " + std::to_string(lineNumber) : "") + ", file: " + file.string());
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name", false);
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 3> cols;
  for (std::string line; std::getline(f, line); ++lineNumber) {
    int pos = 0;
    std::stringstream ss(line);
    if (numberCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Number")
          setCol(numberCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "Members")
          setCol(membersCol, pos);
        else
          error("unrecognized column '" + token + "'", false);
      if (pos != cols.size()) error("not enough columns", false);
    } else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        if (pos == cols.size()) error("too many columns");
        cols[pos] = token;
      }
      if (pos != cols.size()) error("not enough columns");
      std::string number(cols[numberCol]), name(cols[nameCol]), token;
      FileList::List kanjis;
      const bool peers = name.empty();
      if (type == GroupType::Meaning) {
        if (peers) error("Meaning group must have a name");
      } else if (!peers) // if populated, 'name colum' is the first member of a Pattern group
        kanjis.emplace_back(name);
      for (std::stringstream members(cols[membersCol]); std::getline(members, token, ',');)
        kanjis.emplace_back(token);
      List memberKanjis;
      for (const auto& i : kanjis) {
        const auto memberKanji = findKanji(i);
        if (memberKanji.has_value())
          memberKanjis.push_back(*memberKanji);
        else
          printError("failed to find member " + i + " in group " + number);
      }
      if (memberKanjis.empty()) error("group " + number + " has no valid members");
      GroupEntry group;
      if (type == GroupType::Meaning)
        group = std::make_shared<MeaningGroup>(FileListKanji::toInt(number), name, memberKanjis);
      else
        group = std::make_shared<PatternGroup>(FileListKanji::toInt(number), memberKanjis, peers);
      for (const auto& i : memberKanjis)
        checkInsert(i->name(), groups, group);
      list.push_back(group);
    }
  }
}

void KanjiQuiz::printGroups(const GroupMap& groups, const GroupList& groupList) const {
  log() << "Loaded " << groups.size() << " kanji into " << groupList.size() << " groups\n>>> " << KanjiLegend << ":\n";
  for (const auto& i : groupList) {
    if (i->type() == GroupType::Meaning) {
      auto len = MBChar::length(i->name());
      _out << '[' << i->name()
           << (len == 1     ? "　　"
                 : len == 2 ? "　"
                            : "")
           << ' ' << std::setw(2) << std::setfill(' ') << i->members().size() << "] :";
      for (const auto& j : i->members())
        _out << ' ' << j->qualifiedName();
    } else {
      _out << '[' << std::setw(3) << std::setfill('0') << i->number() << "] ";
      for (const auto& j : i->members())
        if (j == i->members()[0]) {
          if (i->peers())
            _out << "　 : " << j->qualifiedName();
          else
            _out << j->qualifiedName() << ':';
        } else
          _out << ' ' << j->qualifiedName();
    }
    _out << '\n';
  }
}

// Top level 'quiz' function

void KanjiQuiz::quiz() const {
  reset();
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
  finalScore();
}

// Helper functions for getting choices and printing score

void KanjiQuiz::addChoices(std::string& prompt, const Choices& choices) {
  std::optional<char> rangeStart = std::nullopt;
  char prevChar;
  auto completeRange = [&prompt, &rangeStart, &prevChar]() {
    if (prevChar != *rangeStart) {
      prompt += '-';
      prompt += prevChar;
    }
  };
  for (const auto& i : choices) {
    if (i.second.empty()) {
      if (!rangeStart.has_value()) {
        if (i != *choices.begin()) prompt += ", ";
        prompt += i.first;
        rangeStart = i.first;
      } else if (i.first - prevChar > 1) {
        // complete range if there was a jump of more than one value
        completeRange();
        prompt += ", ";
        prompt += i.first;
        rangeStart = i.first;
      }
    } else {
      // second value isn't empty so complete any ranges if needed
      if (rangeStart.has_value()) {
        completeRange();
        rangeStart = std::nullopt;
      }
      if (i.first != choices.begin()->first) prompt += ", ";
      prompt += i.first;
      prompt += "=" + i.second;
    }
    prevChar = i.first;
  }
  if (rangeStart.has_value()) completeRange();
}

char KanjiQuiz::getChoice(const std::string& msg, const Choices& choices, std::optional<char> def) const {
  std::string line, prompt(msg + " (");
  addChoices(prompt, choices);
  if (def.has_value()) {
    assert(choices.find(*def) != choices.end());
    prompt += ") default '";
    prompt += *def;
    prompt += "': ";
  } else
    prompt += "): ";
  do {
    _out << prompt;
    std::getline(_in, line);
    if (line.empty() && def.has_value()) return *def;
  } while (line.length() != 1 || choices.find(line[0]) == choices.end());
  return line[0];
}

KanjiQuiz::ListOrder KanjiQuiz::getListOrder() const {
  switch (getChoice("List order", {{'b', "from beginning"}, {'e', "from end"}, {'r', "random"}}, 'r')) {
  case 'b': return ListOrder::FromBeginning;
  case 'e': return ListOrder::FromEnd;
  default: return ListOrder::Random;
  }
}

void KanjiQuiz::finalScore() const {
  _out << "\nFinal score: " << _score << '/' << _question;
  if (!_question)
    _out << '\n';
  else if (_score == _question)
    _out << " - Perfect!\n";
  else {
    int skipped = _question - _score - _mistakes.size();
    if (skipped) _out << ", skipped: " << skipped;
    if (!_mistakes.empty()) {
      _out << " - mistakes:";
      for (auto& i : _mistakes)
        _out << ' ' << i;
    }
    _out << '\n';
  }
}

void KanjiQuiz::reset() const {
  _question = 0;
  _score = 0;
  _mistakes.clear();
  _showMeanings = false;
}

// List Based Quiz

void KanjiQuiz::quiz(ListOrder listOrder, const List& list, bool printFrequency, bool printGrade,
                     bool printLevel) const {
  Choices numberOfChoices;
  for (int i = 2; i < 10; ++i)
    numberOfChoices['0' + i] = "";
  const int choices = getChoice("Number of choices", numberOfChoices, '4') - '0';
  numberOfChoices = {{QuitOption, "quit"}};
  for (int i = 0; i < choices; ++i)
    numberOfChoices['1' + i] = "";
  const char quizStyle = getChoice("Quiz style", {{'k', "kanji to reading"}, {'r', "reading to kanji"}}, 'k');
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
    _out << " (original list had " << list.size() << ", but not all entries have readings yet)";
  _out << '\n';
  std::uniform_int_distribution<> randomReading(0, questions.size() - 1);
  std::uniform_int_distribution<> randomCorrect(1, choices);
  for (auto& i : questions) {
    const int correctChoice = randomCorrect(RandomGen);
    // 'sameReading' set is used to prevent more than one choice having the exact same reading
    FileList::Set sameReading = {i->reading()};
    std::map<int, int> answers = {{correctChoice, _question}};
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
    _out << "\nQuestion " << ++_question << '/' << questions.size() << ".  ";
    if (quizStyle == 'k') {
      _out << "Kanji: '" << i->name() << "'";
      if (printFrequency && i->frequency()) _out << ", Frequency: " << i->frequency();
      if (printGrade && i->grade() != Grades::None) _out << ", Grade: " << i->grade();
      if (printLevel && i->level() != Levels::None) _out << ", Level: " << i->level();
      if (i->hasMeaning()) _out << ", Meaning: '" << i->meaning() << "'";
    } else
      _out << "Reading: '" << i->reading() << "'";
    _out << '\n';
    for (auto& j : answers)
      _out << "    " << j.first << ".  "
           << (quizStyle == 'k' ? questions[j.second]->reading() : questions[j.second]->name()) << '\n';
    const char answer = getChoice(prompt, numberOfChoices);
    if (answer == QuitOption) {
      // when quitting don't count the current question in the final score
      --_question;
      break;
    }
    if (answer - '0' == correctChoice)
      _out << "  Correct! (" << ++_score << '/' << _question << ")\n";
    else {
      _out << "  The correct answer is " << correctChoice << '\n';
      _mistakes.push_back(i->name());
    }
  }
}

// Group Based Quiz

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
  for (auto& i : list) {
    List questions, readings;
    for (auto& j : i->members())
      if (includeMember(j, type)) {
        questions.push_back(j);
        readings.push_back(j);
      }
    if (!_question) {
      log(true) << "Starting quiz for " << list.size() << ' ' << i->type() << " groups\n";
      if (type) log() << "  Note: " << KanjiLegend << '\n';
    }
    _out << "\nQuestion " << ++_question << '/' << list.size() << ".  ";
    _out << (i->peers() ? "peers of entry: " : "name: ") << i->name() << ", showing ";
    if (questions.size() == i->members().size())
      _out << "all " << questions.size();
    else
      _out << questions.size() << " out of " << i->members().size();
    _out << " members\n";

    std::shuffle(questions.begin(), questions.end(), RandomGen);
    std::shuffle(readings.begin(), readings.end(), RandomGen);

    Answers answers;
    Choices choices = {{MeaningOption, ShowMeanings}, {SkipOption, "skip"}, {QuitOption, "quit"}};
    bool repeatQuestion = false, skipGroup = false, stopQuiz = false;
    do {
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

void KanjiQuiz::showGroup(const List& questions, const List& readings, Choices& choices, bool repeatQuestion) const {
  int count = 0;
  for (auto& j : questions) {
    const char choice = (count < 26 ? 'a' + count : 'A' + (count - 26));
    _out << "  Entry: " << std::setw(3) << count + 1 << "  " << j->qualifiedName() << "\t\t" << choice << ":  "
         << readings[count]->reading();
    if (_showMeanings && readings[count]->hasMeaning()) _out << " : " << readings[count]->meaning();
    _out << '\n';
    if (!repeatQuestion) choices[choice] = "";
    ++count;
  }
  _out << '\n';
}

bool KanjiQuiz::getAnswers(Answers& answers, int totalQuestions, Choices& choices, bool& skipGroup,
                           bool& stopQuiz) const {
  for (int i = answers.size(); i < totalQuestions; ++i) {
    bool toggleMeanings = false;
    if (!getAnswer(answers, choices, skipGroup, toggleMeanings)) {
      if (toggleMeanings) {
        _showMeanings = !_showMeanings;
        choices[MeaningOption] = _showMeanings ? HideMeanings : ShowMeanings;
        _out << '\n';
      } else if (!skipGroup) {
        // break out of top quiz loop if user quit in the middle of providing answers
        stopQuiz = true;
      }
      return false;
    }
  }
  return true;
}

bool KanjiQuiz::getAnswer(Answers& answers, Choices& choices, bool& skipGroup, bool& toggleMeaning) const {
  const std::string space = (answers.size() < 9 ? " " : "");
  do {
    if (!answers.empty()) {
      _out << "   ";
      for (int k = 0; k < answers.size(); ++k)
        _out << ' ' << k + 1 << "->" << answers[k];
      _out << '\n';
    }
    const char answer = getChoice("  Select reading for Entry: " + space + std::to_string(answers.size() + 1), choices);
    if (answer == QuitOption) break;
    if (answer == MeaningOption)
      toggleMeaning = true;
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
  } while (!skipGroup && !toggleMeaning);
  return false;
}

void KanjiQuiz::editAnswer(Answers& answers, Choices& choices) const {
  auto getEntry = [this, &answers]() {
    std::map<char, std::string> answersToEdit;
    for (auto k : answers)
      answersToEdit[k] = "";
    const auto index = std::find(answers.begin(), answers.end(), getChoice("    Answer to edit: ", answersToEdit));
    assert(index != answers.end());
    return std::distance(answers.begin(), index);
  };
  const int entry = answers.size() == 1 ? 0 : getEntry();
  // put the answer back as a choice
  choices[answers[entry]] = "";
  auto newChoices = choices;
  newChoices.erase(EditOption);
  newChoices.erase(MeaningOption);
  newChoices.erase(SkipOption);
  newChoices.erase(QuitOption);
  const char answer = getChoice("    New reading for Entry: " + std::to_string(entry + 1), newChoices, answers[entry]);
  answers[entry] = answer;
  choices.erase(answer);
}

void KanjiQuiz::checkAnswers(const Answers& answers, const List& questions, const List& readings,
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
    _out << "  Correct! (" << ++_score << '/' << _question << ")\n";
  else {
    _out << "  Incorrect (got " << count << " right out of " << answers.size() << ")\n";
    _mistakes.push_back(name);
  }
}

} // namespace kanji
