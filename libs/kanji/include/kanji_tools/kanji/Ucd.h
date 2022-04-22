#pragma once

#include <kanji_tools/kanji/MorohashiId.h>
#include <kanji_tools/kanji/Radical.h>
#include <kanji_tools/kanji/UcdLinkTypes.h>
#include <kanji_tools/utils/Symbol.h>

namespace kanji_tools {

// 'UcdEntry' is used to hold the name of an entry from 'ucd.txt' file. The file
// contains both Unicode 'Code' and the UTF-8 string value (having both values
// makes searching and cross-referencing easier), but only the string value is
// stored in this class.
class UcdEntry {
public:
  using Code = char32_t;
  using Name = Radical::Name;

  // throws an exception if 'name' is not a recognized Kanji or if 'code' is not
  // the correct Unicode value for 'name'
  UcdEntry(Code code, Name name);

  [[nodiscard]] Code code() const; // returns 'Code' calculated from '_name'
  [[nodiscard]] auto& name() const { return _name; }

  // 'codeAndName' return Unicode in brackets plus the name, e.g.: [FA30] 侮
  [[nodiscard]] std::string codeAndName() const;
private:
  const std::string _name;
};

class UcdBlock : public Symbol<UcdBlock> {
public:
  inline static const std::string Type{"UcdBlock"};
  using Symbol::Symbol;
};

class UcdVersion : public Symbol<UcdVersion> {
public:
  inline static const std::string Type{"UcdVersion"};
  using Symbol::Symbol;
};

class Pinyin : public Symbol<Pinyin> {
public:
  inline static const std::string Type{"Pinyin"};
  using Symbol::Symbol;
};

// 'Ucd' holds data loaded from 'ucd.txt' which is an extract of the Unicode
// 'ucd.all.flat.xml' file - see scripts/parseUcdAllFlat.sh for more details
class Ucd {
public:
  using Links = std::vector<UcdEntry>;
  using Meaning = const std::string&;
  using Reading = Radical::Reading;
  using Strokes = u_int16_t;

  // max number of strokes and variant strokes in current 'ucd.txt' data
  inline static constexpr Strokes MaxStrokes{53}, MaxVariantStrokes{33};

  Ucd(const UcdEntry&, const std::string& block, const std::string& version,
      Radical::Number, Strokes strokes, Strokes variantStrokes,
      const std::string& pinyin, const std::string& morohashiId,
      const std::string& nelsonIds, const std::string& sources,
      const std::string& jSource, bool joyo, bool jinmei, const Links&,
      UcdLinkTypes, Meaning, Reading onReading, Reading kunReading);

  Ucd(const Ucd&) = delete;

  [[nodiscard]] auto& entry() const { return _entry; }
  [[nodiscard]] auto& block() const { return _block; }
  [[nodiscard]] auto& version() const { return _version; }
  [[nodiscard]] auto& pinyin() const { return _pinyin; }
  [[nodiscard]] auto linkType() const { return _linkType; }
  [[nodiscard]] auto radical() const { return _radical; }
  [[nodiscard]] auto strokes() const { return _strokes; }
  [[nodiscard]] auto variantStrokes() const { return _variantStrokes; }
  [[nodiscard]] auto& morohashiId() const { return _morohashiId; }
  [[nodiscard]] auto& links() const { return _links; }
  [[nodiscard]] auto& nelsonIds() const { return _nelsonIds; }
  [[nodiscard]] auto& jSource() const { return _jSource; }
  [[nodiscard]] auto& meaning() const { return _meaning; }
  [[nodiscard]] auto& onReading() const { return _onReading; }
  [[nodiscard]] auto& kunReading() const { return _kunReading; }

  // values for these fields are stored in bits of '_sources';
  [[nodiscard]] std::string sources() const;
  [[nodiscard]] bool joyo() const;
  [[nodiscard]] bool jinmei() const;

  // 'has' methods
  [[nodiscard]] bool hasVariantStrokes() const;
  [[nodiscard]] bool hasLinks() const;
  [[nodiscard]] bool hasTraditionalLinks() const;
  [[nodiscard]] bool hasNonTraditionalLinks() const;
  // helper methods
  [[nodiscard]] auto code() const { return _entry.code(); }
  [[nodiscard]] auto& name() const { return _entry.name(); }
  [[nodiscard]] bool linkedReadings() const;
  [[nodiscard]] std::string codeAndName() const;
  [[nodiscard]] std::string linkCodeAndNames() const;
private:
  [[nodiscard]] static unsigned char getSources(
      const std::string& sources, bool joyo, bool jinmei);

  const UcdEntry _entry;
  const UcdBlock _block;
  const UcdVersion _version;
  const Pinyin _pinyin;
  const unsigned char _sources;
  const UcdLinkTypes _linkType;
  const Radical::Number _radical;
  // _variantStrokes is 0 if no variants (see 'parseUcdAllFlat.sh')
  const Strokes _strokes, _variantStrokes;
  const MorohashiId _morohashiId;
  const Links _links;
  const std::string _nelsonIds, _jSource, _meaning, _onReading, _kunReading;
};

using UcdPtr = const Ucd*;

} // namespace kanji_tools
