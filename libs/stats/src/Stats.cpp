#include <kanji_tools/stats/Stats.h>
#include <kanji_tools/stats/Utf8Count.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <future>
#include <numeric>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

constexpr auto HelpMessage{R"(kanjiStats [-bhv] file [file ...]:
  -b: show full Kanji breakdown for 'file' (instead of just a summary)
  -h: show help message for command-line options
  -v: show 'before' and 'after' versions of lines changed by Furigana removal
)"};

[[nodiscard]] constexpr double asPercent(size_t amount, size_t total) {
  return static_cast<double>(amount) * 100. / static_cast<double>(total);
}

} // namespace

Stats::Stats(const Args& args, const KanjiDataPtr& data) : _data(data) {
  auto breakdown{false}, endOptions{false}, verbose{false};
  std::vector<String> files;
  for (auto i{KanjiData::nextArg(args)}; i < args.size();
       i = KanjiData::nextArg(args, i))
    if (String arg{args[i]}; !endOptions && arg.starts_with("-")) {
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
        KanjiData::usage("illegal option '" + arg + "' use -h for help");
    } else
      files.emplace_back(arg);
  if (!data->debug() && files.empty())
    KanjiData::usage("please specify at least one option or '-h' for help");
  for (auto& i : files) countKanji(i, breakdown, verbose);
}

void Stats::countKanji(
    const fs::path& top, bool showBreakdown, bool verbose) const {
  // lambda to create a 'pair<shared_ptr<Stats::Pred>, Future<String>>'
  const auto f{[=, this, &top](const auto& pred, const String& name,
                   bool firstCount = false) {
    // NOLINTNEXTLINE
    auto p{std::make_shared<Pred>(_data, top, name, showBreakdown)};
    return std::pair{p, std::async(std::launch::async,
                            [=] { return p->run(pred, verbose, firstCount); })};
  }};
  // 2579 Kanji loaded by this program fall into 'Rare' blocks (and they are all
  // type 'Ucd'). All other types (like Jouyou, Jinmei, etc.) are in 'Common'
  // blocks (see UnicodeBlock.h and KanjiDataTest.cpp 'CommonAndRareBlocks').
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
      f([](const auto& x) { return !isRecognizedUtf8(x); }, "Unrecognized")};
  for (auto& i : totals) out() << i.second.get();
  size_t total{};
  for (size_t i{}; i < IncludeInTotals; ++i) total += totals[i].first->total();
  log() << "Total Kana+Kanji: " << total;
  if (total) {
    out() << " (" << std::fixed << std::setprecision(1);
    size_t totalKanji{};
    for (size_t i{}; i < IncludeInTotals; ++i)
      if (totals[i].first->isKanji())
        totalKanji += totals[i].first->total();
      else if (totals[i].first->total())
        out() << totals[i].first->name() << ": "
              << asPercent(totals[i].first->total(), total) << "%, ";
    out() << "Kanji: " << asPercent(totalKanji, total) << "%)";
  }
  out() << '\n';
}

std::ostream& Stats::log(bool heading) const { return _data->log(heading); }

std::ostream& Stats::out() const { return _data->out(); }

/// Stats::Count

Stats::Count::Count(size_t count, const String& name, const KanjiPtr& entry)
    : _count{count}, _name{name}, _entry{entry} {}

Kanji::Frequency Stats::Count::frequency() const {
  return _entry ? _entry->frequencyOrDefault(KanjiData::maxFrequency())
                : KanjiData::maxFrequency() + 1;
}

KanjiTypes Stats::Count::type() const {
  return _entry ? _entry->type() : KanjiTypes::None;
}

bool Stats::Count::operator<(const Count& x) const {
  return _count > x._count ||
         (_count == x._count && frequency() < x.frequency() ||
             (frequency() == x.frequency() && type() < x.type() ||
                 (type() == x.type() && _name < x._name)));
}

std::ostream& operator<<(std::ostream& os, const Stats::Count& c) {
  static constexpr auto FreqWidth{5};
  os << '[' << c.name() << ' ' << std::right << std::setw(4) << c.count()
     << ']';
  if (c.entry())
    os << std::setw(FreqWidth) << c.entry()->frequencyOrDefault(0) << ", "
       << (c.entry()->hasLevel() ? toString(c.entry()->level()) : String{"--"})
       << ", " << c.entry()->type();
  else
    os << ", " << std::setw(UnicodeStringMaxSize + 2)
       << "U+" + toUnicode(c.name());
  return os;
}

// Stats::Pred

Stats::Pred::Pred(const KanjiDataPtr& data, const KanjiData::Path& top,
    const String& name, bool showBreakdown)
    : _data{data}, _top{top}, _name{name},
      _showBreakdown{showBreakdown}, _isKanji{name.ends_with("Kanji")} {}

