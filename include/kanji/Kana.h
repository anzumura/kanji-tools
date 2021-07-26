#ifndef KANJI_KANA_H
#define KANJI_KANA_H

#include <array>
#include <map>
#include <optional>
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

// 'Kana' is used to represent a Kana 'Monograph' or 'Digraph'. It stores Romaji, Hiragana and Katakana
// as well variant Romaji forms. A 'Monograph' is a single Kana character (large or small) and a 'Digraph'
// is a valid (at least typable using standard IME) two Kana combination. A 'Diagraph' always has a normal
// sized first Kana followed by a small Kana (one of the 5 vowels, 3 y's or 'wa'). This class also holds
// relationships between unaccented (plain) and accented (dakuten and han-dakuten) versions.
class Kana {
public:
  // 'RepeatMark' is for handling repeating kana marks (一の時点) when source is Hiragana or Katakana.
  class RepeatMark {
  public:
    RepeatMark(const RepeatMark&) = delete;
    bool matches(CharType t, const std::string& s) const {
      return t == CharType::Hiragana && _hiragana == s || t == CharType::Katakana && _katakana == s;
    }
    std::string get(CharType target, int flags, const Kana* prevKana) const;
    const std::string& hiragana() const { return _hiragana; }
    const std::string& katakana() const { return _katakana; }
  private:
    friend Kana; // only Kana class can constuct
    RepeatMark(const char* hiragana, const char* katakana, bool dakuten = false)
      : _hiragana(hiragana), _katakana(katakana), _dakuten(dakuten) {
      assert(_hiragana != _katakana);
    }
    const std::string _hiragana;
    const std::string _katakana;
    const bool _dakuten; // true if this instance if for the 'dakuten' (濁点) versions of the marks
  };
  // plain and accented repeat marks
  static const RepeatMark RepeatPlain;
  static const RepeatMark RepeatAccented;
  // provide static const refs for some special-case Kana
  static const Kana& SmallTsu;
  static const Kana& N;
  // 'ProlongMark' (ー) is officially in the Katakana Unicode block, but it can also rarely appear
  // in some (non-standard) Hiragana words like らーめん.
  static const std::string ProlongMark;

  using Map = std::map<std::string, const class Kana*>;
  static const Map& getMap(CharType t) {
    switch (t) {
    case CharType::Romaji: return _romajiMap;
    case CharType::Hiragana: return _hiraganaMap;
    case CharType::Katakana: return _katakanaMap;
    }
  }

  using List = std::vector<std::string>;
  Kana(const char* romaji, const char* hiragana, const char* katakana, const char* hepburn = nullptr,
       const char* kunrei = nullptr)
    : _romaji(romaji), _hiragana(hiragana), _katakana(katakana),
      _hepburn(hepburn ? std::optional(hepburn) : std::nullopt),
      _kunrei(kunrei ? std::optional(kunrei) : std::nullopt) {
    validate();
  }
  // Kana with a set of unique extra variant romaji values (first variant is optionally a 'kunreiVariant')
  Kana(const char* romaji, const char* hiragana, const char* katakana, const List& romajiVariants,
       bool kunreiVariant = false)
    : _romaji(romaji), _hiragana(hiragana), _katakana(katakana), _romajiVariants(romajiVariants),
      _kunreiVariant(kunreiVariant) {
    assert(_kunreiVariant ? !_romajiVariants.empty() : true);
    validate();
  }
  virtual ~Kana() = default;

  // 'dakutenKana' and 'hanDakutenKana' are overridden by derived classes to return the accented versions
  virtual const Kana* dakutenKana() const { return nullptr; }
  virtual const Kana* hanDakutenKana() const { return nullptr; }
  // 'plainKana' returns the unaccented version of a Kana - this will return return 'nullptr' if
  // instance is already an unaccented version or is a combination that doesn't have an equivalent
  // unaccented 'standard combination' such as 'va', 've', 'vo' (ヴォ), etc.. ウォ can be typed with 'u'
  // then 'lo' to get a small 'o', but this is treated as two separate Kana instances ('u' and 'lo').
  const Kana* plainKana() const { return _plainKana; }

  // All small kana have _romaji starting with 'l' (and they are all monographs)
  bool isSmall() const { return _romaji.starts_with("l"); }
  // A 'Kana' instance can either be a single symbol or two symbols. This is enforced by assertions
  // in the constructor as well as unit tests.
  bool isMonograph() const { return _hiragana.length() == 3; }
  bool isDigraph() const { return _hiragana.length() == 6; }
  // Test if the current instance (this) is a 'dakuten' or 'han-dakuten' Kana, i.e., the class of
  // 'this' is 'Kana', but we are a member of a 'DakutenKana' or 'HanDakutenKana' class.
  bool isDakuten() const {
    // special case for a few digraphs starting with 'v', but don't have an unaccented version (see above)
    return _romaji.starts_with("v") || _plainKana && _plainKana->dakutenKana() == this;
  }
  bool isHanDakuten() const { return _plainKana && _plainKana->hanDakutenKana() == this; }

  // 'getRomaji' returns 'Romaji' value based on flags (see 'ConversionFlags' in KanaConvert.h)
  const std::string& getRomaji(int flags) const;

