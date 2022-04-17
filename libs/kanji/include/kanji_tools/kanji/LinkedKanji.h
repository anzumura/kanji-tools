#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

class LinkedKanji : public Kanji {
public:
  [[nodiscard]] Meaning meaning() const override;
  [[nodiscard]] Reading reading() const override;

  [[nodiscard]] OptFreq frequency() const override { return _frequency; }
  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] KanjiPtr link() const override { return _link; }

  [[nodiscard]] bool linkedReadings() const override { return true; }
  [[nodiscard]] OptString newName() const override;
protected:
  LinkedKanji(DataRef, Name, KanjiPtr, UcdPtr);

  // linkedOldKanji must link back to Jouyou and LinkedJinmeiKanji can link to
  // either Jouyou or Jinmei
  [[nodiscard]] static Name checkType(Name, KanjiPtr, bool isJinmei = false);
private:
  const OptFreq _frequency;
  const KenteiKyus _kyu;
  const KanjiPtr _link;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(DataRef, Name, KanjiPtr);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedJinmei;
  }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(DataRef, Name, KanjiPtr);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedOld;
  }
};

} // namespace kanji_tools
