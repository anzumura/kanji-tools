#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/utils/Utils.h>

namespace kanji_tools {

const std::string& LinkedKanji::meaning() const { return _link->meaning(); }

const std::string& LinkedKanji::reading() const { return _link->reading(); }

Kanji::OptString LinkedKanji::newName() const { return _link->name(); }

LinkedKanji::LinkedKanji(const Data& d, const std::string& name,
    const Data::Entry& link, const Ucd* u)
    : Kanji{name, d.getCompatibilityName(name), d.ucdRadical(name, u),
          d.getStrokes(name, u), d.getMorohashiId(u), d.getNelsonIds(u),
          d.getPinyin(u)},
      _frequency{d.frequency(name)}, _kyu{d.kyu(name)}, _link{link} {}

const std::string& LinkedKanji::checkType(
    const std::string& name, const Data::Entry& link, bool isJinmei) {
  if (const auto t{link->type()};
      t != KanjiTypes::Jouyou && (!isJinmei || t != KanjiTypes::Jinmei))
    throw std::domain_error{
        "LinkedKanji " + name + " wanted type '" +
        toString(KanjiTypes::Jouyou) +
        (isJinmei ? std::string{"' or '"} + toString(KanjiTypes::Jinmei)
                  : EmptyString) +
        "' for link " + link->name() + ", but got '" + toString(t) + "'"};
  return name;
}

LinkedJinmeiKanji::LinkedJinmeiKanji(
    const Data& d, const std::string& name, const Data::Entry& link)
    : LinkedKanji{d, checkType(name, link, true), link, d.findUcd(name)} {}

LinkedOldKanji::LinkedOldKanji(
    const Data& d, const std::string& name, const Data::Entry& link)
    : LinkedKanji{d, checkType(name, link), link, d.findUcd(name)} {}

} // namespace kanji_tools
