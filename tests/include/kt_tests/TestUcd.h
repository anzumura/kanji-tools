#pragma once

#include <kt_kanji/Ucd.h>
#include <kt_utils/Utf8.h>

namespace kanji_tools {

/// creates an empty 'Ucd' object via a conversion operator, but any
/// field can be overridden before creating \details for example:
/// \code
///   const Ucd ucd{TestUcd{"龍"}.jinmei(true)};
/// \endcode
class TestUcd final {
private:
  template<typename T> auto& set(T& x, const T& y) {
    x = y;
    return *this;
  }
public:
  using Links = Ucd::Links;
  using Meaning = Ucd::Meaning;
  using Name = Radical::Name;
  using Reading = Ucd::Reading;

  // allow setting 'name' via the ctor since it's the more commonly used field
  explicit TestUcd(Name name = "一") : _name(name) {}

  // conversion operator to create a Ucd object
  [[nodiscard]] explicit operator Ucd() const {
    return Ucd{{_code ? _code : getCode(_name), _name}, _block, _version,
        _radical,
        _variantStrokes ? Strokes{_strokes, _variantStrokes}
                        : Strokes{_strokes},
        _pinyin, _morohashiId, _nelsonIds, _sources, _jSource, _joyo, _jinmei,
        _links, _linkType, _meaning, _onReading, _kunReading};
  }

  auto& code(Code x) { return set(_code, x); }
  auto& name(Name x) { return set(_name, x); }
  auto& block(const String& x) { return set(_block, x); }
  auto& version(const String& x) { return set(_version, x); }
  auto& pinyin(const String& x) { return set(_pinyin, x); }
  auto& linkType(Ucd::LinkTypes x) { return set(_linkType, x); }
  auto& links(const Links& x) {
    _links.clear();
    std::copy(x.begin(), x.end(), std::back_inserter(_links));
    return *this;
  }
  auto& radical(Radical::Number x) { return set(_radical, x); }
  auto& strokes(Strokes::Size x) { return set(_strokes, x); }
  auto& variantStrokes(Strokes::Size x) { return set(_variantStrokes, x); }
  auto& morohashiId(const String& x) { return set(_morohashiId, x); }
  auto& nelsonIds(const String& x) { return set(_nelsonIds, x); }
  auto& jSource(const String& x) { return set(_jSource, x); }
  auto& meaning(const String& x) { return set(_meaning, x); }
  auto& onReading(const String& x) { return set(_onReading, x); }
  auto& kunReading(const String& x) { return set(_kunReading, x); }
  auto& sources(const String& x) { return set(_sources, x); }
  auto& joyo(bool x) { return set(_joyo, x); }
  auto& jinmei(bool x) { return set(_jinmei, x); }
  // compound setters
  auto& ids(const String& m, const String& n) {
    return morohashiId(m).nelsonIds(n);
  }
  auto& sources(const String& s, const String& j) {
    return sources(s).jSource(j);
  }
  auto& links(const Links& x, Ucd::LinkTypes t) { return links(x).linkType(t); }
  auto& readings(Reading on, Reading kun) {
    return onReading(on).kunReading(kun);
  }
  auto& meaningAndReadings(Meaning m, Reading on, Reading kun) {
    return meaning(m).readings(on, kun);
  }
private:
  Code _code{};
  String _name, _block, _version, _pinyin;
  Ucd::LinkTypes _linkType{Ucd::LinkTypes::None};
  Links _links;
  Radical::Number _radical{};
  Strokes::Size _strokes{1}, _variantStrokes{};
  String _morohashiId, _nelsonIds, _sources, _jSource;
  String _meaning, _onReading, _kunReading;
  bool _joyo{}, _jinmei{};
};

} // namespace kanji_tools
