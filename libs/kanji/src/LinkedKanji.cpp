#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/utils/Utils.h>

namespace kanji_tools {

const std::string& LinkedKanji::meaning() const { return _link->meaning(); }

const std::string& LinkedKanji::reading() const { return _link->reading(); }

Kanji::OptString LinkedKanji::newName() const { return _link->name(); }

LinkedKanji::LinkedKanji(const Data& data, const std::string& name,
    const Data::Entry& link, const Ucd* u)
    : Kanji{name, data.getCompatibilityName(name), data.ucdRadical(name, u),
          data.ucdStrokes(name, u), data.getMorohashiId(u),
          data.getNelsonIds(u), data.getPinyin(u)},
      _frequency{data.frequency(name)}, _kyu{data.kyu(name)}, _link{link} {}

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
    const Data& data, const std::string& name, const Data::Entry& link)
    : LinkedKanji{data, checkType(name, link, true), link, data.findUcd(name)} {
}

LinkedOldKanji::LinkedOldKanji(
    const Data& data, const std::string& name, const Data::Entry& link)
    : LinkedKanji{data, checkType(name, link), link, data.findUcd(name)} {}

} // namespace kanji_tools
