#include <kanji_tools/kanji/CustomFileKanji.h>
#include <kanji_tools/utils/MBUtils.h>

#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path JouyouFile{"jouyou"}, JinmeiFile{"jinmei"}, ExtraFile{"extra"};

// This value is used for finding the location of 'data' directory. It will of
// course need to be updated if the number of '.txt' files changes.
constexpr size_t TextFilesInDataDir{10};

} // namespace

Args::Size KanjiData::nextArg(const Args& args, Args::Size current) {
  if (current > args.size())
    throw std::domain_error("current arg '" + std::to_string(current) +
                            "' is greater than args size '" +
                            std::to_string(args.size()) + "'");
  Args::Size result{current};
  if (args && ++result < args.size()) {
    String arg{args[result]};
    // '-data' should be followed by a 'path' so increment by 2. If -data isn't
    // followed by a path then an earlier call to 'getDataDir' would have failed
    // with a call to 'usage' which ends the program.
    if (arg == DataArg) return nextArg(args, result + 1);
    if (arg == DebugArg || arg == InfoArg) return nextArg(args, result);
  }
  return result;
}

Kanji::Frequency KanjiData::maxFrequency() { return _maxFrequency; }

const Pinyin& KanjiData::getPinyin(UcdPtr u) {
  static constexpr Pinyin EmptyPinyin;
  return u ? u->pinyin() : EmptyPinyin;
}

const MorohashiId& KanjiData::getMorohashiId(UcdPtr u) {
  static constexpr MorohashiId EmptyMorohashiId;
  return u ? u->morohashiId() : EmptyMorohashiId;
}

Kanji::NelsonIds KanjiData::getNelsonIds(UcdPtr u) {
  if (u && !u->nelsonIds().empty()) {
    Kanji::NelsonIds ids;
    auto s{u->nelsonIds()};
    std::replace(s.begin(), s.end(), ',', ' ');
    std::stringstream ss{s};
    Kanji::NelsonId id{};
    while (ss >> id) ids.emplace_back(id);
    return ids;
  }
  return EmptyNelsonIds;
}

KanjiData::KanjiData(const Path& dataDir, DebugMode debugMode,
    std::ostream& out, std::ostream& err)
    : _dataDir{dataDir}, _debugMode{debugMode}, _out{out}, _err{err} {
  // Clearing KanjiListFile static data is only needed to help test code, for
  // example KanjiListFile tests can leave some data in these sets before Quiz
  // tests are run (leading to problems loading real files).
  KanjiListFile::clearUniqueCheckData();
  if (fullDebug()) log(true) << "Begin Loading Data\n>>>\n";
}

UcdPtr KanjiData::findUcd(const String& kanjiName) const {
  return _ucd.find(kanjiName);
}

RadicalRef KanjiData::ucdRadical(const String& kanji, UcdPtr u) const {
  if (u) return _radicals.find(u->radical());
  // 'throw' should never happen - every 'Kanji' class instance should have
  // also exist in the data loaded from Unicode.
  throw std::domain_error{"UCD entry not found: " + kanji};
}

Strokes KanjiData::ucdStrokes(const String& kanji, UcdPtr u) const {
  if (u) return u->strokes();
  throw std::domain_error{"UCD entry not found: " + kanji};
}

RadicalRef KanjiData::getRadicalByName(const String& radicalName) const {
  return _radicals.find(radicalName);
}

Kanji::OptString KanjiData::getCompatibilityName(const String& kanji) const {
  const auto u{_ucd.find(kanji)};
  return u && u->name() != kanji ? Kanji::OptString{u->name()} : std::nullopt;
}

const KanjiData::KanjiList& KanjiData::frequencies(size_t f) const {
  return f < FrequencyBuckets ? _frequencies[f] : BaseEnumMap<KanjiList>::Empty;
}

size_t KanjiData::frequencySize(size_t f) const {
  return frequencies(f).size();
}

KanjiTypes KanjiData::getType(const String& name) const {
  const auto i{findKanjiByName(name)};
  return i ? i->type() : KanjiTypes::None;
}

