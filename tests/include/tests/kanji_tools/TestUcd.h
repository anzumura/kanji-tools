#pragma once

#include <kanji_tools/kanji/Ucd.h>

namespace kanji_tools {

// 'TestUcd' creates an empty 'Ucd' instance via a conversion operator, but any
// field can be overridden before creating, for example:
//   const Ucd ucd{TestUcd{"龍"}.jinmei(true)};
class TestUcd {
private:
  template<typename T> auto& set(T& x, const T& y) {
    x = y;
    return *this;
  }
public:
  using Code = UcdEntry::Code;
  using Links = Ucd::Links;
  using Meaning = Ucd::Meaning;
  using Name = UcdEntry::Name;
  using Reading = Ucd::Reading;
  using Strokes = Ucd::Strokes;

  // allow setting 'name' via the ctor since it's the more commonly used field
  TestUcd(Name name = {}) : _name(name) {}

  // conversion opterator to create a Ucd instance
  [[nodiscard]] operator Ucd() const {
    return Ucd{{_code, _name}, _block, _version, _radical, _strokes,
        _variantStrokes, _pinyin, _morohashiId, _nelsonIds, _sources, _jSource,
        _joyo, _jinmei, _links, _linkType, _linkedReadings, _meaning,
        _onReading, _kunReading};
  }

  auto& code(Code x) { return set(_code, x); }
  auto& name(Name x) { return set(_name, x); }
  auto& block(const std::string& x) { return set(_block, x); }
  auto& version(const std::string& x) { return set(_version, x); }
  auto& radical(Radical::Number x) { return set(_radical, x); }
  auto& strokes(Strokes x) { return set(_strokes, x); }
  auto& variantStrokes(Strokes x) { return set(_variantStrokes, x); }
  auto& pinyin(const std::string& x) { return set(_pinyin, x); }
  auto& morohashiId(const std::string& x) { return set(_morohashiId, x); }
  auto& nelsonIds(const std::string& x) { return set(_nelsonIds, x); }
  auto& sources(const std::string& x) { return set(_sources, x); }
  auto& jSource(const std::string& x) { return set(_jSource, x); }
  auto& joyo(bool x) { return set(_joyo, x); }
  auto& jinmei(bool x) { return set(_jinmei, x); }
  auto& links(const Links& x) {
    _links.clear();
    std::copy(x.begin(), x.end(), std::back_inserter(_links));
    return *this;
  }
  auto& linkType(UcdLinkTypes x) { return set(_linkType, x); }
  auto& linkedReadings(bool x) { return set(_linkedReadings, x); }
  auto& meaning(const std::string& x) { return set(_meaning, x); }
  auto& onReading(const std::string& x) { return set(_onReading, x); }
  auto& kunReading(const std::string& x) { return set(_kunReading, x); }
  // compound setters
  auto& ids(const std::string& m, const std::string& n) {
    return morohashiId(m).nelsonIds(n);
  }
  auto& sources(const std::string& s, const std::string& j) {
    return sources(s).jSource(j);
  }
  auto& links(const Links& x, UcdLinkTypes t) { return links(x).linkType(t); }
  auto& readings(Reading on, Reading kun) {
    return onReading(on).kunReading(kun);
  }
  auto& meaningAndReadings(Meaning m, Reading on, Reading kun) {
    return meaning(m).readings(on, kun);
  }
private:
  Code _code{};
  std::string _name, _block, _version;
  Radical::Number _radical{};
  Strokes _strokes{}, _variantStrokes{};
  std::string _pinyin, _morohashiId, _nelsonIds, _sources, _jSource;
  bool _joyo{}, _jinmei{};
  Links _links;
  UcdLinkTypes _linkType{UcdLinkTypes::None};
  bool _linkedReadings{};
  std::string _meaning, _onReading, _kunReading;
};

} // namespace kanji_tools
