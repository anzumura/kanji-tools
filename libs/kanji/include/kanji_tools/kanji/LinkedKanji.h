#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

class LinkedKanji : public Kanji {
public:
  [[nodiscard]] OptFreq frequency() const override { return _frequency; }
  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] const std::string& meaning() const override;
  [[nodiscard]] const std::string& reading() const override;
  [[nodiscard]] OptString newName() const override;
  [[nodiscard]] bool linkedReadings() const override { return true; }

  [[nodiscard]] auto& link() const { return _link; }
protected:
  LinkedKanji(const Data&, const std::string&, const Data::Entry&, const Ucd*);

  // linkedOldKanji must link back to Jouyou and LinkedJinmeiKanji can link to
  // either Jouyou or Jinmei
  [[nodiscard]] static const std::string& checkType(
      const std::string& name, const Data::Entry& link, bool isJinmei = false);
private:
  const OptFreq _frequency;
  const KenteiKyus _kyu;
  const Data::Entry _link;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(const Data&, const std::string& name, const Data::Entry&);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedJinmei;
  }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(const Data&, const std::string& name, const Data::Entry&);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedOld;
  }
};

} // namespace kanji_tools
