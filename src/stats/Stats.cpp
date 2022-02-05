#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/stats/Stats.h>
#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <future>
#include <numeric>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

constexpr auto HelpMessage = R"(kanjiStats [-bhv] file [file ...]:
  -b: show full kanji breakdown for 'file' (instead of just a summary)
  -h: show help message for command-line options
  -v: show 'before' and 'after' versions of lines that changed due to furigana removal
)";

[[nodiscard]] constexpr double asPercent(int amount, int total) { return amount * 100. / total; }

// helper class for ordering and printing out kanji found in files
class Count {
public:
  using OptEntry = Data::OptEntry;

  Count(int f, const std::string& n, OptEntry e) : count(f), name(n), entry(e) {}

  [[nodiscard]] auto frequency() const {
    return entry ? (**entry).frequencyOrDefault(Data::maxFrequency()) : Data::maxFrequency() + 1;
  }
  [[nodiscard]] auto type() const { return entry ? (**entry).type() : KanjiTypes::None; }
  [[nodiscard]] std::string toHex() const;

  // Sort to have largest 'count' first followed by lowest frequency number. Lower frequency
  // means the kanji is more common, but a frequency of '0' means the kanji isn't in the top
  // frequency list so use 'frequencyOrDefault' to return a large number for no-frequency
  // kanji and consider 'not-found' kanji to have even higher (worse) frequency. If kanjis
  // both have the same 'count' and 'frequency' then sort by type then hex (use 'hex' instead of
  // 'name' since sorting by UTF-8 is less consistent).
  [[nodiscard]] auto operator<(const Count& x) const {
    return count > x.count ||
      (count == x.count && frequency() < x.frequency() ||
       (frequency() == x.frequency() && type() < x.type() || (type() == x.type() && toHex() < x.toHex())));
  }

  int count;
  std::string name;
  OptEntry entry;
};

