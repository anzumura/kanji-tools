#pragma once

#include <kanji_tools/utils/EnumArray.h>
#include <kanji_tools/utils/EnumBitmask.h>

#include <cassert>
#include <optional>
#include <vector>

namespace kanji_tools {

// 'CharType' is used to specify 'source' and 'target' types for
// 'KanaConvert::convert' methods
enum class CharType { Hiragana, Katakana, Romaji };
template<> inline constexpr auto is_enumarray<CharType>{true};
inline const auto CharTypes =
  BaseEnumArray<CharType>::create("Hiragana", "Katakana", "Romaji");

// 'ConvertFlags' controls some aspects of conversion (by KanaConvert class).
// For example: Hepburn: off by default, only applies to 'Rōomaji' output
// - convert("つづき", CharType::Romaji) -> "tsuduki"
// - convert("つづき", CharType::Romaji, Hepburn) -> "tsuzuki"
// Kunrei: off by default, only applies to 'Rōmaji' output
// - convert("しつ", CharType::Romaji) -> "shitsu"
// - convert("しつ", CharType::Romaji, Kunrei) -> "situ"
// NoProlongMark: off by default, only applies to 'Hiragana' output
// - convert("rāmen", CharType::Hiragana) -> "らーめん"
// - convert("rāmen", CharType::Hiragana, NoProlongMark) -> "らあめん"
// RemoveSpaces: off by default, only applies when converting from Rōmaji:
// - convert("akai kitsune", CharType::Hiragana) returns "あかい　きつね" (with
// a wide space)
// - convert("akai kitsune", CharType::Hiragana, RemoveSpaces) returns
// "あかいきつね"
//
// Notes:
//
// Prolonged sound marks in Hiragana are non-standard, but output them by
// default in order to support round-trip type conversions, otherwise the above
// example would map "らあめん" back to "raamen" which doesn't match the initial
// value. ConvertFlags suppots bitwise operators so they can be combined using
// '|', for example:
// - convert("rāmen desu.", CharType::Hiragana, ConvertFlags::RemoveSpaces |
// ConvertFlags::NoProlongMark) -> "らあめんです。"
//
// Enabling 'Hepburn' leads to more standard Rōmaji, but the output is ambiguous
// and leads to different Kana if converted back. This affects di (ぢ), dya
// (ぢゃ), dyo (ぢょ), dyu (ぢゅ), du (づ) and wo (を) - these become ji, ja,
// ju, jo, zu and o instead. There's also no support for trying to handle は and
// へ (which in standard Hepburn should map to 'wa' and 'e' if they are used as
// particles) - instead they simply map to 'ha' and 'he' all the time. If both
// Hepburn and Kunrei flags are set then Hepburn is preferred, but will then try
// Kunrei before falling back to the unique '_romaji' value in the Kana class.
enum class ConvertFlags {
  None,
  Hepburn,
  Kunrei,
  NoProlongMark = 4,
  RemoveSpaces = 8
};
template<> inline constexpr auto is_bitmask<ConvertFlags>{true};

// 'Kana' is used to represent a Kana 'Monograph' or 'Digraph'. It stores
// Rōmaji, Hiragana and Katakana as well variant Rōmaji forms. A 'Monograph' is
// a single Kana character (large or small) and a 'Digraph' is a valid (at least
// typable using standard IME) two Kana combination. A 'Diagraph' always has a
// normal sized first Kana followed by a small Kana (one of the 5 vowels, 3 y's
// or 'wa'). This class also holds relationships between unaccented (plain) and
// accented (dakuten and han-dakuten) versions.
class Kana {
public:
  using Map = std::map<std::string, const class Kana*>;
  [[nodiscard]] static auto& getMap(CharType t) {
    switch (t) {
    case CharType::Romaji: return _romajiMap;
    case CharType::Hiragana: return _hiraganaMap;
    case CharType::Katakana: return _katakanaMap;
    }
    __builtin_unreachable(); // prevent gcc 'control reaches end ...' warning
  }

