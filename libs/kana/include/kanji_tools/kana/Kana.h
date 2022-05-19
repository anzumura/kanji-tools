#pragma once

#include <kanji_tools/utils/Bitmask.h>
#include <kanji_tools/utils/EnumList.h>

#include <optional>
#include <vector>

namespace kanji_tools {

// 'CharType' is used by 'Converter' class to specify 'source' and 'target'
// types when converting
enum class CharType : Enum::Size { Hiragana, Katakana, Romaji };
template<> inline constexpr auto is_enumlist<CharType>{true};
inline const auto CharTypes{
    BaseEnumList<CharType>::create("Hiragana", "Katakana", "Romaji")};

// 'ConvertFlags' is also used by 'Converter' class to control some aspects of
// conversion (see comments at the bottom of this file for more details)
enum class ConvertFlags : Enum::Size {
  None,
  Hepburn,
  Kunrei,
  NoProlongMark = 4,
  RemoveSpaces = 8
};
template<> inline constexpr auto is_bitmask<ConvertFlags>{true};

// 'Kana' represents a Kana 'Monograph' or 'Digraph'. It stores Rōmaji, Hiragana
// and Katakana as well as variant Rōmaji forms (see comments at the bottom of
// this file for more details)
class Kana {
public:
  using Map = std::map<String, const class Kana*>;
  using OptString = std::optional<String>;
  template<size_t N> using CharArray = const char (&)[N];

  inline static const OptString EmptyOptString;

  // constants for UTF-8 Kana string and array sizes containing Kana (an array
  // is one bigger to hold the final null character)
  static constexpr uint16_t OneKanaSize{3}; // all Kana are 3 bytes
  static constexpr uint16_t OneKanaArraySize{OneKanaSize + 1},
      TwoKanaSize{OneKanaSize * 2}, TwoKanaArraySize{OneKanaSize * 2 + 1};

  // sizes for Rōmaji strings and arrays
  static constexpr uint16_t RomajiArrayMinSize{2}, RomajiArrayMaxSize{4};
  static constexpr uint16_t RomajiStringMaxSize{RomajiArrayMaxSize - 1};

  // provide static const refs for some special-case Kana
  static const Kana& SmallTsu;
  static const Kana& N;

  // 'ProlongMark' (ー) is officially in the Katakana Unicode block, but it can
  // also rarely appear in some (non-standard) Hiragana words like らーめん.
  inline static const String ProlongMark{"ー"};

  [[nodiscard]] static const Map& getMap(CharType);

  // find corresponding 'Dakuten' Kana, 's' should be a non-accented single
  // Hiragana or Katakana letter
  [[nodiscard]] static OptString findDakuten(const String& s);

  // find corresponding 'HanDakuten' Kana, 's' should be a non-accented single
  // Hiragana or Katakana letter
  [[nodiscard]] static OptString findHanDakuten(const String& s);

  // 'RomajiVariants' holds any further variant Rōmaji values that are unique
  // for this 'Kana' class. These include extra key combinations that also map
  // to the same value such as 'kwa' for クァ (instead of 'qa'), 'fyi' フィ
  // (instead of 'fi'), etc.. '_kunrei' is true if the first entry in '_list' is
  // a 'Kunrei Shiki' value (and then 'Kana::_kunrei' should be nullopt).
  class RomajiVariants {
  public:
    using List = std::vector<String>;
    using RMax = CharArray<RomajiArrayMaxSize>;

    RomajiVariants() = default;
    RomajiVariants(RomajiVariants&&) = default; // only allow moving

    template<size_t R>
    explicit RomajiVariants(CharArray<R> r, bool kunrei = false);

    // all instances with two variants have variants with the same size (like
    // 'fa' (ファ) which has Rōmaji variants of 'fwa' and 'hwa')
    template<size_t R>
    RomajiVariants(CharArray<R> r1, CharArray<R> r2, bool kunrei = false);

    // no instance with three variants has 'kunrei' true, but one has differing
    // sizes so need two template params, i.e, small 'ぇ' with Rōmaji of 'le'
    // has a variant list of 'xe', 'lye' and 'xye'
    template<size_t R> RomajiVariants(CharArray<R> r1, RMax r2, RMax r3);

    [[nodiscard]] auto& list() const { return _list; }
    [[nodiscard]] auto kunrei() const { return _kunrei; }
  private:
    // all Rōmaji variants are either 2 or 3 characters long
    template<size_t R> static consteval void check() {
      static_assert(R > RomajiArrayMinSize && R <= RomajiArrayMaxSize);
    }

    List _list;
    bool _kunrei{false};
  };

  // 'IterationMark' holds Kana iteration marks (一の字点)
  class IterationMark {
  public:
    IterationMark(const IterationMark&) = delete;

