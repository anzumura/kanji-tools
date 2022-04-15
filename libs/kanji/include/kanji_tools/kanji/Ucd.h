#pragma once

#include <kanji_tools/kanji/Radical.h>
#include <kanji_tools/kanji/UcdLinkTypes.h>
#include <kanji_tools/utils/Symbol.h>

#include <bitset>

namespace kanji_tools {

class UcdEntry {
public:
  using Code = char32_t;
  using Name = Radical::Name;

  UcdEntry(Code code, Name name) : _code{code}, _name{name} {}

  [[nodiscard]] auto code() const { return _code; }
  [[nodiscard]] auto& name() const { return _name; }

  // 'codeAndName' return Unicode in brackets plus the name, e.g.: [FA30] 侮
  [[nodiscard]] std::string codeAndName() const;
private:
  const Code _code;
  const std::string _name;
};

class UcdLinks {
public:
  using Links = std::vector<UcdEntry>;

  UcdLinks(const Links& links, UcdLinkTypes type, bool linkedReadings)
      : _links{links}, _type{type}, _linkedReadings{linkedReadings} {}

  [[nodiscard]] auto& links() const { return _links; }
  [[nodiscard]] auto type() const { return _type; }
  [[nodiscard]] auto linkedReadings() const { return _linkedReadings; }

  [[nodiscard]] auto empty() const { return _links.empty(); }
  [[nodiscard]] auto size() const { return _links.size(); }
  [[nodiscard]] std::string codeAndNames() const;
private:
  const Links _links;
  const UcdLinkTypes _type;
  const bool _linkedReadings;
};

class UcdBlock : public Symbol<UcdBlock> {
public:
  UcdBlock(const std::string& s) : Symbol<UcdBlock>{s} {}
};
template<> inline const std::string Symbol<UcdBlock>::Type{"UcdBlock"};

class UcdVersion : public Symbol<UcdVersion> {
public:
  UcdVersion(const std::string& s) : Symbol<UcdVersion>{s} {}
};
template<> inline const std::string Symbol<UcdVersion>::Type{"UcdVersion"};

class Pinyin : public Symbol<Pinyin> {
public:
  Pinyin(const std::string& s) : Symbol<Pinyin>{s} {}
};
template<> inline const std::string Symbol<Pinyin>::Type{"Pinyin"};

// 'Ucd' holds data loaded from 'ucd.txt' which is an extract of the Unicode
// 'ucd.all.flat.xml' file - see scripts/parseUcdAllFlat.sh for more details
class Ucd {
public:
  using Meaning = const std::string&;
  using Reading = Radical::Reading;
  using Strokes = u_int16_t;

  // max number of strokes and variant strokes in current 'ucd.txt' data
  inline static constexpr Strokes MaxStrokes{53}, MaxVariantStrokes{33};

  Ucd(const UcdEntry&, const std::string& block, const std::string& version,
      Radical::Number, Strokes strokes, Strokes variantStrokes,
      const std::string& pinyin, const std::string& morohashiId,
      const std::string& nelsonIds, const std::string& sources,
      const std::string& jSource, bool joyo, bool jinmei, const UcdLinks&,
      Meaning, Reading onReading, Reading kunReading);

  Ucd(const Ucd&) = delete;

  [[nodiscard]] auto& entry() const { return _entry; }
  [[nodiscard]] auto& block() const { return _block.name(); }
  [[nodiscard]] auto& version() const { return _version.name(); }
  [[nodiscard]] auto& pinyin() const { return _pinyin.name(); }
  [[nodiscard]] auto radical() const { return _radical; }
  [[nodiscard]] auto strokes() const { return _strokes; }
  [[nodiscard]] auto variantStrokes() const { return _variantStrokes; }
  [[nodiscard]] auto& morohashiId() const { return _morohashiId; }
  [[nodiscard]] auto& nelsonIds() const { return _nelsonIds; }
  [[nodiscard]] auto& jSource() const { return _jSource; }
  [[nodiscard]] auto& links() const { return _links; }
  [[nodiscard]] auto& meaning() const { return _meaning; }
  [[nodiscard]] auto& onReading() const { return _onReading; }
  [[nodiscard]] auto& kunReading() const { return _kunReading; }

  // values for these fields are stored in _sources bitset;
  [[nodiscard]] std::string sources() const;
  [[nodiscard]] bool joyo() const;
  [[nodiscard]] bool jinmei() const;

  // 'has' methods
  [[nodiscard]] bool hasLinks() const;
  [[nodiscard]] bool hasTraditionalLinks() const;
  [[nodiscard]] bool hasNonTraditionalLinks() const;
  [[nodiscard]] bool hasVariantStrokes() const;
  // helper methods
  [[nodiscard]] auto& name() const { return _entry.name(); }
  [[nodiscard]] auto code() const { return _entry.code(); }
  [[nodiscard]] auto linkType() const { return _links.type(); }
  [[nodiscard]] std::string codeAndName() const;
  [[nodiscard]] std::string linkCodeAndNames() const;
private:
  // there are 6 source types (G, H, J, K, T, V) plus joyo and jinmei flags
  static constexpr auto SourcesSize{8};

  [[nodiscard]] static std::bitset<SourcesSize> getSources(
      const std::string& sources, bool joyo, bool jinmei);

  const UcdEntry _entry;
  const UcdBlock _block;
  const UcdVersion _version;
  const Pinyin _pinyin;
  const Radical::Number _radical;
  // _variantStrokes is 0 if no variants (see 'parseUcdAllFlat.sh')
  const Strokes _strokes, _variantStrokes;
  const std::string _morohashiId, _nelsonIds, _jSource;
  const UcdLinks _links;
  const std::string _meaning, _onReading, _kunReading;
  const std::bitset<SourcesSize> _sources;
};

using UcdPtr = const Ucd*;

} // namespace kanji_tools
