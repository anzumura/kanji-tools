#ifndef KANJI_KANA_CONVERT_H
#define KANJI_KANA_CONVERT_H

#include <map>
#include <string>

namespace kanji {

class KanaConvert {
public:
  enum class Target { Romaji, Hiragana, Katakana };
  static const std::string& toString(Target t) {
    static std::string romaji("Romaji"), hiragana("Hiragana"), katakana("Katakana");
    switch (t) {
    case Target::Romaji: return romaji;
    case Target::Hiragana: return hiragana;
    case Target::Katakana: return katakana;
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
  const Map& romajiMap() const { return _romajiMap; }
  const Map& hiraganaMap() const { return _hiraganaMap; }
  const Map& katakanaMap() const { return _katakanaMap; }
private:
  static Map populate(Target);
  const Map _romajiMap;
  const Map _hiraganaMap;
  const Map _katakanaMap;
};

} // namespace kanji

#endif // KANJI_KANA_CONVERT_H
