#include <kanji_tools/kanji/CustomFileKanji.h>
#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/kanji/UcdFileKanji.h>
#include <kanji_tools/utils/ColumnFile.h>
#include <kanji_tools/utils/MBUtils.h>

#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path JouyouFile{"jouyou"}, JinmeiFile{"jinmei"},
    LinkedJinmeiFile{"linked-jinmei"}, ExtraFile{"extra"};

// This value is used for finding the location of 'data' directory. It will of
// course need to be updated if the number of '.txt' files changes.
constexpr size_t TextFilesInDataDir{12};

} // namespace

Data::ArgCount Data::nextArg(
    ArgCount argc, const char* const* argv, ArgCount currentArg) {
  argSanityCheck(argc, argv);
  if (currentArg > argc)
    throw std::domain_error("currentArg " + std::to_string(currentArg) +
                            " is greater than argc " + std::to_string(argc));
  ArgCount result{currentArg};
  if (argc && ++result < argc) {
    std::string arg{argv[result]};
    // '-data' should be followed by a 'path' so increment by 2. If -data isn't
    // followed by a path then an earlier call to 'getDataDir' would have failed
    // with a call to 'usage' which ends the program.
    if (arg == DataArg) return nextArg(argc, argv, result + 1);
    if (arg == DebugArg || arg == InfoArg) return nextArg(argc, argv, result);
  }
  return result;
}

Data::Data(const Path& dataDir, DebugMode debugMode, std::ostream& out,
    std::ostream& err)
    : _dataDir{dataDir}, _debugMode{debugMode}, _out{out}, _err{err} {
  // Clearing DataFile static data is only needed to help test code, for example
  // DataFile tests can leave some data in these sets before Quiz tests are run
  // (leading to problems loading real files).
  DataFile::clearUniqueCheckData();
  if (fullDebug()) log(true) << "Begin Loading Data\n>>>\n";
}

KanjiTypes Data::getType(const std::string& name) const {
  const auto i{findKanjiByName(name)};
  return i ? (**i).type() : KanjiTypes::None;
}

Kanji::NelsonIds Data::getNelsonIds(const Ucd* u) const {
  if (u && !u->nelsonIds().empty()) {
    Kanji::NelsonIds ids;
    auto s{u->nelsonIds()};
    std::replace(s.begin(), s.end(), ',', ' ');
    std::stringstream ss{s};
    Kanji::NelsonId id;
    while (ss >> id) ids.emplace_back(id);
    return ids;
  }
  return EmptyNelsonIds;
}

fs::path Data::getDataDir(ArgCount argc, const char** argv) {
  argSanityCheck(argc, argv);
  OptPath found;
  for (ArgCount i{1}; !found && i < argc; ++i)
    if (argv[i] == DataArg) {
      if (i + 1 == argc) usage("'-data' must be followed by a directory name");
      const auto data{Path(argv[i + 1])};
      if (!fs::is_directory(data))
        usage("'" + data.string() + "' is not a valid directory");
      found = data;
    }
  // If '-data' wasn't provided then search up directories for 'data' and make
  // sure it contains at least one of the required files (jouyou.txt).
  if (!found) {
    // search up from current directory
    found = searchUpForDataDir(fs::current_path());
    if (!found && argc) {
      // search up from 'argv[0]'
      if (const Path p{argv[0]}; fs::is_regular_file(p))
        found = searchUpForDataDir(p.parent_path());
      if (!found) usage("couldn't find valid 'data' directory");
    }
  }
  return *found;
}

Data::DebugMode Data::getDebugMode(ArgCount argc, const char** argv) {
  argSanityCheck(argc, argv);
  auto result{DebugMode::None};
  const auto setResult{[&result](DebugMode x) {
    if (result != DebugMode::None)
      usage("can only specify one '" + DebugArg + "' or '" + InfoArg +
            "' option");
    result = x;
  }};
  for (ArgCount i{1}; i < argc; ++i)
    if (argv[i] == DebugArg)
      setResult(DebugMode::Full);
    else if (argv[i] == InfoArg)
      setResult(DebugMode::Info);
  return result;
}

