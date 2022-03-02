#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/utils/MBUtils.h>

namespace kanji_tools {

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
const fs::path FrequencyReadingsFile = "frequency-readings.txt";
const fs::path RadicalsFile = "radicals.txt";
const fs::path StrokesFile = "strokes.txt";
const fs::path WikiStrokesFile = "wiki-strokes.txt";
const fs::path UcdFile = "ucd.txt";

} // namespace

KanjiData::KanjiData(size_t argc, const char** argv, std::ostream& out,
                     std::ostream& err)
    : Data(getDataDir(argc, argv), getDebugMode(argc, argv), out, err),
      _levels{LevelDataFile(dataDir() / N5File, JlptLevels::N5, debug()),
              LevelDataFile(dataDir() / N4File, JlptLevels::N4, debug()),
              LevelDataFile(dataDir() / N3File, JlptLevels::N3, debug()),
              LevelDataFile(dataDir() / N2File, JlptLevels::N2, debug()),
              LevelDataFile(dataDir() / N1File, JlptLevels::N1, debug())},
      _kyus{KyuDataFile(dataDir() / K10File, KenteiKyus::K10, debug()),
            KyuDataFile(dataDir() / K9File, KenteiKyus::K9, debug()),
            KyuDataFile(dataDir() / K8File, KenteiKyus::K8, debug()),
            KyuDataFile(dataDir() / K7File, KenteiKyus::K7, debug()),
            KyuDataFile(dataDir() / K6File, KenteiKyus::K6, debug()),
            KyuDataFile(dataDir() / K5File, KenteiKyus::K5, debug()),
            KyuDataFile(dataDir() / K4File, KenteiKyus::K4, debug()),
            KyuDataFile(dataDir() / K3File, KenteiKyus::K3, debug()),
            KyuDataFile(dataDir() / KJ2File, KenteiKyus::KJ2, debug()),
            KyuDataFile(dataDir() / K2File, KenteiKyus::K2, debug()),
            KyuDataFile(dataDir() / KJ1File, KenteiKyus::KJ1, debug()),
            KyuDataFile(dataDir() / K1File, KenteiKyus::K1, debug())},
      _frequency(dataDir() / FrequencyFile, debug()) {
  DataFile::clearUniqueCheckData(); // cleanup data used for unique checks
  _ucd.load(DataFile::getFile(dataDir(), UcdFile));
  _radicals.load(DataFile::getFile(dataDir(), RadicalsFile));
  loadStrokes(DataFile::getFile(dataDir(), StrokesFile));
  loadStrokes(DataFile::getFile(dataDir(), WikiStrokesFile), false);
  loadFrequencyReadings(DataFile::getFile(dataDir(), FrequencyReadingsFile));
  populateJouyou();
  populateJinmei();
  populateExtra();
  for (auto& i : _levels) processList(i);
  // Process '_frequency' before '_kyus' in order to create 'Frequency' type
  // before creating 'Kentei' kanji. This way, the 'Frequency' type is more
  // meaningful since it indicates kanji in the top 2501 frequency list, but not
  // in the other more official types like Jouyou or Jinmei. 'Kentei' has many
  // rare kanji so keep it as the last type to be processed (before UcdKanji).
  processList(_frequency);
  for (auto& i : _kyus) processList(i);
  processUcd();
  checkStrokes();
  if (debug()) {
    if (fullDebug()) log(true) << "Finished Loading Data\n>>>\n";
    printStats();
    printGrades();
    if (fullDebug()) {
      printListStats(AllJlptLevels, &Kanji::level, "Level", true);
      printListStats(AllKenteiKyus, &Kanji::kyu, "Kyu", false);
      _radicals.print(*this);
      _ucd.print(*this);
    }
  }
}

JlptLevels KanjiData::level(const std::string& k) const {
  for (auto& i : _levels)
    if (i.exists(k)) return i.level();
  return JlptLevels::None;
}

KenteiKyus KanjiData::kyu(const std::string& k) const {
  for (auto& i : _kyus)
    if (i.exists(k)) return i.kyu();
  return KenteiKyus::None;
}

void KanjiData::noFreq(long f, bool brackets) const {
  if (f) {
    if (brackets)
      out() << " (";
    else
      out() << ' ';
    out() << "nf " << f;
    if (brackets) out() << ')';
  }
}

