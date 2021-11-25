#ifndef KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H
#define KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'NonLinkedKanji' contains meaning and reading fields and is the base class for FileKanji (an abstract
// base class for JouyouKanji, JinmeiKanji and ExtraKanji), OtherKanji, KenteiKanji and UcdKanji
class NonLinkedKanji : public Kanji {
public:
  const std::string& meaning() const override { return _meaning; }
  const std::string& reading() const override { return _reading; }
protected:
  // constructors called by OtherKanji, KenteiKanji and UcdKanji classes
  NonLinkedKanji(const Data& d, int number, const std::string& name, const std::string& reading, const Ucd* u,
                 bool findFrequency = true, bool findKyu = true)
    : Kanji(number, name, d.getCompatibilityName(name), d.ucdRadical(name, u), d.getStrokes(name, u), d.getPinyin(u),
            d.getNelsonIds(u), JlptLevels::None, findKyu ? d.getKyu(name) : KenteiKyus::None,
            findFrequency ? d.getFrequency(name) : 0),
      _meaning(d.ucd().getMeaning(u)), _reading(reading) {}
  NonLinkedKanji(const Data& d, int number, const std::string& name, const Ucd* u, bool findFrequency = true,
                 bool findKyu = true)
    : NonLinkedKanji(d, number, name, d.ucd().getReadingsAsKana(u), u, findFrequency, findKyu) {}

  // constructors used by FileKanji class (allows controlling strokes and frequency)
  NonLinkedKanji(const Data& d, int number, const std::string& name, const Radical& radical, const std::string& meaning,
                 const std::string& reading, int strokes, bool findFrequency, const Ucd* u)
    : Kanji(number, name, d.getCompatibilityName(name), radical, strokes, d.getPinyin(u), d.getNelsonIds(u),
            d.getLevel(name), d.getKyu(name), findFrequency ? d.getFrequency(name) : 0),
      _meaning(meaning), _reading(reading) {}
  NonLinkedKanji(const Data& d, int number, const std::string& name, const Radical& radical, const std::string& reading,
                 int strokes, bool findFrequency, const Ucd* u)
    : NonLinkedKanji(d, number, name, radical, d.ucd().getMeaning(u), reading, strokes, findFrequency, u) {}
private:
  const std::string _meaning;
  const std::string _reading;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H