KanjiPtr KanjiData::findKanjiByName(const String& s) const {
  const auto i{_compatibilityMap.find(s)};
  if (const auto j{
          _kanjiNameMap.find(i != _compatibilityMap.end() ? i->second : s)};
      j != _kanjiNameMap.end())
    return j->second;
  return {};
}

KanjiPtr KanjiData::findKanjiByFrequency(Kanji::Frequency freq) const {
  if (!freq || freq >= _maxFrequency) return {};
  auto bucket{--freq};
  bucket /= FrequencyEntries;
  if (bucket == FrequencyBuckets)
    --bucket; // last bucket contains FrequencyEntries + 1
  return _frequencies[bucket][freq - bucket * FrequencyEntries];
}

const KanjiData::KanjiList& KanjiData::findByMorohashiId(
    const MorohashiId& id) const {
  if (id) {
    if (const auto i{_morohashiMap.find(id)}; i != _morohashiMap.end())
      return i->second;
  }
  return BaseEnumMap<KanjiList>::Empty;
}

const KanjiData::KanjiList& KanjiData::findByMorohashiId(
    const String& id) const {
  return findByMorohashiId(MorohashiId{id});
}

const KanjiData::KanjiList& KanjiData::findByNelsonId(
    Kanji::NelsonId id) const {
  const auto i{_nelsonMap.find(id)};
  return i != _nelsonMap.end() ? i->second : BaseEnumMap<KanjiList>::Empty;
}

std::ostream& KanjiData::log(bool heading) const {
  return heading ? _out << ">>>\n>>> " : _out << ">>> ";
}

fs::path KanjiData::getDataDir(const Args& args) {
  static const String ExpectedTextFiles{
      std::to_string(TextFilesInDataDir) + " expected '" +
      KanjiListFile::TextFileExtension + "' files"};
  for (Args::Size i{1}; i < args.size(); ++i)
    if (args[i] == DataArg) {
      if (i + 1 == args.size())
        usage("'-data' must be followed by a directory name");
      auto data{Path(args[i + 1])};
      if (!fs::is_directory(data))
        usage("'" + data.string() + "' is not a valid directory");
      if (!isValidDataDir(data))
        usage("'" + data.string() + "' does not contain " + ExpectedTextFiles);
      return data;
    }
  // If '-data' wasn't provided then search up directories for 'data' and make
  // sure it contains at least one of the required files (jouyou.txt).
  static const String NotFound{"couldn't find 'data' directory with " +
                               ExpectedTextFiles +
                               ":\n- searched up from current: "},
      NotFoundEnd{"\nrun in a directory where 'data' can be found or use "
                  "'-data <dir>'"};
  // search up from current directory
  const auto cur{fs::current_path()};
  OptPath found;
  if (found = searchUpForDataDir(cur); !found && args) {
    Path p{args[0]}; // search up from 'args[0]'
    if (p = p.parent_path(); fs::is_directory(p)) {
      static const String Arg0Msg{"\n- searched up from arg0: "};
      if (found = searchUpForDataDir(p.parent_path()); !found)
        usage(NotFound + cur.string() + Arg0Msg + args[0] + NotFoundEnd);
    }
  }
  if (!found) usage(NotFound + cur.string() + NotFoundEnd);
  return *found;
}

KanjiData::DebugMode KanjiData::getDebugMode(const Args& args) {
  auto result{DebugMode::None};
  const auto setResult{[&result](DebugMode x) {
    if (result != DebugMode::None)
      usage("can only specify one '" + DebugArg + "' or '" + InfoArg +
            "' option");
    result = x;
  }};
  for (Args::Size i{1}; i < args.size(); ++i)
    if (args[i] == DebugArg)
      setResult(DebugMode::Full);
    else if (args[i] == InfoArg)
      setResult(DebugMode::Info);
  return result;
}

KanjiData::OptPath KanjiData::searchUpForDataDir(Path parent) {
  OptPath oldParent;
  do {
    // check if 'data' exists and contains the expected number of '.txt' files
    if (const auto data{parent / DataDir};
        fs::is_directory(data) && isValidDataDir(data))
      return data;
    oldParent = parent;
    parent = oldParent->parent_path();
    // 'has_parent_path' seems to always return true, i.e., parent of '/' is
    // '/' so break if new 'parent' is equal to 'oldParent'.
  } while (parent != oldParent);
  return {};
}

