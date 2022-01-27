#ifndef KANJI_TOOLS_KANJI_UCD_H
#define KANJI_TOOLS_KANJI_UCD_H

#include <iostream>
#include <string>
#include <vector>

namespace kanji_tools {

// 'Ucd' holds the data loaded from 'ucd.txt' which is an extract from the official Unicode
// 'ucd.all.flat.xml' file - see comments in scripts/parseUcdAllFlat.sh for more details.
class Ucd {
public:
  // LinkTypes represents how a Ucd link was loaded (from which XML property - see parse script for details)
  enum class LinkTypes { Compatibility, Definition, Jinmei, Semantic, Simplified, Traditional, None };
  static LinkTypes toLinkType(const std::string&);
  static const std::string& toString(LinkTypes);

  class Link {
  public:
    Link(wchar_t code, const std::string& name) : _code(code), _name(name) {}
    auto code() const { return _code; }
    auto& name() const { return _name; }
    std::string codeAndName() const;
  private:
    const wchar_t _code;
    const std::string _name;
  };
  using Links = std::vector<Link>;

  Ucd(wchar_t code, const std::string& name, const std::string& block, const std::string& version, int radical,
      int strokes, int variantStrokes, const std::string& pinyin, const std::string& morohashiId,
      const std::string& nelsonIds, bool joyo, bool jinmei, const Links& links, LinkTypes linkType, bool linkedReadings,
      const std::string& meaning, const std::string& onReading, const std::string& kunReading)
    : _code(code), _name(name), _block(block), _version(version), _radical(radical), _strokes(strokes),
      _variantStrokes(variantStrokes), _pinyin(pinyin), _morohashiId(morohashiId), _nelsonIds(nelsonIds), _joyo(joyo),
      _jinmei(jinmei), _links(links), _linkType(linkType), _linkedReadings(linkedReadings), _meaning(meaning),
      _onReading(onReading), _kunReading(kunReading) {}

  auto code() const { return _code; }
  auto& name() const { return _name; }
  auto& block() const { return _block; }
  auto& version() const { return _version; }
  auto radical() const { return _radical; }
  auto strokes(bool variant = false) const { return _strokes; }
  auto variantStrokes() const { return _variantStrokes; }
  auto& pinyin() const { return _pinyin; }
  auto& morohashiId() const { return _morohashiId; }
  auto& nelsonIds() const { return _nelsonIds; }
  auto joyo() const { return _joyo; }
  auto jinmei() const { return _jinmei; }
  auto& links() const { return _links; }
  auto linkType() const { return _linkType; }
  auto linkedReadings() const { return _linkedReadings; }
  auto& meaning() const { return _meaning; }
  auto& onReading() const { return _onReading; }
  auto& kunReading() const { return _kunReading; }
  // 'has' methods
  auto hasLinks() const { return !_links.empty(); }
  auto hasTraditionalLinks() const { return _linkType == LinkTypes::Traditional; }
  auto hasNonTraditionalLinks() const { return hasLinks() && _linkType != LinkTypes::Traditional; }
  auto hasVariantStrokes() const { return _variantStrokes != 0; }
  // 'getStrokes' will try to retrun '_variantStrokes' if it exists (and if variant is true), otherise
  // it falls back to just return '_strokes'
  auto getStrokes(bool variant) const { return variant && hasVariantStrokes() ? _variantStrokes : _strokes; }
  // 'codeAndName' methods return the Unicode in square brackets plus the name, e.g.: [FA30] 侮
  std::string codeAndName() const;
  std::string linkCodeAndNames() const;
  // 'EmptyString' can be returned by 'linkCodeAndName' and is used by other classes as well
  static const std::string EmptyString;
private:
  const wchar_t _code;
  const std::string _name;
  const std::string _block;
  const std::string _version;
  const int _radical;
  const int _strokes;
  const int _variantStrokes; // 0 if there are no variants (see 'parseUcdAllFlat.sh' for more details)
  const std::string _pinyin;
  const std::string _morohashiId;
  const std::string _nelsonIds;
  const bool _joyo;
  const bool _jinmei;
  const Links _links;
  const LinkTypes _linkType;
  const bool _linkedReadings;
  const std::string _meaning;
  const std::string _onReading;
  const std::string _kunReading;
};

inline auto& operator<<(std::ostream& os, const Ucd::LinkTypes& x) { return os << Ucd::toString(x); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_UCD_H
