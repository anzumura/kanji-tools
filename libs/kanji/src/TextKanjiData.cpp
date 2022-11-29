#include <kt_kanji/Kanji.h>
#include <kt_kanji/OfficialKanji.h>
#include <kt_kanji/TextKanjiData.h>
#include <kt_utils/Utf8.h>

#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path JouyouFile{"jouyou"}, JinmeiFile{"jinmei"}, ExtraFile{"extra"},
    UcdFile{"ucd"}, RadicalsFile{"radicals"},
    FrequencyReadingsFile{"frequency-readings"},
    LinkedJinmeiFile{"linked-jinmei"}, Jlpt{"jlpt"}, Kentei{"kentei"};

} // namespace

TextKanjiData::TextKanjiData(
    const Args& args, std::ostream& out, std::ostream& err)
    : KanjiData{getDataDir(args), getDebugMode(args), out, err},
      // GCOV_EXCL_START
      _levels{dataFile(JlptLevels::N5), dataFile(JlptLevels::N4),
          dataFile(JlptLevels::N3), dataFile(JlptLevels::N2),
          dataFile(JlptLevels::N1)},
      _kyus{dataFile(KenteiKyus::K10), dataFile(KenteiKyus::K9),
          dataFile(KenteiKyus::K8), dataFile(KenteiKyus::K7),
          dataFile(KenteiKyus::K6), dataFile(KenteiKyus::K5),
          dataFile(KenteiKyus::K4), dataFile(KenteiKyus::K3),
          dataFile(KenteiKyus::KJ2), dataFile(KenteiKyus::K2),
          dataFile(KenteiKyus::KJ1), dataFile(KenteiKyus::K1)},
      // GCOV_EXCL_STOP
      _frequency{dataDir() / "frequency"} {
  ListFile::clearUniqueCheckData(); // cleanup data used for unique checks
  getUcd().load(ListFile::getFile(dataDir(), UcdFile));
  radicals().load(ListFile::getFile(dataDir(), RadicalsFile));
  loadFrequencyReadings(ListFile::getFile(dataDir(), FrequencyReadingsFile));
  loadJouyouKanji();
  loadOfficialLinkedKanji(ListFile::getFile(dataDir(), LinkedJinmeiFile));
  loadJinmeiKanji();
  loadExtraKanji();
  for (auto& i : _levels) processList(i);
  // Process '_frequency' before '_kyus' in order to create 'Frequency' type
  // before creating 'Kentei' kanji. This way, the 'Frequency' type is more
  // meaningful since it indicates kanji in the top 2501 frequency list, but not
  // in the other more official types like Jouyou or Jinmei. 'Kentei' has many
  // rare kanji so keep it as the last type to be processed (before UcdKanji).
  processList(_frequency);
  for (auto& i : _kyus) processList(i);
  finishedLoadingData();
}

Kanji::Frequency TextKanjiData::frequency(const String& s) const {
  return _frequency.getIndex(s);
}

JlptLevels TextKanjiData::level(const String& k) const {
  for (auto& i : _levels)
    if (i.exists(k)) return i.level();
  return JlptLevels::None;
}

KenteiKyus TextKanjiData::kyu(const String& k) const {
  for (auto& i : _kyus)
    if (i.exists(k)) return i.kyu();
  return KenteiKyus::None;
}

void TextKanjiData::loadFrequencyReadings(const Path& file) {
  const ColumnFile::Column nameCol{"Name"}, readingCol{"Reading"};
  for (ColumnFile f{file, {nameCol, readingCol}}; f.nextRow();)
    if (!_frequencyReadings.emplace(f.get(nameCol), f.get(readingCol)).second)
      f.error("duplicate name");
}

void TextKanjiData::loadJouyouKanji() {
  auto results{NumberedKanji::fromFile<JouyouKanji>(
      *this, ListFile::getFile(dataDir(), JouyouFile))};
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(hasValue(i->grade()));
    checkInsert(i);
  }
  getTypes()[KanjiTypes::Jouyou] = std::move(results);
}

void TextKanjiData::loadOfficialLinkedKanji(const Path& file) {
  std::ifstream f{file};
  // each line in 'file' should be a Jouyou Kanji followed by the officially
  // recognized 'Jinmei Variant' (so loadJouyouKanji must be called first)
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

void TextKanjiData::loadJinmeiKanji() {
  auto results{NumberedKanji::fromFile<JinmeiKanji>(
      *this, ListFile::getFile(dataDir(), JinmeiFile))};
  for (auto& linkedJinmei{getTypes()[KanjiTypes::LinkedJinmei]};
       const auto& i : results) {
    checkInsert(i);
    for (auto& j : i->oldNames())
      checkInsert(
          linkedJinmei, std::make_shared<LinkedJinmeiKanji>(*this, j, i));
  }
  getTypes()[KanjiTypes::Jinmei] = std::move(results);
}

void TextKanjiData::loadExtraKanji() {
  auto results{NumberedKanji::fromFile<ExtraKanji>(
      *this, ListFile::getFile(dataDir(), ExtraFile))};
  for (const auto& i : results) checkInsert(i);
  getTypes()[KanjiTypes::Extra] = std::move(results);
}

void TextKanjiData::processList(const ListFile& list) {
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

void TextKanjiData::printListData(const ListFile& list,
    const StringList& created, TypeStringList& found) const {
  if (fullDebug()) {
    ListFile::print(
        out(), found[KanjiTypes::LinkedOld], "Linked Old", list.name());
    ListFile::print(out(), created,
        String{"non-Jouyou/Jinmei"} + (hasValue(list.level()) ? "" : "/JLPT"),
        list.name());
    // list.level is None when processing 'frequency.txt' file (so not JLPT)
    if (!(list.kyu()) && !(list.level())) {
      std::vector lists{std::pair{&found[KanjiTypes::Jinmei], ""},
          std::pair{&found[KanjiTypes::LinkedJinmei], "Linked "}};
      for (const auto& i : lists) {
        ListFile::StringList jlptJinmei, otherJinmei;
        for (auto& j : *i.first)
          (hasValue(level(j)) ? jlptJinmei : otherJinmei).emplace_back(j);
        ListFile::print(out(), jlptJinmei,
            String{"JLPT "} + i.second + "Jinmei", list.name());
        ListFile::print(out(), otherJinmei,
            String{"non-JLPT "} + i.second + "Jinmei", list.name());
      }
    } else {
      ListFile::print(out(), found[KanjiTypes::Jinmei], "Jinmei", list.name());
      ListFile::print(
          out(), found[KanjiTypes::LinkedJinmei], "Linked Jinmei", list.name());
    }
  }
}

LevelListFile TextKanjiData::dataFile(JlptLevels x) const {
  return {dataDir() / Jlpt / firstLower(toString(x)), x};
}

KyuListFile TextKanjiData::dataFile(KenteiKyus x) const {
  return {dataDir() / Kentei / firstLower(toString(x)), x};
}

// TextKanjiDataTestAccess

void TextKanjiDataTestAccess::loadFrequencyReadings(
    TextKanjiData& data, const KanjiData::Path& file) {
  data.loadFrequencyReadings(file);
}

void TextKanjiDataTestAccess::loadOfficialLinkedKanji(
    TextKanjiData& data, const KanjiData::Path& file) {
  data.loadOfficialLinkedKanji(file);
}

} // namespace kanji_tools