Data::OptPath Data::searchUpForDataDir(Path parent) {
  OptPath oldParent;
  do {
    // check if 'data' exists and contains the expected number of '.txt' files
    if (const auto data{parent / DataDir};
        fs::is_directory(data) &&
        std::count_if(fs::directory_iterator(data), fs::directory_iterator{},
            [](const auto& i) {
              return i.path().extension() == DataFile::TextFileExtension;
            }) == TextFilesInDataDir)
      return data;
    oldParent = parent;
    parent = oldParent->parent_path();
    // 'has_parent_path' seems to always return true, i.e., parent of '/' is
    // '/' so break if new 'parent' is equal to 'oldParent'.
  } while (parent != oldParent);
  return {};
}

void Data::argSanityCheck(ArgCount argc, const char* const* argv) {
  if (argc && !argv)
    throw std::domain_error(
        "argc is " + std::to_string(argc) + ", but argv is null");
}

bool Data::checkInsert(const Entry& kanji) {
  auto& k{*kanji};
  if (!_kanjiNameMap.emplace(k.name(), kanji).second) {
    printError("failed to insert " + k.name() + " into map");
    return false;
  }
  // Perform some sanity checks on newly created kanji, failures result in error
  // messages getting printed to stderr, but the program is allowed to continue
  // since it can be helpful to see more than one error printed out if something
  // goes wrong. Any failures should be fixed right away.
  insertSanityChecks(k);
  // update _maxFrequency, _compatibilityMap, _morohashiMap and _nelsonMap if
  // applicable
  if (k.frequency() && *k.frequency() >= _maxFrequency)
    _maxFrequency = *k.frequency() + 1U;
  if (k.variant() &&
      !_compatibilityMap.emplace(k.compatibilityName(), k.name()).second)
    printError("failed to insert variant " + k.name() + " into map");
  if (k.morohashiId()) _morohashiMap[*k.morohashiId()].emplace_back(kanji);
  for (const auto id : k.nelsonIds()) _nelsonMap[id].emplace_back(kanji);
  return true;
}

bool Data::checkInsert(List& s, const Entry& kanji) {
  if (!checkInsert(kanji)) return false;
  s.emplace_back(kanji);
  return true;
}

void Data::insertSanityChecks(const Kanji& kanji) const {
  const auto error{[this, &kanji](const std::string& s) {
    std::string v;
    if (kanji.variant()) v = " (non-variant: " + kanji.nonVariantName() + ")";
    printError(kanji.name() + ' ' +
               toUnicode(kanji.name(), BracketType::Square) + ' ' + v + " " +
               s + " in _ucd");
  }};

  const auto kanjiType{kanji.type()};
  if (const auto ucd{_ucd.find(kanji.name())}; !ucd)
    error("not found");
  else if (kanjiType == KanjiTypes::Jouyou && !ucd->joyo())
    error("not marked as 'Joyo'");
  else if (kanjiType == KanjiTypes::Jinmei && !ucd->jinmei())
    error("not marked as 'Jinmei'");
  else if (kanjiType == KanjiTypes::LinkedJinmei && !ucd->jinmei())
    error("with link not marked as 'Jinmei'");
  else if (kanjiType == KanjiTypes::LinkedJinmei && !ucd->hasLinks())
    error("missing 'JinmeiLink' for " + ucd->codeAndName());
  // skipping radical and strokes checks for now
}

void Data::printError(const std::string& msg) const {
  static size_t count;
  _err << "ERROR[" << std::setfill('0') << std::setw(4) << ++count << "] --- "
       << msg << std::setfill(' ') << '\n';
}

void Data::loadStrokes(const Path& file, bool checkDuplicates) {
  std::ifstream f{file};
  Ucd::Strokes strokes{};
  for (std::string line; std::getline(f, line);)
    if (::isdigit(line[0])) {
      const auto newStrokes{std::stoul(line)};
      assert(newStrokes <= std::numeric_limits<Ucd::Strokes>::max());
      assert(newStrokes > strokes);
      strokes = static_cast<Ucd::Strokes>(newStrokes);
    } else {
      assert(strokes != 0); // first line must have a stroke count
      for (std::stringstream ss{line}; std::getline(ss, line, ' ');)
        if (const auto i{_strokes.insert(std::pair(line, strokes))};
            !i.second) {
          if (checkDuplicates)
            printError("duplicate entry in " + file.string() + ": " + line);
          else if (i.first->second != strokes)
            printError("found entry with different count in " + file.string() +
                       ": " + line);
        }
    }
}

