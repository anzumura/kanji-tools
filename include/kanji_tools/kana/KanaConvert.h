#ifndef KANJI_TOOLS_KANA_KANA_CONVERT_H
#define KANJI_TOOLS_KANA_KANA_CONVERT_H

#include <kanji_tools/kana/Kana.h>

#include <set>

namespace kanji_tools {

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

  // 'KanaConvert' constructor defaults the 'target' for conversion to Hiragana and sets 'flags'
  // to 0 (which means no special conversion flags). Calling the below 'convert' functions can
  // also override these values.
  KanaConvert(CharType target = CharType::Hiragana, int flags = 0);

  CharType target() const { return _target; }
  void target(CharType target) { _target = target; }
  int flags() const { return _flags; }
  std::string flagString() const; // return a | separated string representation of current flags or 'none'
  void flags(int flags) { _flags = flags; }

  // Support converting most non-letter ascii from narrow to wide values. These values are also used
  // as delimiters for splitting up input strings when converting from Rōmaji to Kana. Use a '*' for
  // katakana middle dot '・' to keep round-trip translations as non-lossy as possible. For now, don't
  // include '-' (minus) or apostrophe since these could get mixed up with prolong mark 'ー' and special
  // separation handling after 'n' in Romaji output. Backslash maps to ￥ as per usual keyboard input.
  using NarrowDelims = std::map<char, std::string>;
  using WideDelims = std::map<std::string, char>;

  const NarrowDelims& narrowDelims() const { return _narrowDelims; }
  const WideDelims& wideDelims() const { return _wideDelims; }

  // 'convert' has 4 overloads. The first and second use the current values of '_target' and '_flags'.
  // The first and third convert characters of any source type whereas the second and fourth restrict the
  // source type to be converted. If 'source' = 'target' then the original string is returned.
  //
  // Note: a number of delimiters are also supported and get converted from narrow to wide and vice
  // versa (see KanaConvert.cpp 'Delimiters'). Also, when converting from Romaji, case is ignored so
  // both 'Dare' and 'dARe' would convert to 'だれ'. See 'ConversionFlags' for an explanation of
  // available flags that can be used. The second and third overloads update '_target' and '_flags'
  std::string convert(const std::string& input) const;
  std::string convert(CharType source, const std::string& input) const;
  std::string convert(const std::string& input, CharType target, int flags = 0);
  std::string convert(CharType source, const std::string& input, CharType target, int flags = 0);
private:
  // 'verifyData' is called by the constructor and performs various 'asserts' on member data.
  void verifyData() const;

  bool romajiTarget() const { return _target == CharType::Romaji; }
  bool hiraganaTarget() const { return _target == CharType::Hiragana; }
  const std::string& get(const Kana& k) const { return k.get(_target, _flags); }
  const std::string& getN() const { return get(Kana::N); } 
  const std::string& getSmallTsu() const { return get(Kana::SmallTsu); } 

  using Set = std::set<std::string>;

  // 'insertUnique' performs and insert and ensures the value was added by using 'assert' - this can't be
  // done in a single line like 'assert(s.insert(x).second)' since that would result in the code not getting
  // executed when compiling in with asserts disabled, i.e., a 'Release' build.
  static void insertUnique(Set& s, const std::string& x) {
    auto i = s.insert(x);
    assert(i.second);
  }

  std::string convertFromKana(const std::string& input, CharType source, const Set& afterN, const Set& smallKana) const;
  std::string kanaLetters(const std::string& letterGroup, CharType source, int count, const Kana*& prevKana,
                          bool prolong = false) const;
  std::string convertFromRomaji(const std::string& input) const;
  void romajiLetters(std::string& letterGroup, std::string& result) const;
  bool romajiMacronLetter(const std::string& letter, std::string& letterGroup, std::string& result) const;

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
  std::string _narrowDelimList;
  NarrowDelims _narrowDelims;
  WideDelims _wideDelims;

  // Members for the current conversion
  CharType _target;
  int _flags;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANA_KANA_CONVERT_H