  // repeat the first letter of romaji for sokuon (促音) output (special handling for 't' as
  // described in comments above).
  std::string getSokuonRomaji(int flags) const {
    auto& r = getRomaji(flags);
    return (r[0] == 'c' ? 't' : r[0]) + r;
  }
  const std::string& get(CharType t, int flags) const;

  bool containsKana(const std::string& s) const { return s == _hiragana || s == _katakana; }
  bool operator==(const Kana& rhs) const {
    // comparing _romaji is good enough since uniqueness is enforced by the rest of the program
    return _romaji == rhs._romaji;
  }
  const std::string& romaji() const { return _romaji; }
  const std::string& hiragana() const { return _hiragana; }
  const std::string& katakana() const { return _katakana; }
  const List& romajiVariants() const { return _romajiVariants; }
  bool hasHepburn() const { return _hepburn.has_value(); }
  bool hasKunrei() const { return _kunrei.has_value(); }
  bool kunreiVariant() const { return _kunreiVariant; }
private:
  static Map populate(CharType);
  static const Map _romajiMap;
  static const Map _hiraganaMap;
  static const Map _katakanaMap;
  // 'validate' used asserts to make sure the data is valid such as checking lengths and
  // ensuring '_hiragana' is actually valid Hiragana, etc..
  void validate() const;
  // '_romaji' usually holds the Modern Hepburn value, but will sometimes be a Nihon Shiki
  // value in order to ensure a unique value for Kana maps ('di' for ぢ, 'du' for づ, etc.)
  const std::string _romaji;
  const std::string _hiragana;
  const std::string _katakana;
  // '_romajiVariants' holds any further variant Romaji values that are unique for this 'Kana'
  // class. These include extra key combinations that also map to the same value such as
  // 'kwa' for クァ (instead of 'qa'), 'fyi' フィ (instead of 'fi'), etc.
  const List _romajiVariants;
  // '_hepburn' holds an optional 'Modern Hepburn' value for a few cases where it differs
  // from the 'unique' wāpuro romaji. For example, づ can be uniquely identified by 'du',
  // but the correct Hepburn output for this kana is 'zu' which is ambiguous with ず.
  // '_hepburn' (if it's populated) is always a duplicate of another Kana's '_romaji' value.
  const std::optional<std::string> _hepburn = std::nullopt;
  // '_kunrei' holds an optional 'Kunrei Shiki' value for a few cases like 'zya' for じゃ.
  const std::optional<std::string> _kunrei = std::nullopt;
  // '_kunreiVariant' is true if the first entry in '_romajiVariants' is a 'Kunrei Shiki' value. If
  // this is true then '_kunrei' should be nullopt.
  const bool _kunreiVariant = false;
  // '_plainKana' is set to unaccented version by DakutenKana and HanDakutenKana constructors. For
  // example, the DakutenKana instance for け contains '_dakutenKana' Kana げ and in turn, げ will
  // have '_plainKana' set to the original け to allow migration both ways.
  const Kana* _plainKana = nullptr;
  friend class DakutenKana;
  friend class HanDakutenKana;
  Kana(const Kana&) = default; // copy-constructor should only be called by 'friend' derived classes
};

// 'DakutenKana' is for 'k', 's', 't', 'h' row kana which have a dakuten, i.e., か has が. The _romaji
// _hiragana and _katakana members of this class are the unaccented versions and the _dakutenKana
// member is the accented version (the version with a 'dakuten').
class DakutenKana : public Kana {
public:
  DakutenKana(const char* romaji, const char* hiragana, const char* katakana, const Kana& dakutenKana,
              const char* hepburn = nullptr, const char* kunrei = nullptr)
    : Kana(romaji, hiragana, katakana, hepburn, kunrei), _dakutenKana(dakutenKana) {
    _dakutenKana._plainKana = this;
  }
  DakutenKana(const char* romaji, const char* hiragana, const char* katakana, const Kana& dakutenKana,
              const List& romajiVariants, bool kunreiVariant = false)
    : Kana(romaji, hiragana, katakana, romajiVariants, kunreiVariant), _dakutenKana(dakutenKana) {
    _dakutenKana._plainKana = this;
  }
  const Kana* dakutenKana() const override { return &_dakutenKana; }
private:
  Kana _dakutenKana;
};

// 'HanDakutenKana' is only populated for 'h' row kana, i.e., は has ぱ
class HanDakutenKana : public DakutenKana {
public:
  HanDakutenKana(const char* romaji, const char* hiragana, const char* katakana, const Kana& dakutenKana,
                 const Kana& hanDakutenKana, const char* hepburn = nullptr, const char* kunrei = nullptr)
    : DakutenKana(romaji, hiragana, katakana, dakutenKana, hepburn, kunrei), _hanDakutenKana(hanDakutenKana) {
    _hanDakutenKana._plainKana = this;
  }
  HanDakutenKana(const char* romaji, const char* hiragana, const char* katakana, const Kana& dakutenKana,
                 const Kana& hanDakutenKana, const List& romajiVariants, bool kunreiVariant = false)
    : DakutenKana(romaji, hiragana, katakana, dakutenKana, romajiVariants, kunreiVariant),
      _hanDakutenKana(hanDakutenKana) {
    _hanDakutenKana._plainKana = this;
  }
  const Kana* hanDakutenKana() const override { return &_hanDakutenKana; }
private:
  Kana _hanDakutenKana;
};

} // namespace kanji

#endif // KANJI_KANA_H