    [[nodiscard]] bool matches(CharType, const String&) const;
    [[nodiscard]] const String& get(
        CharType target, ConvertFlags, const Kana* prevKana) const;
    [[nodiscard]] auto& hiragana() const { return _hiragana; }
    [[nodiscard]] auto& katakana() const { return _katakana; }
  private:
    friend Kana; // only Kana class can construct
    IterationMark(CharArray<OneKanaArraySize> hiragana,
        CharArray<OneKanaArraySize> katakana, bool dakuten);

    void validate() const;

    const String _hiragana, _katakana;
    const bool _dakuten; // true if this instance is 'dakuten' (濁点) version
  };

  static const IterationMark RepeatPlain, RepeatAccented;

  // return iteration mark or nullptr if 'kana' isn't an iteration mark
  [[nodiscard]] static const IterationMark* findIterationMark(
      CharType, const String& kana);

  virtual ~Kana() = default;
  Kana(const Kana&) = delete;

  // 'dakuten' and 'hanDakuten' are overridden by derived classes to
  // return the accented versions
  [[nodiscard]] virtual const Kana* dakuten() const;
  [[nodiscard]] virtual const Kana* hanDakuten() const;

  // 'plain' returns the unaccented version of this Kana or 'nullptr' if this
  // Kana is unaccented or is a combination that doesn't have an equivalent
  // unaccented 'standard combination' such as 'va', 've', 'vo' (ヴォ), etc..
  // Note: ウォ can be typed with 'u' then 'lo', but is treated as two separate
  // Kana instances ('u' and 'lo') instead of a plain version of 'vo'.
  [[nodiscard]] virtual const Kana* plain() const;

  [[nodiscard]] OptString dakuten(CharType) const;

  [[nodiscard]] OptString hanDakuten(CharType) const;

  // All small Kana have _romaji starting with 'l' (and they are all monographs)
  [[nodiscard]] bool isSmall() const;

  // A 'Kana' instance can either be a single symbol or two symbols. This is
  // enforced by assertions in the constructor as well as unit tests.
  [[nodiscard]] bool isMonograph() const;
  [[nodiscard]] bool isDigraph() const;

  // Test if the current instance (this) is a 'dakuten' or 'han-dakuten' Kana,
  // i.e., the class of 'this' is 'Kana', but we are a member of a 'DakutenKana'
  // or 'HanDakutenKana' class.
  [[nodiscard]] bool isDakuten() const;
  [[nodiscard]] bool isHanDakuten() const;

  // 'getRomaji' returns 'Rōmaji' value based on flags
  [[nodiscard]] const String& getRomaji(ConvertFlags flags) const;

  // repeat the first letter of _romaji for sokuon (促音) output (special
  // handling for 't' as described in comments above).
  [[nodiscard]] String getSokuonRomaji(ConvertFlags) const;

  [[nodiscard]] const String& get(CharType, ConvertFlags) const;

  [[nodiscard]] bool containsKana(const String&) const;

  [[nodiscard]] bool operator==(const Kana&) const;

  [[nodiscard]] auto& romaji() const { return _romaji; }
  [[nodiscard]] auto& hiragana() const { return _hiragana; }
  [[nodiscard]] auto& katakana() const { return _katakana; }
  [[nodiscard]] auto& romajiVariants() const { return _variants.list(); }
  [[nodiscard]] auto kunreiVariant() const { return _variants.kunrei(); }

  // all supported instances of 'Kana' and 'Kana' derived classes are created
  // in 'Kana.cpp' (these ctors shouldn't be used anywhere else)

  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana);

  template<size_t R, size_t A, size_t H, size_t K>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      CharArray<H> hepburn, CharArray<K> kunrei);

  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      RomajiVariants&&);
protected:
  // This move constructor is used by derived 'AccentedKana' class. It moves the
  // non-const '_variants' field, but other fields will get copied because they
  // are all 'const' (copy is fine since the other fields are all short strings
  // that would not benefit from move anyway because of SSO).
  Kana(Kana&&) = default;
private:
  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      const char* hepburn, const char* kunrei, RomajiVariants&&);

  static Map populate(CharType);
  static const Map RomajiMap, HiraganaMap, KatakanaMap;

  // 'validate' uses asserts to make sure the data is valid such as ensuring
  // '_hiragana' is actually valid Hiragana, etc..
  void validate() const;

  const String _romaji, _hiragana, _katakana;
  const OptString _hepburn, _kunrei;

  RomajiVariants _variants; // non-const to allow moving
};

// 'DakutenKana' is for 'k', 's', 't', 'h' row Kana which have a dakuten, i.e.,
// か has が. The _romaji _hiragana and _katakana members of this class are the
// unaccented versions and the _dakuten member is the accented version (the
// version with a 'dakuten').
class DakutenKana : public Kana {
public:
  template<typename... T> explicit DakutenKana(Kana&& dakuten, T&&...);

