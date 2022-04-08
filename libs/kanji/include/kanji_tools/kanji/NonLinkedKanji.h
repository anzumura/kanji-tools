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
  // used by 'UcdFileKanji' and 'ExtraKanji' to populate links from Ucd data
  [[nodiscard]] static LinkNames linkNames(const Ucd*);

  // ctor used by 'CustomFileKanji': has 'meaning' and 'reading'
  NonLinkedKanji(const Data&, const std::string& name, const Radical&,
      const std::string& meaning, const std::string& reading, Strokes,
      const Ucd*);

  // ctor used by 'CustomFileKanji' and 'UcdFileKanji': has 'reading' and
  // looks up 'meaning' from 'ucd.txt'
  NonLinkedKanji(const Data&, const std::string& name, const Radical&,
      const std::string& reading, Strokes strokes, const Ucd*);
private:
  const std::string _meaning;
  const std::string _reading;
};

} // namespace kanji_tools
