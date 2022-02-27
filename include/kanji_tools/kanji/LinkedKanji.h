#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

class LinkedKanji : public Kanji {
public:
  [[nodiscard]] OptInt frequency() const override { return _frequency; }
  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] const std::string& meaning() const override {
    return _link->meaning();
  }
  [[nodiscard]] const std::string& reading() const override {
    return _link->reading();
  }
  [[nodiscard]] OptString newName() const override { return _link->name(); }
  [[nodiscard]] bool linkedReadings() const override { return true; }

  [[nodiscard]] auto& link() const { return _link; }
protected:
  LinkedKanji(const Data& d, const std::string& name, const Data::Entry& link,
              const Ucd* u)
      : Kanji(name, d.getCompatibilityName(name), d.ucdRadical(name, u),
              d.getStrokes(name, u), d.getMorohashiId(u), d.getNelsonIds(u),
              d.getPinyin(u)),
        _frequency(d.getFrequency(name)), _kyu(d.getKyu(name)), _link(link) {}

  // linkedOldKanji must link back to Jouyou and LinkedJinmeiKanji can link to
  // either Jouyou or Jinmei
  [[nodiscard]] static auto& checkType(const std::string& name,
                                       const Data::Entry& link,
                                       bool isJinmei = false) {
    const auto t = link->type();
    if (t != KanjiTypes::Jouyou && (!isJinmei || t != KanjiTypes::Jinmei))
      throw std::domain_error(
        "LinkedKanji " + name + " wanted type '" +
        toString(KanjiTypes::Jouyou) +
        (isJinmei ? std::string("' or '") + toString(KanjiTypes::Jinmei)
                  : std::string()) +
        "' for link " + link->name() + ", but got '" + toString(t) + "'");
    return name;
  }
private:
  const OptInt _frequency;
  const KenteiKyus _kyu;
  const Data::Entry _link;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(const Data& d, const std::string& name,
                    const Data::Entry& link)
      : LinkedKanji(d, checkType(name, link, true), link, d.findUcd(name)) {}

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedJinmei;
  }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(const Data& d, const std::string& name,
                 const Data::Entry& link)
      : LinkedKanji(d, checkType(name, link), link, d.findUcd(name)) {}

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedOld;
  }
};

} // namespace kanji_tools