template<typename T>
void KanjiData::printCount(const std::string& name, T pred,
                           int printExamples) const {
  std::vector<std::pair<KanjiTypes, int>> counts;
  std::map<KanjiTypes, std::vector<std::string>> examples;
  auto total = 0;
  for (auto& l : _types) {
    auto count = 0L;
    if (printExamples)
      for (auto& i : l.second) {
        if (pred(i) && ++count <= printExamples)
          examples[l.first].push_back(i->name());
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
      out() << i.first << ' ' << i.second;
      for (const auto& j : examples[i.first]) out() << ' ' << j;
      total -= i.second;
      if (total) out() << ", ";
    }
    out() << ")\n";
  }
}

void KanjiData::printStats() const {
  log() << "Loaded " << kanjiNameMap().size() << " Kanji (";
  for (auto& i : _types) {
    if (i != *_types.begin()) out() << ' ';
    out() << i.first << ' ' << i.second.size();
  }
  out() << ")\n";
  if (fullDebug()) {
    printCount("  Has JLPT level", [](auto& x) { return x->hasLevel(); });
    printCount("  Has frequency and not in Jouyou or JLPT", [](auto& x) {
      return x->frequency() && x->type() != KanjiTypes::Jouyou &&
             !x->hasLevel();
    });
    printCount("  Jinmei with no frequency and not JLPT", [](auto& x) {
      return x->type() == KanjiTypes::Jinmei && !x->frequency() &&
             !x->hasLevel();
    });
    printCount("  NF (no-frequency)", [](auto& x) { return !x->frequency(); });
    printCount("  Has Strokes", [](auto& x) { return x->strokes() != 0; });
    printCount(
      "  Has Variation Selectors", [](auto& x) { return x->variant(); }, 5);
    printCount("Old Forms", [](auto& x) { return !x->oldNames().empty(); });
  }
}

void KanjiData::printGrades() const {
  log() << "Grade breakdown:\n";
  auto all = 0;
  for (auto& jouyou = _types.at(KanjiTypes::Jouyou); auto i : AllKanjiGrades) {
    const auto grade = [i](auto& x) { return x->grade() == i; };
    if (auto gradeCount = std::count_if(jouyou.begin(), jouyou.end(), grade);
        gradeCount) {
      all += gradeCount;
      log() << "  Total for grade " << i << ": " << gradeCount;
      noFreq(std::count_if(
               jouyou.begin(), jouyou.end(),
               [&grade](auto& x) { return grade(x) && !x->frequency(); }),
             true);
      out() << " (";
      for (const auto level : AllJlptLevels) {
        const auto gradeLevelCount =
          std::count_if(jouyou.begin(), jouyou.end(), [&grade, level](auto& x) {
            return grade(x) && x->level() == level;
          });
        if (gradeLevelCount) {
          gradeCount -= gradeLevelCount;
          out() << level << ' ' << gradeLevelCount;
          if (gradeCount) out() << ", ";
        }
      }
      out() << ")\n";
    }
  }
  log() << "  Total for all grades: " << all << '\n';
}

template<typename T, size_t S>
void KanjiData::printListStats(const IterableEnumArray<T, S>& all,
                               T (Kanji::*p)() const, const std::string& name,
                               bool showNoFrequency) const {
  log() << name << " breakdown:\n";
  auto total = 0;
  for (const auto i : all) {
    std::vector<std::pair<KanjiTypes, int>> counts;
    auto iTotal = 0;
    for (auto& l : _types)
      if (const auto count =
            std::count_if(l.second.begin(), l.second.end(),
                          [i, &p](auto& x) { return ((*x).*p)() == i; });
          count) {
        counts.emplace_back(l.first, count);
        iTotal += count;
      }
    if (iTotal) {
      total += iTotal;
      log() << "  Total for " << name << ' ' << i << ": " << iTotal << " (";
      for (const auto& j : counts) {
        out() << j.first << ' ' << j.second;
        auto& l = _types.at(j.first);
        if (showNoFrequency)
          noFreq(std::count_if(l.begin(), l.end(), [i, &p](auto& x) {
            return ((*x).*p)() == i && !x->frequency();
          }));
        iTotal -= j.second;
        if (iTotal) out() << ", ";
      }
      out() << ")\n";
    }
  }
  log() << "  Total for all " << name << "s: " << total << '\n';
}

} // namespace kanji_tools