void Data::loadFrequencyReadings(const Path& file) {
  const ColumnFile::Column nameCol{"Name"}, readingCol{"Reading"};
  for (ColumnFile f{file, {nameCol, readingCol}}; f.nextRow();)
    if (!_frequencyReadings.emplace(f.get(nameCol), f.get(readingCol)).second)
      f.error("duplicate name");
}

void Data::populateJouyou() {
  auto results{CustomFileKanji::fromFile<JouyouKanji>(
      *this, DataFile::getFile(_dataDir, JouyouFile))};
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(hasValue(i->grade()));
    if (checkInsert(i)) _grades[i->grade()].emplace_back(i);
  }
  _types[KanjiTypes::Jouyou] = std::move(results);
  populateLinkedKanji();
}

void Data::populateLinkedKanji() {
  Path file{DataFile::getFile(_dataDir, LinkedJinmeiFile)};
  std::ifstream f{file};
  // populate _linkedJinmeiKanji that are linked to Jouyou
  auto& linkedJinmei{_types[KanjiTypes::LinkedJinmei]};
  for (std::string line; std::getline(f, line);) {
    std::stringstream ss{line};
    if (std::string jouyou, linked;
        std::getline(ss, jouyou, '\t') && std::getline(ss, linked, '\t')) {
      if (const auto i{_kanjiNameMap.find(jouyou)}; i == _kanjiNameMap.end())
        printError(
            "can't find " + jouyou + " while processing " + file.string());
      else
        checkInsert(linkedJinmei,
            std::make_shared<LinkedJinmeiKanji>(*this, linked, i->second));
    } else
      printError("bad line in " + file.string() + ": " + line);
  }
  // create 'LinkedOld' type kanji (these are the 'old Jouyou' that are not
  // LinkedJinmei created above)
  for (auto& linkedOld{_types[KanjiTypes::LinkedOld]};
       const auto& i : _kanjiNameMap)
    for (auto& j : i.second->oldNames())
      if (!findKanjiByName(j))
        checkInsert(
            linkedOld, std::make_shared<LinkedOldKanji>(*this, j, i.second));
}

void Data::populateJinmei() {
  auto results{CustomFileKanji::fromFile<JinmeiKanji>(
      *this, DataFile::getFile(_dataDir, JinmeiFile))};
  for (auto& linkedJinmei{_types[KanjiTypes::LinkedJinmei]};
       const auto& i : results) {
    checkInsert(i);
    for (auto& j : i->oldNames())
      checkInsert(
          linkedJinmei, std::make_shared<LinkedJinmeiKanji>(*this, j, i));
  }
  _types[KanjiTypes::Jinmei] = std::move(results);
}

void Data::populateExtra() {
  auto results{CustomFileKanji::fromFile<ExtraKanji>(
      *this, DataFile::getFile(_dataDir, ExtraFile))};
  for (const auto& i : results) checkInsert(i);
  _types[KanjiTypes::Extra] = std::move(results);
}

