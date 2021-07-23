#include <kanji/FileStats.h>
#include <kanji/Kanji.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

#include <numeric>

namespace kanji {

namespace fs = std::filesystem;

namespace {

std::ostream& operator<<(std::ostream& os, const FileStats::Count& c) {
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

std::string FileStats::Count::toHex() const {
  auto s = fromUtf8(name);
  std::string result;
  if (s.length() == 1) result = "'\\u" + kanji::toHex(s[0]) + "', ";
  for (auto i : name) {
    if (!result.empty()) result += ' ';
    result += "'\\x" + kanji::toHex(i) + "'";
  }
  return result;
}

FileStats::FileStats(int argc, const char** argv, DataPtr data) : _data(data) {
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
        Data::usage("Unrecognized argument '" + arg + "' use -h for help");
    } else
      files.push_back(argv[i]);
  }
  if (files.empty()) Data::usage("please specify at least one option or '-h' for help");
  for (auto& i : files)
    countKanji(i, breakdown, verbose);
}

int FileStats::Count::frequency() const {
  return entry.has_value() ? (**entry).frequencyOrDefault(Data::maxFrequency()) : Data::maxFrequency() + 1;
}

Types FileStats::Count::type() const { return entry.has_value() ? (**entry).type() : Types::None; }

template<typename Pred>
int FileStats::processCount(const fs::path& top, const Pred& pred, const std::string& name, bool showBreakdown,
                            bool& firstCount, bool verbose) const {
  const bool isKanji = name.ends_with("Kanji");
  const bool isHiragana = name == "Hiragana";
  const bool isUnrecognized = name == "Unrecognized";
  if (isHiragana && verbose)
    log() << "Showing all furigana replacements:\n";
  // Remove furigana when processing Hiragana or MB-Letter to remove the effect on counts, i.e., furigana
  // in .txt files will artificially inflate Hiragana count (and MB-Letter because of the wide brackets)
  const bool removeFurigana = isHiragana || name == "Katakana" || name == "MB-Letter";
  MBCharCountIf count(pred, removeFurigana ? std::optional(MBCharCount::RemoveFurigana) : std::nullopt,
                      MBCharCount::DefaultReplace, isHiragana && verbose);
  count.addFile(top, isKanji || isUnrecognized);
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
      if (count.errors()) out() << ", found " << count.errors() << " errors!";
      out() << '\n';
      firstCount = false;
    }
    printTotalAndUnique(name, total, frequency.size());
    if (isKanji) {
      out() << ", 100.00%\n";
      printKanjiTypeCounts(frequency, total);
    } else
      out() << '\n';
    // print line-by-line breakdown
    if (isUnrecognized || isKanji && showBreakdown) {
      log() << "Showing Breakdown for '" << name << "':\n";
      out() << "  " << (showBreakdown ? "Rank  [Val #] Freq, LV, Type (No.) ==" : "[Val #], Missing Unicode,")
            << " Highest Count File\n";
      FileList::List missing;
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
      if (showBreakdown) FileList::print(missing, "missing");
    }
  }
  return total;
}

void FileStats::printKanjiTypeCounts(const std::set<Count>& frequency, int total) const {
  std::map<Types, int> totalKanjiPerType, uniqueKanjiPerType;
  std::map<Types, std::vector<Count>> found;
  for (const auto& i : frequency) {
    auto t = i.type();
    totalKanjiPerType[t] += i.count;
    uniqueKanjiPerType[t]++;
    auto& j = found[t];
    if (j.size() < MaxExamples) j.push_back(i);
  }
  for (auto t : AllTypes) {
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

void FileStats::countKanji(const fs::path& top, bool showBreakdown, bool verbose) const {
  static const int IncludeInTotals = 4; // only include Kanji and full-width kana in total and percents
  bool firstCount = true;
  auto f = [this, &top, showBreakdown, verbose, &firstCount](const auto& x, const auto& y) {
    return std::make_pair(this->processCount(top, x, y, showBreakdown, firstCount, verbose), y);
  };
  std::array totals{f([](const auto& x) { return isCommonKanji(x); }, "Common Kanji"),
                    f([](const auto& x) { return isRareKanji(x); }, "Rare Kanji"),
                    f([](const auto& x) { return isHiragana(x); }, "Hiragana"),
                    f([](const auto& x) { return isKatakana(x); }, "Katakana"),
                    f([](const auto& x) { return isMBPunctuation(x, false); }, "MB-Punctuation"),
                    f([](const auto& x) { return isMBSymbol(x); }, "MB-Symbol"),
                    f([](const auto& x) { return isMBLetter(x); }, "MB-Letter"),
                    f([](const auto& x) { return !isRecognizedMB(x); }, "Unrecognized")};
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

void FileStats::printTotalAndUnique(const std::string& name, int total, int unique) const {
  log() << std::right << std::setw(16) << name << ": " << std::setw(6) << total << ", unique: " << std::setw(4)
        << unique;
}

} // namespace kanji