bool KanjiData::isValidDataDir(const Path& p) {
  return std::count_if(fs::directory_iterator(p), fs::directory_iterator{},
             [](const auto& i) {
               return i.path().extension() == KanjiListFile::TextFileExtension;
             }) == TextFilesInDataDir;
}

bool KanjiData::checkInsert(const KanjiPtr& kanji, UcdPtr ucd) {
  auto& k{*kanji};
  if (!_kanjiNameMap.emplace(k.name(), kanji).second) {
    printError("failed to insert '" + k.name() + "' into map");
    return false;
  }
  // Perform some sanity checks on newly created kanji, failures result in error
  // messages getting printed to stderr, but the program is allowed to continue
  // since it can be helpful to see more than one error printed out if something
  // goes wrong. Any failures should be fixed right away.
  insertSanityChecks(k, ucd);
  // update MaxFrequency, _compatibilityMap, _morohashiMap and _nelsonMap if
  // applicable
  if (k.frequency() && k.frequency() >= _maxFrequency)
    _maxFrequency = k.frequency() + 1U;
  if (k.variant() &&
      !_compatibilityMap.emplace(k.compatibilityName(), k.name()).second)
    printError("failed to insert variant '" + k.name() + "' into map");
  if (k.morohashiId()) _morohashiMap[k.morohashiId()].emplace_back(kanji);
  for (const auto id : k.nelsonIds()) _nelsonMap[id].emplace_back(kanji);
  return true;
}

bool KanjiData::checkInsert(KanjiList& s, const KanjiPtr& kanji) {
  if (!checkInsert(kanji)) return false;
  s.emplace_back(kanji);
  return true;
}

void KanjiData::insertSanityChecks(const Kanji& kanji, UcdPtr ucdIn) const {
  const auto error{[this, &kanji](const String& s) {
    String v;
    if (kanji.variant()) v = " (non-variant: " + kanji.nonVariantName() + ")";
    printError(kanji.name() + ' ' +
               toUnicode(kanji.name(), BracketType::Square) + v + " " + s +
               " in _ucd");
  }};

  const auto kanjiType{kanji.type()};
  if (const auto ucd{ucdIn ? ucdIn : _ucd.find(kanji.name())}; !ucd)
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

void KanjiData::printError(const String& msg) const {
  static size_t count;
  _err << "ERROR[" << std::setfill('0') << std::setw(4) << ++count << "] --- "
       << msg << std::setfill(' ') << '\n';
}

void KanjiData::loadFrequencyReadings(const Path& file) {
  const ColumnFile::Column nameCol{"Name"}, readingCol{"Reading"};
  for (ColumnFile f{file, {nameCol, readingCol}}; f.nextRow();)
    if (!_frequencyReadings.emplace(f.get(nameCol), f.get(readingCol)).second)
      f.error("duplicate name");
}

void KanjiData::populateJouyou() {
  auto results{CustomFileKanji::fromFile<JouyouKanji>(
      *this, KanjiListFile::getFile(_dataDir, JouyouFile))};
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(hasValue(i->grade()));
    if (checkInsert(i)) _grades[i->grade()].emplace_back(i);
  }
  _types[KanjiTypes::Jouyou] = std::move(results);
}