std::string Count::toHex() const {
  std::string result;
  if (const auto s = fromUtf8(name); s.length() == 1) result = "'\\u" + kanji_tools::toHex(s[0]) + "', ";
  for (const auto i : name) {
    if (!result.empty()) result += ' ';
    result += "'\\x" + kanji_tools::toHex(i) + "'";
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const Count& c) {
  os << '[' << c.name << ' ' << std::right << std::setw(4) << c.count << ']';
  if (c.entry)
    os << std::setw(5) << (**c.entry).frequencyOrDefault(0) << ", "
       << ((**c.entry).hasLevel() ? toString((**c.entry).level()) : std::string("--")) << ", " << (**c.entry).type();
  else
    os << ", " << c.toHex();
  return os;
}

// 'IncludeInTotals' of 4 indicates only Kanji and full-width kana should be included in totals and percents
// 'MaxExamples' is the maximum number of examples to show for each kanji type when printing stats
constexpr int IncludeInTotals = 4, MaxExamples = 5;

// 'StatsPred' is a helper class for gathering stats matching a predicate (Pred) function across a group of
// files. The 'run' method is thread-safe and the results can be retrieved via the 'total' and 'str' methods.
class StatsPred {
public:
  StatsPred(const DataPtr data, const fs::path& top, const std::string& name, bool showBreakdown)
    : _data(data), _top(top), _name(name), _showBreakdown(showBreakdown) {}

  template<typename Pred> void run(const Pred&, bool verbose, bool firstCount);

  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto total() const { return _total; }
  [[nodiscard]] auto str() const { return _os.str(); }
private:
  // 'DisplayValues' are for ostream 'set' functions
  enum IntDisplayValues { UniqueCountWidth = 4, TotalCountWidth = 6, TypeNameWidth = 16 };
  enum PercentDisplayValues { PercentPrecision = 2, PercentWidth = 6 };

  using CountSet = std::set<Count>;

  void printHeaderInfo(const MBCharCount&);
  void printTotalAndUnique(const std::string& name, int total, int unique);
  void printKanjiTypeCounts(const std::set<Count>&);
  void printExamples(const CountSet&);
  void printBreakdown(const CountSet&, const MBCharCount&);

  const DataPtr _data;
  const fs::path& _top;
  const std::string _name;
  const bool _showBreakdown;
  int _total = 0;
  std::stringstream _os;
};

template<typename Pred> void StatsPred::run(const Pred& pred, bool verbose, bool firstCount) {
  const auto isKanji(_name.ends_with("Kanji")), isHiragana(_name == "Hiragana"),
    isUnrecognized(_name == "Unrecognized");
  const auto isCommonKanji(isKanji && _name.starts_with("Common"));

  // Remove furigana when processing Hiragana or MB-Letter to remove the effect on counts, i.e., furigana
  // in .txt files will artificially inflate Hiragana count (and MB-Letter because of the wide brackets)
  const auto removeFurigana(isHiragana || _name == "Katakana" || _name == "MB-Letter");

  if (isHiragana && verbose) _os << ">>> Showing all furigana replacements:\n";

  MBCharCountIf count(pred, removeFurigana ? std::optional(MBCharCount::RemoveFurigana) : std::nullopt,
                      MBCharCount::DefaultReplace, isHiragana && verbose);
  count.addFile(_top, isKanji || isUnrecognized || isHiragana && verbose);
  if (firstCount) printHeaderInfo(count);
  CountSet frequency;
  for (const auto& i : count.map()) {
    _total += i.second;
    frequency.emplace(i.second, i.first, isKanji ? _data->findKanjiByName(i.first) : std::nullopt);
  }
  if (_total) {
    printTotalAndUnique(_name, _total, frequency.size());
    if (isCommonKanji) { // only show percentage and type breakdown for 'Common Kanji'
      _os << ", 100.00%\n";
      printKanjiTypeCounts(frequency);
    } else if (isKanji) // for 'Rare' and 'Non-UCD' Kanji show up to 'MaxExamples' entries in a single line
      printExamples(frequency);
    else
      _os << '\n';
    if (isUnrecognized || isKanji && _showBreakdown) printBreakdown(frequency, count);
  }
}

void StatsPred::printHeaderInfo(const MBCharCount& count) {
  auto filename = _top.filename();
  _os << ">>> Stats for: " << (filename.has_filename() ? filename.string() : _top.parent_path().filename().string());
  if (count.files() > 1) {
    _os << " (" << count.files() << (count.files() > 1 ? " files" : " file");
    if (count.directories() > 1) _os << " from " << count.directories() << " directories";
    _os << ')';
  }
  _os << " - showing " << MaxExamples << " most frequent kanji per type";
  if (count.errors()) _os << ", errors: " << count.errors();
  if (count.variants()) _os << ", variants: " << count.errors();
  _os << '\n';
}

void StatsPred::printTotalAndUnique(const std::string& name, int total, int unique) {
  _os << ">>> " << std::right << std::setw(TypeNameWidth) << name << ": " << std::setw(TotalCountWidth) << total
      << ", unique: " << std::setw(UniqueCountWidth) << unique;
}

void StatsPred::printKanjiTypeCounts(const std::set<Count>& frequency) {
  std::map<KanjiTypes, int> totalKanjiPerType, uniqueKanjiPerType;
  std::map<KanjiTypes, std::vector<Count>> found;
  for (const auto& i : frequency) {
    const auto t = i.type();
    totalKanjiPerType[t] += i.count;
    uniqueKanjiPerType[t]++;
    if (auto& j = found[t]; j.size() < MaxExamples) j.push_back(i);
  }
  for (const auto t : AllKanjiTypes)
    if (const auto i = uniqueKanjiPerType.find(t); i != uniqueKanjiPerType.end()) {
      auto totalForType = totalKanjiPerType[t];
      printTotalAndUnique(std::string("[") + toString(t) + "] ", totalForType, i->second);
      _os << ", " << std::setw(PercentWidth) << std::fixed << std::setprecision(PercentPrecision)
          << asPercent(totalForType, _total) << "%  (";
      const auto& j = found[t];
      for (size_t k = 0; k < j.size(); ++k) {
        if (k) _os << ", ";
        _os << j[k].name << ' ' << j[k].count;
      }
      _os << ")\n";
    }
}

void StatsPred::printExamples(const CountSet& frequency) {
  _os << std::setw(12) << '(';
  for (auto i = 0; auto& j : frequency) {
    if (i) _os << ", ";
    _os << j.name << ' ' << j.count;
    if (++i == MaxExamples) break;
  }
  _os << ")\n";
}

void StatsPred::printBreakdown(const CountSet& frequency, const MBCharCount& count) {
  _os << ">>> Showing Breakdown for '" << _name << "':\n";
  _os << "  " << (_showBreakdown ? "Rank  [Val #] Freq, LV, Type (No.) ==" : "[Val #], Missing Unicode,")
      << " Highest Count File\n";
  DataFile::List missing;
  for (auto rank = 0; auto& i : frequency) {
    _os << "  ";
    if (_showBreakdown) _os << std::left << std::setw(5) << ++rank << ' ';
    _os << i;
    if (!i.entry) {
      missing.push_back(i.name);
      if (const auto tags = count.tags(i.name); tags != nullptr) {
        std::string file;
        for (auto maxCount = 0; auto& j : *tags)
          if (j.second > maxCount) {
            maxCount = j.second;
            file = j.first;
          }
        _os << (_showBreakdown ? " == " : ", ") << file;
      }
    }
    _os << '\n';
  }
  if (_showBreakdown) DataFile::print(_os, missing, "missing");
}

} // namespace

