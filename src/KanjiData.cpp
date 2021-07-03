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

} // namespace

KanjiData::KanjiData(int argc, const char** argv, bool startQuiz)
  : Data(getDataDir(argc, argv), getDebug(argc, argv)), _n5(_dataDir / N5File, Levels::N5),
    _n4(_dataDir / N4File, Levels::N4), _n3(_dataDir / N3File, Levels::N3), _n2(_dataDir / N2File, Levels::N2),
    _n1(_dataDir / N1File, Levels::N1), _frequency(_dataDir / FrequencyFile, Levels::None) {
  FileList::clearUniqueCheckData(); // cleanup static data used for unique checking
  loadRadicals(FileList::getFile(_dataDir, RadicalsFile));
  loadStrokes(FileList::getFile(_dataDir, StrokesFile));
  loadStrokes(FileList::getFile(_dataDir, WikiStrokesFile), false);
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
  std::string line, promptMsg(msg + " (");
  std::optional<char> range = std::nullopt;
  for (const auto& i : choices)
    if (i.second.empty()) {
      if (!range.has_value()) {
        promptMsg += i.first;
        promptMsg += '-';
      }
      range = i.first;
    } else {
      if (range.has_value()) {
        promptMsg += *range;
        range = std::nullopt;
      }
      if (i.first != choices.begin()->first) promptMsg += ", ";
      promptMsg += i.first;
      promptMsg += "=" + i.second;
    }
  if (def.has_value()) {
    assert(choices.find(*def) != choices.end());
    promptMsg += std::string(") default '");
    promptMsg += *def;
    promptMsg += "': ";
  } else
    promptMsg += "): ";
  do {
    std::cout << promptMsg;
    std::getline(std::cin, line);
    if (line.empty() && def.has_value()) return *def;
  } while (line.length() != 1 || choices.find(line[0]) == choices.end());
  return line[0];
}

void KanjiData::quiz() const {
  char choice = getChoice("Quiz type", {{'f', "frequency"}, {'g', "grade"}, {'l', "JLPT level"}}, 'g');
  if (choice == 'f') {
    choice = getChoice("Choose list",
                       {{'1', "1-500"}, {'2', "501-1000"}, {'3', "1001-1500"}, {'4', "1501-2000"}, {'5', "2001-2501"}});
    quiz(frequencyList(choice - '1'));
  } else if (choice == 'g') {
    choice = getChoice("Choose grade",
                       {{'1', ""}, {'2', ""}, {'3', ""}, {'4', ""}, {'5', ""}, {'6', ""}, {'s', "Secondary School"}});
    quiz(gradeList(AllGrades[choice == 's' ? 6 : choice - '1']));
  } else {
    choice = getChoice("Choose level", {{'1', "N5"}, {'2', "N4"}, {'3', "N3"}, {'4', "N2"}, {'5', "N1"}});
    quiz(levelList(AllLevels[choice - '1']));
  }
}

void KanjiData::quiz(const List& list) const {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  List readings;
  for (auto& i : list)
    if (i->type() == Types::Jouyou || i->type() == Types::Extra) readings.push_back(i);
  std::cout << ">>> Starting quiz for " << readings.size() << " kanji";
  if (readings.size() < list.size())
    std::cout << " (original list had " << list.size() << ", but not all entries have readings yet)";
  std::cout << '\n';
  std::uniform_int_distribution<> randomReading(0, readings.size() - 1);
  std::uniform_int_distribution<> randomCorrect(1, 4);
  int question = 0, score = 0;
  for (auto& i : readings) {
    int correctChoice = randomCorrect(gen);
    // 'sameReading' set is used to prevent more than one choice having the exact same reading
    std::set<std::string> sameReading = {getReading(i)};
    std::map<int, int> choices = {{correctChoice, question}};
    for (int j = 1; j < 5; ++j) {
      if (j != correctChoice) {
        do {
          int choice = randomReading(gen);
          if (sameReading.insert(getReading(readings[choice])).second) {
            choices[j] = choice;
            break;
          }
        } while(true);
      }
    }
    std::cout << "\nQuestion " << ++question << ". '" << i->name() << "', meaning: " << getMeaning(i) << '\n';
    for (auto& j : choices)
      std::cout << "    " << j.first << ".  " << getReading(readings[j.second]) << '\n';
    char answer = getChoice("  Select correct reading", {{'1', ""}, {'2', ""}, {'3', ""}, {'4', ""}, {'q', "quit"}});
    if (answer == 'q') {
      // when quitting don't count the current question in the final score
      --question;
      break;
    }
    if (answer - '0' == correctChoice)
      std::cout << "  Correct! (" << ++score << '/' << question << ")\n";
    else
      std::cout << "  The correct answer is " << correctChoice << '\n';
  }
  std::cout << "\nFinal score: " << score << '/' << question << '\n';
}

const std::string& KanjiData::getReading(const Entry& k) const {
  if (k->type() == Types::Jouyou) return static_cast<const JouyouKanji&>(*k).reading();
  if (k->type() == Types::Extra) return static_cast<const ExtraKanji&>(*k).reading();
  throw std::domain_error("kanji doesn't have a reading");
}

const std::string& KanjiData::getMeaning(const Entry& k) const {
  if (k->type() == Types::Jouyou) return static_cast<const JouyouKanji&>(*k).meaning();
  if (k->type() == Types::Extra) return static_cast<const ExtraKanji&>(*k).meaning();
  throw std::domain_error("kanji doesn't have a meaning");
}

} // namespace kanji
