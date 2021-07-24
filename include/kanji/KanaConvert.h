#ifndef KANJI_KANA_CONVERT_H
#define KANJI_KANA_CONVERT_H

#include <map>
#include <set>
#include <string>

namespace kanji {

enum class CharType;

// 'KanaConvert' supports converting between Rōmaji (ローマジ), Hiragana (平仮名) and Katakana
// (片仮名). When Rōmaji is the output target, Revised Hepburn System (ヘボン式) is used, but
// for Rōmaji input many more letter combinations are supported such as:
// - Kunrei-shiki (訓令式) Rōmaji: si -> し, sya -> しゃ, syu -> しゅ, syo ->　しょ, ti -> ち, tu -> つ
//                                hu -> ふ, tya -> ちゃ, tyu -> ちゅ, tyo -> ちょ, ...
// - Nihon-shiki (日本式) Rōmaji: di -> ぢ,　du -> づ (plus Kunrei)
// - Wāpuro (ワープロ) Rōmaji combinations: ou -> おう, ...
// Letters with a macron (like ō, ā, ī) are supported for Rōmaji input, but when converting to
// Hiragana they are ambiguous, i.e., ō maps to either おお or おう so for simplicity the prolong
// mark (ー) is used (this can be overridded by a flat to produce the double vowel like おお).
// Note, when typing kana 'macchi' and 'kocchi' produce "マッチ" and "こっち" respectively, but
// this is not standard Hepburn. Instead the standard is 'matchi' and 'kotchi', but either way
// is accepted as input to the 'convert' function (when converting from kana to romaji the
// standard form is used as output).
class KanaConvert {
public:
  KanaConvert();

  // 'ConversionFlags' can be used to control some aspects of conversion. For example:
  // Hepburn: off by default, only applies to 'romaji' output
  // - convert("つづき", CharType::Romaji) -> "tsuduki"
  // - convert("つづき", CharType::Romaji, Hepburn) -> "tsuzuki"
  // Kunrei: off by default, only applies to 'romaji' output
  // - convert("しつ", CharType::Romaji) -> "shitsu"
  // - convert("しつ", CharType::Romaji, Kunrei) -> "situ"
  // NoProlongMark: off by default, only applies to 'hiragana' output
  // - convert("rāmen", CharType::Hiragana) -> "らーめん"
  // - convert("rāmen", CharType::Hiragana, NoProlongMark) -> "らあめん"
  // RemoveSpaces: off by default, only applies when converting from Romaji:
  // - convert("akai kitsune", CharType::Hiragana) returns "あかい　きつね" (with a wide space)
  // - convert("akai kitsune", CharType::Hiragana, RemoveSpaces) returns "あかいきつね"
  //
  // Notes:
  //
  // Prolonged sound marks in hiragana are non-standard, but output them by default in order to
  // support round-trip type conversions, otherwise the above example would map "らあめん" back
  // to "raamen" which doesn't match the initial value.
  // Flags can be combined the usual way using '\', for example:
  // - convert("rāmen desu.", CharType::Hiragana, RemoveSpaces | NoProlongMark) -> "らあめんです。"
  //
  // Enabling 'Hepburn' leads to more standard romaji, but the output is ambiguous and leads to
  // different kana if converted back. This affects di (ぢ), dya (ぢゃ), dyo (ぢょ), dyu (ぢゅ),
  // du (づ) and wo (を) - these become ji, ja, ju, jo, zu and o instead. There's also no support
  // for trying to handle は and へ (which in standard Hepburn should map to 'wa' and 'e' if they
  // are used as particles) - instead they simply map to 'ha' and 'he' all the time. If both
  // Hepburn and Kunrei flags are set then Hepburn is preferred, but will then try Kunrei before
  // falling back to the unique '_romaji' value in the Kana class.
  enum ConversionFlags { Hepburn = 1, Kunrei = 2, NoProlongMark = 4, RemoveSpaces = 8 };

  // The first overload of 'convert' returns a string based on 'input' with all 'non-target'
  // kana or romaji characters converted to 'target'. The second version only converts 'source'
  // type characters to 'target' (the original string is returned if 'source' is the same as
  // 'target'). Note: a number of delimiters are also supported and get converted from narrow tp
  // wide and vice versa (see KanaConvert.cpp 'Delimiters'). Also, when converting from Romaji,
  // case is ignored so both 'Dare' and 'dARe' would convert to 'だれ'. See 'ConversionFlags' for
  // an explanation of available flags that can be used.
  std::string convert(const std::string& input, CharType target, int flags = 0) const;
  std::string convert(const std::string& input, CharType source, CharType target, int flags = 0) const;
private:
  // 'verifyData' is called by the constructor and performs various 'asserts' on member data.
  void verifyData() const;
  using Set = std::set<std::string>;
  std::string convertFromKana(const std::string& input, CharType source, CharType target, int flags, const Set& afterN,
                              const Set& smallKana) const;
  std::string kanaLetters(const std::string& letterGroup, CharType source, CharType target, int flags, int count,
                          const Kana*& prevKana, bool prolong = false) const;
  std::string convertFromRomaji(const std::string& input, CharType target, int flags) const;
  void romajiLetters(std::string& letterGroup, std::string& result, CharType target, int flags) const;
  // Either '_apostrophe' or '_dash' can be used to separate 'n' in the middle of Romaji words
  // like gin'iro, kan'atsu, kan-i, etc. for input. For Rōmaji output, '_apostrophe' is used. Note,
  // dash is used in 'Traditional Hepburn' whereas apostrophe is used in 'Modern (revised) Hepburn'.
  const char _apostrophe = '\'';
  const char _dash = '-';
  // '_repeatingConsonents' is used for processing small 'tsu' for sokuon output
  std::set<char> _repeatingConsonents;
  // '_markAfterN...' sets contain the 8 kana symbols (5 vowels and 3 y's) that should be proceedeed
  // with _apostrophe when producing Romaji output if they follow 'n'.
  Set _markAfterNHiragana;
  Set _markAfterNKatakana;
  // '_digraphSecond...' sets contain the 9 small kana symbols (5 vowels, 3 y's, and 'wa') that form
  // the second parts of digraphs.
  Set _digraphSecondHiragana;
  Set _digraphSecondKatakana;
  // Punctuation and word delimiter handling
  std::string _narrowDelims;
  std::map<char, std::string> _narrowToWideDelims;
  std::map<std::string, char> _wideToNarrowDelims;
};

} // namespace kanji

#endif // KANJI_KANA_CONVERT_H
