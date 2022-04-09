#pragma once

#include <kanji_tools/kanji/Kanji.h>

namespace kanji_tools {

class TestKanji : public Kanji {
public:
  inline static const Radical TestRadical{1, {}, {}, {}, {}};
  inline static const std::string Meaning{"test"}, Reading{"テスト"};

  TestKanji(const std::string& name, const OptString& compatibilityName = {})
      : Kanji{name, compatibilityName, TestRadical, 0, {}, {}, {}} {}

  KanjiTypes type() const override { return _type; }
  const std::string& meaning() const override { return Meaning; }
  const std::string& reading() const override { return Reading; }

  void type(KanjiTypes t) { _type = t; }
private:
  KanjiTypes _type{KanjiTypes::None};
};

} // namespace kanji_tools
