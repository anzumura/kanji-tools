#include <kanji_tools/stats/MBCount.h>
#include <kanji_tools/stats/Stats.h>
#include <kanji_tools/utils/UnicodeBlock.h>
#include <kanji_tools/utils/Utils.h>

#include <future>
#include <numeric>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

// LCOV_EXCL_START - gcov bug
namespace {

constexpr auto HelpMessage{R"(kanjiStats [-bhv] file [file ...]:
  -b: show full Kanji breakdown for 'file' (instead of just a summary)
  -h: show help message for command-line options
  -v: show 'before' and 'after' versions of lines changed by Furigana removal
)"};
// LCOV_EXCL_STOP

[[nodiscard]] constexpr double asPercent(size_t amount, size_t total) {
  return static_cast<double>(amount) * 100. / static_cast<double>(total);
}

// helper class for ordering and printing out kanji found in files
class Count {
public: // LCOV_EXCL_LINE - gcov bug
  Count(size_t f, const std::string& n, Data::OptEntry e)
      : count{f}, name{n}, entry{e} {}

  [[nodiscard]] auto frequency() const {
    return entry ? (**entry).frequencyOrDefault(Data::maxFrequency())
                 : Data::maxFrequency() + 1;
  }
  [[nodiscard]] auto type() const {
    return entry ? (**entry).type() : KanjiTypes::None;
  }

  // Sort to have largest 'count' first followed by lowest frequency number.
  // Lower frequency means the kanji is more common, but a frequency of '0'
  // means the kanji isn't in the top frequency list so use 'frequencyOrDefault'
  // to return a large number for no-frequency kanji and consider 'not-found'
  // kanji to have even higher (worse) frequency. If kanjis both have the same
  // 'count' and 'frequency' then sort by type then name.
  [[nodiscard]] auto operator<(const Count& x) const {
    return count > x.count ||
           (count == x.count && frequency() < x.frequency() ||
               (frequency() == x.frequency() && type() < x.type() ||
                   (type() == x.type() && name < x.name)));
  }

  size_t count;
  std::string name;
  Data::OptEntry entry;
};

std::ostream& operator<<(std::ostream& os, const Count& c) {
  os << '[' << c.name << ' ' << std::right << std::setw(4) << c.count << ']';
  if (c.entry)
    os << std::setw(5) << (**c.entry).frequencyOrDefault(0) << ", "
       << ((**c.entry).hasLevel() ? toString((**c.entry).level())
                                  : std::string{"--"})
       << ", " << (**c.entry).type();
  else
    os << ", " << std::setw(7) << "U+" + toUnicode(c.name);
  return os;
}

// 'IncludeInTotals' of 4 indicates only Kanji and full-width kana should be
// included in totals and percents 'MaxExamples' is the maximum number of
// examples to show for each kanji type when printing stats
constexpr size_t IncludeInTotals{5}, MaxExamples{5};

// 'StatsPred' is a helper class for gathering stats matching a predicate (Pred)
// function across a group of files. The 'run' method is thread-safe and the
// results can be retrieved via the 'total' and 'str' methods.
class StatsPred {
public:
  StatsPred(const DataPtr data, const fs::path& top, const std::string& name,
      bool showBreakdown)
      : _data{data}, _top{top}, _name{name},
        _showBreakdown{showBreakdown}, _isKanji{name.ends_with("Kanji")} {}

  template<typename Pred>
  [[nodiscard]] std::string run(const Pred&, bool verbose, bool firstCount);

  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto total() const { return _total; }
  [[nodiscard]] auto isKanji() const { return _isKanji; }
private:
  // 'DisplayValues' are for ostream 'set' functions
  enum IntDisplayValues {
    UniqueCountWidth = 4,
    TotalCountWidth = 6,
    TypeNameWidth = 16
  };
  enum PercentDisplayValues { PercentPrecision = 2, PercentWidth = 6 };

  using CountSet = std::set<Count>;

  void printHeaderInfo(const MBCount&);
  void printTotalAndUnique(
      const std::string& name, size_t total, size_t unique);
  void printKanjiTypeCounts(const std::set<Count>&);
  void printExamples(const CountSet&);
  void printBreakdown(const CountSet&, const MBCount&);

