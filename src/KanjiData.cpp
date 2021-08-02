#include <kanji/Group.h>
#include <kanji/KanjiData.h>
#include <kanji/MBUtils.h>

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
const fs::path UcdFile = "ucd.txt";

} // namespace

KanjiData::KanjiData(int argc, const char** argv, std::ostream& out, std::ostream& err)
  : Data(getDataDir(argc, argv), getDebug(argc, argv), out, err), _n5(_dataDir / N5File, Levels::N5, _debug),
    _n4(_dataDir / N4File, Levels::N4, _debug), _n3(_dataDir / N3File, Levels::N3, _debug),
    _n2(_dataDir / N2File, Levels::N2, _debug), _n1(_dataDir / N1File, Levels::N1, _debug),
    _frequency(_dataDir / FrequencyFile, Levels::None, _debug) {
  FileList::clearUniqueCheckData(); // cleanup static data used for unique checking
  _ucd.load(FileList::getFile(_dataDir, UcdFile));
  _radicals.load(FileList::getFile(_dataDir, RadicalsFile));
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
  if (_debug) {
    log(true) << "Finished Loading Data\n>>>\n";
    printStats();
    printGrades();
    printLevels();
    _radicals.print(*this);
    _ucd.print(*this);
  }
}

Levels KanjiData::getLevel(const std::string& k) const {
  if (_n1.exists(k)) return Levels::N1;
  if (_n2.exists(k)) return Levels::N2;
  if (_n3.exists(k)) return Levels::N3;
  if (_n4.exists(k)) return Levels::N4;
  if (_n5.exists(k)) return Levels::N5;
  return Levels::None;
}

void KanjiData::noFreq(int f, bool brackets) const {
  if (f) {
    if (brackets)
      _out << " (";
    else
      _out << ' ';
    _out << "nf " << f;
    if (brackets) _out << ')';
  }
}

template<typename T> void KanjiData::printCount(const std::string& name, T pred, int printExamples) const {
  std::vector<std::pair<Types, int>> counts;
  std::map<Types, std::vector<std::string>> examples;
  int total = 0;
  for (auto& l : _types) {
    int count = 0;
    if (printExamples)
      for (auto& i : l.second) {
        if (pred(i) && ++count <= printExamples) examples[l.first].push_back(i->name());
      }
    else
      count = std::count_if(l.second.begin(), l.second.end(), pred);
    if (count) {
      counts.emplace_back(l.first, count);
      total += count;
    }
  }
  if (total) {
    log() << name << ' ' << total << " (";
    for (const auto& i : counts) {
      _out << i.first << ' ' << i.second;
      for (const auto& j : examples[i.first])
        _out << ' ' << j;
      total -= i.second;
      if (total) _out << ", ";
    }
    _out << ")\n";
  }
}

void KanjiData::printStats() const {
  log() << "Loaded " << _map.size() << " Kanji (";
  for (const auto& i : _types) {
    if (i != *_types.begin()) _out << ' ';
    _out << i.first << ' ' << i.second.size();
  }
  _out << ")\n";
  printCount("  Has JLPT level", [](const auto& x) { return x->hasLevel(); });
  printCount("  Has frequency and not in Jouyou or JLPT",
             [](const auto& x) { return x->frequency() && x->type() != Types::Jouyou && !x->hasLevel(); });
  printCount("  Jinmei with no frequency and not JLPT",
             [](const auto& x) { return x->type() == Types::Jinmei && !x->frequency() && !x->hasLevel(); });
  printCount("  NF (no-frequency)", [](const auto& x) { return !x->frequency(); });
  printCount("  Has Strokes", [](const auto& x) { return x->strokes() != 0; });
  printCount(
    "  Has Variation Selectors", [](const auto& x) { return x->variant(); }, 5);
  printCount("Old Forms", [](const auto& x) { return x->oldName().has_value(); });
  // some old kanjis have a non-zero frequency
  printCount("  Old Has Frequency", [this](const auto& x) { return x->oldFrequency(*this) != 0; });
  // some old kanjis have stroke counts
  printCount("  Old Has Strokes", [this](const auto& x) { return x->oldStrokes(*this) != 0; });
  // no old kanjis should have a JLPT level, i.e.: they all should have Level 'None'
  printCount("  Old Has Level", [this](const auto& x) { return x->oldLevel(*this) != Levels::None; });
  // old kanjis should only have types of LinkedJinmei, Other or None
  for (auto i : AllTypes)
    printCount(std::string("  Old is type ") + toString(i),
               [this, i](const auto& x) { return x->oldName().has_value() && x->oldType(*this) == i; });
}

void KanjiData::printGrades() const {
  log() << "Grade breakdown:\n";
  int all = 0;
  const auto& jouyou = _types.at(Types::Jouyou);
  for (auto i : AllGrades) {
    auto grade = [i](const auto& x) { return x->grade() == i; };
    auto gradeCount = std::count_if(jouyou.begin(), jouyou.end(), grade);
    if (gradeCount) {
      all += gradeCount;
      log() << "  Total for grade " << i << ": " << gradeCount;
      noFreq(
        std::count_if(jouyou.begin(), jouyou.end(), [&grade](const auto& x) { return grade(x) && !x->frequency(); }),
        true);
      _out << " (";
      for (auto level : AllLevels) {
        const auto gradeLevelCount = std::count_if(
          jouyou.begin(), jouyou.end(), [&grade, level](const auto& x) { return grade(x) && x->level() == level; });
        if (gradeLevelCount) {
          gradeCount -= gradeLevelCount;
          _out << level << ' ' << gradeLevelCount;
          if (gradeCount) _out << ", ";
        }
      }
      _out << ")\n";
    }
  }
  log() << "  Total for all grades: " << all << '\n';
}

void KanjiData::printLevels() const {
  log() << "Level breakdown:\n";
  int total = 0;
  for (auto level : AllLevels) {
    std::vector<std::pair<Types, int>> counts;
    int levelTotal = 0;
    for (const auto& l : _types) {
      int count =
        std::count_if(l.second.begin(), l.second.end(), [level](const auto& x) { return x->level() == level; });
      if (count) {
        counts.emplace_back(l.first, count);
        levelTotal += count;
      }
    }
    if (levelTotal) {
      total += levelTotal;
      log() << "  Total for level " << level << ": " << levelTotal << " (";
      for (const auto& j : counts) {
        _out << j.first << ' ' << j.second;
        const auto& l = _types.at(j.first);
        noFreq(
          std::count_if(l.begin(), l.end(), [level](const auto& x) { return x->level() == level && !x->frequency(); }));
        levelTotal -= j.second;
        if (levelTotal) std::cout << ", ";
      }
      _out << ")\n";
    }
  }
  log() << "  Total for all levels: " << total << '\n';
}

} // namespace kanji
