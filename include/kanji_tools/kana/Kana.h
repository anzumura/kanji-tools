#pragma once

#include <kanji_tools/utils/EnumArray.h>
#include <kanji_tools/utils/EnumBitmask.h>

#include <optional>
#include <vector>

namespace kanji_tools {

// 'CharType' is used to specify 'source' and 'target' types for
// 'KanaConvert::convert' methods
enum class CharType { Hiragana, Katakana, Romaji };

template<> inline constexpr auto is_enumarray<CharType>{true};

inline const auto CharTypes{
    BaseEnumArray<CharType>::create("Hiragana", "Katakana", "Romaji")};

// 'ConvertFlags' controls some aspects of conversion (by KanaConvert class).
// For example: Hepburn: off by default, only applies to 'Rōmaji' output
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
    case CharType::Romaji: return RomajiMap;
    case CharType::Hiragana: return HiraganaMap;
    case CharType::Katakana: return KatakanaMap;
    }
    __builtin_unreachable(); // prevent gcc 'control reaches end ...' warning
  }

  using OptString = std::optional<std::string>;

  // find corresponding 'Dakuten' Kana, 's' should be a non-accented single
  // Hiragana or Katakana letter
  static OptString findDakuten(const std::string& s) {
    auto i{HiraganaMap.find(s)};
    if (i != HiraganaMap.end()) return i->second->dakuten(CharType::Hiragana);
    i = KatakanaMap.find(s);
    if (i != KatakanaMap.end()) return i->second->dakuten(CharType::Katakana);
    return {};
  }

  // find corresponding 'HanDakuten' Kana, 's' should be a non-accented single
  // Hiragana or Katakana letter
  static OptString findHanDakuten(const std::string& s) {
    auto i{HiraganaMap.find(s)};
    if (i != HiraganaMap.end())
      return i->second->hanDakuten(CharType::Hiragana);
    i = KatakanaMap.find(s);
    if (i != KatakanaMap.end())
      return i->second->hanDakuten(CharType::Katakana);
    return {};
  }

  // 'RomajiVariants' holds any further variant Rōmaji values that are unique
  // for this 'Kana' class. These include extra key combinations that also map
  // to the same value such as 'kwa' for クァ (instead of 'qa'), 'fyi' フィ
  // (instead of 'fi'), etc.. '_kunrei' is true if the first entry in '_list' is
  // a 'Kunrei Shiki' value (and then 'Kana::_kunrei' should be nullopt).
  class RomajiVariants {
  public:
    RomajiVariants() = default;
    RomajiVariants(RomajiVariants&&) = default; // only allow moving (no copy)

    template<size_t V>
    RomajiVariants(const char (&v)[V], bool kunrei = false)
        : _list{v}, _kunrei{kunrei} {
      static_assert(check(V));
    }

    // all instances with two variants have variants with the same size (like
    // 'fa' (ファ) which has Rōmaji variants of 'fwa' and 'hwa')
    template<size_t V>
    RomajiVariants(
        const char (&v1)[V], const char (&v2)[V], bool kunrei = false)
        : _list{v1, v2}, _kunrei{kunrei} {
      static_assert(check(V));
    }

    // no instance with 3 variants has 'kunrei' true, but one has differing
    // sizes so need two template params, i.e, small 'ぇ' with Rōmaji of 'le'
    // has a variant list of 'xe', 'lye' and 'xye'
    template<size_t V1, size_t V2>
    RomajiVariants(
        const char (&v1)[V1], const char (&v2)[V2], const char (&v3)[V2])
        : _list{v1, v2, v3} {
      static_assert(check(V1) && check(V2));
    }

    [[nodiscard]] auto& list() const { return _list; }
    [[nodiscard]] auto kunrei() const { return _kunrei; }
  private:
    [[nodiscard]] static consteval bool check(size_t x) {
      return x > 2 && x < 5;
    }

    std::vector<std::string> _list;
    bool _kunrei{false};
  };

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
    [[nodiscard]] const std::string& get(
        CharType target, ConvertFlags flags, const Kana* prevKana) const;
    [[nodiscard]] auto& hiragana() const { return _hiragana; }
    [[nodiscard]] auto& katakana() const { return _katakana; }
  private:
    friend Kana; // only Kana class can constuct
    template<size_t N>
    RepeatMark(
        const char (&hiragana)[N], const char (&katakana)[N], bool dakuten)
        : _hiragana{hiragana}, _katakana{katakana}, _dakuten{dakuten} {
      static_assert(N == 4);
      validate();
    }

    void validate() const;

    const std::string _hiragana, _katakana;
    const bool _dakuten; // true if this instance is 'dakuten' (濁点) version
  };

  // plain and accented repeat marks
  inline static const RepeatMark RepeatPlain{"ゝ", "ヽ", false},
      RepeatAccented{"ゞ", "ヾ", true};

  // provide static const refs for some special-case Kana
  static const Kana& SmallTsu;
  static const Kana& N;

  // 'ProlongMark' (ー) is officially in the Katakana Unicode block, but it can
  // also rarely appear in some (non-standard) Hiragana words like らーめん.
  inline static const std::string ProlongMark{"ー"};

  template<size_t R, size_t N>
  Kana(const char (&romaji)[R], const char (&hiragana)[N],
      const char (&katakana)[N])
      : Kana{romaji, hiragana, katakana, {}, {}, {}} {}

  template<size_t R, size_t N, size_t H, size_t K>
  Kana(const char (&romaji)[R], const char (&hiragana)[N],
      const char (&katakana)[N], const char (&hepburn)[H],
      const char (&kunrei)[K])
      : Kana{romaji, hiragana, katakana, hepburn, kunrei, {}} {
    static_assert(H < 5 && K < 5);
    static_assert(N == 4 && H > 1 && K > 1 || N == 7 && H > 2 && K > 2);
  }

  // Kana with a set of unique extra variant Rōmaji values (first variant is
  // optionally a 'kunreiVariant')
  template<size_t R, size_t N>
  Kana(const char (&romaji)[R], const char (&hiragana)[N],
      const char (&katakana)[N], RomajiVariants&& variants)
      : Kana{romaji, hiragana, katakana, {}, {}, std::move(variants)} {}

  virtual ~Kana() = default;
  Kana(const Kana&) = delete;
  // operator= is not generated since there are const members

  // 'dakuten' and 'hanDakuten' are overridden by derived classes to
  // return the accented versions
  [[nodiscard]] virtual const Kana* dakuten() const { return nullptr; }
  [[nodiscard]] virtual const Kana* hanDakuten() const { return nullptr; }

  // 'plain' returns the unaccented version of this Kana or 'nullptr' if this
  // Kana is unaccented or is a combination that doesn't have an equivalent
  // unaccented 'standard combination' such as 'va', 've', 'vo' (ヴォ), etc..
  // Note: ウォ can be typed with 'u' then 'lo', but is treated as two separate
  // Kana instances ('u' and 'lo') instead of a plain version of 'vo'.
  [[nodiscard]] virtual const Kana* plain() const { return nullptr; }

  [[nodiscard]] OptString dakuten(CharType t) const {
    if (const auto i{dakuten()}; i) return i->get(t, ConvertFlags::None);
    return {};
  }

  [[nodiscard]] OptString hanDakuten(CharType t) const {
    if (const auto i{hanDakuten()}; i) return i->get(t, ConvertFlags::None);
    return {};
  }

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
    return _romaji.starts_with("v") || plain() && plain()->dakuten() == this;
  }
  [[nodiscard]] auto isHanDakuten() const {
    return plain() && plain()->hanDakuten() == this;
  }

  // 'getRomaji' returns 'Rōmaji' value based on flags
  [[nodiscard]] const std::string& getRomaji(ConvertFlags flags) const;

  // repeat the first letter of _romaji for sokuon (促音) output (special
  // handling for 't' as described in comments above).
  [[nodiscard]] std::string getSokuonRomaji(ConvertFlags flags) const {
    auto& r{getRomaji(flags)};
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
  [[nodiscard]] auto& romajiVariants() const { return _variants.list(); }
  [[nodiscard]] auto kunreiVariant() const { return _variants.kunrei(); }
protected:
  // This move constructor is used by derived 'AccentedKana' class. It moves the
  // non-const '_variants' field, but other fields will get copied because they
  // are all 'const' (copy is fine since the other fields are all short strings
  // that would not benefit from move anyway because of SSO).
  Kana(Kana&&) = default;
private:
  template<size_t R, size_t N>
  Kana(const char (&romaji)[R], const char (&hiragana)[N],
      const char (&katakana)[N], const char* hepburn, const char* kunrei,
      RomajiVariants&& variants)
      : _romaji{romaji}, _hiragana{hiragana}, _katakana{katakana},
        _hepburn{hepburn ? OptString(hepburn) : std::nullopt},
        _kunrei{kunrei ? OptString(kunrei) : std::nullopt}, _variants{std::move(
                                                                variants)} {
    // Romaji can't be longer than 3 (so '< 5' when including the final '\0')
    static_assert(R < 5);
    // Hiragana and Katakana must be the same size (3 or 6) and also check that
    // Romaji is at least 1 character for a monograph or 2 for a digraph
    static_assert(N == 4 && R > 1 || N == 7 && R > 2);
    validate();
  }

  static Map populate(CharType);
  static const Map RomajiMap, HiraganaMap, KatakanaMap;

  // 'validate' uses asserts to make sure the data is valid such as checking
  // sizes and ensuring '_hiragana' is actually valid Hiragana, etc..
  void validate() const;

  // '_romaji' usually holds the Modern Hepburn value, but will sometimes be a
  // Nihon Shiki value in order to ensure a unique value for Kana maps ('di' for
  // ぢ, 'du' for づ, etc.)
  const std::string _romaji, _hiragana, _katakana;

  // '_hepburn' holds an optional 'Modern Hepburn' value for a few cases where
  // it differs from the 'unique' Wāpuro Rōmaji. For example, づ can be uniquely
  // identified by 'du', but the correct Hepburn output for this Kana is 'zu'
  // which is ambiguous with ず. '_hepburn' (if it's populated) is always a
  // duplicate of another Kana's '_romaji' value.
  const OptString _hepburn;

  // '_kunrei' holds an optional 'Kunrei Shiki' value like 'zya' for じゃ.
  const OptString _kunrei;

  RomajiVariants _variants; // non-const to allow moving
};

