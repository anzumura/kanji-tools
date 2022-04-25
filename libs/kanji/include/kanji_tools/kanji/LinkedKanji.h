#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

class LinkedKanji : public Kanji {
public:
  [[nodiscard]] Meaning meaning() const override;
  [[nodiscard]] Reading reading() const override;

  [[nodiscard]] Frequency frequency() const override { return _frequency; }
  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] KanjiPtr link() const override { return _link; }

  [[nodiscard]] bool linkedReadings() const override { return true; }
  [[nodiscard]] OptString newName() const override;
protected:
  LinkedKanji(DataRef, Name, const KanjiPtr&, UcdPtr);

  // linkedOldKanji must link back to Jouyou and LinkedJinmeiKanji can link to
  // either Jouyou or Jinmei
  [[nodiscard]] static Name linkType(Name, const Kanji&, bool isJouyou = true);
private:
  const Frequency _frequency;
  const KenteiKyus _kyu;
  const KanjiPtr _link;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(DataRef, Name, const KanjiPtr&);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedJinmei;
  }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(DataRef, Name, const KanjiPtr&);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedOld;
  }
};

} // namespace kanji_tools
