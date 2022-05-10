#include <kanji_tools/kanji/LinkedKanji.h>

namespace kanji_tools {

Kanji::Meaning LinkedKanji::meaning() const { return _link->meaning(); }

Kanji::Reading LinkedKanji::reading() const { return _link->reading(); }

Kanji::OptString LinkedKanji::newName() const { return _link->name(); }

LinkedKanji::LinkedKanji(
    DataRef data, Name name, const KanjiPtr& link, UcdPtr u)
    : Kanji{data, name, data.ucdRadical(name, u), data.ucdStrokes(name, u), u},
      _frequency{data.frequency(name)}, _kyu{data.kyu(name)}, _link{link} {}

Kanji::Name LinkedKanji::linkType(Name name, const Kanji& link, bool isJouyou) {
  if (const auto t{link.type()};
      t != KanjiTypes::Jouyou && (isJouyou || t != KanjiTypes::Jinmei))
    throw std::domain_error{
        "LinkedKanji " + name + " wanted type '" +
        toString(KanjiTypes::Jouyou) +
        (isJouyou ? EmptyString
                  : String{"' or '"} + toString(KanjiTypes::Jinmei)) +
        "' for link " + link.name() + ", but got '" + toString(t) + "'"};
  return name;
}

LinkedJinmeiKanji::LinkedJinmeiKanji(
    DataRef data, Name name, const KanjiPtr& link)
    : LinkedKanji{
          data, linkType(name, *link, false), link, data.findUcd(name)} {}

LinkedOldKanji::LinkedOldKanji(DataRef data, Name name, const KanjiPtr& link)
    : LinkedKanji{data, linkType(name, *link), link, data.findUcd(name)} {}

} // namespace kanji_tools
