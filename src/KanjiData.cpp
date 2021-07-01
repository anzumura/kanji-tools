#include <kanji/Group.h>
#include <kanji/KanjiData.h>
#include <kanji/MBChar.h>

#include <numeric>
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
  os << '[' << c.name << ' ' << std::right << std::setw(4) << c.count << ']';
  if (c.entry.has_value())
    os << std::setw(5) << (**c.entry).frequency() << ", "
       << ((**c.entry).hasLevel() ? toString((**c.entry).level()) : std::string("--")) << ", " << (**c.entry).type()
       << " (" << (**c.entry).number() << ')';
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

template<typename Pred>
int KanjiData::processCount(const fs::path& top, const Pred& pred, const std::string& name) const {
  static MBCharCount::OptRegex RemoveFurigana("（[^）]+）");
  const bool isKanji = name == "Kanji";
  const bool removeFurigana = name == "Hiragana" || name == "MB-Punctuation";
  MBCharCountIf count(pred, removeFurigana ? RemoveFurigana : std::nullopt);
  count.addFile(top, isKanji);
  auto& m = count.map();
  std::set<Count> frequency;
  int total = 0, rank = 0;
  for (const auto& i : m) {
    total += i.second;
    frequency.emplace(i.second, i.first, isKanji ? findKanji(i.first) : std::nullopt);
  }
  if (isKanji) {
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

void KanjiData::countKanji(const fs::path& top) const {
  static const int IncludeInTotals = 3; // only include Kanji and full-width kana in total and percents
  auto f = [this, &top](const auto& x, const auto& y) { return std::make_pair(this->processCount(top, x, y), y); };
  std::array totals{f([this](const auto& x) { return !this->isWideNonKanji(x); }, "Kanji"),
                    f([this](const auto& x) { return this->isHiragana(x); }, "Hiragana"),
                    f([this](const auto& x) { return this->isKatakana(x); }, "Katakana"),
                    f([this](const auto& x) { return this->isWidePunctuation(x, false); }, "MB-Punctuation"),
                    f([this](const auto& x) { return this->isWideLetter(x); }, "MB-Letter"),
                    f([this](const auto& x) { return this->isHalfWidthKana(x); }, "Half-Width Kana")};
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

} // namespace kanji
