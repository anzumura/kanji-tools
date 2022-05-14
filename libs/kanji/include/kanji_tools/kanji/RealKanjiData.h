#pragma once

#include <kanji_tools/kanji/KanjiData.h>

namespace kanji_tools {

// 'RealKanjiData' is a concrete implementation of 'KanjiData' with methods to
// load and (debug) print data loaded from .txt files in the 'data' directory.
class RealKanjiData : public KanjiData {
public:
  explicit RealKanjiData(const Args& = {}, std::ostream& out = std::cout,
      std::ostream& err = std::cerr);

  // Implement the base class functions used during Kanji construction
  [[nodiscard]] Kanji::Frequency frequency(const String& s) const override;
  [[nodiscard]] JlptLevels level(const String&) const override;
  [[nodiscard]] KenteiKyus kyu(const String&) const override;
private:
  // functions to print loaded data if _debug is true
  void noFreq(std::ptrdiff_t f, bool brackets = false) const;
  template<typename T>
  void printCount(const String& name, T pred, size_t = 0) const;
  void printStats() const;
  void printGrades() const;
  template<typename T, Enum::Size S>
  void printListStats(const EnumListWithNone<T, S>&, T (Kanji::*)() const,
      const String&, bool showNoFrequency) const;

  LevelListFile dataFile(JlptLevels) const;
  KyuListFile dataFile(KenteiKyus) const;

  template<typename V, size_t N> using List = const std::array<const V, N - 1>;

  // '_levels' (for JLPT) loaded from files under 'data/jlpt'
  List<LevelListFile, AllJlptLevels.size()> _levels;

  // '_kyus' (for Kanji Kentei) loaded from files under 'data/kentei'
  List<KyuListFile, AllKenteiKyus.size()> _kyus;

  // top 2501 frequency kanji loaded from 'data/frequency.txt'
  const KanjiListFile _frequency;
};

} // namespace kanji_tools