template<typename T>
String Stats::Pred::run(const T& pred, bool verbose, bool firstCount) {
  const auto isHiragana{_name == "Hiragana"},
      isUnrecognized{_name == "Unrecognized"},
      isCommonKanji{_isKanji && _name.starts_with("Common")};

  // Remove Furigana when processing Hiragana or MB-Letter to remove the effect
  // on counts, i.e., Furigana in .txt files will artificially inflate Hiragana
  // count (and MB-Letter because of the wide brackets)
  const auto removeFurigana{
      isHiragana || _name == "Katakana" || _name == "MB-Letter"};

  if (isHiragana && verbose) _os << ">>> Showing all Furigana replacements:\n";

  Utf8CountIf count{pred,
      removeFurigana ? std::optional(Utf8Count::RemoveFurigana) : std::nullopt,
      Utf8Count::DefaultReplace, isHiragana && verbose ? &_os : nullptr};
  count.addFile(_top, _isKanji || isUnrecognized || isHiragana && verbose);
  if (firstCount) printHeaderInfo(count);
  CountSet frequency;
  for (const auto& i : count.map()) {
    _total += i.second;
    frequency.emplace(
        i.second, i.first, _isKanji ? _data->findByName(i.first) : KanjiPtr{});
  }
  if (_total) {
    printTotalAndUnique(_name, _total, frequency.size());
    if (isCommonKanji) { // only show type breakdown for 'Common Kanji'
      _os << ", 100.00%\n";
      printKanjiTypeCounts(frequency);
    } else if (_isKanji)
      // for 'Rare' and 'Non-UCD' Kanji show up to 'MaxExamples' entries
      printRareExamples(frequency);
    else
      _os << '\n';
    if (isUnrecognized || _isKanji && _showBreakdown)
      printBreakdown(frequency, count);
  }
  return _os.str();
}

void Stats::Pred::printHeaderInfo(const Utf8Count& count) {
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

void Stats::Pred::printTotalAndUnique(
    const String& name, size_t total, size_t unique) {
  _os << ">>> " << std::right << std::setw(TypeNameWidth) << name << ": "
      << std::setw(TotalCountWidth) << total
      << ", unique: " << std::setw(UniqueCountWidth) << unique;
}

void Stats::Pred::printKanjiTypeCounts(const std::set<Count>& frequency) {
  std::map<KanjiTypes, size_t> totalKanjiPerType, uniqueKanjiPerType;
  std::map<KanjiTypes, std::vector<Count>> found;
  for (const auto& i : frequency) {
    const auto t{i.type()};
    totalKanjiPerType[t] += i.count();
    uniqueKanjiPerType[t]++;
    if (auto& j{found[t]}; j.size() < MaxExamples) j.emplace_back(i);
  }
  for (const auto t : AllKanjiTypes)
    if (const auto i{uniqueKanjiPerType.find(t)};
        i != uniqueKanjiPerType.end()) {
      auto totalForType{totalKanjiPerType[t]};
      printTotalAndUnique(
          String{"["} + toString(t) + "] ", totalForType, i->second);
      _os << ", " << std::setw(PercentWidth) << std::fixed
          << std::setprecision(PercentPrecision)
          << asPercent(totalForType, _total) << "%  (";
      const auto& j{found[t]};
      for (size_t k{}; k < j.size(); ++k) {
        if (k) _os << ", ";
        _os << j[k].name() << ' ' << j[k].count();
      }
      _os << ")\n";
    }
}

void Stats::Pred::printRareExamples(const CountSet& frequency) {
  // percents aren't shown before rare examples so add spacing to align output
  static constexpr auto SkipPercentageWidth{12};
  _os << std::setw(SkipPercentageWidth) << '(';
  for (size_t i{}; auto& j : frequency) {
    if (i) _os << ", ";
    _os << j.name() << ' ' << j.count();
    if (++i == MaxExamples) break;
  }
  _os << ")\n";
}

void Stats::Pred::printBreakdown(
    const CountSet& frequency, const Utf8Count& count) {
  _os << ">>> Showing Breakdown for '" << _name << "':\n  Rank  [Val Num]"
      << (_isKanji && !_name.starts_with("Non-UCD")
                 ? " Freq, LV, Type"
                 : ", Unicode, Highest Count File")
      << '\n';
  static constexpr auto MinRankWidth{5};
  for (size_t rank{}; auto& i : frequency) {
    _os << "  ";
    if (_showBreakdown)
      _os << std::left << std::setw(MinRankWidth) << ++rank << ' ';
    _os << i;
    if (!i.entry()) {
      if (const auto tags{count.tags(i.name())}; tags) {
        String file;
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

} // namespace kanji_tools
