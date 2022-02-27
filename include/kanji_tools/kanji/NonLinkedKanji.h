#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'NonLinkedKanji' contains meaning and reading fields and is the base class
// for CustomFileKanji (an abstract base class for JouyouKanji, JinmeiKanji and
// ExtraKanji), FrequencyKanji, KenteiKanji and UcdKanji
class NonLinkedKanji : public Kanji {
public:
  [[nodiscard]] const std::string& meaning() const override { return _meaning; }
  [[nodiscard]] const std::string& reading() const override { return _reading; }
protected:
  // used by UcdFileKanji and ExtraKanji to populate links from Ucd data
  [[nodiscard]] static auto getLinkNames(const Ucd* u) {
    LinkNames result;
    if (u && u->hasLinks())
      std::transform(u->links().begin(), u->links().end(),
                     std::back_inserter(result),
                     [](const auto& i) { return i.name(); });
    return result;
  }

  // constructor used by 'CustomFileKanji': has 'meaning' and 'reading'
  NonLinkedKanji(const Data& d, const std::string& name, const Radical& radical,
                 const std::string& meaning, const std::string& reading,
                 int strokes, const Ucd* u)
      : Kanji(name, d.getCompatibilityName(name), radical, strokes,
              d.getMorohashiId(u), d.getNelsonIds(u), d.getPinyin(u)),
        _meaning(meaning), _reading(reading) {}

  // constructor used by 'CustomFileKanji' and 'UcdFileKanji': has 'reading' and
  // looks up 'meaning'
  NonLinkedKanji(const Data& d, const std::string& name, const Radical& rad,
                 const std::string& reading, int strokes, const Ucd* u)
      : NonLinkedKanji(d, name, rad, d.ucd().getMeaning(u), reading, strokes,
                       u) {}
private:
  const std::string _meaning;
  const std::string _reading;
};

} // namespace kanji_tools
