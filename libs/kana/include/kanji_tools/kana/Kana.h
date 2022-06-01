#pragma once

#include <kanji_tools/utils/Bitmask.h>
#include <kanji_tools/utils/EnumList.h>

#include <optional>
#include <vector>

namespace kanji_tools { /// \kana_group{Kana}
/// Kana class hierarchy and associated #CharType and #ConvertFlags enums

/// used by Converter class for 'source' and 'target' types when converting as
/// well as Kana::get(), Kana::IterationMark::get() and other functions
enum class CharType : Enum::Size {
  Hiragana, ///< Japanese Hiragana (平仮名) syllable script
  Katakana, ///< Japanese Katakana (片仮名) syllable script
  Romaji    ///< Rōmaji (ローマ字), Japanese written in Latin script
};
/// enable #CharType to be used in an EnumList
template<> inline constexpr auto is_enumlist<CharType>{true};
/// create an EnumList for #CharType
inline const auto CharTypes{
    BaseEnumList<CharType>::create("Hiragana", "Katakana", "Romaji")};

/// used by Converter class to control some aspects of conversion
///
/// \details Here are some examples of how these flags affect conversion:
/// \code
///   using enum ConvertFlags;
///   // Hepburn: only affects Rōmaji output
///   convert("つづき", CharType::Romaji);          // "tsuduki"
///   convert("つづき", CharType::Romaji, Hepburn); // "tsuzuki"
///   // Kunrei: only affects Rōmaji output
///   convert("しつ", CharType::Romaji);         // "shitsu"
///   convert("しつ", CharType::Romaji, Kunrei); // "situ"
///   // ProlongMark: only affects Hiragana output
///   convert("rāmen", CharType::Hiragana);                // "らーめん"
///   convert("rāmen", CharType::Hiragana, NoProlongMark); // "らあめん"
///   // RemoveSpaces: only applies when converting from Rōmaji
///   convert("akai kitsune", CharType::Hiragana); // output has a wide space
///   convert("akai kitsune", CharType::Hiragana, RemoveSpaces); // no spaces
/// \endcode
///
/// Prolonged sound marks in Hiragana are non-standard, but use by default in
/// order to support round-trip type conversions, otherwise "rāmen" would map to
/// "らあめん" which would map back to "raamen" (not the initial value).
///
/// ConvertFlags support bitwise operators so they can be combined, for example:
/// \code
///   convert("rāmen desu.", CharType::Hiragana, RemoveSpaces | NoProlongMark);
/// \endcode
/// results in "らあめんです。"
///
/// \note Enabling 'Hepburn' flag for get() and convert() functions results in
/// *more standard* Rōmaji, but the output is ambiguous and leads to different
/// Kana if converted back. This affects di (ぢ), dya (ぢゃ), dyo (ぢょ), dyu
/// (ぢゅ), du (づ) and wo (を) - these become ji, ja, ju, jo, zu and o instead.
/// There's also no support for trying to handle は and へ (which in standard
/// Hepburn should map to 'wa' and 'e' if they are used as particles) - instead
/// they always map to 'ha' and 'he'. If both Hepburn and Kunrei flags are set
/// then Hepburn is preferred, but will then try Kunrei before falling back to
/// the unique '_romaji' value in the Kana class.
enum class ConvertFlags : Enum::Size {
  None,              ///< no value (the default)
  Hepburn,           ///< use Hepburn style Rōmaji
  Kunrei,            ///< use Kunrei style Rōmaji
  NoProlongMark = 4, ///< don't use ProlongMark (ー) in Hiragana output
  RemoveSpaces = 8   ///< remove spaces in %Kana output
};
/// enable bitwise operators for #ConvertFlags
template<> inline constexpr auto is_bitmask<ConvertFlags>{true};

/// class that represents a ('Monograph' or 'Digraph') Kana value \kana{Kana}
///
/// A 'Monograph' is a single Kana character (large or small) and a 'Digraph' is
/// a valid (at least typeable with standard IME) two Kana combo. Diagraphs are
/// always a full sized Kana character followed by a small Kana (one of the 5
/// vowels, 3 y's or 'wa').
///
/// Note on some attributes:
/// \li `_romaji`: usually holds the 'Modern Hepburn' value, but will sometimes
/// be a 'Nihon Shiki' value in order to ensure a unique value for Kana maps
/// ('di' for ぢ, 'du' for づ, etc.).
/// \li `_hepburn`: holds an optional 'Modern Hepburn' value for a few cases
/// where it differs from the 'unique' Wāpuro Rōmaji. For example, づ can be
/// uniquely identified by 'du', but the correct Hepburn output for this Kana is
/// 'zu' which is ambiguous with ず. If '_hepburn' is populated it will always
/// be a duplicate of another Kana's '_romaji' value.
/// \li `_kunrei`: holds an optional 'Kunrei Shiki' value like 'zya' for じゃ.
class Kana {
public:
  using Map = std::map<String, const class Kana*>;
  using OptString = std::optional<String>;
  template<size_t N> using CharArray = const char (&)[N];

