#pragma once

#include <kt_kana/KanaEnums.h>

#include <optional>
#include <vector>

namespace kanji_tools { /// \kana_group{Kana}
/// Kana class hierarchy

/// class that represents a 'Monograph' or 'Digraph' Kana value \kana{Kana}
///
/// A 'Monograph' is a single Kana character (large or small) and a 'Digraph' is
/// a valid (at least typeable with standard IMEs) two Kana combo. Diagraphs are
/// always a full sized Kana followed by a small Kana (5 vowels, 3 y's or 'wa').
/// See #_romaji, #_hepburn and #_kunrei members for more details.
class Kana {
public:
  using Map = std::map<String, const class Kana*>;
  using OptString = std::optional<String>;
  template<size_t N> using CharArray = const char (&)[N];

  inline static const OptString EmptyOptString; ///< empty OptString

  static constexpr uint16_t OneKanaSize{3}, ///< all Kana are 3 bytes UTF-8
      RomajiArrayMin{2}, ///< Rōmaji char array min size (including null)
      RomajiArrayMax{4}; ///< Rōmaji char array max size (including null)

  static constexpr uint16_t OneKanaArraySize{
      OneKanaSize + 1},             ///< Kana char array (including null)
      TwoKanaSize{OneKanaSize * 2}, ///< UTF-8 String containing two Kana
      TwoKanaArraySize{OneKanaSize * 2 + 1}, ///< char array size for two Kana
      RomajiStringMax{RomajiArrayMax - 1};   ///< max Rōmaji String size

  static const Kana& SmallTsu; ///< reference to 'small tsu' global instance
  static const Kana& N;        ///< reference to 'n' global instance

  /// Prolong Mark (ー) is officially in the Katakana Unicode block, but it can
  /// also occasionally appear in some (non-standard) Hiragana like らーめん.
  inline static const String ProlongMark{"ー"};

  /// return global Kana map for given #CharType
  [[nodiscard]] static const Map& getMap(CharType);

  /// find 'dakuten' version of `s` ("と" returns "ど", "セ" returns "ゼ", etc.)
  /// \param s should be a non-accented Hiragana or Katakana UTF-8 String
  /// \return 'dakuten' version of `s` or `std::nullopt` if not found
  [[nodiscard]] static OptString findDakuten(const String& s);

  /// find 'han-dakuten' version of `s` ("ひ" returns "ぴ", etc.)
  /// \param s should be a non-accented Hiragana or Katakana UTF-8 String
  /// \return 'han-dakuten' version of `s` or `std::nullopt` if not found
  [[nodiscard]] static OptString findHanDakuten(const String& s);

  /// holds any further variant Rōmaji values for a Kana object \kana{Kana}
  ///
  /// This includes IME key combos that map to the same value like 'kwa' for
  /// クァ (instead of 'qa'), 'fyi' フィ (instead of 'fi'), etc.. #_kunrei is
  /// true if the first entry in the list is 'Kunrei Shiki' (then Kana::_kunrei
  /// should be `std::nullopt`).
  class RomajiVariants final {
  public:
    using List = std::vector<String>;
    using RMax = CharArray<RomajiArrayMax>;

    RomajiVariants() = default; ///< default ctor (for an empty list)
    RomajiVariants(RomajiVariants&&) = default; ///< only allow moving

    /// ctor for one variant
    template<size_t R>
    explicit RomajiVariants(CharArray<R> r, bool kunrei = false);

    /// ctor for two variants, variants are same size like 'fa' (ファ) which has
    /// variants of 'fwa' and 'hwa'
    template<size_t R>
    RomajiVariants(CharArray<R> r1, CharArray<R> r2, bool kunrei = false);

    /// ctor for three variants, these instances never have `kunrei` true, but
    /// one has differing sizes so need two template params, i.e, small 'ぇ'
    /// with Rōmaji of 'le' has a variant list of 'xe', 'lye' and 'xye'
    template<size_t R> RomajiVariants(CharArray<R> r1, RMax r2, RMax r3);

    /// return list of variants
    [[nodiscard]] auto& list() const { return _list; }

    /// return true if the first variant is a 'kunrei' variant
    [[nodiscard]] auto kunrei() const { return _kunrei; }
  private:
    /// all Rōmaji variants are either 2 or 3 characters long
    template<size_t R> static consteval void check() {
      static_assert(R > RomajiArrayMin && R <= RomajiArrayMax);
    }

    List _list;
    bool _kunrei{false};
  };

  /// holds Kana iteration marks (一の字点) \kana{Kana}
  class IterationMark final {
  public:
    IterationMark(const IterationMark&) = delete; ///< deleted copy ctor

