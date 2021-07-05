#ifndef KANJI_KANJI_COUNT
#define KANJI_KANJI_COUNT

#include <kanji/KanjiData.h>

namespace kanji {

// 'KanjiCount' will count all multi-byte characters in 'top' file and if 'top' is a directroy
// then all the regulars under top will be processed (recursively). The 'count' for each unique
// kanji (frequency) will be displayed (non-kanji are not included).
class KanjiCount : public KanjiData {
public:
  KanjiCount(int argc, const char** argv);
  // helper class for printing out kanji found in files
  class Count {
  public:
    Count(int f, const std::string& n, OptEntry e) : count(f), name(n), entry(e) {}
    // Sot to have largest 'count' first followed by lowest frequency number. Lower frequency
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
  void countKanji(const std::filesystem::path& top, bool showBreakdown = false) const;
  template<typename Pred> int processCount(const std::filesystem::path&, const Pred&, const std::string&, bool) const;
  // the following print functions are called after loading all data if -debug flag is specified
  void printStats() const;
  void printGrades() const;
  void printLevels() const;
  void printRadicals() const;
  template<typename T> void printCount(const std::string& name, T pred) const;
};

} // namespace kanji

#endif // KANJI_KANJI_COUNT
