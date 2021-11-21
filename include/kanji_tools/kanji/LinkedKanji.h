#ifndef KANJI_TOOLS_KANJI_LINKED_KANJI_H
#define KANJI_TOOLS_KANJI_LINKED_KANJI_H

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

class LinkedKanji : public Kanji {
public:
  const std::string& meaning() const override { return _link->meaning(); }
  const std::string& reading() const override { return _link->reading(); }
  const Data::Entry& link() const { return _link; }
  OptString newName() const override { return _link->name(); }
protected:
  LinkedKanji(const Data& d, int number, const std::string& name, const Data::Entry& link)
    : Kanji(number, name, d.getCompatibilityName(name), d.ucdRadical(name), d.getStrokes(name), d.getPinyin(name),
            JlptLevels::None, d.getKyu(name), d.getFrequency(name)),
      _link(link) {}

  // linkedOldKanji must link back to Jouyou and LinkedJinmeiKanji can link to either Jouyou or Jinmei
  static const std::string& checkType(const std::string& name, const Data::Entry& link, bool isJinmei = false) {
    KanjiTypes t = link->type();
    if (t != KanjiTypes::Jouyou && (!isJinmei || t != KanjiTypes::Jinmei))
      throw std::domain_error("LinkedKanji " + name + " wanted type '" + toString(KanjiTypes::Jouyou) +
                              (isJinmei ? std::string("' or '") + toString(KanjiTypes::Jinmei) : std::string()) +
                              "' for link " + link->name() + ", but got '" + toString(t) + "'");
    return name;
  }
private:
  const Data::Entry _link;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(const Data& d, int number, const std::string& name, const Data::Entry& link)
    : LinkedKanji(d, number, checkType(name, link, true), link) {}

  KanjiTypes type() const override { return KanjiTypes::LinkedJinmei; }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(const Data& d, int number, const std::string& name, const Data::Entry& link)
    : LinkedKanji(d, number, checkType(name, link), link) {}

  KanjiTypes type() const override { return KanjiTypes::LinkedOld; }
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_LINKED_KANJI_H