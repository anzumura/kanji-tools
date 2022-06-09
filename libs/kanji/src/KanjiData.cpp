#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/utils/Utf8.h>

#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

// This value is used for finding the location of 'data' directory. It will of
// course need to be updated if the number of '.txt' files changes.
constexpr size_t TextFilesInDataDir{10};

} // namespace

Args::Size KanjiData::nextArg(const Args& args, Args::Size current) {
  if (current > args.size())
    throw DomainError("current arg '" + std::to_string(current) +
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
  static constexpr MorohashiId EmptyMorohashiId; // LCOV_EXCL_LINE
  return u ? u->morohashiId() : EmptyMorohashiId;
}

Kanji::NelsonIds KanjiData::getNelsonIds(UcdPtr u) {
  Kanji::NelsonIds ids;
  if (u && !u->nelsonIds().empty()) {
    auto s{u->nelsonIds()};
    std::replace(s.begin(), s.end(), ',', ' ');
    std::stringstream ss{s};
    Kanji::NelsonId id{};
    while (ss >> id) ids.emplace_back(id);
  }
  return ids;
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
  // 'throw' should never happen - every 'Kanji' class instance should exist in
  // the data loaded from Unicode (in 'data/ucd.txt').
  throw DomainError{"UCD entry not found: " + kanji};
}

Strokes KanjiData::ucdStrokes(const String& kanji, UcdPtr u) const {
  if (u) return u->strokes();
  throw DomainError{"UCD entry not found: " + kanji};
}

RadicalRef KanjiData::getRadicalByName(const String& radicalName) const {
  return _radicals.find(radicalName);
}

Kanji::OptString KanjiData::getCompatibilityName(const String& kanji) const {
  const auto u{_ucd.find(kanji)};
  return u && u->name() != kanji ? Kanji::OptString{u->name()} : std::nullopt;
}

const KanjiData::List& KanjiData::frequencyList(size_t bucket) const {
  return bucket < FrequencyBuckets ? _frequencies[bucket]
                                   : BaseEnumMap<List>::Empty;
}

KanjiTypes KanjiData::getType(const String& name) const {
  const auto i{findByName(name)};
  return i ? i->type() : KanjiTypes::None;
}

KanjiPtr KanjiData::findByName(const String& s) const {
  const auto i{_compatibilityMap.find(s)};
  if (const auto j{_nameMap.find(i != _compatibilityMap.end() ? i->second : s)};
      j != _nameMap.end())
    return j->second;
  return {};
}

KanjiPtr KanjiData::findByFrequency(Kanji::Frequency freq) const {
  if (!freq || freq >= _maxFrequency) return {};
  auto bucket{--freq};
  bucket /= FrequencyEntries;
  if (bucket == FrequencyBuckets)
    --bucket; // last bucket contains FrequencyEntries + 1
  return _frequencies[bucket][freq - bucket * FrequencyEntries];
}

const KanjiData::List& KanjiData::findByMorohashiId(
    const MorohashiId& id) const {
  if (id) {
    if (const auto i{_morohashiMap.find(id)}; i != _morohashiMap.end())
      return i->second;
  }
  return BaseEnumMap<List>::Empty;
}

const KanjiData::List& KanjiData::findByMorohashiId(const String& id) const {
  return findByMorohashiId(MorohashiId{id});
}

const KanjiData::List& KanjiData::findByNelsonId(Kanji::NelsonId id) const {
  const auto i{_nelsonMap.find(id)};
  return i != _nelsonMap.end() ? i->second : BaseEnumMap<List>::Empty;
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
  static const Path DataDir{"data"};
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
  if (!_nameMap.emplace(k.name(), kanji).second) {
    printError("failed to insert '" + k.name() + "' into map");
    return false;
  }
  // Perform some sanity checks on newly created kanji, failures result in error
  // messages getting printed to stderr, but the program is allowed to continue
  // since it can be helpful to see more than one error printed out if something
  // goes wrong. Any failures should be fixed right away.
  insertSanityChecks(k, ucd);
  if (k.hasGrade()) _grades[k.grade()].emplace_back(kanji);
  if (k.variant() &&
      !_compatibilityMap.emplace(k.compatibilityName(), k.name()).second)
    printError("failed to insert variant '" + k.name() + "' into map");
  if (k.morohashiId()) _morohashiMap[k.morohashiId()].emplace_back(kanji);
  for (const auto id : k.nelsonIds()) _nelsonMap[id].emplace_back(kanji);
  return true;
}

bool KanjiData::checkInsert(List& s, const KanjiPtr& kanji) {
  if (!checkInsert(kanji)) return false;
  s.emplace_back(kanji);
  return true;
}

void KanjiData::addToKyus(const KanjiPtr& kanji) {
  assert(kanji->hasKyu());
  _kyus[kanji->kyu()].emplace_back(kanji);
}

void KanjiData::addToLevels(const KanjiPtr& kanji) {
  assert(kanji->hasLevel());
  _levels[kanji->level()].emplace_back(kanji);
}

void KanjiData::addToFrequencies(const KanjiPtr& kanji) {
  assert(kanji->frequency());
  const auto bucket{(kanji->frequency() - 1U) / FrequencyEntries};
  _frequencies[bucket < FrequencyBuckets ? bucket : FrequencyBuckets - 1]
      .emplace_back(kanji);
  if (kanji->frequency() >= _maxFrequency)
    _maxFrequency = kanji->frequency() + 1U;
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

void KanjiData::processUcd() {
  // Calling 'findKanjiByName' checks for a 'variation selector' version of
  // 'name' so use it instead of checking for a match in _nameMap directly
  // (this avoids creating 52 redundant kanji when processing 'ucd.txt').
  for (auto& newKanji{_types[KanjiTypes::Ucd]}; const auto& i : _ucd.map())
    if (!findByName(i.second.name()))
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
          _out, l, toString(t) + " Kanji with different strokes", "_ucd");
    }
  }
}

} // namespace kanji_tools