  [[nodiscard]] const Kana* dakuten() const override;
protected:
  // 'AccentedKana' holds a pointer back to a corresponding 'plain' version.
  // This class is used by both 'DakutenKana' and 'HanDakutenKana' classes to
  // hold the accented versions.
  class AccentedKana : public Kana {
  public:
    // Custom move-constructor that moves 'k' into base class fields via
    // protected move constructor and sets '_plain' to 'p'.
    AccentedKana(Kana&& k, const Kana& p);

    [[nodiscard]] const Kana* plain() const override;
  private:
    // '_plain' is set to unaccented version by DakutenKana and HanDakutenKana
    // constructors. For example, the DakutenKana instance for け contains
    // '_dakuten' Kana げ and in turn, げ will have '_plain' set to the original
    // け to allow lookup both ways.
    const Kana& _plain;
  };
private:
  const AccentedKana _dakuten;
};

// 'HanDakutenKana' (semi-voiced) is only populated for 'h' row Kana. This class
// also derives from 'DakutenKana' since 'h' row Kana also have voiced versions,
// i.e., 'ha' (は) has semi-voiced 'pa' (ぱ) and voiced 'ba' (ば).
class HanDakutenKana : public DakutenKana {
public:
  template<typename... T> explicit HanDakutenKana(Kana&& hanDakuten, T&&...);

  [[nodiscard]] const Kana* hanDakuten() const override;
private:
  const AccentedKana _hanDakuten;
};

// A 'Monograph' is a single Kana character (large or small) and a 'Digraph' is
// a valid (at least typeable with standard IME) two Kana combination. Diagraphs
// are always a full sized Kana followed by a small Kana (one of the 5 vowels, 3
// y's or 'wa').

//// More details for 'ConvertFlags' enum ////
// - 'Hepburn': off by default, only applies to 'Rōmaji' output
//   convert("つづき", CharType::Romaji) -> tsuduki
//   convert("つづき", CharType::Romaji, Hepburn) -> tsuzuki
// - 'Kunrei': off by default, only applies to 'Rōmaji' output
//   convert("しつ", CharType::Romaji) -> shitsu
//   convert("しつ", CharType::Romaji, Kunrei) -> situ
// - 'NoProlongMark': off by default, only applies to 'Hiragana' output
//   convert("rāmen", CharType::Hiragana) -> らーめん
//   convert("rāmen", CharType::Hiragana, NoProlongMark) -> らあめん
// - 'RemoveSpaces': off by default, only applies when converting from Rōmaji:
//   convert("akai kitsune", CharType::Hiragana) -> あかい　きつね (wide space)
//   convert("akai kitsune", CharType::Hiragana, RemoveSpaces) -> あかいきつね
//
// Prolonged sound marks in Hiragana are non-standard, but output them by
// default in order to support round-trip type conversions, otherwise the above
// example would map "らあめん" back to "raamen" which doesn't match the initial
// value. ConvertFlags supports bitwise operators so they can be combined using
// '|', for example:
//  convert("rāmen desu.", CharType::Hiragana, ConvertFlags::RemoveSpaces |
//      ConvertFlags::NoProlongMark) -> らあめんです。
//
// Enabling 'Hepburn' leads to more standard Rōmaji, but the output is ambiguous
// and leads to different Kana if converted back. This affects di (ぢ), dya
// (ぢゃ), dyo (ぢょ), dyu (ぢゅ), du (づ) and wo (を) - these become ji, ja,
// ju, jo, zu and o instead. There's also no support for trying to handle は and
// へ (which in standard Hepburn should map to 'wa' and 'e' if they are used as
// particles) - instead they simply map to 'ha' and 'he' all the time. If both
// Hepburn and Kunrei flags are set then Hepburn is preferred, but will then try
// Kunrei before falling back to the unique '_romaji' value in the Kana class.

//// More details for 'Kana' class ////
// - '_romaji': usually holds the 'Modern Hepburn' value, but will sometimes be
//   a 'Nihon Shiki' value in order to ensure a unique value for Kana maps ('di'
//   for ぢ, 'du' for づ, etc.).
// - '_hepburn': holds an optional 'Modern Hepburn' value for a few cases where
//   it differs from the 'unique' Wāpuro Rōmaji. For example, づ can be uniquely
//   identified by 'du', but the correct Hepburn output for this Kana is 'zu'
//   which is ambiguous with ず. '_hepburn' (if it's populated) is always a
//   duplicate of another Kana's '_romaji' value.
// - '_kunrei': holds an optional 'Kunrei Shiki' value like 'zya' for じゃ.

} // namespace kanji_tools
