#ifndef KANJI_FILE_STATS
#define KANJI_FILE_STATS

#include <kanji/Data.h>

namespace kanji {

// 'FileStats' will count all multi-byte characters in 'top' file and if 'top' is a directroy
// then all the regulars under top will be processed (recursively). The 'count' for each unique
// kanji (frequency) will be displayed (non-kanji are not included).
class FileStats {
public:
  using OptEntry = Data::OptEntry;
  // Command line options must specify one or more files and 'data' class is used to lookup kanji
  // found in files - see HelpMessage in FileStats.cpp for more details on command line options.
  FileStats(int argc, const char** argv, DataPtr data);
  FileStats(const FileStats&) = delete;
  // helper class for ordering and printing out kanji found in files
  class Count {
  public:
    Count(int f, const std::string& n, OptEntry e) : count(f), name(n), entry(e) {}
    // Sort to have largest 'count' first followed by lowest frequency number. Lower frequency
    // means the kanji is more common, but a frequency of '0' means the kanji isn't in the top
    // frequency list so use 'frequencyOrDefault' to return a large number for no-frequency
    // kanji and consider 'not-found' kanji to have even higher (worse) frequency. If kanjis
    // both have the same 'count' and 'frequency' then sort by name.
    bool operator<(const Count& x) const {
      return count > x.count ||
        (count == x.count && getFrequency() < x.getFrequency() || getFrequency() == x.getFrequency() && name < x.name);
    }
    int getFrequency() const;
    int count;
    std::string name;
    OptEntry entry;
  };
private:
  std::ostream& log(bool heading = false) const { return _data->log(heading); }
  std::ostream& out() const { return _data->out(); }
  void countKanji(const std::filesystem::path& top, bool showBreakdown = false) const;
  template<typename Pred> int processCount(const std::filesystem::path&, const Pred&, const std::string&, bool) const;

  const DataPtr _data;
};

} // namespace kanji

#endif // KANJI_FILE_STATS