#pragma once

#include <kanji_tools/kanji/Ucd.h>

namespace kanji_tools {

// 'TestUcd' creates a 'Ucd' instance for Kanji '一' by default, but any field
// can be overridden before calling 'create', for example:
//   TestUcd{}.name("龍").joyo(false).create();
class TestUcd {
private:
  template<typename T> auto& set(T& x, const T& y) {
    x = y;
    return *this;
  }

  char32_t _code{U'\x4e00'};
  std::string _name{"一"}, _block{"CJK"}, _version{"1.1"};
  Radical::Number _radical{1};
  Ucd::Strokes _strokes{1}, _variantStrokes{0};
  std::string _pinyin{"yī"}, _morohashiId{"1"}, _nelsonIds{"1"},
      _sources{"GHJKTV"}, _jSource{"J0-306C"};
  bool _joyo{true}, _jinmei{false};
  Ucd::Links _links{};
  UcdLinkTypes _linkType{UcdLinkTypes::None};
  bool _linkedReadings{false};
  std::string _meaning{"one; a, an; alone"}, _onReading{"ICHI ITSU"},
      _kunReading{"HITOTSU HITOTABI HAJIME"};
public:
  [[nodiscard]] Ucd create() const {
    return Ucd{_code, _name, _block, _version, _radical, _strokes,
        _variantStrokes, _pinyin, _morohashiId, _nelsonIds, _sources, _jSource,
        _joyo, _jinmei, _links, _linkType, _linkedReadings, _meaning,
        _onReading, _kunReading};
  }

  auto& code(char32_t x) { return set(_code, x); }
  auto& name(const std::string& x) { return set(_name, x); }
  auto& block(const std::string& x) { return set(_block, x); }
  auto& version(const std::string& x) { return set(_version, x); }
  auto& radical(Radical::Number x) { return set(_radical, x); }
  auto& strokes(Ucd::Strokes x) { return set(_strokes, x); }
  auto& variantStrokes(Ucd::Strokes x) { return set(_variantStrokes, x); }
  auto& pinyin(const std::string& x) { return set(_pinyin, x); }
  auto& morohashiId(const std::string& x) { return set(_morohashiId, x); }
  auto& nelsonIds(const std::string& x) { return set(_nelsonIds, x); }
  auto& sources(const std::string& x) { return set(_sources, x); }
  auto& jSource(const std::string& x) { return set(_jSource, x); }
  auto& joyo(bool x) { return set(_joyo, x); }
  auto& jinmei(bool x) { return set(_jinmei, x); }
  auto& links(const Ucd::Links& x) {
    _links.clear();
    std::copy(x.begin(), x.end(), std::back_inserter(_links));
    return *this;
  }
  auto& linkType(UcdLinkTypes x) { return set(_linkType, x); }
  auto& linkedReadings(bool x) { return set(_linkedReadings, x); }
  auto& meaning(const std::string& x) { return set(_meaning, x); }
  auto& onReading(const std::string& x) { return set(_onReading, x); }
  auto& kunReading(const std::string& x) { return set(_kunReading, x); }
};

} // namespace kanji_tools
