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
  if (c.entry.has_value())
    os << std::setw(5) << (**c.entry).frequency() << ", "
       << ((**c.entry).hasLevel() ? toString((**c.entry).level()) : std::string("--")) << ", " << (**c.entry).type()
       << " (" << (**c.entry).number() << ')';
  else
    os << ", " << c.toHex();
  return os;
}

constexpr auto HelpMessage = "\
kanjiStats [-bhv] file [file ...]:\n\
  -b: show full kanji breakdown for 'file' (instead of just a summary)\n\
  -h: show help message for command-line options\n\
  -v: show 'before' and 'after' versions of lines that changed due to furigana removal\n";

} // namespace

std::string Stats::Count::toHex() const {
  auto s = fromUtf8(name);
  std::string result;
  if (s.length() == 1) result = "'\\u" + kanji_tools::toHex(s[0]) + "', ";
  for (auto i : name) {
    if (!result.empty()) result += ' ';
    result += "'\\x" + kanji_tools::toHex(i) + "'";
  }
  return result;
}

Stats::Stats(int argc, const char** argv, DataPtr data) : _data(data) {
  bool breakdown = false, endOptions = false, verbose = false;
  std::vector<std::string> files;
  for (int i = Data::nextArg(argc, argv); i < argc; i = Data::nextArg(argc, argv, i)) {
    std::string arg = argv[i];
    if (!endOptions && arg.starts_with("-")) {
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
      files.push_back(argv[i]);
  }
  if (!data->debug() && files.empty()) Data::usage("please specify at least one option or '-h' for help");
  for (auto& i : files)
    countKanji(i, breakdown, verbose);
}

int Stats::Count::frequency() const {
  return entry.has_value() ? (**entry).frequencyOrDefault(Data::maxFrequency()) : Data::maxFrequency() + 1;
}

KanjiTypes Stats::Count::type() const { return entry.has_value() ? (**entry).type() : KanjiTypes::None; }

template<typename Pred>
int Stats::processCount(const fs::path& top, const Pred& pred, const std::string& name, bool showBreakdown,
                        bool& firstCount, bool verbose) const {
  const bool isKanji = name.ends_with("Kanji");
  const bool isHiragana = name == "Hiragana";
  const bool isUnrecognized = name == "Unrecognized";
  const bool isCommonKanji = isKanji && name.starts_with("Common");
  if (isHiragana && verbose) log() << "Showing all furigana replacements:\n";
  // Remove furigana when processing Hiragana or MB-Letter to remove the effect on counts, i.e., furigana
  // in .txt files will artificially inflate Hiragana count (and MB-Letter because of the wide brackets)
  const bool removeFurigana = isHiragana || name == "Katakana" || name == "MB-Letter";
  MBCharCountIf count(pred, removeFurigana ? std::optional(MBCharCount::RemoveFurigana) : std::nullopt,
                      MBCharCount::DefaultReplace, isHiragana && verbose);
  count.addFile(top, isKanji || isUnrecognized || isHiragana && verbose);
  auto& m = count.map();
  std::set<Count> frequency;
  int total = 0, rank = 0;
  for (const auto& i : m) {
    total += i.second;
    frequency.emplace(i.second, i.first, isKanji ? _data->findKanji(i.first) : std::nullopt);
  }
  if (total) {
    if (firstCount) {
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
      firstCount = false;
    }
    printTotalAndUnique(name, total, frequency.size());
    if (isCommonKanji) {
      // only show percentage and type breakdown for 'Common Kanji'
      out() << ", 100.00%\n";
      printKanjiTypeCounts(frequency, total);
    } else if (isKanji) {
      // for 'Rare' and 'Non-UCD' Kanji show up to 'MaxExamples' entries in a single line since they
      // all should be type 'None'
      out() << std::setw(12) << '(';
      int i = 0;
      for (const auto& j : frequency) {
        if (i) out() << ", ";
        out() << j.name << ' ' << j.count;
        if (++i == MaxExamples) break;
      }
      out() << ")\n";
    } else
      out() << '\n';
    // print line-by-line breakdown
    if (isUnrecognized || isKanji && showBreakdown) {
      log() << "Showing Breakdown for '" << name << "':\n";
      out() << "  " << (showBreakdown ? "Rank  [Val #] Freq, LV, Type (No.) ==" : "[Val #], Missing Unicode,")
            << " Highest Count File\n";
      DataFile::List missing;
      for (const auto& i : frequency) {
        out() << "  ";
        if (showBreakdown) out() << std::left << std::setw(5) << ++rank << ' ';
        out() << i;
        if (!i.entry.has_value()) {
          missing.push_back(i.name);
          auto tags = count.tags(i.name);
          if (tags != nullptr) {
            int maxCount = 0;
            std::string file;
            for (const auto& j : *tags)
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
  }
  return total;
}

void Stats::printKanjiTypeCounts(const std::set<Count>& frequency, int total) const {
  std::map<KanjiTypes, int> totalKanjiPerType, uniqueKanjiPerType;
  std::map<KanjiTypes, std::vector<Count>> found;
  for (const auto& i : frequency) {
    auto t = i.type();
    totalKanjiPerType[t] += i.count;
    uniqueKanjiPerType[t]++;
    auto& j = found[t];
    if (j.size() < MaxExamples) j.push_back(i);
  }
  for (auto t : AllKanjiTypes) {
    auto i = uniqueKanjiPerType.find(t);
    if (i != uniqueKanjiPerType.end()) {
      int totalForType = totalKanjiPerType[t];
      printTotalAndUnique(std::string("[") + toString(t) + "] ", totalForType, i->second);
      out() << ", " << std::setw(6) << std::fixed << std::setprecision(2) << totalForType * 100. / total << "%  (";
      auto& j = found[t];
      for (int k = 0; k < j.size(); ++k) {
        if (k) out() << ", ";
        out() << j[k].name << ' ' << j[k].count;
      }
      out() << ")\n";
    }
  }
}

void Stats::countKanji(const fs::path& top, bool showBreakdown, bool verbose) const {
  static const int IncludeInTotals = 4; // only include Kanji and full-width kana in total and percents
  bool firstCount = true;
  auto f = [this, &top, showBreakdown, verbose, &firstCount](const auto& x, const auto& y) {
    return std::make_pair(this->processCount(top, x, y, showBreakdown, firstCount, verbose), y);
  };
  std::array totals{f([](const auto& x) { return isHiragana(x); }, "Hiragana"),
                    f([](const auto& x) { return isKatakana(x); }, "Katakana"),
                    f([](const auto& x) { return isCommonKanji(x); }, "Common Kanji"),
                    f([](const auto& x) { return isRareKanji(x); }, "Rare Kanji"),
                    f([this](const auto& x) { return isKanji(x) && !_data->ucd().find(x); }, "Non-UCD Kanji"),
                    f([](const auto& x) { return isMBPunctuation(x, false); }, "MB-Punctuation"),
                    f([](const auto& x) { return isMBSymbol(x); }, "MB-Symbol"),
                    f([](const auto& x) { return isMBLetter(x); }, "MB-Letter"),
                    f([](const auto& x) { return !isRecognizedCharacter(x); }, "Unrecognized")};
  int total = 0;
  for (int i = 0; i < IncludeInTotals; ++i)
    total += totals[i].first;
  log() << "Total Kanji+Kana: " << total;
  if (total) {
    out() << " (" << std::fixed << std::setprecision(1);
    for (int i = 0; i < IncludeInTotals; ++i)
      if (totals[i].first) {
        if (totals[i].second != totals[0].second) out() << ", ";
        out() << totals[i].second << ": " << totals[i].first * 100. / total << "%";
      }
    out() << ')';
  }
  out() << '\n';
}

void Stats::printTotalAndUnique(const std::string& name, int total, int unique) const {
  log() << std::right << std::setw(16) << name << ": " << std::setw(6) << total << ", unique: " << std::setw(4)
        << unique;
}

} // namespace kanji_tools