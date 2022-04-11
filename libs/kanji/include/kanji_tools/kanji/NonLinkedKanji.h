#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'NonLinkedKanji' contains meaning and reading fields and is the base class
// for CustomFileKanji (an abstract base class for JouyouKanji, JinmeiKanji and
// ExtraKanji), FrequencyKanji, KenteiKanji and UcdKanji
class NonLinkedKanji : public Kanji {
public:
  [[nodiscard]] Meaning meaning() const override { return _meaning; }
  [[nodiscard]] Reading reading() const override { return _reading; }
protected:
  // used by 'UcdFileKanji' and 'ExtraKanji' to populate links from Ucd data
  [[nodiscard]] static LinkNames linkNames(UcdPtr);

  // ctor used by 'CustomFileKanji'
  NonLinkedKanji(DataRef data, Name name, RadicalRef radical, Strokes strokes,
      Meaning meaning, Reading reading, UcdPtr u)
      : Kanji{data, name, radical, strokes, u}, _meaning{meaning},
        _reading{reading} {}

  // ctor used by 'CustomFileKanji' and 'UcdFileKanji': looks up 'meaning' and
  // 'strokes' from 'ucd.txt'
  NonLinkedKanji(
      DataRef data, Name name, RadicalRef radical, Reading reading, UcdPtr u)
      : NonLinkedKanji{data, name, radical, data.ucdStrokes(name, u),
            data.ucd().getMeaning(u), reading, u} {}
private:
  const std::string _meaning;
  const std::string _reading;
};

} // namespace kanji_tools
