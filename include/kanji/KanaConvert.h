#ifndef KANJI_KANA_CONVERT_H
#define KANJI_KANA_CONVERT_H

#include <array>
#include <map>
#include <set>
#include <string>

namespace kanji {

// 'CharType' is used to specify 'source' and 'target' types for 'KanaConvert::convert' methods
enum class CharType { Hiragana, Katakana, Romaji };
constexpr std::array CharTypes{CharType::Hiragana, CharType::Katakana, CharType::Romaji};
inline const std::string& toString(CharType t) {
  static std::string romaji("Romaji"), hiragana("Hiragana"), katakana("Katakana");
  switch (t) {
  case CharType::Hiragana: return hiragana;
  case CharType::Katakana: return katakana;
  case CharType::Romaji: return romaji;
  }
}

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
  KanaConvert();

  // 'ConversionFlags' can be used to control some aspects of conversion. For example:
  // RemoveSpaces: off by default, only applies when converting from Romaji:
  // - convert("akai kitsune", CharType::Hiragana) returns "あかい　きつね" (with a wide space)
  // - convert("akai kitsune", CharType::Hiragana, RemoveSpaces) returns "あかいきつね"
  // NoProlongedMark: off by default, only applies to 'hiragana' output
  // - convert("rāmen", CharType::Hiragana) returns "らーめん"
  // - convert("rāmen", CharType::Hiragana, NoProlongedMark) returns "らあめん"
  // Prolonged marks in hiragana are non-standard, but output them by default in order to support
  // round-trip type conversions, otherwise the above example would map "らあめん" back to "raamen"
  // which doesn't match the initial value.
  // Flags can be combined the usual way using '\', for example:
  // - convert("rāmen desu.", CharType::Hiragana, RemoveSpaces | NoProlongedMark) returns "らあめんです。"
  enum ConversionFlags { RemoveSpaces = 1, NoProlongMark = 2 };

  // The first overload of 'convert' returns a string based on 'input' with all 'non-target' kana
  // or romaji characters converted to 'target'. The second version only converts 'source' type
  // characters to 'target' (the original string is returned if 'source' is the same as 'target').
  // Note: a number of delimiters are also supported and get converted from narrow to wide and
  // vice versa (see KanaConvert.cpp 'Delimiters'). Also, when converting from Romaji, case
  // is ignored so both 'Dare' and 'dARe' would convert to 'だれ'. See 'ConversionFlags' for an
  // explanation of available flags that can be used.
  std::string convert(const std::string& input, CharType target, int flags = 0) const;
  std::string convert(const std::string& input, CharType source, CharType target, int flags = 0) const;

  // 'Kana' is a helper class for storing relationships between Romaji, Hiragana and Katakana
  struct Kana {
    Kana(const std::string& r, const std::string& h, const std::string& k)
      : romaji(r), hiragana(h), katakana(k), variant(false) {}
    const std::string& get(CharType t) const {
      switch (t) {
      case CharType::Romaji: return romaji;
      case CharType::Hiragana: return hiragana;
      case CharType::Katakana: return katakana;
      }
    }
    // repeat the first letter of romaji for sokuon (促音) output
    std::string getSokuonRomaji() const { return romaji[0] + romaji; }
    bool contains(const std::string& s) const { return s == romaji || s == hiragana || s == katakana; }
    bool operator==(const Kana& rhs) const {
      return romaji == rhs.romaji && hiragana == rhs.hiragana && katakana == rhs.katakana && variant == rhs.variant;
    }
    const std::string romaji;
    const std::string hiragana;
    const std::string katakana;
    const bool variant;
  protected:
    Kana(const std::string& r, const std::string& h, const std::string& k, bool v)
      : romaji(r), hiragana(h), katakana(k), variant(v) {}
  };
  struct VariantKana : Kana {
    VariantKana(const std::string& r, const std::string& h, const std::string& k) : Kana(r, h, k, true) {}
  };

  using Map = std::map<std::string, const Kana*>;
  const Map& romajiMap() const { return _romajiMap; }
  const Map& hiraganaMap() const { return _hiraganaMap; }
  const Map& katakanaMap() const { return _katakanaMap; }
private:
  static Map populate(CharType);
  // 'verifyData' is called by the constructor and performs various 'asserts' on member data.
  void verifyData() const;
  using Set = std::set<std::string>;
  std::string convertFromKana(const std::string& input, CharType target, const Map& sourceMap, const Set& markAfterN,
                              const Set& smallKana) const;
  std::string kanaLetters(const Map&, const std::string&, int count, CharType target, bool prolonged = false) const;
  std::string convertFromRomaji(const std::string& input, CharType target, int flags) const;
  void romajiLetters(std::string& letterGroup, std::string& result, CharType target) const;

  const Map _romajiMap;
  const Map _hiraganaMap;
  const Map _katakanaMap;
  const Kana& _smallTsu;
  const Kana& _n;
  // '_prolongMark' (ー) is officially in the Katakana Unicode block, but it can also rarely appear
  // in some (non-standard) Hiragana words like らーめん.
  const std::string _prolongMark;
  // Either '_apostrophe' or '_dash' should be used to separate 'n' in the middle of Romaji words
  // like gin'iro, kan'atsu, kan-i, etc. for input. For Rōmaji output, '_apostrophe' is used.
  const char _apostrophe = '\'';
  const char _dash = '-';
  // '_repeatingConsonents' is used for processing of small 'tsu' for sokuon output
  std::set<char> _repeatingConsonents;
  // '_mark' sets contain kana symbols that should be proceedeed with _apostrophe when
  // producing Romaji output if they follow 'n'.
  Set _markHiraganaAfterN;
  Set _markKatakanaAfterN;
  // '_small' sets contain small kana symbols that form the second parts of digraphs
  Set _smallHiragana;
  Set _smallKatakana;
  // Punctuation and word delimiter handling
  std::string _narrowDelims;
  std::map<char, std::string> _narrowToWideDelims;
  std::map<std::string, char> _wideToNarrowDelims;
};

} // namespace kanji

#endif // KANJI_KANA_CONVERT_H