    /// return true if `s` is an iteration mark for type `t` \details
    /// \li `matches(CharType::Hiragana, "ゞ")` returns true
    /// \li `matches(CharType::Katakana, "ゞ")` returns false
    /// \li `matches(CharType::Katakana, "か")` returns false
    /// CharType::Romaji will always return false
    [[nodiscard]] bool matches(CharType t, const String& s) const;

    /// return the iteration mark for `target`, if `target` is Hiragana or
    /// Katakana then the corresponding data member is returned, otherwise a
    /// Rōmaji String is returned based on `flags` and `prevKana` \details
    /// \code
    ///   using enum CharType;
    ///   auto* prev{Kana::getMap(Romaji).find("tsu")->second};
    ///   auto flags{ConvertFlags::None};
    ///   Kana::RepeatPlain.get(Hiragana, flags, prev);  // returns "ゝ"
    ///   Kana::RepeatPlain.get(Romaji, flags, prev);    // returns "tsu"
    ///   Kana::RepeatAccented.get(Romaji, flags, prev); // returns "du"
    /// \endcode
    [[nodiscard]] const String& get(
        CharType target, ConvertFlags flags, const Kana* prevKana) const;

    /// return Hiragana iteration mark
    [[nodiscard]] auto& hiragana() const { return _hiragana; }

    /// return Katakana iteration mark
    [[nodiscard]] auto& katakana() const { return _katakana; }
  private:
    friend Kana; // only Kana class can construct
    IterationMark(CharArray<OneKanaArraySize> hiragana,
        CharArray<OneKanaArraySize> katakana, bool dakuten);

    void validate() const;

    const String _hiragana, _katakana;
    const bool _dakuten; ///< true if this instance is 'dakuten' (濁点) version
  };

  static const IterationMark RepeatPlain, ///< plain iteration marks: "ゝ", "ヽ"
      RepeatAccented; ///< accented iteration marks: "ゞ", "ヾ"

  /// return iteration mark or nullptr if `kana` isn't an iteration mark
  [[nodiscard]] static const IterationMark* findIterationMark(
      CharType, const String& kana);

  virtual ~Kana() = default;  ///< default dtor
  Kana(const Kana&) = delete; ///< deleted copy ctor

  /// DakutenKana return accented Kana, base class returns `nullptr`
  [[nodiscard]] virtual const Kana* dakuten() const;

  /// HanDakutenKana return accented Kana, base class returns `nullptr`
  [[nodiscard]] virtual const Kana* hanDakuten() const;

  /// return the unaccented version of this Kana or `nullptr` if this Kana is
  /// unaccented or is a combination that doesn't have an equivalent unaccented
  /// 'standard combination' such as 'va', 've', 'vo' (ヴォ), etc..
  ///
  /// \note ウォ can be typed with 'u' then 'lo', but is treated as two separate
  /// Kana instances ('u' and 'lo') instead of a plain version of 'vo'.
  [[nodiscard]] virtual const Kana* plain() const;

  /// return 'dakuten' string for the given #CharType or `std::nullopt` if this
  /// instance doesn't have a 'dakuten' version (like 'ma')
  [[nodiscard]] OptString dakuten(CharType) const;

  /// return 'han-dakuten' string for the given #CharType or `std::nullopt` if
  /// this instance doesn't have a 'han-dakuten' version (like 'ka')
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
  /// type is AccentedKana (and is contained in a HanDakutenKana object)
  [[nodiscard]] bool isHanDakuten() const;

  /// return Rōmaji value based on `flags`
  [[nodiscard]] const String& getRomaji(ConvertFlags flags) const;

  /// repeat the first letter of #_romaji for sokuon (促音) output (special
  /// handling for 't' as described in comments above).
  [[nodiscard]] String getSokuonRomaji(ConvertFlags) const;

  [[nodiscard]] const String& get(CharType, ConvertFlags) const;

  /// return true if `s` is equal to #_hiragana or #_katakana
  [[nodiscard]] bool containsKana(const String& s) const;

  [[nodiscard]] bool operator==(const Kana&) const; ///< equal operator

  [[nodiscard]] auto& romaji() const { return _romaji; }
  [[nodiscard]] auto& hiragana() const { return _hiragana; }
  [[nodiscard]] auto& katakana() const { return _katakana; }
  [[nodiscard]] auto& romajiVariants() const { return _variants.list(); }
  [[nodiscard]] auto kunreiVariant() const { return _variants.kunrei(); }

  /// instances of Kana classes are created in Kana.cpp - this ctor shouldn't be
  /// used anywhere else
  ///
  /// \tparam R size of `romaji` char array
  /// \tparam A size of `hiragana` and `katakana` char arrays
  /// \param romaji unique Rōmaji reading (see Kana class docs)
  /// \param hiragana UTF-8 Hiragana value
  /// \param katakana UTF-8 Katakana value
  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana);

