#ifndef KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H
#define KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'NonLinkedKanji' contains meaning and reading fields and is the base class for CustomFileKanji (an abstract
// base class for JouyouKanji, JinmeiKanji and ExtraKanji), OtherKanji, KenteiKanji and UcdKanji
class NonLinkedKanji : public Kanji {
public:
  const std::string& meaning() const override { return _meaning; }
  const std::string& reading() const override { return _reading; }
protected:
  NonLinkedKanji(const Data& d, int number, const std::string& name, const Radical& radical, const std::string& meaning,
                 const std::string& reading, int strokes, const Ucd* u, bool findFrequency, bool findLevel = true,
                 bool findKyu = true)
    : Kanji(number, name, d.getCompatibilityName(name), radical, strokes, d.getPinyin(u), d.getNelsonIds(u),
            findLevel ? d.getLevel(name) : JlptLevels::None, findKyu ? d.getKyu(name) : KenteiKyus::None,
            findFrequency ? d.getFrequency(name) : 0),
      _meaning(meaning), _reading(reading) {}
  NonLinkedKanji(const Data& d, int number, const std::string& name, const Radical& radical, const std::string& reading,
                 int strokes, const Ucd* u, bool findFrequency, bool findLevel = true, bool findKyu = true)
    : NonLinkedKanji(d, number, name, radical, d.ucd().getMeaning(u), reading, strokes, u, findFrequency, findLevel,
                     findKyu) {}
private:
  const std::string _meaning;
  const std::string _reading;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H