  inline static const OptString EmptyOptString;
  /// all Kana are 3 bytes UTF-8 values
  static constexpr uint16_t OneKanaSize{3};
  /// a char array is one larger to hold final null value @{
  static constexpr uint16_t OneKanaArraySize{OneKanaSize + 1},
      TwoKanaSize{OneKanaSize * 2}, TwoKanaArraySize{OneKanaSize * 2 + 1}; ///@}
  /// Rōmaji string and array sizes @{
  static constexpr uint16_t RomajiArrayMinSize{2}, RomajiArrayMaxSize{4};
  static constexpr uint16_t RomajiStringMaxSize{RomajiArrayMaxSize - 1}; ///@}

  static const Kana& SmallTsu; ///< reference to 'small tsu' global instance
  static const Kana& N;        ///< reference to 'n' global instance

  /// Prolong Mark (ー) is officially in the Katakana Unicode block, but it can
  /// also occasionally appear in some (non-standard) Hiragana like らーめん.
  inline static const String ProlongMark{"ー"};

  /// return global Kana map for given #CharType
  [[nodiscard]] static const Map& getMap(CharType);

  /// find corresponding DakutenKana, `s` should be a non-accented single
  /// Hiragana or Katakana letter
  [[nodiscard]] static OptString findDakuten(const String& s);

  /// find corresponding HanDakutenKana, `s` should be a non-accented single
  /// Hiragana or Katakana letter
  [[nodiscard]] static OptString findHanDakuten(const String& s);

  /// holds any further variant Rōmaji values for a Kana object \kana{Kana}
  ///
  /// This includes key combos that map to the same value like 'kwa' for  クァ
  /// (instead of 'qa'), 'fyi' フィ (instead of 'fi'), etc.. `_kunrei` is true
  /// if the first entry in the list is 'Kunrei Shiki' (then `Kana::_kunrei`
  /// should be nullopt).
  class RomajiVariants {
  public:
    using List = std::vector<String>;
    using RMax = CharArray<RomajiArrayMaxSize>;

    RomajiVariants() = default;                 ///< default ctor
    RomajiVariants(RomajiVariants&&) = default; ///< only allow moving

    template<size_t R>
    explicit RomajiVariants(CharArray<R> r, bool kunrei = false);

    /// all instances with two variants have variants with the same size (like
    /// 'fa' (ファ) which has Rōmaji variants of 'fwa' and 'hwa')
    template<size_t R>
    RomajiVariants(CharArray<R> r1, CharArray<R> r2, bool kunrei = false);

    /// no instance with three variants has `kunrei` true, but one has differing
    /// sizes so need two template params, i.e, small 'ぇ' with Rōmaji of 'le'
    /// has a variant list of 'xe', 'lye' and 'xye'
    template<size_t R> RomajiVariants(CharArray<R> r1, RMax r2, RMax r3);

    [[nodiscard]] auto& list() const { return _list; }
    [[nodiscard]] auto kunrei() const { return _kunrei; }
  private:
    /// all Rōmaji variants are either 2 or 3 characters long
    template<size_t R> static consteval void check() {
      static_assert(R > RomajiArrayMinSize && R <= RomajiArrayMaxSize);
    }

    List _list;
    bool _kunrei{false};
  };

  /// holds Kana iteration marks (一の字点) \kana{Kana}
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

  /// return iteration mark or nullptr if `kana` isn't an iteration mark
  [[nodiscard]] static const IterationMark* findIterationMark(
      CharType, const String& kana);

  virtual ~Kana() = default;  ///< default dtor
  Kana(const Kana&) = delete; ///< deleted copy ctor

  /// DakutenKana overrides to return accented Kana, base returns `nullptr`
  [[nodiscard]] virtual const Kana* dakuten() const;

  /// HanDakutenKana overrides to return accented Kana, base returns `nullptr`
  [[nodiscard]] virtual const Kana* hanDakuten() const;

  /// return the unaccented version of this Kana or `nullptr` if this Kana is
  /// unaccented or is a combination that doesn't have an equivalent unaccented
  /// 'standard combination' such as 'va', 've', 'vo' (ヴォ), etc.. \note ウォ
  /// can be typed with 'u' then 'lo', but is treated as two separate Kana
  /// instances ('u' and 'lo') instead of a plain version of 'vo'.
  [[nodiscard]] virtual const Kana* plain() const;

  [[nodiscard]] OptString dakuten(CharType) const;

  [[nodiscard]] OptString hanDakuten(CharType) const;

  /// return true if this is a small Kana (small Kana are also all 'Monographs')
  [[nodiscard]] bool isSmall() const;
  /// return true if this is a Monograph (single UTF-8 character Kana object)
  [[nodiscard]] bool isMonograph() const;
  /// return true if this is a Digraph (two UTF-8 character Kana object)
  [[nodiscard]] bool isDigraph() const;

