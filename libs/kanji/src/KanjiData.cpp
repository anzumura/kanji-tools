#include <kt_kanji/KanjiData.h>
#include <kt_utils/Utf8.h>

#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

// This value is used for finding the location of 'data' directory. It will of
// course need to be updated if the number of '.txt' files changes.
constexpr size_t TextFilesInDataDir{10};

constexpr auto MaxVariantSelectorExamples{5};

} // namespace

// KanjiData public methods

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

void KanjiData::usage(const String& msg) { ListFile::usage(msg); }

Kanji::Frequency KanjiData::maxFrequency() noexcept { return _maxFrequency; }

const Pinyin& KanjiData::getPinyin(UcdPtr u) noexcept {
  static constexpr Pinyin EmptyPinyin; // LCOV_EXCL_LINE
  return u ? u->pinyin() : EmptyPinyin;
}

const MorohashiId& KanjiData::getMorohashiId(UcdPtr u) noexcept {
  static constexpr MorohashiId EmptyMorohashiId;
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
                                   : BaseEnumMap<List>::Empty; // LCOV_EXCL_LINE
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

void KanjiData::printError(const String& msg) const {
  static size_t count;
  _err << "ERROR[" << std::setfill('0') << std::setw(4) << ++count << "] --- "
       << msg << std::setfill(' ') << '\n';
}

std::ostream& KanjiData::log(bool heading) const {
  return heading ? _out << ">>>\n>>> " : _out << ">>> ";
}

// KanjiData protected methods

KanjiData::KanjiData(const Path& dataDir, DebugMode debugMode,
    std::ostream& out, std::ostream& err)
    : _dataDir{dataDir}, _debugMode{debugMode}, _out{out}, _err{err} {
  // Clearing ListFile static data is only needed to help test code, for
  // example ListFile tests can leave some data in these sets before Quiz
  // tests are run (leading to problems loading real files).
  ListFile::clearUniqueCheckData();
  if (fullDebug()) log(true) << "Begin Loading Data\n>>>\n";
}

void KanjiData::finishedLoadingData() {
  processUcd();
  if (fullDebug()) log(true) << "Finished Loading Data\n>>>\n";
  if (debug()) {
    printCountsAndStats();
    printGrades();
    if (fullDebug()) {
      printListStats<&Kanji::level>(AllJlptLevels, "Level", true);
      printListStats<&Kanji::kyu>(AllKenteiKyus, "Kyu", false);
      _radicals.print(*this);
      _ucd.print(*this);
    }
  }
}

fs::path KanjiData::getDataDir(const Args& args) {
  static const String ExpectedTextFiles{
      std::to_string(TextFilesInDataDir) + " expected '" +
      ListFile::TextFileExtension + "' files"};
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

// KanjiData private methods

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
               return i.path().extension() == ListFile::TextFileExtension;
             }) == TextFilesInDataDir;
}

void KanjiData::insertSanityChecks(const Kanji& kanji, UcdPtr u) const {
  const auto error{[this, &kanji](const String& s) {
    String v;
    if (kanji.variant()) v = " (non-variant: " + kanji.nonVariantName() + ")";
    printError(kanji.name() + ' ' +
               toUnicode(kanji.name(), BracketType::Square) + v + " " + s +
               " in _ucd");
  }};

  const auto kanjiType{kanji.type()};
  if (const auto ucd{u ? u : _ucd.find(kanji.name())}; !ucd)
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

void KanjiData::processUcd() {
  // Calling findByName() checks for a 'variation selector' version of 'name' so
  // use it instead of checking for a match in _nameMap directly (this avoids
  // creating 52 redundant Kanji when processing 'ucd.txt').
  for (auto& newKanji{_types[KanjiTypes::Ucd]}; const auto& i : _ucd.map())
    if (!findByName(i.second.name()))
      checkInsert(newKanji, std::make_shared<UcdKanji>(*this, i.second));
  if (fullDebug()) checkStrokes();
}

void KanjiData::checkStrokes() const {
  // Jouyou and Extra type Kanji load strokes from their own files so print
  // any differences with data in _ucd (other types shouldn't have any diffs)
  for (auto t : AllKanjiTypes) {
    ListFile::StringList l;
    for (auto& i : _types[t])
      if (const auto u{findUcd(i->name())};
          u && i->strokes().value() != u->strokes().value())
        l.emplace_back(i->name());
    ListFile::print(
        _out, l, toString(t) + " Kanji with different strokes", "_ucd");
  }
}

void KanjiData::printCountsAndStats() const {
  log() << "Loaded " << _nameMap.size() << " Kanji (";
  for (auto i{AllKanjiTypes.begin()}; auto& j : _types) {
    if (i != AllKanjiTypes.begin()) _out << ' ';
    _out << *i++ << ' ' << j.size();
  }
  _out << ")\n";
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

template <auto Pred>
void KanjiData::printCount(const String& name, size_t printExamples) const {
  std::vector<std::pair<KanjiTypes, size_t>> counts;
  std::map<KanjiTypes, std::vector<String>> examples;
  size_t total{};
  for (auto i{AllKanjiTypes.begin()}; auto& l : _types) {
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
      _out << i.first << ' ' << i.second;
      for (const auto& j : examples[i.first]) _out << ' ' << j;
      total -= i.second;
      if (total) _out << ", ";
    }
    _out << ")\n";
  }
}

void KanjiData::printGrades() const {
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
      _out << " (";
      for (const auto level : AllJlptLevels) {
        const auto gradeLevelCount{static_cast<size_t>(std::count_if(
            jouyou.begin(), jouyou.end(), [&grade, level](auto& x) {
              return grade(x) && x->level() == level;
            }))};
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

template <auto F, typename T>
void KanjiData::printListStats(
    const T& list, const String& name, bool showNoFreq) const {
  log() << name << " breakdown:\n";
  size_t total{};
  for (const auto i : list) {
    std::vector<std::pair<KanjiTypes, size_t>> counts;
    size_t iTotal{};
    for (auto j{AllKanjiTypes.begin()}; auto& l : _types) {
      const auto t{*j++};
      if (const auto c{static_cast<size_t>(std::count_if(
              l.begin(), l.end(), [i](auto& x) { return ((*x).*F)() == i; }))};
          c) {
        counts.emplace_back(t, c);
        iTotal += c;
      }
    }
    if (iTotal) {
      total += iTotal;
      log() << "  Total for " << name << ' ' << i << ": " << iTotal << " (";
      for (const auto& j : counts) {
        _out << j.first << ' ' << j.second;
        auto& l{types()[j.first]};
        if (showNoFreq)
          noFreq(std::count_if(l.begin(), l.end(),
              [i](auto& x) { return ((*x).*F)() == i && !x->frequency(); }));
        iTotal -= j.second;
        if (iTotal) _out << ", ";
      }
      _out << ")\n";
    }
  }
  log() << "  Total for all " << name << "s: " << total << '\n';
}

void KanjiData::noFreq(std::ptrdiff_t f, bool brackets) const {
  if (f) {
    if (brackets)
      _out << " (";
    else
      _out << ' ';
    _out << "nf " << f;
    if (brackets) _out << ')';
  }
}

} // namespace kanji_tools
