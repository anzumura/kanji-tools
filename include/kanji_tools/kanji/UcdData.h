#ifndef KANJI_TOOLS_KANJI_UCD_DATA_H
#define KANJI_TOOLS_KANJI_UCD_DATA_H

#include <kanji_tools/kana/KanaConvert.h>
#include <kanji_tools/kanji/Ucd.h>

#include <filesystem>

namespace kanji_tools {

// 'UcdData': holds data loaded from Unicode UCD XML
class UcdData {
public:
  using Map = std::map<std::string, Ucd>;

  UcdData() {}
  UcdData(const UcdData&) = delete;

  // 'getMeaning' returns 'meaning' loaded from UCD file
  const std::string& getMeaning(const std::string& kanjiName) const { return getMeaning(find(kanjiName)); }
  const std::string& getMeaning(const Ucd* u) const { return u ? u->meaning() : Ucd::EmptyString; }

  // 'getReadingsAsKana' returns string starting with 'onReading' converted Katakana followed
  // by 'kunReading' converted to Hiragana
  std::string getReadingsAsKana(const Ucd*) const;

  // 'find' returns a pointer to a Ucd instance if 'kanjiName' is in _ucdMap. If 'kanjiName'
  // has a 'variation selector' then _linkedJinmei then _linkedOther maps are used to get a
  // Ucd variant (variant returned is the same displayed character for Jinmei ones)
  const Ucd* find(const std::string& kanjiName) const;

  const Map& map() const { return _map; }

  // 'load' and 'print' are called by 'KanjiData'
  void load(const std::filesystem::path&);
  void print(const class Data&) const;
private:
  void printVariationSelectorKanji(const Data&) const;

  Map _map;

  // '_linked...' are maps from standard Kanji to variant forms loaded from 'ucd.txt'
  // For example, FA67 (逸) is a variant of 9038 (逸) which can also be constructed by a
  // variation selector, i.e., L"\u9038\uFE01" (逸︁). Note:
  // - if a variant is marked as 'Jinmei' it will be put in '_linkedJinmei'
  // - otherwise it will be put in '_linkedOther'
  std::map<std::string, std::string> _linkedJinmei;
  std::map<std::string, std::vector<std::string>> _linkedOther;

  // '_converter' is used by 'getReadingsAsKana' to convert the Romaji readings loaded
  // from UCD (Katakana for 'onReading' and Hiragana for 'kunReading').
  mutable KanaConvert _converter;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_UCD_DATA_H
