#include <kanji/Group.h>
#include <kanji/KanjiData.h>
#include <kanji/MBUtils.h>

namespace kanji {

namespace fs = std::filesystem;

namespace {

const fs::path N5File = "jlpt/n5.txt";
const fs::path N4File = "jlpt/n4.txt";
const fs::path N3File = "jlpt/n3.txt";
const fs::path N2File = "jlpt/n2.txt";
const fs::path N1File = "jlpt/n1.txt";
const fs::path K10File = "kentei/k10.txt";
const fs::path K9File = "kentei/k9.txt";
const fs::path K8File = "kentei/k8.txt";
const fs::path K7File = "kentei/k7.txt";
const fs::path K6File = "kentei/k6.txt";
const fs::path K5File = "kentei/k5.txt";
const fs::path K4File = "kentei/k4.txt";
const fs::path K3File = "kentei/k3.txt";
const fs::path KJ2File = "kentei/kj2.txt";
const fs::path K2File = "kentei/k2.txt";
const fs::path KJ1File = "kentei/kj1.txt";
const fs::path K1File = "kentei/k1.txt";
const fs::path FrequencyFile = "frequency.txt";
const fs::path RadicalsFile = "radicals.txt";
const fs::path StrokesFile = "strokes.txt";
const fs::path WikiStrokesFile = "wiki-strokes.txt";
const fs::path OtherReadingsFile = "other-readings.txt";
const fs::path UcdFile = "ucd.txt";

} // namespace

KanjiData::KanjiData(int argc, const char** argv, std::ostream& out, std::ostream& err)
  : Data(getDataDir(argc, argv), getDebug(argc, argv), out, err),
    _levels{LevelFileList(_dataDir / N5File, Levels::N5, _debug), LevelFileList(_dataDir / N4File, Levels::N4, _debug),
            LevelFileList(_dataDir / N3File, Levels::N3, _debug), LevelFileList(_dataDir / N2File, Levels::N2, _debug),
            LevelFileList(_dataDir / N1File, Levels::N1, _debug)},
    _kyus{KyuFileList(_dataDir / K10File, Kyus::K10, _debug), KyuFileList(_dataDir / K9File, Kyus::K9, _debug),
          KyuFileList(_dataDir / K8File, Kyus::K8, _debug),   KyuFileList(_dataDir / K7File, Kyus::K7, _debug),
          KyuFileList(_dataDir / K6File, Kyus::K6, _debug),   KyuFileList(_dataDir / K5File, Kyus::K5, _debug),
          KyuFileList(_dataDir / K4File, Kyus::K4, _debug),   KyuFileList(_dataDir / K3File, Kyus::K3, _debug),
          KyuFileList(_dataDir / KJ2File, Kyus::KJ2, _debug), KyuFileList(_dataDir / K2File, Kyus::K2, _debug),
          KyuFileList(_dataDir / KJ1File, Kyus::KJ1, _debug), KyuFileList(_dataDir / K1File, Kyus::K1, _debug)},
    _frequency(_dataDir / FrequencyFile, _debug) {
  FileList::clearUniqueCheckData(); // cleanup static data used for unique checking
  _ucd.load(FileList::getFile(_dataDir, UcdFile));
  _radicals.load(FileList::getFile(_dataDir, RadicalsFile));
  loadStrokes(FileList::getFile(_dataDir, StrokesFile));
  loadStrokes(FileList::getFile(_dataDir, WikiStrokesFile), false);
  loadOtherReadings(FileList::getFile(_dataDir, OtherReadingsFile));
  populateJouyou();
  populateJinmei();
  populateExtra();
  for (auto& i : _levels)
    processList(i);
  // Process '_frequency' before '_kyus' in order create 'Other' type first before creating
  // 'Kentei' kanji. This way, the 'Other' type is more meaningful since it indicates kanji
  // in the top 2501 frequency list, but not in other more official types like Jouyou or
  // Jinmei. 'Kentei' has many rare kanji so keep it as the last type to be processed.
  processList(_frequency);
  for (auto& i : _kyus)
    processList(i);
  checkStrokes();
  if (_debug) {
    log(true) << "Finished Loading Data\n>>>\n";
    printStats();
    printGrades();
    printListStats(AllLevels, &Kanji::level, "Level", true);
    printListStats(AllKyus, &Kanji::kyu, "Kyu", false);
    _radicals.print(*this);
    _ucd.print(*this);
  }
}

Levels KanjiData::getLevel(const std::string& k) const {
  for (auto& i : _levels)
    if (i.exists(k)) return i.level();
  return Levels::None;
}

Kyus KanjiData::getKyu(const std::string& k) const {
  for (auto& i : _kyus)
    if (i.exists(k)) return i.kyu();
  return Kyus::None;
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
  printCount("Old Forms", [](const auto& x) { return !x->oldNames().empty(); });
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

template<typename T, size_t S>
void KanjiData::printListStats(const std::array<T, S>& all, T (Kanji::*p)() const, const std::string& name,
                               bool showNoFrequency) const {
  log() << name << " breakdown:\n";
  int total = 0;
  for (auto i : all) {
    std::vector<std::pair<Types, int>> counts;
    int iTotal = 0;
    for (const auto& l : _types) {
      int count = std::count_if(l.second.begin(), l.second.end(), [i, &p](const auto& x) { return ((*x).*p)() == i; });
      if (count) {
        counts.emplace_back(l.first, count);
        iTotal += count;
      }
    }
    if (iTotal) {
      total += iTotal;
      log() << "  Total for " << name << i << ": " << iTotal << " (";
      for (const auto& j : counts) {
        _out << j.first << ' ' << j.second;
        const auto& l = _types.at(j.first);
        if (showNoFrequency)
          noFreq(
            std::count_if(l.begin(), l.end(), [i, &p](const auto& x) { return ((*x).*p)() == i && !x->frequency(); }));
        iTotal -= j.second;
        if (iTotal) std::cout << ", ";
      }
      _out << ")\n";
    }
  }
  log() << "  Total for all " << name << "s: " << total << '\n';
}

} // namespace kanji
