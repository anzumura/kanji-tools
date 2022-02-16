#ifndef KANJI_TOOLS_KANJI_UCD_H
#define KANJI_TOOLS_KANJI_UCD_H

#include <kanji_tools/kanji/UcdLinkTypes.h>

#include <vector>

namespace kanji_tools {

// 'Ucd' holds the data loaded from 'ucd.txt' which is an extract from the official Unicode
// 'ucd.all.flat.xml' file - see comments in scripts/parseUcdAllFlat.sh for more details.
class Ucd {
public:
  class Link {
  public:
    Link(char32_t code, const std::string& name) : _code(code), _name(name) {}
    [[nodiscard]] auto code() const { return _code; }
    [[nodiscard]] auto& name() const { return _name; }
    [[nodiscard]] std::string codeAndName() const;
  private:
    const char32_t _code;
    const std::string _name;
  };
  using Links = std::vector<Link>;

  Ucd(char32_t code, const std::string& name, const std::string& block, const std::string& version, int radical,
      int strokes, int variantStrokes, const std::string& pinyin, const std::string& morohashiId,
      const std::string& nelsonIds, bool joyo, bool jinmei, const Links& links, UcdLinkTypes linkType,
      bool linkedReadings, const std::string& meaning, const std::string& onReading, const std::string& kunReading)
    : _code(code), _name(name), _block(block), _version(version), _radical(radical), _strokes(strokes),
      _variantStrokes(variantStrokes), _pinyin(pinyin), _morohashiId(morohashiId), _nelsonIds(nelsonIds), _joyo(joyo),
      _jinmei(jinmei), _links(links), _linkType(linkType), _linkedReadings(linkedReadings), _meaning(meaning),
      _onReading(onReading), _kunReading(kunReading) {}

  [[nodiscard]] auto code() const { return _code; }
  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto& block() const { return _block; }
  [[nodiscard]] auto& version() const { return _version; }
  [[nodiscard]] auto radical() const { return _radical; }
  [[nodiscard]] auto strokes() const { return _strokes; }
  [[nodiscard]] auto variantStrokes() const { return _variantStrokes; }
  [[nodiscard]] auto& pinyin() const { return _pinyin; }
  [[nodiscard]] auto& morohashiId() const { return _morohashiId; }
  [[nodiscard]] auto& nelsonIds() const { return _nelsonIds; }
  [[nodiscard]] auto joyo() const { return _joyo; }
  [[nodiscard]] auto jinmei() const { return _jinmei; }
  [[nodiscard]] auto& links() const { return _links; }
  [[nodiscard]] auto linkType() const { return _linkType; }
  [[nodiscard]] auto linkedReadings() const { return _linkedReadings; }
  [[nodiscard]] auto& meaning() const { return _meaning; }
  [[nodiscard]] auto& onReading() const { return _onReading; }
  [[nodiscard]] auto& kunReading() const { return _kunReading; }
  // 'has' methods
  [[nodiscard]] auto hasLinks() const { return !_links.empty(); }
  [[nodiscard]] auto hasTraditionalLinks() const { return _linkType == UcdLinkTypes::Traditional; }
  [[nodiscard]] auto hasNonTraditionalLinks() const { return hasLinks() && _linkType != UcdLinkTypes::Traditional; }
  [[nodiscard]] auto hasVariantStrokes() const { return _variantStrokes != 0; }
  // 'getStrokes' will try to retrun '_variantStrokes' if it exists (and if variant is true), otherise
  // it falls back to just return '_strokes'
  [[nodiscard]] auto getStrokes(bool variant) const {
    return variant && hasVariantStrokes() ? _variantStrokes : _strokes;
  }
  // 'codeAndName' methods return the Unicode in square brackets plus the name, e.g.: [FA30] 侮
  [[nodiscard]] std::string codeAndName() const;
  [[nodiscard]] std::string linkCodeAndNames() const;
  // 'EmptyString' can be returned by 'linkCodeAndName' and is used by other classes as well
  static const std::string EmptyString;
private:
  const char32_t _code;
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
  const UcdLinkTypes _linkType;
  const bool _linkedReadings;
  const std::string _meaning;
  const std::string _onReading;
  const std::string _kunReading;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_UCD_H
