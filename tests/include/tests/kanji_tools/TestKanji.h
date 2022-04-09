#pragma once

#include <kanji_tools/kanji/Kanji.h>

namespace kanji_tools {

class TestKanji : public Kanji {
public:
  inline static const Radical TestRadical{1, {}, {}, {}, {}};
  inline static const std::string Meaning{"test"}, Reading{"テスト"};

  TestKanji(const std::string& name)
      : Kanji{name, name, TestRadical, 0, {}, {}, {}} {}

  KanjiTypes type() const override { return KanjiTypes::None; }
  const std::string& meaning() const override { return Meaning; }
  const std::string& reading() const override { return Reading; }
};

} // namespace kanji_tools
