#ifndef KANJI_UCD_DATA_H
#define KANJI_UCD_DATA_H

#include <kanji/KanaConvert.h>
#include <kanji/Ucd.h>

#include <filesystem>
#include <iostream>
#include <map>

namespace kanji {

// 'UcdData': holds data loaded from Unicode UCD XML
class UcdData {
public:
  using Map = std::map<std::string, Ucd>;

  UcdData() {}
  UcdData(const UcdData&) = delete;

  // 'getMeaning' returns the 'meaning' loaded from UCD file for given 's'. Almost all kanji
  // from UCD have meanings, but a few are empty. Also, return empty string if not found.
  const std::string& getMeaning(const std::string& s) const {
    auto u = find(s);
    return u ? u->meaning() : Ucd::EmptyString;
  }
  // 'getReadingsAsKana' finds the UCD kanji for 's' and returns one string starting with
  // 'onReading' converted Katakana followed by 'kunReading' converted to Hiragana.
  std::string getReadingsAsKana(const std::string& s) const;
  // 'findUcd' will return a pointer to a Ucd instance if 's' is in _ucdMap. If 's' has a
  // 'variation selector' then _ucdLinkedJinmei then _ucdLinkedOther maps are used to get
  // a Ucd variant (the variant returned is the same displayed character for Jinmei ones)
  const Ucd* find(const std::string& s) const;
  const Map& map() const { return _map; }
  // 'load' and 'printStats' are called by 'KanjiData'
  void load(const std::filesystem::path&);
  void printStats() const;
  // 'toInt' is a helper method used during file loading
  static int toInt(const std::string& s) {
    try {
      return std::stoi(s);
    } catch (...) {
      throw std::invalid_argument("failed to convert to int: " + s);
    }
  }
private:
  static void usage(const std::string& msg) { throw std::domain_error(msg); }
  std::ostream& out() const { return std::cout; }
  std::ostream& log() const { return out() << ">>> "; }

  Map _map;
  // '_ucdLinked...' are maps from standard Kanji to variant forms loaded from 'ucd.txt'
  // For example, FA67 (逸) is a variant of 9038 (逸) which can also be constructed by a
  // variation selector, i.e., L"\u9038\uFE01" (逸︁). Note:
  // - if a variant is marked as 'Jinmei' it will be put in '_ucdLinkedJinmei'
  // - otherwise it will be put in '_ucdLinkedOther'
  std::map<std::string, std::string> _linkedJinmei;
  std::map<std::string, std::string> _linkedOther;
  // '_converter' is used by 'ucdReadingsToKana' to convert the Romaji readings loaded in from
  // UCD to Katakana and Hiragana.
  mutable KanaConvert _converter;
};

} // namespace kanji

#endif // KANJI_UCD_DATA_H
