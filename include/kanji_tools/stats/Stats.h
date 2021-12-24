#ifndef KANJI_TOOLS_STATS_STATS_H
#define KANJI_TOOLS_STATS_STATS_H

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'Stats' counts all multi-byte characters in 'top' file and if 'top' is a directroy than all
// the regulars under top will be processed (recursively). The 'count' for each unique kanji
// (frequency) will be displayed (non-kanji are not included).
class Stats {
public:
  using OptEntry = Data::OptEntry;

  // Command line options must specify one or more files and 'data' class is used to lookup kanji
  // found in files - see HelpMessage in Stats.cpp for more details on command line options.
  Stats(int argc, const char** argv, DataPtr data);

  Stats(const Stats&) = delete;

  // helper class for ordering and printing out kanji found in files
  class Count {
  public:
    Count(int f, const std::string& n, OptEntry e) : count(f), name(n), entry(e) {}

    // Sort to have largest 'count' first followed by lowest frequency number. Lower frequency
    // means the kanji is more common, but a frequency of '0' means the kanji isn't in the top
    // frequency list so use 'frequencyOrDefault' to return a large number for no-frequency
    // kanji and consider 'not-found' kanji to have even higher (worse) frequency. If kanjis
    // both have the same 'count' and 'frequency' then sort by type then hex (use 'hex' instead of
    // 'name' since sorting by UTF-8 is less consistent).
    bool operator<(const Count& x) const {
      return count > x.count ||
        (count == x.count && frequency() < x.frequency() ||
         (frequency() == x.frequency() && type() < x.type() || (type() == x.type() && toHex() < x.toHex())));
    }

    int frequency() const {
      return entry ? (**entry).frequencyOrDefault(Data::maxFrequency()) : Data::maxFrequency() + 1;
    }
    KanjiTypes type() const { return entry ? (**entry).type() : KanjiTypes::None; }
    std::string toHex() const;

    int count;
    std::string name;
    OptEntry entry;
  };
private:
  // 'IncludeInTotals' of 4 indicates only Kanji and full-width kana should be included in totals and percents
  // 'MaxExamples' is the maximum number of examples to show for each kanji type when printing stats
  enum Values { IncludeInTotals = 4, MaxExamples = 5, TotalCountWidth = 6, TypeNameWidth = 16 };

  std::ostream& log(bool heading = false) const { return _data->log(heading); }
  std::ostream& out() const { return _data->out(); }

  void countKanji(const std::filesystem::path& top, bool showBreakdown, bool verbose) const;

  template<typename Pred>
  int processCount(const std::filesystem::path&, const Pred&, const std::string&, bool, bool, bool) const;

  using CountSet = std::set<Count>;

  void printHeaderInfo(const std::filesystem::path&, const class MBCharCount&) const;
  void printTotalAndUnique(const std::string& name, int total, int unique) const;
  void printKanjiTypeCounts(const std::set<Count>&, int total) const;
  void printExamples(const CountSet&) const;
  void printBreakdown(const std::string& name, bool showBreakdown, const CountSet&, const MBCharCount&) const;

  const DataPtr _data;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_STATS_STATS_H
