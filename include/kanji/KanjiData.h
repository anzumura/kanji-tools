#ifndef KANJI_KANJI_DATA
#define KANJI_KANJI_DATA

#include <kanji/Data.h>

namespace kanji {

// 'KanjiData' is a mainly a container class that holds data about various Kanji, Kana and multi-byte
// punctuation. Data is loaded from files in a 'data' directory that needs to have all the required
// files (such as jouyou.txt, jinmei.txt, etc. - see README file for more details).
class KanjiData : public Data {
public:
  KanjiData(int argc, const char** argv);
  // Implementations of the 'Data' base class functions used during Kanji construction
  int getFrequency(const std::string& s) const override { return _frequency.get(s); }
  Levels getLevel(const std::string&) const override;
  // functions for classifying a 'non-kanji' utf-8 encoded character
  bool isHiragana(const std::string& s) const { return _hiragana.exists(s); }
  bool isKatakana(const std::string& s) const { return _katakana.exists(s); }
  bool isFullWidthKana(const std::string& s) const { return isHiragana(s) || isKatakana(s); }
  bool isHalfWidthKana(const std::string& s) const { return _halfwidthKana.exists(s); }
  bool isKana(const std::string& s) const { return isFullWidthKana(s) || isHalfWidthKana(s); }
  // 'isPunctuation' tests for wide space directly here since this character is skipped by FileList constructor
  bool isWidePunctuation(const std::string& s) const { return s == "　" || _punctuation.exists(s); }
  bool isWideLetter(const std::string& s) const { return _wideLetters.exists(s); }
  // 'isWideNonKanji' returns true if the character is in any of the 'non-kanji files
  bool isWideNonKanji(const std::string& s) const { return isKana(s) || isWideLetter(s) || isWidePunctuation(s); }

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
  // 'countKanji' will count all multi-byte characters in 'top' file and if 'top' is a directroy
  // then all the regulars under top will be processed (recursively). The 'count' for each unique
  // kanji (frequency) will be displayed (non-kanji are not included).
  void countKanji(const std::filesystem::path& top) const;

  // 'n1-n5' and 'frequency' lists are loaded from simple files with one kanji per line
  const FileList _n5;
  const FileList _n4;
  const FileList _n3;
  const FileList _n2;
  const FileList _n1;
  const FileList _frequency;
  // lists to help determin the type of a wide character
  const FileList _hiragana;
  const FileList _katakana;
  const FileList _halfwidthKana;
  const FileList _wideLetters;
  const FileList _punctuation;
};

} // namespace kanji

#endif // KANJI_KANJI_DATA
