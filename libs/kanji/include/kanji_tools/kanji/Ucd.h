#pragma once

#include <kanji_tools/kanji/MorohashiId.h>
#include <kanji_tools/kanji/Radical.h>
#include <kanji_tools/kanji/Strokes.h>
#include <kanji_tools/utils/EnumList.h>
#include <kanji_tools/utils/Symbol.h>

namespace kanji_tools {

// 'UcdEntry' is used to hold the name of an entry from 'ucd.txt' file. The file
// contains both Unicode 'Code' and the UTF-8 string value (having both values
// makes searching and cross-referencing easier), but only the string value is
// stored in this class.
class UcdEntry {
public:
  using Name = Radical::Name;

  // throws an exception if 'name' is not a recognized Kanji or if 'code' is not
  // the correct Unicode value for 'name'
  UcdEntry(Code code, Name name);

  [[nodiscard]] Code code() const; // returns 'Code' calculated from '_name'
  [[nodiscard]] auto& name() const { return _name; }

  // 'codeAndName' return Unicode in brackets plus the name, e.g.: [FA30] 侮
  [[nodiscard]] String codeAndName() const;
private:
  const String _name;
};

class UcdBlock : public Symbol<UcdBlock> {
public:
  inline static const String Type{"UcdBlock"};
  using Symbol::Symbol;
};

class UcdVersion : public Symbol<UcdVersion> {
public:
  inline static const String Type{"UcdVersion"};
  using Symbol::Symbol;
};

class Pinyin : public Symbol<Pinyin> {
public:
  inline static const String Type{"Pinyin"};
  using Symbol::Symbol;
};

// 'Ucd' holds data loaded from 'ucd.txt' which is an extract of the Unicode
// 'ucd.all.flat.xml' file - see scripts/parseUcdAllFlat.sh for more details
class Ucd {
public:
  using Links = std::vector<UcdEntry>;
  using Meaning = const String&;
  using Reading = Radical::Reading;

  // 'LinkTypes' represent the XML property from which the link was loaded -
  // see parseUcdAllFlat.sh for details. '_R' means the link was also used to
  // pull in readings. The script uses '*' for reading links so '*' has also
  // been used in 'AllUcdLinkTypes' EnumList). Put _R first to allow a '<'
  // comparision to find all reading links. Note, there is no non '_R' type for
  // 'Semantic' by design.
  enum class LinkTypes : EnumContainer::Size {
    Compatibility_R,
    Definition_R,
    Jinmei_R,
    Semantic_R,
    Simplified_R,
    Traditional_R,
    Compatibility,
    Definition,
    Jinmei,
    Simplified,
    Traditional,
    None
  };

  Ucd(const UcdEntry&, const String& block, const String& version,
      Radical::Number, Strokes, const String& pinyin, const String& morohashiId,
      const String& nelsonIds, const String& sources, const String& jSource,
      bool joyo, bool jinmei, Links, LinkTypes, Meaning, Reading onReading,
      Reading kunReading);

  Ucd(const Ucd&) = delete;

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

  // values for these fields are stored in bits of '_sources';
  [[nodiscard]] String sources() const;
  [[nodiscard]] bool joyo() const;
  [[nodiscard]] bool jinmei() const;

  // 'has' methods
  [[nodiscard]] bool hasLinks() const;
  [[nodiscard]] bool hasTraditionalLinks() const;
  [[nodiscard]] bool hasNonTraditionalLinks() const;
  // helper methods
  [[nodiscard]] auto code() const { return _entry.code(); }
  [[nodiscard]] auto& name() const { return _entry.name(); }
  [[nodiscard]] bool linkedReadings() const;
  [[nodiscard]] String codeAndName() const;
  [[nodiscard]] String linkCodeAndNames() const;
private:
  [[nodiscard]] static unsigned char getSources(
      const String& sources, bool joyo, bool jinmei);

  const UcdEntry _entry;
  const UcdBlock _block;
  const UcdVersion _version;
  const Pinyin _pinyin;
  const unsigned char _sources;
  const LinkTypes _linkType;
  const Radical::Number _radical;
  const Strokes _strokes;
  const MorohashiId _morohashiId;
  const Links _links;
  const String _nelsonIds, _jSource, _meaning, _onReading, _kunReading;
};

using UcdPtr = const Ucd*;

template<> inline constexpr auto is_enumlist_with_none<Ucd::LinkTypes>{true};
inline const auto AllUcdLinkTypes{
    BaseEnumList<Ucd::LinkTypes>::create("Compatibility*", "Definition*",
        "Jinmei*", "Semantic*", "Simplified*", "Traditional*", "Compatibility",
        "Definition", "Jinmei", "Simplified", "Traditional")};

} // namespace kanji_tools