  /// \doc Kana(CharArray<R>, CharArray<A>, CharArray<A>)
  /// \tparam H size of `hepburn` char array
  /// \tparam K size of `kunrei` char array
  /// \param hepburn Hepburn reading
  /// \param kunrei Kunrei reading
  template<size_t R, size_t A, size_t H, size_t K>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      CharArray<H> hepburn, CharArray<K> kunrei);

  /// \doc Kana(CharArray<R>, CharArray<A>, CharArray<A>)
  /// \param variants list of one or more Rōmaji variants
  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      RomajiVariants&& variants);
protected:
  /// move ctor used by AccentedKana - moves #_variants and copies other fields
  /// since they are 'const' (copy is fine since other fields are short strings
  /// that wouldn't benefit from move anyway because of SSO).
  Kana(Kana&&) = default;
private:
  template<size_t R, size_t A>
  Kana(CharArray<R> romaji, CharArray<A> hiragana, CharArray<A> katakana,
      const char* hepburn, const char* kunrei, RomajiVariants&&);

  static Map populate(CharType);
  static const Map RomajiMap, HiraganaMap, KatakanaMap;

  /// called by ctors to make sure data is valid (via asserts) such as ensuring
  /// #_hiragana is actually valid Hiragana, etc..
  void validate() const;

  /// usually 'Modern Hepburn', but sometimes 'Nihon Shiki' ('di' for ぢ, 'du'
  /// for づ, etc.) to ensure uniqueness (for keys and round-trip processing)
  const String _romaji;

  const String _hiragana; ///< Hiragana value
  const String _katakana; ///< Katakana value

  /// 'Modern Hepburn' is only populated if it's not the same as #_romaji. For
  /// example, づ can be uniquely identified by 'du', but the Hepburn value for
  /// this Kana is 'zu' which is ambiguous with ず. If #_hepburn is populated it
  /// will always be a duplicate of another Kana's #_romaji value.
  const OptString _hepburn;

  /// 'Kunrei Shiki' is only populated if it's not the same as #_romaji (like
  /// 'zya' for じゃ) and this instance doesn't have any entries in #_variants
  const OptString _kunrei;

  /// list of Rōmaji variants (non-const to allow moving)
  RomajiVariants _variants;
};

/// class for Kana that have voiced versions \kana{Kana}
///
/// This class has instances for all Kana (Monograph and Digraph) in 'k', 's',
/// 't', and 'h' rows as well as 'u'. For example 'ka' has #_dakuten of 'ga',
/// 'sha' has 'ja', 'u' has 'vu', etc.. romaji(), hiragana(), etc. members of
/// this class have unaccented values (like 'ka') and #_dakuten has the accented
/// values (like 'ga').
class DakutenKana : public Kana {
public:
  /// `dakuten` should be a Kana object with accented values (like 'ga') and the
  /// base class ctor is called with the remaining parameters in `T`
  template<typename... T> explicit DakutenKana(Kana&& dakuten, T&&...);

  /// return #_dakuten Kana (which is an instance of AccentedKana)
  [[nodiscard]] const Kana* dakuten() const final;
protected:
  /// represents an accented Kana \kana{Kana}
  ///
  /// This class is used by both DakutenKana and HanDakutenKana classes to hold
  /// the related accented versions of Kana values.
  class AccentedKana final : public Kana {
  public:
    /// move ctor that moves `k` into base class fields and sets #_plain to `p`
    AccentedKana(Kana&& k, const Kana& p);

    /// return #_plain Kana
    [[nodiscard]] const Kana* plain() const final;
  private:
    /// populated by unaccented version by DakutenKana and HanDakutenKana, for
    /// example DakutenKana instance for け contains #_dakuten げ and in turn,
    /// げ will have #_plain set to the original け to allow lookup both ways
    const Kana& _plain;
  };
private:
  const AccentedKana _dakuten;
};

/// class for Kana that have semi-voiced versions (so the 'h' row) \kana{Kana}
///
/// This class derives from DakutenKana since 'h' row Kana have both voiced and
/// semi-voiced, i.e., 'ha' (は) has semi-voiced 'pa' (ぱ) and voiced 'ba' (ば).
class HanDakutenKana final : public DakutenKana {
public:
  /// `hanDakuten` should be a Kana object with accented values (like 'pa') and
  /// the base class ctor is called with the remaining parameters in `T`
  template<typename... T> explicit HanDakutenKana(Kana&& hanDakuten, T&&...);

  /// return #_hanDakuten Kana (which is an instance of AccentedKana)
  [[nodiscard]] const Kana* hanDakuten() const final;
private:
  const AccentedKana _hanDakuten;
};

/// \end_group
} // namespace kanji_tools
