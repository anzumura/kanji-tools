#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/RealKanjiData.h>
#include <kanji_tools/utils/Utf8.h>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path UcdFile{"ucd"}, RadicalsFile{"radicals"},
    FrequencyReadingsFile{"frequency-readings"},
    LinkedJinmeiFile{"linked-jinmei"}, Jlpt{"jlpt"}, Kentei{"kentei"};

constexpr auto MaxVariantSelectorExamples{5};

} // namespace

RealKanjiData::RealKanjiData(
    const Args& args, std::ostream& out, std::ostream& err)
    : KanjiData{getDataDir(args), getDebugMode(args), out, err},
      _levels{dataFile(JlptLevels::N5), dataFile(JlptLevels::N4),
          dataFile(JlptLevels::N3), dataFile(JlptLevels::N2),
          dataFile(JlptLevels::N1)},
      _kyus{dataFile(KenteiKyus::K10), dataFile(KenteiKyus::K9),
          dataFile(KenteiKyus::K8), dataFile(KenteiKyus::K7),
          dataFile(KenteiKyus::K6), dataFile(KenteiKyus::K5),
          dataFile(KenteiKyus::K4), dataFile(KenteiKyus::K3),
          dataFile(KenteiKyus::KJ2), dataFile(KenteiKyus::K2),
          dataFile(KenteiKyus::KJ1), dataFile(KenteiKyus::K1)},
      _frequency{dataDir() / "frequency"} {
  KanjiListFile::clearUniqueCheckData(); // cleanup data used for unique checks
  ucd().load(KanjiListFile::getFile(dataDir(), UcdFile));
  radicals().load(KanjiListFile::getFile(dataDir(), RadicalsFile));
  loadFrequencyReadings(
      KanjiListFile::getFile(dataDir(), FrequencyReadingsFile));
  populateJouyou();
  populateLinkedKanji(KanjiListFile::getFile(dataDir(), LinkedJinmeiFile));
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
      radicals().print(*this);
      ucd().print(*this);
    }
  }
}

Kanji::Frequency RealKanjiData::frequency(const String& s) const {
  return _frequency.getIndex(s);
}

JlptLevels RealKanjiData::level(const String& k) const {
  for (auto& i : _levels)
    if (i.exists(k)) return i.level();
  return JlptLevels::None;
}

KenteiKyus RealKanjiData::kyu(const String& k) const {
  for (auto& i : _kyus)
    if (i.exists(k)) return i.kyu();
  return KenteiKyus::None;
}

void RealKanjiData::noFreq(std::ptrdiff_t f, bool brackets) const {
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
void RealKanjiData::printCount(
    const String& name, T pred, size_t printExamples) const {
  std::vector<std::pair<KanjiTypes, size_t>> counts;
  std::map<KanjiTypes, std::vector<String>> examples;
  size_t total{};
  for (auto i{AllKanjiTypes.begin()}; auto& l : types()) {
    size_t count{};
    const auto t{*i++};
    if (printExamples)
      for (auto& j : l) {
        if (pred(j) && ++count <= printExamples)
          examples[t].emplace_back(j->name());
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

void RealKanjiData::printStats() const {
  log() << "Loaded " << kanjiNameMap().size() << " Kanji (";
  for (auto i{AllKanjiTypes.begin()}; auto& j : types()) {
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
    printCount("  Has Variant Strokes",
        [](auto& x) { return x->strokes().hasVariant(); });
    printCount(
        "  Has Variation Selectors", [](auto& x) { return x->variant(); },
        MaxVariantSelectorExamples);
    printCount("Old Forms", [](auto& x) { return !x->oldNames().empty(); });
  }
}

void RealKanjiData::printGrades() const {
  log() << "Grade breakdown:\n";
  size_t all{};
  for (auto& jouyou{types(KanjiTypes::Jouyou)}; auto i : AllKanjiGrades) {
    const auto grade{[i](auto& x) { return x->grade() == i; }};
    if (auto gradeCount{static_cast<size_t>(
            std::count_if(jouyou.begin(), jouyou.end(), grade))};
        gradeCount) {
      all += gradeCount;
      log() << "  Total for grade " << i << ": " << gradeCount;
      noFreq(std::count_if(jouyou.begin(), jouyou.end(),
                 [&grade](auto& x) { return grade(x) && !x->frequency(); }),
          true);
      out() << " (";
      for (const auto level : AllJlptLevels) {
        const auto gradeLevelCount{static_cast<size_t>(std::count_if(
            jouyou.begin(), jouyou.end(), [&grade, level](auto& x) {
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

template<typename T, EnumContainer::Size S>
void RealKanjiData::printListStats(const EnumListWithNone<T, S>& all,
    T (Kanji::*p)() const, const String& name, bool showNoFrequency) const {
  log() << name << " breakdown:\n";
  size_t total{};
  for (const auto i : all) {
    std::vector<std::pair<KanjiTypes, size_t>> counts;
    size_t iTotal{};
    for (auto j{AllKanjiTypes.begin()}; auto& l : types()) {
      const auto t{*j++};
      if (const auto c{static_cast<size_t>(std::count_if(l.begin(), l.end(),
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

LevelListFile RealKanjiData::dataFile(JlptLevels x) const {
  return {dataDir() / Jlpt / firstLower(toString(x)), x};
}

KyuListFile RealKanjiData::dataFile(KenteiKyus x) const {
  return {dataDir() / Kentei / firstLower(toString(x)), x};
}

} // namespace kanji_tools