  using OptString = std::optional<std::string>;

  // find corresponding 'Dakuten' Kana, 's' should be a non-accented single
  // Hiragana or Katakana letter
  static OptString findDakuten(const std::string& s) {
    auto i{_hiraganaMap.find(s)};
    if (i != _hiraganaMap.end()) return i->second->dakuten(CharType::Hiragana);
    i = _katakanaMap.find(s);
    if (i != _katakanaMap.end()) return i->second->dakuten(CharType::Katakana);
    return std::nullopt;
  }

  // find corresponding 'HanDakuten' Kana, 's' should be a non-accented single
  // Hiragana or Katakana letter
  static OptString findHanDakuten(const std::string& s) {
    auto i = _hiraganaMap.find(s);
    if (i != _hiraganaMap.end())
      return i->second->hanDakuten(CharType::Hiragana);
    i = _katakanaMap.find(s);
    if (i != _katakanaMap.end())
      return i->second->hanDakuten(CharType::Katakana);
    return std::nullopt;
  }

  // 'RepeatMark' is for handling repeating Kana marks (一の時点) when source is
  // Hiragana or Katakana.
  class RepeatMark {
  public:
    RepeatMark(const RepeatMark&) = delete;
    // operator= is not generated since there are const members

    [[nodiscard]] auto matches(CharType t, const std::string& s) const {
      return t == CharType::Hiragana && _hiragana == s ||
             t == CharType::Katakana && _katakana == s;
    }
    [[nodiscard]] std::string get(CharType target, ConvertFlags flags,
                                  const Kana* prevKana) const;
    [[nodiscard]] auto& hiragana() const { return _hiragana; }
    [[nodiscard]] auto& katakana() const { return _katakana; }
  private:
    friend Kana; // only Kana class can constuct
    RepeatMark(const char* hiragana, const char* katakana, bool dakuten)
        : _hiragana(hiragana), _katakana(katakana), _dakuten(dakuten) {
      assert(_hiragana != _katakana);
    }
    const std::string _hiragana;
    const std::string _katakana;
    const bool _dakuten; // true if this instance if 'dakuten' (濁点) version
  };

  // plain and accented repeat marks
  inline static const auto RepeatPlain = RepeatMark("ゝ", "ヽ", false);
  inline static const auto RepeatAccented = RepeatMark("ゞ", "ヾ", true);

  // provide static const refs for some special-case Kana
  static const Kana& SmallTsu;
  static const Kana& N;

  // 'ProlongMark' (ー) is officially in the Katakana Unicode block, but it can
  // also rarely appear in some (non-standard) Hiragana words like らーめん.
  inline static const std::string ProlongMark{"ー"};

  using List = std::vector<std::string>;

  Kana(const char* romaji, const char* hiragana, const char* katakana,
       const char* hepburn = nullptr, const char* kunrei = nullptr)
      : _romaji(romaji), _hiragana(hiragana), _katakana(katakana),
        _hepburn(hepburn ? std::optional(hepburn) : std::nullopt),
        _kunrei(kunrei ? std::optional(kunrei) : std::nullopt) {
    validate();
  }

  // Kana with a set of unique extra variant Rōmaji values (first variant is
  // optionally a 'kunreiVariant')
  Kana(const char* romaji, const char* hiragana, const char* katakana,
       const List& romajiVariants, bool kunreiVariant = false)
      : _romaji(romaji), _hiragana(hiragana), _katakana(katakana),
        _romajiVariants(romajiVariants), _kunreiVariant(kunreiVariant) {
    assert(_kunreiVariant ? !_romajiVariants.empty() : true);
    validate();
  }

  // operator= is not generated since there are const members
  virtual ~Kana() = default;

  // 'dakutenKana' and 'hanDakutenKana' are overridden by derived classes to
  // return the accented versions
  [[nodiscard]] virtual const Kana* dakutenKana() const { return nullptr; }
  [[nodiscard]] virtual const Kana* hanDakutenKana() const { return nullptr; }