  const DataPtr _data;
  const fs::path& _top;
  const std::string _name;
  const bool _showBreakdown;
  const bool _isKanji;
  size_t _total{};
  std::stringstream _os;
};

template<typename Pred>
std::string StatsPred::run(const Pred& pred, bool verbose, bool firstCount) {
  const auto isHiragana{_name == "Hiragana"},
      isUnrecognized{_name == "Unrecognized"},
      isCommonKanji{_isKanji && _name.starts_with("Common")};

  // Remove Furigana when processing Hiragana or MB-Letter to remove the effect
  // on counts, i.e., furigana in .txt files will artificially inflate Hiragana
  // count (and MB-Letter because of the wide brackets)
  const auto removeFurigana{
      isHiragana || _name == "Katakana" || _name == "MB-Letter"};

  if (isHiragana && verbose) _os << ">>> Showing all Furigana replacements:\n";

  MBCountIf count{pred,
      removeFurigana ? std::optional(MBCount::RemoveFurigana) : std::nullopt,
      MBCount::DefaultReplace, isHiragana && verbose ? &_os : nullptr};
  count.addFile(_top, _isKanji || isUnrecognized || isHiragana && verbose);
  if (firstCount) printHeaderInfo(count);
  CountSet frequency;
  for (const auto& i : count.map()) {
    _total += i.second;
    frequency.emplace(i.second, i.first,
        _isKanji ? _data->findKanjiByName(i.first) : std::nullopt);
  }
  if (_total) {
    printTotalAndUnique(_name, _total, frequency.size());
    if (isCommonKanji) { // only show type breakdown for 'Common Kanji'
      _os << ", 100.00%\n";
      printKanjiTypeCounts(frequency);
    } else if (_isKanji)
      // for 'Rare' and 'Non-UCD' Kanji show up to 'MaxExamples' entries
      printExamples(frequency);
    else
      _os << '\n';
    if (isUnrecognized || _isKanji && _showBreakdown)
      printBreakdown(frequency, count);
  }
  return _os.str();
}

void StatsPred::printHeaderInfo(const MBCount& count) {
  const auto file{_top.filename()};
  _os << ">>> Stats for: '"
      << (file.has_filename() ? file : _top.parent_path().filename()).string()
      << '\'';
  if (count.files() > 1) {
    _os << " (" << count.files() << (count.files() > 1 ? " files" : " file");
    if (count.directories() > 1)
      _os << " from " << count.directories() << " directories";
    _os << ')';
  }
  _os << " - showing top " << MaxExamples << " Kanji per type";
  if (count.replacements() || count.combiningMarks() || count.variants())
    _os << "\n>>> Furigana Removed: " << count.replacements()
        << ", Combining Marks Replaced: " << count.combiningMarks()
        << ", Variation Selectors: " << count.variants();
  if (count.errors()) _os << ", Errors: " << count.errors();
  _os << '\n';
}

void StatsPred::printTotalAndUnique(
    const std::string& name, size_t total, size_t unique) {
  _os << ">>> " << std::right << std::setw(TypeNameWidth) << name << ": "
      << std::setw(TotalCountWidth) << total
      << ", unique: " << std::setw(UniqueCountWidth) << unique;
}

void StatsPred::printKanjiTypeCounts(const std::set<Count>& frequency) {
  std::map<KanjiTypes, size_t> totalKanjiPerType, uniqueKanjiPerType;
  std::map<KanjiTypes, std::vector<Count>> found;
  for (const auto& i : frequency) {
    const auto t{i.type()};
    totalKanjiPerType[t] += i.count;
    uniqueKanjiPerType[t]++;
    if (auto& j{found[t]}; j.size() < MaxExamples) j.emplace_back(i);
  }
  for (const auto t : AllKanjiTypes)
    if (const auto i{uniqueKanjiPerType.find(t)};
        i != uniqueKanjiPerType.end()) {
      auto totalForType{totalKanjiPerType[t]};
      printTotalAndUnique(
          std::string{"["} + toString(t) + "] ", totalForType, i->second);
      _os << ", " << std::setw(PercentWidth) << std::fixed
          << std::setprecision(PercentPrecision)
          << asPercent(totalForType, _total) << "%  (";
      const auto& j{found[t]};
      for (size_t k{}; k < j.size(); ++k) {
        if (k) _os << ", ";
        _os << j[k].name << ' ' << j[k].count;
      }
      _os << ")\n";
    }
}

void StatsPred::printExamples(const CountSet& frequency) {
  _os << std::setw(12) << '(';
  for (size_t i{}; auto& j : frequency) {
    if (i) _os << ", ";
    _os << j.name << ' ' << j.count;
    if (++i == MaxExamples) break;
  }
  _os << ")\n";
}

void StatsPred::printBreakdown(
    const CountSet& frequency, const MBCount& count) {
  _os << ">>> Showing Breakdown for '" << _name << "':\n  Rank  [Val Num]"
      << (_isKanji && !_name.starts_with("Non-UCD")
                 ? " Freq, LV, Type"
                 : ", Unicode, Highest Count File")
      << '\n';
  for (size_t rank{}; auto& i : frequency) {
    _os << "  ";
    if (_showBreakdown) _os << std::left << std::setw(5) << ++rank << ' ';
    _os << i;
    if (!i.entry) {
      if (const auto tags{count.tags(i.name)}; tags) {
        std::string file;
        for (size_t maxCount{}; auto& j : *tags)
          if (j.second > maxCount) {
            maxCount = j.second;
            file = j.first;
          }
        _os << ", " << file;
      }
    }
    _os << '\n';
  }
}

} // namespace

