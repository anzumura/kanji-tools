#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/utils/MBUtils.h>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path Jlpt{"jlpt"}, Kentei{"kentei"},
  FrequencyReadingsFile{"frequency-readings"}, RadicalsFile{"radicals"},
  StrokesFile{"strokes"}, WikiStrokesFile{"wiki-strokes"}, UcdFile{"ucd"};

} // namespace

KanjiData::KanjiData(u_int8_t argc, const char** argv, std::ostream& out,
                     std::ostream& err)
    : Data(getDataDir(argc, argv), getDebugMode(argc, argv), out, err),
      _levels{LevelDataFile(dataDir(Jlpt, "n5"), JlptLevels::N5, debug()),
              LevelDataFile(dataDir(Jlpt, "n4"), JlptLevels::N4, debug()),
              LevelDataFile(dataDir(Jlpt, "n3"), JlptLevels::N3, debug()),
              LevelDataFile(dataDir(Jlpt, "n2"), JlptLevels::N2, debug()),
              LevelDataFile(dataDir(Jlpt, "n1"), JlptLevels::N1, debug())},
      _kyus{KyuDataFile(dataDir(Kentei, "k10"), KenteiKyus::K10, debug()),
            KyuDataFile(dataDir(Kentei, "k9"), KenteiKyus::K9, debug()),
            KyuDataFile(dataDir(Kentei, "k8"), KenteiKyus::K8, debug()),
            KyuDataFile(dataDir(Kentei, "k7"), KenteiKyus::K7, debug()),
            KyuDataFile(dataDir(Kentei, "k6"), KenteiKyus::K6, debug()),
            KyuDataFile(dataDir(Kentei, "k5"), KenteiKyus::K5, debug()),
            KyuDataFile(dataDir(Kentei, "k4"), KenteiKyus::K4, debug()),
            KyuDataFile(dataDir(Kentei, "k3"), KenteiKyus::K3, debug()),
            KyuDataFile(dataDir(Kentei, "kJ2"), KenteiKyus::KJ2, debug()),
            KyuDataFile(dataDir(Kentei, "k2"), KenteiKyus::K2, debug()),
            KyuDataFile(dataDir(Kentei, "kJ1"), KenteiKyus::KJ1, debug()),
            KyuDataFile(dataDir(Kentei, "k1"), KenteiKyus::K1, debug())},
      _frequency(dataDir() / "frequency", debug()) {
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
                           size_t printExamples) const {
  std::vector<std::pair<KanjiTypes, size_t>> counts;
  std::map<KanjiTypes, std::vector<std::string>> examples;
  size_t total{};
  for (auto i{AllKanjiTypes.begin()}; auto& l : _types) {
    size_t count{};
    const auto t{*i++};
    if (printExamples)
      for (auto& j : l) {
        if (pred(j) && ++count <= printExamples)
          examples[t].push_back(j->name());
      }
    else
      count = static_cast<size_t>(std::count_if(l.begin(), l.end(), pred));
    if (count) {
      counts.emplace_back(t, count);
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
  for (auto i{AllKanjiTypes.begin()}; auto& j : _types) {
    if (i != AllKanjiTypes.begin()) out() << ' ';
    out() << *i++ << ' ' << j.size();
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
  size_t all{};
  for (auto& jouyou{types(KanjiTypes::Jouyou)}; auto i : AllKanjiGrades) {
    const auto grade{[i](auto& x) { return x->grade() == i; }};
    if (auto gradeCount{static_cast<size_t>(
          std::count_if(jouyou.begin(), jouyou.end(), grade))};
        gradeCount) {
      all += gradeCount;
      log() << "  Total for grade " << i << ": " << gradeCount;
      noFreq(std::count_if(
               jouyou.begin(), jouyou.end(),
               [&grade](auto& x) { return grade(x) && !x->frequency(); }),
             true);
      out() << " (";
      for (const auto level : AllJlptLevels) {
        const auto gradeLevelCount{static_cast<size_t>(
          std::count_if(jouyou.begin(), jouyou.end(), [&grade, level](auto& x) {
            return grade(x) && x->level() == level;
          }))};
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
  size_t total{};
  for (const auto i : all) {
    std::vector<std::pair<KanjiTypes, size_t>> counts;
    size_t iTotal{};
    for (auto j{AllKanjiTypes.begin()}; auto& l : _types) {
      const auto t{*j++};
      if (const auto c{static_cast<size_t>(
            std::count_if(l.begin(), l.end(),
                          [i, &p](auto& x) { return ((*x).*p)() == i; }))};
          c) {
        counts.emplace_back(t, c);
        iTotal += c;
      }
    }
    if (iTotal) {
      total += iTotal;
      log() << "  Total for " << name << ' ' << i << ": " << iTotal << " (";
      for (const auto& j : counts) {
        out() << j.first << ' ' << j.second;
        auto& l{types(j.first)};
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