Stats::Stats(size_t argc, const char** argv, DataPtr data) : _data(data) {
  auto breakdown = false, endOptions = false, verbose = false;
  std::vector<std::string> files;
  for (auto i = Data::nextArg(argc, argv); i < argc; i = Data::nextArg(argc, argv, i))
    if (std::string arg = argv[i]; !endOptions && arg.starts_with("-")) {
      if (arg == "-h") {
        out() << HelpMessage;
        return;
      }
      if (arg == "-b")
        breakdown = true;
      else if (arg == "-v")
        verbose = true;
      else if (arg == "--")
        endOptions = true;
      else
        Data::usage("illegal option '" + arg + "' use -h for help");
    } else
      files.push_back(arg);
  if (!data->debug() && files.empty()) Data::usage("please specify at least one option or '-h' for help");
  for (auto& i : files) countKanji(i, breakdown, verbose);
}

void Stats::countKanji(const fs::path& top, bool showBreakdown, bool verbose) const {
  const auto f = [=, this, &top](const auto& pred, const std::string& name, bool firstCount = false) {
    auto p = std::make_shared<StatsPred>(_data, top, name, showBreakdown);
    return std::pair(std::async(std::launch::async, [=] { return p->run(pred, verbose, firstCount); }), p);
  };
  std::array totals{f([](const auto& x) { return isHiragana(x); }, "Hiragana", true),
                    f([](const auto& x) { return isKatakana(x); }, "Katakana"),
                    f([](const auto& x) { return isCommonKanji(x); }, "Common Kanji"),
                    f([](const auto& x) { return isRareKanji(x); }, "Rare Kanji"),
                    f([this](const auto& x) { return isKanji(x) && !_data->ucd().find(x); }, "Non-UCD Kanji"),
                    f([](const auto& x) { return isMBPunctuation(x, false); }, "MB-Punctuation"),
                    f([](const auto& x) { return isMBSymbol(x); }, "MB-Symbol"),
                    f([](const auto& x) { return isMBLetter(x); }, "MB-Letter"),
                    f([](const auto& x) { return !isRecognizedCharacter(x); }, "Unrecognized")};
  for (auto& i : totals) {
    i.first.wait();
    out() << i.second->str();
  }
  auto total = 0;
  for (auto i = 0; i < IncludeInTotals; ++i) total += totals[i].second->total();
  log() << "Total Kanji+Kana: " << total;
  if (total) {
    out() << " (" << std::fixed << std::setprecision(1);
    auto totalPrinted = false;
    for (auto i = 0; i < IncludeInTotals; ++i)
      if (totals[i].second->total()) {
        if (totalPrinted)
          out() << ", ";
        else
          totalPrinted = true;
        out() << totals[i].second->name() << ": " << asPercent(totals[i].second->total(), total) << "%";
      }
    out() << ')';
  }
  out() << '\n';
}

} // namespace kanji_tools
