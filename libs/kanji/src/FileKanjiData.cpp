#include <kt_kanji/FileKanjiData.h>
#include <kt_kanji/Kanji.h>
#include <kt_kanji/OfficialKanji.h>
#include <kt_utils/Utf8.h>

#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path JouyouFile{"jouyou"}, JinmeiFile{"jinmei"}, ExtraFile{"extra"},
    UcdFile{"ucd"}, RadicalsFile{"radicals"},
    FrequencyReadingsFile{"frequency-readings"},
    LinkedJinmeiFile{"linked-jinmei"}, Jlpt{"jlpt"}, Kentei{"kentei"};

constexpr auto MaxVariantSelectorExamples{5};

} // namespace

FileKanjiData::FileKanjiData(
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
  getUcd().load(KanjiListFile::getFile(dataDir(), UcdFile));
  radicals().load(KanjiListFile::getFile(dataDir(), RadicalsFile));
  loadFrequencyReadings(
      KanjiListFile::getFile(dataDir(), FrequencyReadingsFile));
  populateJouyou();
  populateOfficialLinkedKanji(
      KanjiListFile::getFile(dataDir(), LinkedJinmeiFile));
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

Kanji::Frequency FileKanjiData::frequency(const String& s) const {
  return _frequency.getIndex(s);
}

JlptLevels FileKanjiData::level(const String& k) const {
  for (auto& i : _levels)
    if (i.exists(k)) return i.level();
  return JlptLevels::None;
}

KenteiKyus FileKanjiData::kyu(const String& k) const {
  for (auto& i : _kyus)
    if (i.exists(k)) return i.kyu();
  return KenteiKyus::None;
}

void FileKanjiData::populateJouyou() {
  auto results{NumberedKanji::fromFile<JouyouKanji>(
      *this, KanjiListFile::getFile(dataDir(), JouyouFile))};
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(hasValue(i->grade()));
    checkInsert(i);
  }
  getTypes()[KanjiTypes::Jouyou] = std::move(results);
}

void FileKanjiData::populateJinmei() {
  auto results{NumberedKanji::fromFile<JinmeiKanji>(
      *this, KanjiListFile::getFile(dataDir(), JinmeiFile))};
  for (auto& linkedJinmei{getTypes()[KanjiTypes::LinkedJinmei]};
       const auto& i : results) {
    checkInsert(i);
    for (auto& j : i->oldNames())
      checkInsert(
          linkedJinmei, std::make_shared<LinkedJinmeiKanji>(*this, j, i));
  }
  getTypes()[KanjiTypes::Jinmei] = std::move(results);
}

void FileKanjiData::populateExtra() {
  auto results{NumberedKanji::fromFile<ExtraKanji>(
      *this, KanjiListFile::getFile(dataDir(), ExtraFile))};
  for (const auto& i : results) checkInsert(i);
  getTypes()[KanjiTypes::Extra] = std::move(results);
}

void FileKanjiData::populateOfficialLinkedKanji(const Path& file) {
  std::ifstream f{file};
  // each line in 'file' should be a Jouyou Kanji followed by the officially
  // recognized 'Jinmei Variant' (so populateJouyou must be called first)
  auto& linkedJinmei{getTypes()[KanjiTypes::LinkedJinmei]};
  for (String line; std::getline(f, line);) {
    std::stringstream ss{line};
    if (String jouyou, linked;
        std::getline(ss, jouyou, '\t') && std::getline(ss, linked, '\t')) {
      if (const auto i{nameMap().find(jouyou)}; i == nameMap().end())
        usage("'" + jouyou + "' not found - file: " + file.filename().string());
      else
        checkInsert(linkedJinmei,
            std::make_shared<LinkedJinmeiKanji>(*this, linked, i->second));
    } else
      usage("bad line '" + line + "' - file: " + file.filename().string());
  }
  // create 'LinkedOld' type kanji (these are the 'old Jouyou' that are not
  // LinkedJinmei created above)
  for (auto& old{getTypes()[KanjiTypes::LinkedOld]}; const auto& i : nameMap())
    for (auto& j : i.second->oldNames())
      if (!findByName(j))
        checkInsert(old, std::make_shared<LinkedOldKanji>(*this, j, i.second));
}

void FileKanjiData::loadFrequencyReadings(const Path& file) {
  const ColumnFile::Column nameCol{"Name"}, readingCol{"Reading"};
  for (ColumnFile f{file, {nameCol, readingCol}}; f.nextRow();)
    if (!_frequencyReadings.emplace(f.get(nameCol), f.get(readingCol)).second)
      f.error("duplicate name");
}