  [[nodiscard]] OptString dakuten(CharType t) const {
    auto i = dakutenKana();
    if (i) return i->get(t, ConvertFlags::None);
    return {};
  }

  [[nodiscard]] OptString hanDakuten(CharType t) const {
    auto i = hanDakutenKana();
    if (i) return i->get(t, ConvertFlags::None);
    return {};
  }

  // 'plainKana' returns the unaccented version of a Kana - this will return
  // return 'nullptr' if instance is already an unaccented version or is a
  // combination that doesn't have an equivalent unaccented 'standard
  // combination' such as 'va', 've', 'vo' (ヴォ), etc.. ウォ can be typed with
  // 'u' then 'lo' to get a small 'o', but this is treated as two separate Kana
  // instances ('u' and 'lo').
  [[nodiscard]] auto plainKana() const { return _plainKana; }

  // All small Kana have _romaji starting with 'l' (and they are all monographs)
  [[nodiscard]] auto isSmall() const { return _romaji.starts_with("l"); }

  // A 'Kana' instance can either be a single symbol or two symbols. This is
  // enforced by assertions in the constructor as well as unit tests.
  [[nodiscard]] auto isMonograph() const { return _hiragana.size() == 3; }
  [[nodiscard]] auto isDigraph() const { return _hiragana.size() == 6; }

  // Test if the current instance (this) is a 'dakuten' or 'han-dakuten' Kana,
  // i.e., the class of 'this' is 'Kana', but we are a member of a 'DakutenKana'
  // or 'HanDakutenKana' class.
  [[nodiscard]] auto isDakuten() const {
    // special case for a few digraphs starting with 'v', but don't have an
    // unaccented version (see above)
    return _romaji.starts_with("v") ||
           _plainKana && _plainKana->dakutenKana() == this;
  }
  [[nodiscard]] auto isHanDakuten() const {
    return _plainKana && _plainKana->hanDakutenKana() == this;
  }

  // 'getRomaji' returns 'Rōmaji' value based on flags
  [[nodiscard]] const std::string& getRomaji(ConvertFlags flags) const;

  // repeat the first letter of _romaji for sokuon (促音) output (special
  // handling for 't' as described in comments above).
  [[nodiscard]] std::string getSokuonRomaji(ConvertFlags flags) const {
    auto& r = getRomaji(flags);
    return (r[0] == 'c' ? 't' : r[0]) + r;
  }

  [[nodiscard]] const std::string& get(CharType t, ConvertFlags flags) const;

  [[nodiscard]] auto containsKana(const std::string& s) const {
    return s == _hiragana || s == _katakana;
  }

  // comparing _romaji is good enough since uniqueness is enforced by the rest
  // of the program
  [[nodiscard]] auto operator==(const Kana& rhs) const {
    return _romaji == rhs._romaji;
  }

  [[nodiscard]] auto& romaji() const { return _romaji; }
  [[nodiscard]] auto& hiragana() const { return _hiragana; }
  [[nodiscard]] auto& katakana() const { return _katakana; }
  [[nodiscard]] auto& romajiVariants() const { return _romajiVariants; }
  [[nodiscard]] auto kunreiVariant() const { return _kunreiVariant; }
private:
  static Map populate(CharType);
  static const Map _romajiMap;
  static const Map _hiraganaMap;
  static const Map _katakanaMap;

  // 'validate' uses asserts to make sure the data is valid such as checking
  // sizes and ensuring '_hiragana' is actually valid Hiragana, etc..
  void validate() const;

  // '_romaji' usually holds the Modern Hepburn value, but will sometimes be a
  // Nihon Shiki value in order to ensure a unique value for Kana maps ('di' for
  // ぢ, 'du' for づ, etc.)
  const std::string _romaji;
  const std::string _hiragana;
  const std::string _katakana;

