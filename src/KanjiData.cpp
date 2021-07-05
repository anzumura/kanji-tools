#include <kanji/Group.h>
#include <kanji/KanjiData.h>
#include <kanji/MBChar.h>

#include <numeric>
#include <random>

namespace kanji {

namespace fs = std::filesystem;

namespace {

const fs::path N1File = "n1.txt";
const fs::path N2File = "n2.txt";
const fs::path N3File = "n3.txt";
const fs::path N4File = "n4.txt";
const fs::path N5File = "n5.txt";
const fs::path FrequencyFile = "frequency.txt";
const fs::path RadicalsFile = "radicals.txt";
const fs::path StrokesFile = "strokes.txt";
const fs::path WikiStrokesFile = "wiki-strokes.txt";
const fs::path OtherReadingsFile = "other-readings.txt";
const fs::path MeaningGroupFile = "meaning-groups.txt";
const fs::path PatternGroupFile = "pattern-groups.txt";

std::ostream& operator<<(std::ostream& os, const KanjiData::Count& c) {
  os << '[' << c.name << ' ' << std::right << std::setw(4) << c.count << ']';
  if (c.entry.has_value())
    os << std::setw(5) << (**c.entry).frequency() << ", "
       << ((**c.entry).hasLevel() ? toString((**c.entry).level()) : std::string("--")) << ", " << (**c.entry).type()
       << " (" << (**c.entry).number() << ')';
  return os;
}

static std::random_device RandomDevice;
static std::mt19937 RandomGen(RandomDevice());

} // namespace

KanjiData::KanjiData(int argc, const char** argv, bool startQuiz)
  : Data(getDataDir(argc, argv), getDebug(argc, argv)), _n5(_dataDir / N5File, Levels::N5),
    _n4(_dataDir / N4File, Levels::N4), _n3(_dataDir / N3File, Levels::N3), _n2(_dataDir / N2File, Levels::N2),
    _n1(_dataDir / N1File, Levels::N1), _frequency(_dataDir / FrequencyFile, Levels::None) {
  FileList::clearUniqueCheckData(); // cleanup static data used for unique checking
  loadRadicals(FileList::getFile(_dataDir, RadicalsFile));
  loadStrokes(FileList::getFile(_dataDir, StrokesFile));
  loadStrokes(FileList::getFile(_dataDir, WikiStrokesFile), false);
  loadOtherReadings(FileList::getFile(_dataDir, OtherReadingsFile));
  populateJouyou();
  populateJinmei();
  populateExtra();
  processList(_n5);
  processList(_n4);
  processList(_n3);
  processList(_n2);
  processList(_n1);
  processList(_frequency);
  checkStrokes();
  loadGroup(FileList::getFile(_dataDir, MeaningGroupFile), _meaningGroups, _meaningGroupList, GroupType::Meaning);
  loadGroup(FileList::getFile(_dataDir, PatternGroupFile), _patternGroups, _patternGroupList, GroupType::Pattern);
  if (_debug) {
    printStats();
    printGrades();
    printLevels();
    printRadicals();
    printGroups(_meaningGroups, _meaningGroupList);
    printGroups(_patternGroups, _patternGroupList);
  }
  for (int i = _debug ? 3 : 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-b") {
      if (++i == argc) usage("-b must be followed by a file or directory name");
      countKanji(argv[i], true);
      startQuiz = false;
    } else if (arg == "-c") {
      if (++i == argc) usage("-c must be followed by a file or directory name");
      countKanji(argv[i]);
      startQuiz = false;
    } else if (arg == "-h") {
      std::cout << "command line options:\n  -b file: show wide-character counts and full kanji breakdown for 'file'\n"
                << "  -c file: show wide-character counts for 'file'\n"
                << "  -h: show help message for command-line options\n";
      return;
    } else
      usage("unrecognized arg: " + arg);
  }
  if (startQuiz && !_debug) quiz();
}

Levels KanjiData::getLevel(const std::string& k) const {
  if (_n1.exists(k)) return Levels::N1;
  if (_n2.exists(k)) return Levels::N2;
  if (_n3.exists(k)) return Levels::N3;
  if (_n4.exists(k)) return Levels::N4;
  if (_n5.exists(k)) return Levels::N5;
  return Levels::None;
}

int KanjiData::Count::getFrequency() const {
  return entry.has_value() ? (**entry).frequencyOrDefault(MaxFrequency) : MaxFrequency + 1;
}

