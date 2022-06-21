#pragma once

#include <kt_kanji/KanjiData.h>

namespace kanji_tools { /// \kanji_group{FileKanjiData}
/// FileKanjiData class for loading Kanji from '.txt' file

/// Implementation of KanjiData to load from text files \kanji{FileKanjiData}
///
/// The bulk of functionality for loading Kanji from '.txt' files is contained
/// in this class, whereas the base class has functionality to support adding,
/// validating, holding and looking up Kanji.
class FileKanjiData : public KanjiData {
public:
  explicit FileKanjiData(const Args& = {}, std::ostream& out = std::cout,
      std::ostream& err = std::cerr);

  [[nodiscard]] Kanji::Frequency frequency(const String& s) const final;
  [[nodiscard]] JlptLevels level(const String&) const final;
  [[nodiscard]] KenteiKyus kyu(const String&) const final;
protected:
  /// should be called immediately after populateJouyou() to load linked Kanji
  /// \details This function creates a LinkedJinmeiKanji for each line in `file`
  /// (line should start with a JouyouKanji). It will then create LinkedOldKanji
  /// for all JouyouKanji 'oldNames' that aren't already LinkedJinmeiKanji.
  void populateOfficialLinkedKanji(const Path& file);

  /// load any readings from `file` to use for FrequencyKanji instead of falling
  /// back to 'ucd.txt' readings, must be called before populateList()
  void loadFrequencyReadings(const Path& file);
private:
  using StringList = KanjiListFile::StringList;
  using TypeStringList = std::map<KanjiTypes, StringList>;

  /// calls NumberedKanji::fromFile() to load Jouyou Kanji
  void populateJouyou();

  /// calls NumbererKanji::fromFile() to load Jinmei and some LinkedJinmei Kanji
  void populateJinmei();

  /// calls NumberedKanji::fromFile() to load Extra Kanji
  void populateExtra();

  /// load/process Kanji from `list` (includes frequency, JLPT and Kentei Kyus)
  void processList(const KanjiListFile& list);

  /// print details about data loaded from `list` if debug is enabled
  void printListData(const KanjiListFile& list, const StringList& created,
      TypeStringList& found) const;

  /// print totals per Kanji type and if fullDebug() is true then also print
  /// various stats per type like 'Has JLPT Level', 'Has frequency', etc.
  void printCountsAndStats() const;

  /// print total per Kanji type matching a predicate function followed examples
  /// \tparam Pred predicate function taking a Kanji
  /// \param name stat name (like 'Has JLPT Level')
  /// \param printExamples number of examples to print, `0` means no examples
  template <auto Pred>
  void printCount(const String& name, size_t printExamples = 0) const;

  /// print breakdown per Kanji 'grade' including total and total JLPT level
  void printGrades() const;

  /// print details per Kanji type for a given enum list
  /// \tparam F Kanji member function, currently Kanji::level() and Kanji::kyu()
  /// \tparam T type of enum list
  /// \param list enum list, currently AllJlptLevels and AllKenteiKyus
  /// \param name list name
  /// \param showNoFreq if true, then counts without frequencies are included
  template <auto F, typename T>
  void printListStats(const T& list, const String& name, bool showNoFreq) const;

  /// helper function for printing 'no frequency' totals
  /// \param f no frequency count to print
  /// \param brackets if true then put round brackets around `f`
  void noFreq(std::ptrdiff_t f, bool brackets = false) const;

  LevelListFile dataFile(JlptLevels) const;
  KyuListFile dataFile(KenteiKyus) const;

  template <typename V, size_t N> using List = const std::array<const V, N - 1>;

  /// for (JLPT) levels loaded from files under 'data/jlpt'
  List<LevelListFile, AllJlptLevels.size()> _levels;

  /// for (Kanji Kentei) kyus loaded from files under 'data/kentei'
  List<KyuListFile, AllKenteiKyus.size()> _kyus;

  /// top 2501 frequency kanji loaded from 'data/frequency.txt'
  const KanjiListFile _frequency;

  /// holds readings loaded from 'frequency-readings.txt' for FrequencyKanji
  /// that aren't part of any other group (so not Jouyou or Jinmei).
  std::map<String, String> _frequencyReadings;
};

/// \end_group
} // namespace kanji_tools
