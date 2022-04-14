#pragma once

#include <kanji_tools/kanji/Radical.h>
#include <kanji_tools/kanji/UcdLinkTypes.h>

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

// 'Ucd' holds the data loaded from 'ucd.txt' which is an extract from the
// official Unicode 'ucd.all.flat.xml' file - see comments in
// scripts/parseUcdAllFlat.sh for more details.
class Ucd {
public:
  using Meaning = const std::string&;
  using Reading = Radical::Reading;
  using Strokes = u_int16_t;

  // max number of strokes and variant strokes in current 'ucd.txt' data
  inline static constexpr Strokes MaxStrokes{53}, MaxVariantStrokes{33};

  Ucd(const UcdEntry& entry, const std::string& block,
      const std::string& version, Radical::Number radical, Strokes strokes,
      Strokes variantStrokes, const std::string& pinyin,
      const std::string& morohashiId, const std::string& nelsonIds,
      const std::string& sources, const std::string& jSource, bool joyo,
      bool jinmei, const UcdLinks& links, Meaning meaning, Reading onReading,
      Reading kunReading)
      : _entry{entry}, _block{block}, _version{version}, _radical{radical},
        _strokes{strokes}, _variantStrokes{variantStrokes}, _pinyin{pinyin},
        _morohashiId{morohashiId}, _nelsonIds{nelsonIds}, _sources{sources},
        _jSource{jSource}, _joyo{joyo}, _jinmei{jinmei}, _links{links},
        _meaning{meaning}, _onReading{onReading}, _kunReading{kunReading} {}

  Ucd(const Ucd&) = delete;

  [[nodiscard]] auto& entry() const { return _entry; }
  [[nodiscard]] auto& block() const { return _block; }
  [[nodiscard]] auto& version() const { return _version; }
  [[nodiscard]] auto radical() const { return _radical; }
  [[nodiscard]] auto strokes() const { return _strokes; }
  [[nodiscard]] auto variantStrokes() const { return _variantStrokes; }
  [[nodiscard]] auto& pinyin() const { return _pinyin; }
  [[nodiscard]] auto& morohashiId() const { return _morohashiId; }
  [[nodiscard]] auto& nelsonIds() const { return _nelsonIds; }
  [[nodiscard]] auto& sources() const { return _sources; }
  [[nodiscard]] auto& jSource() const { return _jSource; }
  [[nodiscard]] auto joyo() const { return _joyo; }
  [[nodiscard]] auto jinmei() const { return _jinmei; }
  [[nodiscard]] auto& links() const { return _links; }
  [[nodiscard]] auto& meaning() const { return _meaning; }
  [[nodiscard]] auto& onReading() const { return _onReading; }
  [[nodiscard]] auto& kunReading() const { return _kunReading; }

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
  const UcdEntry _entry;
  const std::string _block, _version;
  const Radical::Number _radical;
  // _variantStrokes is 0 if no variants (see 'parseUcdAllFlat.sh')
  const Strokes _strokes, _variantStrokes;
  const std::string _pinyin, _morohashiId, _nelsonIds, _sources, _jSource;
  const bool _joyo, _jinmei;
  const UcdLinks _links;
  const std::string _meaning, _onReading, _kunReading;
};

using UcdPtr = const Ucd*;

} // namespace kanji_tools