void FileKanjiData::processList(const KanjiListFile& list) {
  const auto kenteiList{hasValue(list.kyu())};
  StringList created;
  TypeStringList found;
  auto& newKanji{
      getTypes()[kenteiList ? KanjiTypes::Kentei : KanjiTypes::Frequency]};
  for (size_t i{}; i < list.list().size(); ++i) {
    auto& name{list.list()[i]};
    KanjiPtr kanji;
    if (const auto j{findByName(name)}; j) {
      kanji = j;
      if (debug() && !kenteiList && kanji->type() != KanjiTypes::Jouyou)
        found[kanji->type()].push_back(name);
    } else {
      if (kenteiList)
        kanji = std::make_shared<KenteiKanji>(*this, name, list.kyu());
      else {
        // Kanji wasn't found in 'nameMap' so it only exists in 'frequency.txt'
        // file - these are considered 'Frequency' type and by definition are
        // not part of Jouyou or Jinmei (so also not part of JLPT levels)
        const auto reading{_frequencyReadings.find(name)};
        kanji = reading == _frequencyReadings.end()
                    ? std::make_shared<FrequencyKanji>(*this, name, i + 1)
                    : std::make_shared<FrequencyKanji>(
                          *this, name, reading->second, i + 1);
      }
      checkInsert(newKanji, kanji);
      // don't print out kentei 'created' since there more than 2,000 outside of
      // the other types
      if (debug() && !kenteiList) created.emplace_back(name);
    }
    if (kenteiList)
      addToKyus(kanji);
    else if (hasValue(list.level()))
      addToLevels(kanji);
    else
      addToFrequencies(kanji);
  }
  printListData(list, created, found);
}

void FileKanjiData::printListData(const KanjiListFile& list,
    const StringList& created, TypeStringList& found) const {
  if (fullDebug()) {
    KanjiListFile::print(
        out(), found[KanjiTypes::LinkedOld], "Linked Old", list.name());
    KanjiListFile::print(out(), created,
        String{"non-Jouyou/Jinmei"} + (hasValue(list.level()) ? "" : "/JLPT"),
        list.name());
    // list.level is None when processing 'frequency.txt' file (so not JLPT)
    if (!(list.kyu()) && !(list.level())) {
      std::vector lists{std::pair{&found[KanjiTypes::Jinmei], ""},
          std::pair{&found[KanjiTypes::LinkedJinmei], "Linked "}};
      for (const auto& i : lists) {
        KanjiListFile::StringList jlptJinmei, otherJinmei;
        for (auto& j : *i.first)
          (hasValue(level(j)) ? jlptJinmei : otherJinmei).emplace_back(j);
        KanjiListFile::print(out(), jlptJinmei,
            String{"JLPT "} + i.second + "Jinmei", list.name());
        KanjiListFile::print(out(), otherJinmei,
            String{"non-JLPT "} + i.second + "Jinmei", list.name());
      }
    } else {
      KanjiListFile::print(
          out(), found[KanjiTypes::Jinmei], "Jinmei", list.name());
      KanjiListFile::print(
          out(), found[KanjiTypes::LinkedJinmei], "Linked Jinmei", list.name());
    }
  }
}

void FileKanjiData::noFreq(std::ptrdiff_t f, bool brackets) const {
  if (f) {
    if (brackets)
      out() << " (";
    else
      out() << ' ';
    out() << "nf " << f;
    if (brackets) out() << ')';
  }
}

template <auto Pred>
void FileKanjiData::printCount(const String& name, size_t printExamples) const {
  std::vector<std::pair<KanjiTypes, size_t>> counts;
  std::map<KanjiTypes, std::vector<String>> examples;
  size_t total{};
  for (auto i{AllKanjiTypes.begin()}; auto& l : types()) {
    size_t count{};
    const auto t{*i++};
    if (printExamples)
      for (auto& j : l) {
        if (Pred(j) && ++count <= printExamples)
          examples[t].emplace_back(j->name());
      }
    else
      count = static_cast<size_t>(std::count_if(l.begin(), l.end(), Pred));
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

void FileKanjiData::printStats() const {
  log() << "Loaded " << nameMap().size() << " Kanji (";
  for (auto i{AllKanjiTypes.begin()}; auto& j : types()) {
    if (i != AllKanjiTypes.begin()) out() << ' ';
    out() << *i++ << ' ' << j.size();
  }
  out() << ")\n";
  if (fullDebug()) {
    printCount<[](auto& x) { return x->hasLevel(); }>("  Has JLPT level");
    printCount<[](auto& x) {
      return x->frequency() && !x->is(KanjiTypes::Jouyou) && !x->hasLevel();
    }>("  Has frequency and not in Jouyou or JLPT");
    printCount<[](auto& x) {
      return x->is(KanjiTypes::Jinmei) && !x->frequency() && !x->hasLevel();
    }>("  Jinmei with no frequency and not JLPT");
    printCount<[](auto& x) { return !x->frequency(); }>("  NF (no-frequency)");
    printCount<[](auto& x) { return x->strokes().hasVariant(); }>(
        "  Has Variant Strokes");
    printCount<[](auto& x) { return x->variant(); }>(
        "  Has Variation Selectors", MaxVariantSelectorExamples);
    printCount<[](auto& x) { return !x->oldNames().empty(); }>("Old Forms");
  }
}

void FileKanjiData::printGrades() const {
  log() << "Grade breakdown:\n";
  size_t all{};
  for (auto& jouyou{types()[KanjiTypes::Jouyou]}; auto i : AllKanjiGrades) {
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

template <typename T, Enum::Size S>
void FileKanjiData::printListStats(const EnumListWithNone<T, S>& all,
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
        auto& l{types()[j.first]};
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

LevelListFile FileKanjiData::dataFile(JlptLevels x) const {
  return {dataDir() / Jlpt / firstLower(toString(x)), x};
}

KyuListFile FileKanjiData::dataFile(KenteiKyus x) const {
  return {dataDir() / Kentei / firstLower(toString(x)), x};
}

} // namespace kanji_tools
