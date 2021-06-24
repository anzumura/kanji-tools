#ifndef KANJI_KANJI_DATA
#define KANJI_KANJI_DATA

#include <kanji/Data.h>

namespace kanji {

// 'KanjiData' is a mainly a container class that holds data about various Kanji, Kana and multi-byte
// punctuation. Data is loaded from files in a 'data' directory that needs to have all the required
// files (such as jouyou.txt, jinmei.txt, etc. - see README file for more details).
class KanjiData : public Data {
public:
  KanjiData(int argc, char** argv);
  // Implementations of the 'Data' base class functions used during Kanji construction
  int getFrequency(const std::string& s) const override { return _frequency.get(s); }
  Levels getLevel(const std::string&) const override;
  // functions for classifying a utf-8 encoded character
  bool isHiragana(const std::string& s) const { return _hiragana.exists(s); }
  bool isKatakana(const std::string& s) const { return _katakana.exists(s); }
  bool isFullWidthKana(const std::string& s) const { return isHiragana(s) || isKatakana(s); }
  bool isHalfwidthKana(const std::string& s) const { return _halfwidth.exists(s); }
  bool isKana(const std::string& s) const { return isFullWidthKana(s) || isHalfwidthKana(s); }
  bool isWidePunctuation(const std::string& s) const { return _punctuation.exists(s); }
  bool isKanaOrPunctuation(const std::string& s) const { return isKana(s) || isWidePunctuation(s); }
private:
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
  const FileList _halfwidth;
  const FileList _punctuation;
};

} // namespace kanji

#endif // KANJI_KANJI_DATA
