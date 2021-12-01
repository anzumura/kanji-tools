#ifndef KANJI_TOOLS_KANJI_UCD_H
#define KANJI_TOOLS_KANJI_UCD_H

#include <string>

namespace kanji_tools {

// 'Ucd' holds the data loaded from 'ucd.txt' which is an extract from the official Unicode
// 'ucd.all.flat.xml' file - see comments in scripts/parseUcdAllFlat.sh for more details.
class Ucd {
public:
  Ucd(wchar_t code, const std::string& name, const std::string& block, const std::string& version, int radical,
      int strokes, int variantStrokes, const std::string& pinyin, const std::string& morohashiId,
      const std::string& nelsonIds, bool joyo, bool jinmei, wchar_t linkCode, const std::string& linkName,
      const std::string& linkType, const std::string& meaning, const std::string& onReading,
      const std::string& kunReading)
    : _code(code), _name(name), _block(block), _version(version), _pinyin(pinyin), _morohashiId(morohashiId),
      _nelsonIds(nelsonIds), _radical(radical), _strokes(strokes), _variantStrokes(variantStrokes), _joyo(joyo),
      _jinmei(jinmei), _linkCode(linkCode), _linkName(linkName), _linkType(linkType), _meaning(meaning),
      _onReading(onReading), _kunReading(kunReading) {}

  wchar_t code() const { return _code; }
  const std::string& name() const { return _name; }
  const std::string& block() const { return _block; }
  const std::string& version() const { return _version; }
  int radical() const { return _radical; }
  int strokes(bool variant = false) const { return _strokes; }
  int variantStrokes() const { return _variantStrokes; }
  const std::string& pinyin() const { return _pinyin; }
  const std::string& morohashiId() const { return _morohashiId; }
  const std::string& nelsonIds() const { return _nelsonIds; }
  bool joyo() const { return _joyo; }
  bool jinmei() const { return _jinmei; }
  wchar_t linkCode() const { return _linkCode; }
  const std::string& linkName() const { return _linkName; }
  const std::string& linkType() const { return _linkType; }
  const std::string& meaning() const { return _meaning; }
  const std::string& onReading() const { return _onReading; }
  const std::string& kunReading() const { return _kunReading; }
  // 'has' methods
  bool hasLink() const { return _linkCode != 0; }
  bool hasVariantStrokes() const { return _variantStrokes != 0; }
  // 'getStrokes' will try to retrun '_variantStrokes' if it exists (and if variant is true), otherise
  // it falls back to just return '_strokes'
  int getStrokes(bool variant) const { return variant && hasVariantStrokes() ? _variantStrokes : _strokes; }
  // 'codeAndName' methods return the Unicode in square brackets plus the name, e.g.: [FA30] 侮
  std::string codeAndName() const;
  std::string linkCodeAndName() const;
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
  const wchar_t _linkCode; // 0 for 'no link'
  const std::string _linkName;
  const std::string _linkType;
  const std::string _meaning;
  const std::string _onReading;
  const std::string _kunReading;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_UCD_H
