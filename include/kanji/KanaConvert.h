#ifndef KANJI_KANA_CONVERT_H
#define KANJI_KANA_CONVERT_H

#include <array>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

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

  // 'Kana' is a helper class for storing relationships between Romaji, Hiragana and Katakana
  class Kana {
  public:
    using List = std::vector<std::string>;
    Kana(const char* romaji, const char* hiragana, const char* katakana, const char* hepburn = nullptr,
         const char* kunrei = nullptr)
      : _romaji(romaji), _hiragana(hiragana), _katakana(katakana),
        _hepburn(hepburn ? std::optional(hepburn) : std::nullopt),
        _kunrei(kunrei ? std::optional(kunrei) : std::nullopt) {}
    // Kana with a set of unique extra variant romaji values (first variant is optionally a 'kunreiVariant')
    Kana(const char* romaji, const char* hiragana, const char* katakana, const List& variants,
         bool kunreiVariant = false)
      : _romaji(romaji), _hiragana(hiragana), _katakana(katakana), _variants(variants), _kunreiVariant(kunreiVariant) {
      assert(_kunreiVariant ? !_variants.empty() : true);
    }
    const std::string& getRomaji(int flags) const {
      return (flags & Hepburn) && _hepburn.has_value() ? *_hepburn
        : (flags & Kunrei) && _kunreiVariant           ? _variants[0]
        : (flags & Kunrei) && _kunrei.has_value()      ? *_kunrei
                                                       : _romaji;
    }
    // repeat the first letter of romaji for sokuon (促音) output (special handling for 't' as
    // described in comments above).
    std::string getSokuonRomaji(int flags) const {
      auto& r = getRomaji(flags);
      return (r[0] == 'c' ? 't' : r[0]) + r;
    }
    const std::string& get(CharType t, int flags) const {
      switch (t) {
      case CharType::Romaji: return getRomaji(flags);
      case CharType::Hiragana: return _hiragana;
      case CharType::Katakana: return _katakana;
      }
    }
    bool containsKana(const std::string& s) const { return s == _hiragana || s == _katakana; }
    bool operator==(const Kana& rhs) const {
      // comparing _romaji is good enough since uniqueness is enforced by the rest of the program
      return _romaji == rhs._romaji;
    }
    const std::string& romaji() const { return _romaji; }
    const std::string& hiragana() const { return _hiragana; }
    const std::string& katakana() const { return _katakana; }
    const List& variants() const { return _variants; }
  private:
    // '_romaji' usually holds the Modern Hepburn value, but will sometimes be a Nihon Shiki
    // value in order to ensure a unique value for Kana maps ('di' for ぢ, 'du' for づ, etc.)
    const std::string _romaji;
    const std::string _hiragana;
    const std::string _katakana;
    // '_variants' holds any further variant Romaji values that are unique for this 'Kana'
    // class. These include extra key combinations that also map to the same value such as
    // 'kwa' for クァ (instead of 'qa'), 'fyi' フィ (instead of 'fi'), etc.
    const List _variants;
    // '_hepburn' holds an optional 'Modern Hepburn' value for a few cases where it differs
    // from the 'unique' wāpuro romaji. For example, づ can be uniquely identified by 'du',
    // but the correct Hepburn output for this kana is 'zu' which is ambiguous with ず.
    // '_hepburn' (if it's populated) is always a duplicate of another Kana's '_romaji' value.
    const std::optional<std::string> _hepburn = std::nullopt;
    // '_kunrei' holds an optional 'Kunrei Shiki' value for a few cases like 'zya' for じゃ.
    const std::optional<std::string> _kunrei = std::nullopt;
    // '_kunreiVariant' is true if the first entry in '_variants' is a 'Kunrei Shiki' value. If
    // this is true then '_kunrei' should be nullopt.
    const bool _kunreiVariant = false;
  };
  // 'DakutenKana' is for 'k', 's', 't', 'h' row kana which have a dakuten, i.e., か has が
  class DakutenKana : public Kana {
  public:
    DakutenKana(const char* romaji, const char* hiragana, const char* katakana, const Kana& dakutenKana,
                const char* hepburn = nullptr, const char* kunrei = nullptr)
      : Kana(romaji, hiragana, katakana, hepburn, kunrei), _dakutenKana(dakutenKana) {}
    DakutenKana(const char* romaji, const char* hiragana, const char* katakana, const Kana& dakutenKana,
                const List& variants, bool kunreiVariant = false)
      : Kana(romaji, hiragana, katakana, variants, kunreiVariant), _dakutenKana(dakutenKana) {}
    const Kana& dakutenKana() const { return _dakutenKana; }
  private:
    const Kana _dakutenKana;
  };
  // 'HanDakutenKana' is only populated for 'h' row kana, i.e., は has ぱ
  class HanDakutenKana : public DakutenKana {
  public:
    HanDakutenKana(const char* romaji, const char* hiragana, const char* katakana, const Kana& dakutenKana,
                   const Kana& hanDakutenKana, const char* hepburn = nullptr, const char* kunrei = nullptr)
      : DakutenKana(romaji, hiragana, katakana, dakutenKana, hepburn, kunrei), _hanDakutenKana(hanDakutenKana) {}
    HanDakutenKana(const char* romaji, const char* hiragana, const char* katakana, const Kana& dakutenKana,
                   const Kana& hanDakutenKana, const List& variants, bool kunreiVariant = false)
      : DakutenKana(romaji, hiragana, katakana, dakutenKana, variants, kunreiVariant), _hanDakutenKana(hanDakutenKana) {
    }
    const Kana& hanDakutenKana() const { return _hanDakutenKana; }
  private:
    const Kana _hanDakutenKana;
  };

  using Map = std::map<std::string, const Kana*>;
  const Map& romajiMap() const { return _romajiMap; }
  const Map& hiraganaMap() const { return _hiraganaMap; }
  const Map& katakanaMap() const { return _katakanaMap; }
private:
  // 'RepeatMark' if for handling repeating kana marks (一の時点) when source is Hiragana or Katakana.
  class RepeatMark {
  public:
    RepeatMark(const char* hiragana, const char* katakana, bool dakuten = false)
      : _hiragana(hiragana), _katakana(katakana), _dakuten(dakuten) {
      assert(_hiragana != _katakana);
    }
    const std::string& get(CharType target) const { return target == CharType::Hiragana ? _hiragana : _katakana; }
    std::string getRomaji(const std::string& prevKana, int flags) const;
  private:
    const std::string _hiragana;
    const std::string _katakana;
    const bool _dakuten; // true if this instance if for the 'dakuten' (濁点) versions of the marks
  };

  static Map populate(CharType);
  // 'verifyData' is called by the constructor and performs various 'asserts' on member data.
  void verifyData() const;
  using Set = std::set<std::string>;
  std::string convertFromKana(const std::string& input, CharType target, int flags, const Map& sourceMap,
                              const Set& afterN, const Set& smallKana) const;
  std::string kanaLetters(const Map& sourceMap, const std::string& letterGroup, int count, CharType target, int flags,
                          bool prolonged = false) const;
  std::string convertFromRomaji(const std::string& input, CharType target, int flags) const;
  void romajiLetters(std::string& letterGroup, std::string& result, CharType target, int flags) const;

  const Map _romajiMap;
  const Map _hiraganaMap;
  const Map _katakanaMap;
  const Kana& _smallTsu;
  const Kana& _n;
  // '_prolongMark' (ー) is officially in the Katakana Unicode block, but it can also rarely appear
  // in some (non-standard) Hiragana words like らーめん.
  const std::string _prolongMark;
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
