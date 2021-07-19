#ifndef KANJI_KANA_CONVERT_H
#define KANJI_KANA_CONVERT_H

#include <map>
#include <string>

namespace kanji {

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
  std::string convert(const std::string& input, CharType target) const;
  std::string convert(const std::string& input, CharType source, CharType target) const;
  const Map& romajiMap() const { return _romajiMap; }
  const Map& hiraganaMap() const { return _hiraganaMap; }
  const Map& katakanaMap() const { return _katakanaMap; }
private:
  static Map populate(CharType);
  const Map _romajiMap;
  const Map _hiraganaMap;
  const Map _katakanaMap;
};

} // namespace kanji

#endif // KANJI_KANA_CONVERT_H
