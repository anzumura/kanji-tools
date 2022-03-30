#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'KanjiData' is mainly a container class that holds data about various Kanji,
// Kana and multi-byte punctuation. Data is loaded from files in a 'data'
// directory that needs to have all the required files (such as jouyou.txt,
// jinmei.txt, etc. - see README file for more details).
class KanjiData : public Data {
public:
  KanjiData(ArgCount argc = 0, const char** argv = {},
      std::ostream& out = std::cout, std::ostream& err = std::cerr);

  // Implement the base class functions used during Kanji construction
  [[nodiscard]] Kanji::OptFreq frequency(const std::string& s) const override {
    const auto x{_frequency.get(s)};
    return x ? Kanji::OptFreq{x} : std::nullopt;
  }
  [[nodiscard]] JlptLevels level(const std::string&) const override;
  [[nodiscard]] KenteiKyus kyu(const std::string&) const override;
private:
  inline static const Path Jlpt{"jlpt"}, Kentei{"kentei"};

  // functions to print loaded data if _debug is true
  void noFreq(long f, bool brackets = false) const; // print no-freq count
  template<typename T>
  void printCount(const std::string& name, T pred, size_t = 0) const;
  void printStats() const;
  void printGrades() const;
  template<typename T, size_t S>
  void printListStats(const IterableEnumArray<T, S>&, T (Kanji::*)() const,
      const std::string&, bool showNoFrequency) const;

  template<typename T> auto dataFile(T t) const {
    const auto f{[this, t](const auto& dir) {
      return dataDir() / dir / firstLower(kanji_tools::toString(t));
    }}; // LCOV_EXCL_LINE - for some reason Clang marks this line as 0 coverage
    if constexpr (std::is_same_v<T, JlptLevels>)
      return LevelDataFile{f(Jlpt), t};
    else {
      static_assert(std::is_same_v<T, KenteiKyus>);
      return KyuDataFile{f(Kentei), t};
    }
  }

  template<typename V, size_t N> using List = const std::array<const V, N - 1>;

  // '_levels' (for JLPT) loaded from files under 'data/jlpt'
  List<LevelDataFile, AllJlptLevels.size()> _levels;

  // '_kyus' (for Kanji Kentei) loaded from files under 'data/kentei'
  List<KyuDataFile, AllKenteiKyus.size()> _kyus;

  // top 2501 frequency kanji loaded from 'data/frequency.txt'
  const DataFile _frequency;
};

} // namespace kanji_tools