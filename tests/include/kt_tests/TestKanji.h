#pragma once

#include <kt_kanji/Kanji.h>

namespace kanji_tools {

class TestKanji final : public Kanji {
public:
  inline static const Radical TestRadical{1, {}, {}, {}, {}};
  inline static const String TestMeaning{"test"}, TestReading{"テスト"};

  explicit TestKanji(Name name, const OptString& compatibilityName = {})
      : Kanji{name, compatibilityName, TestRadical, Strokes{1}, {}, {}, {}} {}

  [[nodiscard]] KanjiTypes type() const final { return _type; }
  [[nodiscard]] Meaning meaning() const final { return TestMeaning; }
  [[nodiscard]] Reading reading() const final { return TestReading; }

  void type(KanjiTypes t) { _type = t; }
private:
  KanjiTypes _type{KanjiTypes::None};
};

} // namespace kanji_tools
