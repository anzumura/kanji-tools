#include <kanji/Group.h>
#include <kanji/KanjiData.h>
#include <kanji/MBChar.h>

#include <tuple>

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
const fs::path HiraganaFile = "hiragana.txt";
const fs::path KatakanaFile = "katakana.txt";
const fs::path HalfwidthKanaFile = "halfwidth-kana.txt";
const fs::path WideLettersFile = "wide-letters.txt";
const fs::path PunctuationFile = "punctuation.txt";
const fs::path MeaningGroupFile = "meaning-groups.txt";
const fs::path PatternGroupFile = "pattern-groups.txt";

std::ostream& operator<<(std::ostream& os, const KanjiData::Count& c) {
  os << '[' << c.name << ' ' << std::right << std::setw(3) << c.count << ']';
  if (c.entry.has_value())
    os << " - freq: " << std::setw(4) << (**c.entry).frequency() << ", "
       << ((**c.entry).hasLevel() ? toString((**c.entry).level()) : std::string("--")) << ", " << (**c.entry).type()
       << ": " << (**c.entry).number();
  return os;
}

} // namespace

KanjiData::KanjiData(int argc, const char** argv)
  : Data(getDataDir(argc, argv), getDebug(argc, argv)), _n5(_dataDir / N5File, Levels::N5),
    _n4(_dataDir / N4File, Levels::N4), _n3(_dataDir / N3File, Levels::N3), _n2(_dataDir / N2File, Levels::N2),
    _n1(_dataDir / N1File, Levels::N1), _frequency(_dataDir / FrequencyFile, Levels::None),
    _hiragana(_dataDir / HiraganaFile), _katakana(_dataDir / KatakanaFile),
    _halfwidthKana(_dataDir / HalfwidthKanaFile), _wideLetters(_dataDir / WideLettersFile),
    _punctuation(_dataDir / PunctuationFile) {
  FileList::clearUniqueCheckData(); // cleanup static data used for unique checking
  loadRadicals(FileList::getFile(_dataDir, RadicalsFile));
  loadStrokes(FileList::getFile(_dataDir, StrokesFile));
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
  for (int i = 2; i < argc; ++i)
    if (std::string(argv[i]) == "-count" && i + 1 < argc) countKanji(argv[i + 1]);
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

void KanjiData::countKanji(const fs::path& top) const {
  auto pred = [this](const auto& x) { return !this->isWideNonKanji(x); };
  MBCharCount count(pred);
  count.addFile(top);
  auto& m = count.map();
  std::set<Count> frequency;
  int total = 0;
  for (const auto& i : m) {
    total += i.second;
    frequency.emplace(i.second, i.first, findKanji(i.first));
  }
  std::cout << "Total kanji: " << total << ", unique: " << frequency.size() << ", directories: " << count.directories()
            << ", files: " << count.files() << '\n';
  total = 0;
  FileList::List missing;
  std::map<Types, int> types;
  for (const auto& i : frequency) {
    std::cout << "  " << std::left << std::setw(5) << ++total << ' ' << i << '\n';
    if (!i.entry.has_value())
      missing.push_back(i.name);
    else
      types[(**i.entry).type()]++;
  }
  FileList::print(missing, "missing");
  std::cout << ">>> Type Totals:\n";
  for (auto i : types)
    std::cout << "  " << i.first << ": " << i.second << '\n';
}

} // namespace kanji
