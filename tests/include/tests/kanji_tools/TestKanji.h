#pragma once

#include <kanji_tools/kanji/Kanji.h>

namespace kanji_tools {

class TestKanji : public Kanji {
public:
  inline static const Radical TestRadical{1, {}, {}, {}, {}};
  inline static const String TestMeaning{"test"}, TestReading{"テスト"};

  explicit TestKanji(Name name, const OptString& compatibilityName = {})
      : Kanji{name, compatibilityName, TestRadical, Strokes{1}, {}, {}, {}} {}

  [[nodiscard]] KanjiTypes type() const override { return _type; }
  [[nodiscard]] Meaning meaning() const override { return TestMeaning; }
  [[nodiscard]] Reading reading() const override { return TestReading; }

  void type(KanjiTypes t) { _type = t; }
private:
  KanjiTypes _type{KanjiTypes::None};
};

} // namespace kanji_tools
