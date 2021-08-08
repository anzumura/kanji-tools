#ifndef KANJI_KANJI_DATA_H
#define KANJI_KANJI_DATA_H

#include <kanji/Data.h>

namespace kanji {

// 'KanjiData' is a mainly a container class that holds data about various Kanji, Kana and multi-byte
// punctuation. Data is loaded from files in a 'data' directory that needs to have all the required
// files (such as jouyou.txt, jinmei.txt, etc. - see README file for more details).
class KanjiData : public Data {
public:
  KanjiData(int argc, const char** argv, std::ostream& out = std::cout, std::ostream& err = std::cerr);

  // Implementations of the 'Data' base class functions used during Kanji construction
  int getFrequency(const std::string& s) const override { return _frequency.get(s); }
  Levels getLevel(const std::string&) const override;
  Kyus getKyu(const std::string&) const override;
private:
  // functions to print loaded data if _debug is true
  void noFreq(int f, bool brackets = false) const; // 'noFreq' is a helper function for printing no-frequency counts
  template<typename T> void printCount(const std::string& name, T pred, int printExamples = 0) const;
  void printStats() const;
  void printGrades() const;
  template<typename T, size_t S>
  void printListStats(const std::array<T, S>&, T (Kanji::*)() const, const std::string&, bool showNoFrequency) const;

  // '_levels' (for JLPT) are loaded from files under 'data/jlpt'
  const std::array<const LevelFileList, AllLevels.size() - 1> _levels;
  // '_kyus' (for Kanji Kentei) are loaded from files under 'data/kentei'
  const std::array<const KyuFileList, AllKyus.size() - 1> _kyus;
  // '_frequency' (for top 2501 frequency kanji) is loaded from data/frequency.txt
  const FileList _frequency;
};

} // namespace kanji

#endif // KANJI_KANJI_DATA_H
