#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

class LinkedKanji : public Kanji {
public:
  using Link = const Data::Entry&;

  [[nodiscard]] OptFreq frequency() const override { return _frequency; }
  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] Meaning meaning() const override;
  [[nodiscard]] Reading reading() const override;
  [[nodiscard]] OptString newName() const override;
  [[nodiscard]] bool linkedReadings() const override { return true; }

  [[nodiscard]] auto& link() const { return _link; }
protected:
  LinkedKanji(DataRef, Name, Link, UcdPtr);

  // linkedOldKanji must link back to Jouyou and LinkedJinmeiKanji can link to
  // either Jouyou or Jinmei
  [[nodiscard]] static Name checkType(Name, Link, bool isJinmei = false);
private:
  const OptFreq _frequency;
  const KenteiKyus _kyu;
  const Data::Entry _link;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(DataRef, Name, Link);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedJinmei;
  }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(DataRef, Name, Link);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedOld;
  }
};

} // namespace kanji_tools