  /// return true if this is a 'dakuten' (voiced) Kana, i.e., 'this' type is
  /// AccentedKana (and is contained in a DakutenKana object)
  [[nodiscard]] bool isDakuten() const;
  /// return true if this is a 'han-dakuten' (semi-voiced) Kana, i.e., 'this'
  /// type is AccentedKana (and is contained in a DakutenKana object)
  [[nodiscard]] bool isHanDakuten() const;

  /// return Rōmaji value based on `flags`
  [[nodiscard]] const String& getRomaji(ConvertFlags flags) const;

  /// repeat the first letter of #_romaji for sokuon (促音) output (special
  /// handling for 't' as described in comments above).
  [[nodiscard]] String getSokuonRomaji(ConvertFlags) const;

  [[nodiscard]] const String& get(CharType, ConvertFlags) const;

  /// return true if `s` is equal to #_hiragana or #_katakana
  [[nodiscard]] bool containsKana(const String& s) const;

  [[nodiscard]] bool operator==(const Kana&) const;

  [[nodiscard]] auto& romaji() const { return _romaji; }
  [[nodiscard]] auto& hiragana() const { return _hiragana; }
  [[nodiscard]] auto& katakana() const { return _katakana; }
  [[nodiscard]] auto& romajiVariants() const { return _variants.list(); }
  [[nodiscard]] auto kunreiVariant() const { return _variants.kunrei(); }

  /// all supported instances of Kana and its derived classes are created in
  /// 'Kana.cpp' (the following ctors shouldn't be used anywhere else) @{
  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana);
  template<size_t R, size_t A, size_t H, size_t K>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      CharArray<H> hepburn, CharArray<K> kunrei);
  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      RomajiVariants&&); ///@}
protected:
  /// move ctor used by AccentedKana to move the non-const '_variants' field.
  /// other fields will get copied because they are 'const' (copy is fine since
  /// the other fields are all short strings that would not benefit from move
  /// anyway because of SSO).
  Kana(Kana&&) = default;
private:
  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      const char* hepburn, const char* kunrei, RomajiVariants&&);

  static Map populate(CharType);
  static const Map RomajiMap, HiraganaMap, KatakanaMap;

  /// called by ctors to make sure data is valid (via asserts) such as ensuring
  /// '_hiragana' is actually valid Hiragana, etc..
  void validate() const;

  const String _romaji, _hiragana, _katakana;
  const OptString _hepburn, _kunrei;

  RomajiVariants _variants; ///< non-const to allow moving
};

/// class for Kana with voiced versions \kana{Kana}
///
/// This class has instances for all Monograph and Digraph versions of 'k', 's',
/// 't', and 'h' row Kana, e.g., 'ka' (か) has 'ga' (が), plus 'u' (which has
/// 'vu'). Members of this class hold the unaccented values (like 'ka') and the
/// `_dakuten` member holds the accented value (like 'ga').
class DakutenKana : public Kana {
public:
  /// `dakuten` should be Kana object with accented values (like 'ga') and the
  /// base class ctor is called with the remaining parameters in `T`
  template<typename... T> explicit DakutenKana(Kana&& dakuten, T&&...);

  /// return 'dakuten' Kana
  [[nodiscard]] const Kana* dakuten() const override;
protected:
  /// represents an accented Kana \kana{Kana}
  ///
  /// This class is used by both DakutenKana and HanDakutenKana classes to hold
  /// the related accented versions of Kana values.
  class AccentedKana : public Kana {
  public:
    /// move ctor that moves `k` into base class fields and sets '_plain' to `p`
    AccentedKana(Kana&& k, const Kana& p);

    /// return plain Kana
    [[nodiscard]] const Kana* plain() const override;
  private:
    /// unaccented version by DakutenKana and HanDakutenKana ctors, for example,
    /// the DakutenKana instance for け contains `_dakuten` Kana げ and in turn,
    /// げ will have `_plain` set to the original け to allow lookup both ways
    const Kana& _plain;
  };
private:
  const AccentedKana _dakuten;
};

/// class for Kana with semi-voiced versions (so only 'h' row) \kana{Kana}
///
/// This class derives from DakutenKana since 'h' row Kana have both voiced and
/// semi-voiced, i.e., 'ha' (は) has semi-voiced 'pa' (ぱ) and voiced 'ba' (ば).
class HanDakutenKana : public DakutenKana {
public:
  /// `hanDakuten` should be Kana object with accented values (like 'pa') and
  /// the base class ctor is called with the remaining parameters in `T`
  template<typename... T> explicit HanDakutenKana(Kana&& hanDakuten, T&&...);

  /// return 'han-dakuten' Kana
  [[nodiscard]] const Kana* hanDakuten() const override;
private:
  const AccentedKana _hanDakuten;
};

/// \end_group
} // namespace kanji_tools