  // '_romajiVariants' holds any further variant Rōmaji values that are unique
  // for this 'Kana' class. These include extra key combinations that also map
  // to the same value such as 'kwa' for クァ (instead of 'qa'), 'fyi' フィ
  // (instead of 'fi'), etc.
  const List _romajiVariants;

  // '_hepburn' holds an optional 'Modern Hepburn' value for a few cases where
  // it differs from the 'unique' Wāpuro Rōmaji. For example, づ can be uniquely
  // identified by 'du', but the correct Hepburn output for this Kana is 'zu'
  // which is ambiguous with ず.
  // '_hepburn' (if it's populated) is always a duplicate of another Kana's
  // '_romaji' value.
  const std::optional<std::string> _hepburn = std::nullopt;

  // '_kunrei' holds an optional 'Kunrei Shiki' value for a few cases like 'zya'
  // for じゃ.
  const std::optional<std::string> _kunrei = std::nullopt;

  // '_kunreiVariant' is true if the first entry in '_romajiVariants' is a
  // 'Kunrei Shiki' value. If this is true then '_kunrei' should be nullopt.
  const bool _kunreiVariant = false;

  // '_plainKana' is set to unaccented version by DakutenKana and HanDakutenKana
  // constructors. For example, the DakutenKana instance for け contains
  // '_dakutenKana' Kana げ and in turn, げ will have '_plainKana' set to the
  // original け to allow migration both ways.
  const Kana* _plainKana = nullptr;

  friend class DakutenKana;
  friend class HanDakutenKana;
  // copy-constructor should only be called by 'friend' derived classes
  Kana(const Kana&) = default;
};

// 'DakutenKana' is for 'k', 's', 't', 'h' row Kana which have a dakuten, i.e.,
// か has が. The _romaji _hiragana and _katakana members of this class are the
// unaccented versions and the _dakutenKana member is the accented version (the
// version with a 'dakuten').
class DakutenKana : public Kana {
public:
  DakutenKana(const char* romaji, const char* hiragana, const char* katakana,
              const Kana& dakutenKana, const char* hepburn = nullptr,
              const char* kunrei = nullptr)
      : Kana(romaji, hiragana, katakana, hepburn, kunrei),
        _dakutenKana(dakutenKana) {
    _dakutenKana._plainKana = this;
  }
  DakutenKana(const char* romaji, const char* hiragana, const char* katakana,
              const Kana& dakutenKana, const List& romajiVariants,
              bool kunreiVariant = false)
      : Kana(romaji, hiragana, katakana, romajiVariants, kunreiVariant),
        _dakutenKana(dakutenKana) {
    _dakutenKana._plainKana = this;
  }
  [[nodiscard]] const Kana* dakutenKana() const override {
    return &_dakutenKana;
  }
private:
  Kana _dakutenKana;
};

// 'HanDakutenKana' is only populated for 'h' row Kana, i.e., は has ぱ
class HanDakutenKana : public DakutenKana {
public:
  HanDakutenKana(const char* romaji, const char* hiragana, const char* katakana,
                 const Kana& dakutenKana, const Kana& hanDakutenKana,
                 const char* hepburn = nullptr, const char* kunrei = nullptr)
      : DakutenKana(romaji, hiragana, katakana, dakutenKana, hepburn, kunrei),
        _hanDakutenKana(hanDakutenKana) {
    _hanDakutenKana._plainKana = this;
  }
  HanDakutenKana(const char* romaji, const char* hiragana, const char* katakana,
                 const Kana& dakutenKana, const Kana& hanDakutenKana,
                 const List& romajiVariants, bool kunreiVariant = false)
      : DakutenKana(romaji, hiragana, katakana, dakutenKana, romajiVariants,
                    kunreiVariant),
        _hanDakutenKana(hanDakutenKana) {
    _hanDakutenKana._plainKana = this;
  }
  [[nodiscard]] const Kana* hanDakutenKana() const override {
    return &_hanDakutenKana;
  }
private:
  Kana _hanDakutenKana;
};

} // namespace kanji_tools