void Data::processList(const DataFile& list) {
  const auto kenteiList{hasValue(list.kyu())};
  DataFile::List created;
  std::map<KanjiTypes, DataFile::List> found;
  auto& newKanji{
      _types[kenteiList ? KanjiTypes::Kentei : KanjiTypes::Frequency]};
  for (size_t i{}; i < list.list().size(); ++i) {
    auto& name{list.list()[i]};
    Entry kanji;
    if (const auto j{findKanjiByName(name)}; j) {
      kanji = *j;
      if (debug() && !kenteiList && kanji->type() != KanjiTypes::Jouyou)
        found[kanji->type()].push_back(name);
    } else {
      if (kenteiList)
        kanji = std::make_shared<KenteiKanji>(*this, name, list.kyu());
      else {
        // kanji wasn't already in _kanjiNameMap so it only exists in the
        // 'frequency.txt' file - these kanjis are considered 'Frequency' type
        // and by definition are not part of Jouyou or Jinmei (so also not part
        // of JLPT levels)
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
    if (kenteiList) {
      assert(kanji->kyu() == list.kyu());
      _kyus[list.kyu()].emplace_back(kanji);
    } else if (hasValue(list.level())) {
      assert(kanji->level() == list.level());
      _levels[list.level()].emplace_back(kanji);
    } else {
      assert(kanji->frequency());
      const auto index{(*kanji->frequency() - 1U) / FrequencyEntries};
      _frequencies[index < FrequencyBuckets ? index : FrequencyBuckets - 1]
          .emplace_back(kanji);
    }
  }
  if (fullDebug()) {
    DataFile::print(
        _out, found[KanjiTypes::LinkedOld], "Linked Old", list.name());
    DataFile::print(_out, created,
        std::string{"non-Jouyou/Jinmei"} +
            (hasValue(list.level()) ? "" : "/JLPT"),
        list.name());
    // list.level is None when processing 'frequency.txt' file (so not JLPT)
    if (!kenteiList && !(list.level())) {
      std::vector lists{std::pair{&found[KanjiTypes::Jinmei], ""},
          std::pair{&found[KanjiTypes::LinkedJinmei], "Linked "}};
      for (const auto& i : lists) {
        DataFile::List jlptJinmei, otherJinmei;
        for (auto& j : *i.first)
          (hasValue(level(j)) ? jlptJinmei : otherJinmei).emplace_back(j);
        DataFile::print(_out, jlptJinmei,
            std::string{"JLPT "} + i.second + "Jinmei", list.name());
        DataFile::print(_out, otherJinmei,
            std::string{"non-JLPT "} + i.second + "Jinmei", list.name());
      }
    } else {
      DataFile::print(_out, found[KanjiTypes::Jinmei], "Jinmei", list.name());
      DataFile::print(
          _out, found[KanjiTypes::LinkedJinmei], "Linked Jinmei", list.name());
    }
  }
}

void Data::processUcd() {
  // Calling 'findKanjiByName' checks for a 'variation selector' version of
  // 'name' so use it instead of checking for a match in _kanjiNameMap directly
  // (this avoids creating 52 redundant kanji when processing 'ucd.txt').
  for (auto& newKanji{_types[KanjiTypes::Ucd]}; const auto& i : _ucd.map())
    if (!findKanjiByName(i.second.name()))
      checkInsert(newKanji, std::make_shared<UcdKanji>(*this, i.second));
}

void Data::checkStrokes() const {
  DataFile::List strokesFrequency, strokesNotFound, strokeDiffs, vStrokeDiffs,
      missingDiffs, missingUcd;
  for (const auto& i : _strokes) {
    const auto u{findUcd(i.first)};
    const auto ucdStrokes{getStrokes(i.first, u, false, true)};
    const auto k{findKanjiByName(i.first)};
    if (ucdStrokes) {
      // If a Kanji object exists, prefer to use its 'strokes' since this it's
      // more accurate, i.e., there are some incorrect stroke counts in
      // 'wiki-strokes.txt', but they aren't used because the actual stroke
      // count comes from 'jouyou.txt', 'jinmei.txt' or 'extra.txt'
      if (k) {
        if ((**k).variant()) {
          if ((**k).strokes() != getStrokes(i.first, u, true, true))
            vStrokeDiffs.emplace_back(i.first);
        } else if ((**k).strokes() != ucdStrokes)
          strokeDiffs.emplace_back(i.first);
      } else if (i.second != ucdStrokes)
        missingDiffs.emplace_back(i.first);
    } else
      missingUcd.emplace_back(i.first);
    if (k) {
      if ((**k).type() == KanjiTypes::Frequency)
        strokesFrequency.emplace_back(i.first);
    } else
      strokesNotFound.emplace_back(i.first);
  }
  if (debug()) {
    DataFile::print(_out, strokesNotFound, "Kanji not loaded", "_strokes");
    DataFile::print(
        _out, vStrokeDiffs, "Variant kanji with differrent strokes", "_ucd");
    DataFile::print(_out, missingDiffs,
        "'_stokes only' Kanji with differrent strokes", "_ucd");
    DataFile::print(
        _out, missingUcd, "Kanji in _strokes, but not found", "_ucd");
    if (fullDebug()) {
      DataFile::print(
          _out, strokesFrequency, "Kanji in 'Frequency' group", "_strokes");
      DataFile::print(
          _out, strokeDiffs, "Kanji with differrent strokes", "_ucd");
    }
  }
}

} // namespace kanji_tools
