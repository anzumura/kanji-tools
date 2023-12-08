#pragma once

#include <kt_kanji/MorohashiId.h>
#include <kt_kanji/Radical.h>
#include <kt_kanji/Strokes.h>
#include <kt_utils/EnumList.h>
#include <kt_utils/Symbol.h>

namespace kanji_tools { /// \kanji_group{Ucd}
/// Pinyin and Ucd classes for data loaded from Unicode UCD data

/// holds a 'hànyǔ pīnyīn' (漢語拼音) from 'kMandarin' XML property \kanji{Ucd}
///
/// There are currently 1,337 unique Pinyin values so Symbol is a good fit. This
/// class is used as a member data field in both Ucd and Kanji classes.
class Pinyin final : public Symbol<Pinyin> {
public:
  inline static const String Type{"Pinyin"};
  using Symbol::Symbol;
};

/// holds data loaded from 'ucd.txt' \kanji{Ucd}
///
/// 'ucd.txt' is an extract of some XML properties in Unicode 'ucd.all.flat.xml'
/// file - see scripts/parseUcdAllFlat.sh for more details
class Ucd final {
public:
  /// represent the XML property from which the link was loaded \details '_R'
  /// means the link was also used to pull in readings. The script uses '*' for
  /// reading links so '*' has also been used in the names in 'AllUcdLinkTypes'.
  /// '_R' are first to allow a '<' comparison to find all reading links. Note,
  /// there is no non '_R' type for 'Semantic' by design.
  enum class LinkTypes : Enum::Size {
    Compatibility_R, ///< *kCompatibilityVariant* link also used for 'reading'
    Definition_R,    ///< *kDefinition based* link and also used for 'reading'
    Jinmei_R,        ///< *kJinmeiyoKanji* link also used for 'reading'
    Semantic_R,      ///< *kSemanticVariant* link also used for 'reading'
    Simplified_R,    ///< *kSimplifiedVariant* link also used for 'reading'
    Traditional_R,   ///< *kTraditionalVariant* link also used for 'reading'
    Compatibility,   ///< *kCompatibilityVariant* link
    Definition,      ///< *kDefinition based* link
    Jinmei,          ///< *KJinmeiyoKanji* link
    Simplified,      ///< *kSimplifiedVariant* link
    Traditional,     ///< *kTraditionalVariant* link
    None             ///< no link
  };

  /// holds the String name of an entry from 'ucd.txt' file \kanji{Ucd}
  class Entry final {
  public:
    using Name = Radical::Name;

    /// ctor for creating an Entry from 'ucd.txt' data
    /// \param code UTF-32 code point
    /// \param name UTF-8 String value
    /// \throw DomainError if `name` is not in a recognized Unicode Kanji block
    ///     or if `code` is not the correct Unicode value for `name`
    Entry(Code code, Name name);

    /// return UTF-32 Code calculated from UTF-8 name()
    [[nodiscard]] Code code() const;

    /// return UTF-8 String
    [[nodiscard]] auto& name() const { return _name; }

    /// return Unicode in brackets plus the name, e.g.: [FA30] 侮
    [[nodiscard]] String codeAndName() const;

  private:
    const String _name;
  };

  /// Unicode (short) block name from 'blk' XML property \kanji{Ucd}
  class Block final : public Symbol<Block> {
  public:
    inline static const String Type{"Ucd::Block"};
    using Symbol::Symbol;
  };

  /// Unicode version name from 'age' XML property \kanji{Ucd}
  class Version final : public Symbol<Version> {
  public:
    inline static const String Type{"Ucd::Version"};
    using Symbol::Symbol;
  };

  using Links = std::vector<Entry>;
  using Meaning = const String&;
  using Reading = Radical::Reading;

  /// create a Ucd object, see scripts/parseUcdAllFlat.sh for details on fields
  Ucd(const Entry&, const String& block, const String& version, Radical::Number,
      Strokes, const String& pinyin, const String& morohashiId,
      const String& nelsonIds, const String& sources, const String& jSource,
      bool joyo, bool jinmei, Links, LinkTypes, Meaning, Reading onReading,
      Reading kunReading);

  Ucd(const Ucd&) = delete; ///< deleted copy ctor

  [[nodiscard]] auto& entry() const { return _entry; }
  [[nodiscard]] auto& block() const { return _block; }
  [[nodiscard]] auto& version() const { return _version; }
  [[nodiscard]] auto& pinyin() const { return _pinyin; }
  [[nodiscard]] auto linkType() const { return _linkType; }
  [[nodiscard]] auto radical() const { return _radical; }
  [[nodiscard]] auto strokes() const { return _strokes; }
  [[nodiscard]] auto& morohashiId() const { return _morohashiId; }
  [[nodiscard]] auto& links() const { return _links; }
  [[nodiscard]] auto& nelsonIds() const { return _nelsonIds; }
  [[nodiscard]] auto& jSource() const { return _jSource; }
  [[nodiscard]] auto& meaning() const { return _meaning; }
  [[nodiscard]] auto& onReading() const { return _onReading; }
  [[nodiscard]] auto& kunReading() const { return _kunReading; }

  /// values for these fields are stored as bits in #_sources data member @{
  [[nodiscard]] String sources() const;
  [[nodiscard]] bool joyo() const;
  [[nodiscard]] bool jinmei() const; ///@}

  [[nodiscard]] bool hasLinks() const;
  [[nodiscard]] bool hasTraditionalLinks() const;
  [[nodiscard]] bool hasNonTraditionalLinks() const;

  [[nodiscard]] auto code() const { return _entry.code(); }
  [[nodiscard]] auto& name() const { return _entry.name(); }
  [[nodiscard]] bool linkedReadings() const;
  [[nodiscard]] String codeAndName() const;
  [[nodiscard]] String linkCodeAndNames() const;

private:
  [[nodiscard]] static uint8_t getSources(
      const String& sources, bool joyo, bool jinmei);

  const Entry _entry;
  const Block _block;
  const Version _version;
  const Pinyin _pinyin;
  const uint8_t _sources;
  const LinkTypes _linkType;
  const Radical::Number _radical;
  const Strokes _strokes;
  const MorohashiId _morohashiId;
  const Links _links;
  const String _nelsonIds, _jSource, _meaning, _onReading, _kunReading;
};

using UcdPtr = const Ucd*;

/// enable Ucd::LinkTypes to be used in an EnumList
template <> inline constexpr auto is_enumlist_with_none<Ucd::LinkTypes>{true};
/// create an EnumList for Ucd::LinkTypes
inline const auto AllUcdLinkTypes{
    BaseEnumList<Ucd::LinkTypes>::create("Compatibility*", "Definition*",
        "Jinmei*", "Semantic*", "Simplified*", "Traditional*", "Compatibility",
        "Definition", "Jinmei", "Simplified", "Traditional")};

/// \end_group
} // namespace kanji_tools