template<typename Pred>
int KanjiData::processCount(const fs::path& top, const Pred& pred, const std::string& name, bool showBreakdown) const {
  // Furigana in a .txt file is usually a Kanji followed by one or more Hiragana characters inside
  // wide brackets. For now use a 'regex' that matches one Kanji followed by bracketed Hiragana (and
  // replace it with just the Kanji match). This should catch most reasonable examples.
  static const MBCharCount::OptRegex Furigana(std::wstring(L"([") + KanjiRange + L"]{1})（[" + HiraganaRange + L"]+）");
  const bool isKanji = name == "Kanji";
  const bool isUnrecognized = name == "Unrecognized";
  // remove furigana when processing Hiragana or MB-Letter to remove the effect on counts, i.e., furigana
  // in .txt files will artificially inflate Hiragana count (and MB-Letter because of the wide brackets)
  const bool removeFurigana = name == "Hiragana" || name == "MB-Letter";
  MBCharCountIf count(pred, removeFurigana ? Furigana : std::nullopt, "$1");
  count.addFile(top, isKanji || isUnrecognized);
  auto& m = count.map();
  std::set<Count> frequency;
  int total = 0, rank = 0;
  for (const auto& i : m) {
    total += i.second;
    frequency.emplace(i.second, i.first, isKanji ? findKanji(i.first) : std::nullopt);
  }
  if (total && (isUnrecognized || isKanji && showBreakdown)) {
    std::cout << "Rank  [Kanji #] Freq, LV, Type (No.) == Highest Count File (if not found)\n";
    FileList::List missing;
    std::map<Types, int> types;
    for (const auto& i : frequency) {
      std::cout << std::left << std::setw(5) << ++rank << ' ' << i;
      if (i.entry.has_value())
        types[(**i.entry).type()]++;
      else {
        missing.push_back(i.name);
        auto tags = count.tags(i.name);
        if (tags != nullptr) {
          int maxCount = 0;
          std::string file;
          for (const auto& j : *tags)
            if (j.second > maxCount) {
              maxCount = j.second;
              file = j.first;
            }
          std::cout << " == " << file;
        }
      }
      std::cout << '\n';
    }
    if (!types.empty()) {
      std::cout << ">>> Types:\n";
      for (auto i : types)
        std::cout << "  " << i.first << ": " << i.second << '\n';
    }
    FileList::print(missing, "missing");
  }
  if (total)
    std::cout << ">>>" << std::right << std::setw(17) << name << ": " << std::setw(6) << total
              << ", unique: " << std::setw(4) << frequency.size() << " (directories: " << count.directories()
              << ", files: " << count.files() << ")\n";
  return total;
}

void KanjiData::countKanji(const fs::path& top, bool showBreakdown) const {
  static const int IncludeInTotals = 3; // only include Kanji and full-width kana in total and percents
  auto f = [this, &top, showBreakdown](const auto& x, const auto& y) {
    return std::make_pair(this->processCount(top, x, y, showBreakdown), y);
  };
  std::array totals{f([](const auto& x) { return isKanji(x); }, "Kanji"),
                    f([](const auto& x) { return isHiragana(x); }, "Hiragana"),
                    f([](const auto& x) { return isKatakana(x); }, "Katakana"),
                    f([](const auto& x) { return isWidePunctuation(x, false); }, "MB-Punctuation"),
                    f([](const auto& x) { return isWideLetter(x); }, "MB-Letter"),
                    f([](const auto& x) { return !isRecognizedWide(x); }, "Unrecognized")};
  int total = 0;
  for (int i = 0; i < IncludeInTotals; ++i)
    total += totals[i].first;
  std::cout << ">>> Total Kanji+Kana: " << total << " (" << std::fixed << std::setprecision(1);
  for (int i = 0; i < IncludeInTotals; ++i)
    if (totals[i].first) {
      if (totals[i].second != totals[0].second) std::cout << ", ";
      std::cout << totals[i].second << ": " << totals[i].first * 100. / total << "%";
    }
  std::cout << ")\n";
}

char KanjiData::getChoice(const std::string& msg, const Choices& choices, std::optional<char> def) {
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

KanjiData::ListOrder KanjiData::getListOrder() {
  switch (getChoice("List order", {{'b', "from beginning"}, {'e', "from end"}, {'r', "random"}}, 'r')) {
  case 'b': return ListOrder::FromBeginning;
  case 'e': return ListOrder::FromEnd;
  default: return ListOrder::Random;
  }
}

void KanjiData::quiz() const {
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

void KanjiData::quiz(ListOrder listOrder, const List& list, bool printFrequency, bool printGrade,
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

bool KanjiData::includeMember(const Entry& k, MemberType type) {
  return k->hasReading() && (k->is(Types::Jouyou) || type && k->hasLevel() || type > 1 && k->frequency() || type > 2);
}

void KanjiData::quiz(ListOrder listOrder, const GroupList& list) const {
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

void KanjiData::quiz(const GroupList& list, MemberType type) const {
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

void KanjiData::finalScore(int questionsAnswered, int score, const FileList::List& mistakes) {
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
