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
private:
  // functions to print loaded data if _debug is true
  void noFreq(int f, bool brackets = false) const; // 'noFreq' is a helper function for printing no-frequency counts
  template<typename T> void printCount(const std::string& name, T pred, int printExamples = 0) const;
  void printStats() const;
  void printGrades() const;
  void printLevels() const;
  void printRadicals() const;
  // 'n1-n5' and 'frequency' lists are loaded from simple files with one kanji per line
  const FileList _n5;
  const FileList _n4;
  const FileList _n3;
  const FileList _n2;
  const FileList _n1;
  const FileList _frequency;
};

} // namespace kanji

#endif // KANJI_KANJI_DATA_H
