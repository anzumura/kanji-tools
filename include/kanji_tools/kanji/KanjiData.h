#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'KanjiData' is mainly a container class that holds data about various Kanji,
// Kana and multi-byte punctuation. Data is loaded from files in a 'data'
// directory that needs to have all the required files (such as jouyou.txt,
// jinmei.txt, etc. - see README file for more details).
class KanjiData : public Data {
public:
  KanjiData(u_int8_t argc, const char** argv, std::ostream& out = std::cout,
      std::ostream& err = std::cerr);

  // Implement the base class functions used during Kanji construction
  [[nodiscard]] Kanji::OptU16 frequency(const std::string& s) const override {
    const auto x{_frequency.get(s)};
    return x ? Kanji::OptU16{x} : std::nullopt;
  }
  [[nodiscard]] JlptLevels level(const std::string&) const override;
  [[nodiscard]] KenteiKyus kyu(const std::string&) const override;
private:
  // functions to print loaded data if _debug is true
  void noFreq(long f, bool brackets = false) const; // print no-freq count
  template<typename T>
  void printCount(
      const std::string& name, T pred, size_t printExamples = 0) const;
  void printStats() const;
  void printGrades() const;
  template<typename T, size_t S>
  void printListStats(const IterableEnumArray<T, S>&, T (Kanji::*)() const,
      const std::string&, bool showNoFrequency) const;

  // '_levels' (for JLPT) loaded from files under 'data/jlpt'
  const std::array<const LevelDataFile, AllJlptLevels.size() - 1> _levels;

  // '_kyus' (for Kanji Kentei) loaded from files under 'data/kentei'
  const std::array<const KyuDataFile, AllKenteiKyus.size() - 1> _kyus;

  // top 2501 frequency kanji loaded from 'data/frequency.txt'
  const DataFile _frequency;
};

} // namespace kanji_tools
