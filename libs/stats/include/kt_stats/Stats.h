#pragma once

#include <kt_kanji/KanjiData.h>

#include <sstream>

namespace kanji_tools { /// \stats_group{Stats}
/// Stats class used by 'kanjiStats' `main` program

/// prints stats about multi-byte characters for one or more files \stats{Stats}
///
/// Output consists of one line for each kind of character being counted as well
/// as further breakdowns for Kanji by type (including showing the most frequent
/// examples). Totals and percentages are also calculated. See README.md or test
/// code for sample output.
class Stats final {
public:
  /// print stats for files provided in `args`
  /// \param args command-line args (see Stats.cpp HelpMessage for more details)
  /// \param data used for validating Kanji and printing
  /// \throw DomainError for invalid `args`
  Stats(const Args& args, const KanjiDataPtr& data);

  Stats(const Stats&) = delete; ///< deleted copy ctor

  /// class for ordering and printing out Kanji found in files \stats{Stats}
  class Count final {
  public:
    /// create a Count object
    /// \param count number of occurrences of `entry`
    /// \param name UTF-8 String name of `entry`
    /// \param entry can be nullptr if no Kanji object was found for `name` in
    ///     data loaded by this program (shouldn't happen for any normal text)
    Count(size_t count, const String& name, const KanjiPtr& entry);

    /// return frequency of entry() or KanjiData::maxFrequency() if entry() has
    /// no frequency or KanjiData::maxFrequency() + 1 if entry() is nullptr
    /// \details higher numbers for 'no frequency' and 'not found' help sorting
    [[nodiscard]] Kanji::Frequency frequency() const;

    /// return entry type or 'None' if entry is nullptr
    [[nodiscard]] KanjiTypes type() const;

    /// put higher counts first then order by ascending frequency() if counts
    /// are the same (then sort by type() and name() to be deterministic)
    [[nodiscard]] bool operator<(const Count&) const;

    [[nodiscard]] auto count() const { return _count; }
    [[nodiscard]] auto& name() const { return _name; }
    [[nodiscard]] auto& entry() const { return _entry; }
  private:
    size_t _count;
    String _name;
    KanjiPtr _entry;
  };
private:
  /// class for gathering stats matching a predicate function \stats{Stats}
  class Pred final {
  public:
    /// one Pred object is created for each different stat being gathered like
    /// Hiragana, Katakana, Rare Kanji, etc..
    /// \param data used to lookup Kanji
    /// \param top file or directory to process (recursively)
    /// \param name 'stat' name (like 'Hiragana')
    /// \param showBreakdown true if detailed breakdown should also be included
    /// \details detailed breakdown prints one line per unique character, sorted
    /// by frequency and including info about the character (like JLPT and type)
    Pred(const KanjiDataPtr& data, const KanjiData::Path& top,
        const String& name, bool showBreakdown);

    template<typename T>
    [[nodiscard]] String run(const T&, bool verbose, bool firstCount);

    [[nodiscard]] auto& name() const { return _name; }
    [[nodiscard]] auto total() const { return _total; }
    [[nodiscard]] auto isKanji() const { return _isKanji; }
  private:
    /// for ostream 'set' functions
    enum IntDisplayValues {
      UniqueCountWidth = 4,
      TotalCountWidth = 6,
      TypeNameWidth = 16
    };
    enum PercentDisplayValues { PercentPrecision = 2, PercentWidth = 6 };

    using CountSet = std::set<Count>;

    void printHeaderInfo(const class Utf8Count&);
    void printTotalAndUnique(const String& name, size_t total, size_t unique);
    void printKanjiTypeCounts(const std::set<Count>&);
    void printRareExamples(const CountSet&);
    void printBreakdown(const CountSet&, const Utf8Count&);

    const KanjiDataPtr _data;
    const KanjiData::Path& _top;
    const String _name;
    const bool _showBreakdown;
    const bool _isKanji;
    size_t _total{};
    std::stringstream _os;
  };

  /// number of 'statistics' rows to include in totals and percents, i.e.,
  /// Hiragana, Katakana and the 3 Kanji rows (Common, Rare and Non-UCD)
  static constexpr size_t IncludeInTotals{5};

  /// maximum number of kanji to show per type in regular 'stats' output
  static constexpr size_t MaxExamples{5};

  [[nodiscard]] std::ostream& log(bool heading = false) const;
  [[nodiscard]] std::ostream& out() const;

  /// called by ctor for each file (or directory) in command-line args
  /// \param top regular file or directory to process
  /// \param showBreakdown true for detailed breakdown, see Pred()
  /// \param verbose true if before/after versions of lines should be printed
  ///     whenever Furigana is removed
  void countKanji(
      const KanjiData::Path& top, bool showBreakdown, bool verbose) const;

  const KanjiDataPtr _data;
};

/// stream operator for Stats::Count
std::ostream& operator<<(std::ostream&, const Stats::Count&);

/// \end_group
} // namespace kanji_tools
