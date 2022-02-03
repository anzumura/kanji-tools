#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/stats/Stats.h>
#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <numeric>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

std::ostream& operator<<(std::ostream& os, const Stats::Count& c) {
  os << '[' << c.name << ' ' << std::right << std::setw(4) << c.count << ']';
  if (c.entry)
    os << std::setw(5) << (**c.entry).frequencyOrDefault(0) << ", "
       << ((**c.entry).hasLevel() ? toString((**c.entry).level()) : std::string("--")) << ", " << (**c.entry).type();
  else
    os << ", " << c.toHex();
  return os;
}

constexpr auto HelpMessage = R"(kanjiStats [-bhv] file [file ...]:
  -b: show full kanji breakdown for 'file' (instead of just a summary)
  -h: show help message for command-line options
  -v: show 'before' and 'after' versions of lines that changed due to furigana removal
)";

[[nodiscard]] constexpr double asPercent(int amount, int total) { return amount * 100. / total; }

} // namespace

std::string Stats::Count::toHex() const {
  std::string result;
  if (const auto s = fromUtf8(name); s.length() == 1) result = "'\\u" + kanji_tools::toHex(s[0]) + "', ";
  for (const auto i : name) {
    if (!result.empty()) result += ' ';
    result += "'\\x" + kanji_tools::toHex(i) + "'";
  }
  return result;
}

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
  const auto f = [this, &top, showBreakdown, verbose](const auto& x, const auto& y, bool firstCount = false) {
    return std::pair(processCount(top, x, y, showBreakdown, firstCount, verbose), y);
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
  auto total = 0;
  for (auto i = 0; i < IncludeInTotals; ++i) total += totals[i].first;
  log() << "Total Kanji+Kana: " << total;
  if (total) {
    out() << " (" << std::fixed << std::setprecision(1);
    for (auto i = 0; i < IncludeInTotals; ++i)
      if (totals[i].first) {
        if (totals[i].second != totals[0].second) out() << ", ";
        out() << totals[i].second << ": " << asPercent(totals[i].first, total) << "%";
      }
    out() << ')';
  }
  out() << '\n';
}

template<typename Pred>
int Stats::processCount(const fs::path& top, const Pred& pred, const std::string& name, bool showBreakdown,
                        bool firstCount, bool verbose) const {
  const auto isKanji(name.ends_with("Kanji")), isHiragana(name == "Hiragana"), isUnrecognized(name == "Unrecognized");
  const auto isCommonKanji(isKanji && name.starts_with("Common"));

  // Remove furigana when processing Hiragana or MB-Letter to remove the effect on counts, i.e., furigana
  // in .txt files will artificially inflate Hiragana count (and MB-Letter because of the wide brackets)
  const auto removeFurigana(isHiragana || name == "Katakana" || name == "MB-Letter");

  if (isHiragana && verbose) log() << "Showing all furigana replacements:\n";

  MBCharCountIf count(pred, removeFurigana ? std::optional(MBCharCount::RemoveFurigana) : std::nullopt,
                      MBCharCount::DefaultReplace, isHiragana && verbose);
  count.addFile(top, isKanji || isUnrecognized || isHiragana && verbose);
  if (firstCount) printHeaderInfo(top, count);
  CountSet frequency;
  auto total = 0;
  for (const auto& i : count.map()) {
    total += i.second;
    frequency.emplace(i.second, i.first, isKanji ? _data->findKanjiByName(i.first) : std::nullopt);
  }
  if (total) {
    printTotalAndUnique(name, total, frequency.size());
    if (isCommonKanji) { // only show percentage and type breakdown for 'Common Kanji'
      out() << ", 100.00%\n";
      printKanjiTypeCounts(frequency, total);
    } else if (isKanji) // for 'Rare' and 'Non-UCD' Kanji show up to 'MaxExamples' entries in a single line
      printExamples(frequency);
    else
      out() << '\n';
    if (isUnrecognized || isKanji && showBreakdown) printBreakdown(name, showBreakdown, frequency, count);
  }
  return total;
}

void Stats::printHeaderInfo(const fs::path& top, const MBCharCount& count) const {
  auto filename = top.filename();
  log() << "Stats for: " << (filename.has_filename() ? filename.string() : top.parent_path().filename().string());
  if (count.files() > 1) {
    out() << " (" << count.files() << (count.files() > 1 ? " files" : " file");
    if (count.directories() > 1) out() << " from " << count.directories() << " directories";
    out() << ')';
  }
  out() << " - showing " << MaxExamples << " most frequent kanji per type";
  if (count.errors()) out() << ", errors: " << count.errors();
  if (count.variants()) out() << ", variants: " << count.errors();
  out() << '\n';
}

void Stats::printTotalAndUnique(const std::string& name, int total, int unique) const {
  log() << std::right << std::setw(TypeNameWidth) << name << ": " << std::setw(TotalCountWidth) << total
        << ", unique: " << std::setw(UniqueCountWidth) << unique;
}

void Stats::printKanjiTypeCounts(const std::set<Count>& frequency, int total) const {
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
      out() << ", " << std::setw(PercentWidth) << std::fixed << std::setprecision(PercentPrecision)
            << asPercent(totalForType, total) << "%  (";
      const auto& j = found[t];
      for (size_t k = 0; k < j.size(); ++k) {
        if (k) out() << ", ";
        out() << j[k].name << ' ' << j[k].count;
      }
      out() << ")\n";
    }
}

void Stats::printExamples(const CountSet& frequency) const {
  out() << std::setw(12) << '(';
  for (auto i = 0; auto& j : frequency) {
    if (i) out() << ", ";
    out() << j.name << ' ' << j.count;
    if (++i == MaxExamples) break;
  }
  out() << ")\n";
}

void Stats::printBreakdown(const std::string& name, bool showBreakdown, const CountSet& frequency,
                           const MBCharCount& count) const {
  log() << "Showing Breakdown for '" << name << "':\n";
  out() << "  " << (showBreakdown ? "Rank  [Val #] Freq, LV, Type (No.) ==" : "[Val #], Missing Unicode,")
        << " Highest Count File\n";
  DataFile::List missing;
  for (auto rank = 0; auto& i : frequency) {
    out() << "  ";
    if (showBreakdown) out() << std::left << std::setw(5) << ++rank << ' ';
    out() << i;
    if (!i.entry) {
      missing.push_back(i.name);
      if (const auto tags = count.tags(i.name); tags != nullptr) {
        std::string file;
        for (auto maxCount = 0; auto& j : *tags)
          if (j.second > maxCount) {
            maxCount = j.second;
            file = j.first;
          }
        out() << (showBreakdown ? " == " : ", ") << file;
      }
    }
    out() << '\n';
  }
  if (showBreakdown) DataFile::print(missing, "missing");
}

} // namespace kanji_tools
