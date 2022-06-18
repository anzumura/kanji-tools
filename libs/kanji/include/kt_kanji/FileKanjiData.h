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

  void populateJouyou();
  void populateJinmei();
  void populateExtra();

  void processList(const KanjiListFile&);

  // functions to print loaded data if _debug is true
  void printListData(
      const KanjiListFile&, const StringList&, TypeStringList&) const;
  void noFreq(std::ptrdiff_t f, bool brackets = false) const;
  template<auto Pred> void printCount(const String& name, size_t = 0) const;
  void printStats() const;
  void printGrades() const;
  template<typename T, Enum::Size S>
  void printListStats(const EnumListWithNone<T, S>&, T (Kanji::*)() const,
      const String&, bool showNoFrequency) const;

  LevelListFile dataFile(JlptLevels) const;
  KyuListFile dataFile(KenteiKyus) const;

  template<typename V, size_t N> using List = const std::array<const V, N - 1>;

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
