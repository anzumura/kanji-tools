#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/utils/Utils.h>

namespace kanji_tools {

Kanji::Meaning LinkedKanji::meaning() const { return _link->meaning(); }

Kanji::Reading LinkedKanji::reading() const { return _link->reading(); }

Kanji::OptString LinkedKanji::newName() const { return _link->name(); }

LinkedKanji::LinkedKanji(DataRef data, Name name, Link link, UcdPtr u)
    : Kanji{data, name, data.ucdRadical(name, u), data.ucdStrokes(name, u), u},
      _frequency{data.frequency(name)}, _kyu{data.kyu(name)}, _link{link} {}

Kanji::Name LinkedKanji::checkType(Name name, Link link, bool isJinmei) {
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

LinkedJinmeiKanji::LinkedJinmeiKanji(DataRef data, Name name, Link link)
    : LinkedKanji{data, checkType(name, link, true), link, data.findUcd(name)} {
}

LinkedOldKanji::LinkedOldKanji(DataRef data, Name name, const Data::Entry& link)
    : LinkedKanji{data, checkType(name, link), link, data.findUcd(name)} {}

} // namespace kanji_tools