// 'DakutenKana' is for 'k', 's', 't', 'h' row Kana which have a dakuten, i.e.,
// か has が. The _romaji _hiragana and _katakana members of this class are the
// unaccented versions and the _dakuten member is the accented version (the
// version with a 'dakuten').
class DakutenKana : public Kana {
public:
  template<typename... T>
  DakutenKana(Kana&& dakuten, T&&... t)
      : Kana{std::forward<T>(t)...}, _dakuten{std::move(dakuten), *this} {}

  [[nodiscard]] const Kana* dakuten() const override { return &_dakuten; }
protected:
  // 'AccentedKana' holds a pointer back to a corresponding 'plain' version.
  // This class is used by both 'DakutenKana' and 'HanDakutenKana' classes to
  // hold the accented versions.
  class AccentedKana : public Kana {
  public:
    // Custom move-constructor that moves 'k' into base class fields via
    // protected move constructor and sets '_plain'.
    AccentedKana(Kana&& k, const Kana& p) : Kana{std::move(k)}, _plain{&p} {}

    [[nodiscard]] const Kana* plain() const override { return _plain; }
  private:
    // '_plain' is set to unaccented version by DakutenKana and HanDakutenKana
    // constructors. For example, the DakutenKana instance for け contains
    // '_dakuten' Kana げ and in turn, げ will have '_plain' set to the original
    // け to allow lookup both ways.
    const Kana* const _plain;
  };
private:
  const AccentedKana _dakuten;
};

// 'HanDakutenKana' (semi-voiced) is only populated for 'h' row Kana. This class
// also derives from 'DakutenKana' since 'h' row Kana also have voiced versions,
// i.e., 'ha' (は) has semi-voiced 'pa' (ぱ) and voiced 'ba' (ば).
class HanDakutenKana : public DakutenKana {
public:
  template<typename... T>
  HanDakutenKana(Kana&& hanDakuten, T&&... t)
      : DakutenKana{std::forward<T>(t)...}, _hanDakuten{
                                                std::move(hanDakuten), *this} {}

  [[nodiscard]] const Kana* hanDakuten() const override { return &_hanDakuten; }
private:
  const AccentedKana _hanDakuten;
};

} // namespace kanji_tools
