#ifndef KANJI_KANA_CONVERT_H
#define KANJI_KANA_CONVERT_H

#include <map>
#include <string>

namespace kanji {

// 'KanaConvert' supports converting between Rōmaji (ローマジ), Hiragana (平仮名) and Katakana
// (片仮名). When Rōmaji is the output target, Revised Hepburn System (ヘボン式) is used, but
// for Rōmaji input many more letter combinations are supported such as:
// - Kunrei-shiki (訓令式) Rōmaji: si -> し, sya -> しゃ, syu -> しゅ, syo ->　しょ, ti -> ち, tu -> つ
//                                hu -> ふ, tya -> ちゃ, tyu -> ちゅ, tyo -> ちょ, ...
// - Nihon-shiki (日本式) Rōmaji: di -> ぢ,　du -> づ (plus Kunrei)
// - Wāpuro (ワープロ) Rōmaji combinations: ou -> おう, ...
// Letters with a macron (like ō, ā, ī) are supported for Rōmaji input, but when converting to
// Hiragana they are ambiguous, i.e., ō maps to either おお or おう so for simplicity the same
// vowel is always used (so おお).
class KanaConvert {
public:
  enum class CharType { Romaji, Hiragana, Katakana };
  static const std::string& toString(CharType t) {
    static std::string romaji("Romaji"), hiragana("Hiragana"), katakana("Katakana");
    switch (t) {
    case CharType::Romaji: return romaji;
    case CharType::Hiragana: return hiragana;
    case CharType::Katakana: return katakana;
    }
  }
  class Kana {
  public:
    Kana(const std::string& r, const std::string& h, const std::string& k, bool v = false)
      : romaji(r), hiragana(h), katakana(k), variant(v) {}
    const std::string& get(CharType t) const {
      switch (t) {
      case CharType::Romaji: return romaji;
      case CharType::Hiragana: return hiragana;
      case CharType::Katakana: return katakana;
      }
    }
    const std::string romaji;
    const std::string hiragana;
    const std::string katakana;
    const bool variant;
  };
  using Map = std::map<std::string, Kana>;
  KanaConvert();

  // The first overload of 'convert' returns a string based on 'input' with all 'non-target' kana
  // or romaji characters converted to 'target'. The second version only converts 'source' type
  // characters to 'target' (the original string is returned if 'source' is the same as 'target').
  // By default space characters are preserved, but this can be overridden by setting 'keepSpaces'
  // to 'false'. For example:
  // - convert("akai kitsune", CharType::Hiragana) returns "あかい　きつね" (with a wide space)
  // - convert("akai kitsune", CharType::Hiragana, false) returns "あかいきつね"
  // Note: a number of delimiters are also supported and get converted from narrow to wide and
  // vice versa (see KanaConvert.cpp 'Delimiters'). Also, when converting from Romaji, case
  // is ignored so both 'Dare' and 'dARe' would convert to 'だれ'. 'keepSpaces' only applies when
  // converting from Romaji.
  std::string convert(const std::string& input, CharType target, bool keepSpaces = true) const;
  std::string convert(const std::string& input, CharType source, CharType target, bool keepSpaces = true) const;

  const Map& romajiMap() const { return _romajiMap; }
  const Map& hiraganaMap() const { return _hiraganaMap; }
  const Map& katakanaMap() const { return _katakanaMap; }
private:
  static Map populate(CharType);
  std::string convertFromRomaji(const std::string& input, CharType target) const;
  void convertRomajiLetters(std::string& letterGroup, std::string& result, CharType target) const;

  const Map _romajiMap;
  const Map _hiraganaMap;
  const Map _katakanaMap;
  const Kana _n;
  // Either '_apostrophe' or '_dash' should be used to separate 'n' in the middle of Romaji words
  // like gin'iro, kan'atsu, kan-i, etc. for input. For Rōmaji output, '_apostrophe' is used.
  const char _apostrophe = '\'';
  const char _dash = '-';
  std::string _narrowDelims;
  std::map<char, std::string> _narrowToWideDelims;
  std::map<std::string, char> _wideToNarrowDelims;
};

} // namespace kanji

#endif // KANJI_KANA_CONVERT_H