void KanjiData::populateLinkedKanji(const Path& file) {
  std::ifstream f{file};
  // each line in 'file' should be a Jouyou Kanji followed by the officially
  // recognized 'Jinmei Variant' (so populateJouyou must be called first)
  auto& linkedJinmei{_types[KanjiTypes::LinkedJinmei]};
  for (String line; std::getline(f, line);) {
    std::stringstream ss{line};
    if (String jouyou, linked;
        std::getline(ss, jouyou, '\t') && std::getline(ss, linked, '\t')) {
      if (const auto i{_kanjiNameMap.find(jouyou)}; i == _kanjiNameMap.end())
        usage("'" + jouyou + "' not found - file: " + file.filename().string());
      else
        checkInsert(linkedJinmei,
            std::make_shared<LinkedJinmeiKanji>(*this, linked, i->second));
    } else
      usage("bad line '" + line + "' - file: " + file.filename().string());
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

void KanjiData::populateJinmei() {
  auto results{CustomFileKanji::fromFile<JinmeiKanji>(
      *this, KanjiListFile::getFile(_dataDir, JinmeiFile))};
  for (auto& linkedJinmei{_types[KanjiTypes::LinkedJinmei]};
       const auto& i : results) {
    checkInsert(i);
    for (auto& j : i->oldNames())
      checkInsert(
          linkedJinmei, std::make_shared<LinkedJinmeiKanji>(*this, j, i));
  }
  _types[KanjiTypes::Jinmei] = std::move(results);
}

void KanjiData::populateExtra() {
  auto results{CustomFileKanji::fromFile<ExtraKanji>(
      *this, KanjiListFile::getFile(_dataDir, ExtraFile))};
  for (const auto& i : results) checkInsert(i);
  _types[KanjiTypes::Extra] = std::move(results);
}

void KanjiData::processList(const KanjiListFile& list) {
  const auto kenteiList{hasValue(list.kyu())};
  KanjiListFile::StringList created;
  std::map<KanjiTypes, KanjiListFile::StringList> found;
  auto& newKanji{
      _types[kenteiList ? KanjiTypes::Kentei : KanjiTypes::Frequency]};
  for (size_t i{}; i < list.list().size(); ++i) {
    auto& name{list.list()[i]};
    KanjiPtr kanji;
    if (const auto j{findKanjiByName(name)}; j) {
      kanji = j;
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
      const auto index{(kanji->frequency() - 1U) / FrequencyEntries};
      _frequencies[index < FrequencyBuckets ? index : FrequencyBuckets - 1]
          .emplace_back(kanji);
    }
  }
  if (fullDebug()) {
    KanjiListFile::print(
        _out, found[KanjiTypes::LinkedOld], "Linked Old", list.name());
    KanjiListFile::print(_out, created,
        String{"non-Jouyou/Jinmei"} + (hasValue(list.level()) ? "" : "/JLPT"),
        list.name());
    // list.level is None when processing 'frequency.txt' file (so not JLPT)
    if (!kenteiList && !(list.level())) {
      std::vector lists{std::pair{&found[KanjiTypes::Jinmei], ""},
          std::pair{&found[KanjiTypes::LinkedJinmei], "Linked "}};
      for (const auto& i : lists) {
        KanjiListFile::StringList jlptJinmei, otherJinmei;
        for (auto& j : *i.first)
          (hasValue(level(j)) ? jlptJinmei : otherJinmei).emplace_back(j);
        KanjiListFile::print(_out, jlptJinmei,
            String{"JLPT "} + i.second + "Jinmei", list.name());
        KanjiListFile::print(_out, otherJinmei,
            String{"non-JLPT "} + i.second + "Jinmei", list.name());
      }
    } else {
      KanjiListFile::print(
          _out, found[KanjiTypes::Jinmei], "Jinmei", list.name());
      KanjiListFile::print(
          _out, found[KanjiTypes::LinkedJinmei], "Linked Jinmei", list.name());
    }
  }
}

void KanjiData::processUcd() {
  // Calling 'findKanjiByName' checks for a 'variation selector' version of
  // 'name' so use it instead of checking for a match in _kanjiNameMap directly
  // (this avoids creating 52 redundant kanji when processing 'ucd.txt').
  for (auto& newKanji{_types[KanjiTypes::Ucd]}; const auto& i : _ucd.map())
    if (!findKanjiByName(i.second.name()))
      checkInsert(newKanji, std::make_shared<UcdKanji>(*this, i.second));
}

void KanjiData::checkStrokes() const {
  if (fullDebug()) {
    // Jouyou and Extra type Kanji load strokes from their own files so print
    // any differences with data in _ucd (other types shouldn't have any diffs)
    for (auto t : AllKanjiTypes) {
      KanjiListFile::StringList l;
      for (auto& i : _types[t])
        if (const auto u{findUcd(i->name())};
            u && i->strokes().value() != u->strokes().value())
          l.emplace_back(i->name());
      KanjiListFile::print(
          _out, l, toString(t) + " Kanji with differrent strokes", "_ucd");
    }
  }
}

} // namespace kanji_tools
