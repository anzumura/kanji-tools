#ifndef KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H
#define KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'NonLinkedKanji' contains meaning and reading fields and is the base class for CustomFileKanji (an abstract
// base class for JouyouKanji, JinmeiKanji and ExtraKanji), FrequencyKanji, KenteiKanji and UcdKanji
class NonLinkedKanji : public Kanji {
public:
  const std::string& meaning() const override { return _meaning; }
  const std::string& reading() const override { return _reading; }
protected:
  // 'getLinkNames' is used by UcdFileKanji and ExtraKanji to populate links from Ucd data
  static LinkNames getLinkNames(const Ucd* u) {
    LinkNames result;
    if (u && u->hasLinks())
      std::transform(u->links().begin(), u->links().end(), std::back_inserter(result),
                     [](const auto& i) { return i.name(); });
    return result;
  }
  NonLinkedKanji(const Data& d, const std::string& name, const Radical& radical, const std::string& meaning,
                 const std::string& reading, int strokes, const Ucd* u, bool findKyu = true)
    : Kanji(name, d.getCompatibilityName(name), radical, strokes, findKyu ? d.getKyu(name) : KenteiKyus::None,
            d.getMorohashiId(u), d.getNelsonIds(u), d.getPinyin(u)),
      _meaning(meaning), _reading(reading) {}
  NonLinkedKanji(const Data& d, const std::string& name, const Radical& radical, const std::string& reading,
                 int strokes, const Ucd* u, bool findKyu = true)
    : NonLinkedKanji(d, name, radical, d.ucd().getMeaning(u), reading, strokes, u, findKyu) {}
private:
  const std::string _meaning;
  const std::string _reading;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_NON_LINKED_KANJI_H