Stats::Stats(const Args& args, DataPtr data) : _data(data) {
  auto breakdown{false}, endOptions{false}, verbose{false};
  std::vector<std::string> files;
  for (auto i{Data::nextArg(args)}; i < args.size(); i = Data::nextArg(args, i))
    if (std::string arg{args[i]}; !endOptions && arg.starts_with("-")) {
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
      files.emplace_back(arg);
  if (!data->debug() && files.empty())
    Data::usage("please specify at least one option or '-h' for help");
  for (auto& i : files) countKanji(i, breakdown, verbose);
}

void Stats::countKanji(
    const fs::path& top, bool showBreakdown, bool verbose) const {
  const auto f{[=, this, &top](const auto& pred, const std::string& name,
                   bool firstCount = false) {
    auto p{std::make_shared<StatsPred>(_data, top, name, showBreakdown)};
    return std::pair(std::async(std::launch::async,
                         [=] { return p->run(pred, verbose, firstCount); }),
        p);
  }};
  // 2579 Kanji are loaded into this program fall into the 'Rare' blocks (and
  // they are all type 'Ucd'). All other types (like Jouyou, Jinmei, etc.) are
  // in the 'Common' blocks (see comments in UnicodeBlock.h and
  // KanjiDataTest.cpp 'CommonAndRareBlocks').
  std::array totals{
      f([](const auto& x) { return isHiragana(x); }, "Hiragana", true),
      f([](const auto& x) { return isKatakana(x); }, "Katakana"),
      f([this](const auto& x) { return isCommonKanji(x) && _data->findUcd(x); },
          "Common Kanji"),
      f([this](const auto& x) { return isRareKanji(x) && _data->findUcd(x); },
          "Rare Kanji"),
      f([this](const auto& x) { return isKanji(x) && !_data->findUcd(x); },
          "Non-UCD Kanji"),
      f([](const auto& x) { return isMBPunctuation(x); }, "MB-Punctuation"),
      f([](const auto& x) { return isMBSymbol(x); }, "MB-Symbol"),
      f([](const auto& x) { return isMBLetter(x); }, "MB-Letter"),
      f([](const auto& x) { return !isRecognizedMBChar(x); }, "Unrecognized")};
  for (auto& i : totals) out() << i.first.get();
  size_t total{};
  for (size_t i{}; i < IncludeInTotals; ++i) total += totals[i].second->total();
  log() << "Total Kana+Kanji: " << total;
  if (total) {
    out() << " (" << std::fixed << std::setprecision(1);
    size_t totalKanji{};
    for (size_t i{}; i < IncludeInTotals; ++i)
      if (totals[i].second->isKanji())
        totalKanji += totals[i].second->total();
      else if (totals[i].second->total())
        out() << totals[i].second->name() << ": "
              << asPercent(totals[i].second->total(), total) << "%, ";
    out() << "Kanji: " << asPercent(totalKanji, total) << "%)";
  }
  out() << '\n';
}

} // namespace kanji_tools
