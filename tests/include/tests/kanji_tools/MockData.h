#pragma once

#include <gmock/gmock.h>
#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

class MockData : public Data {
public:
  explicit MockData() : Data{{}, Data::DebugMode::None} {}
  MOCK_METHOD(Kanji::Frequency, frequency, (const String&), (const, override));
  MOCK_METHOD(JlptLevels, level, (const String&), (const, override));
  MOCK_METHOD(KenteiKyus, kyu, (const String&), (const, override));
  MOCK_METHOD(
      RadicalRef, ucdRadical, (const String&, UcdPtr), (const, override));
  MOCK_METHOD(Strokes, ucdStrokes, (const String&, UcdPtr), (const, override));
  MOCK_METHOD(RadicalRef, getRadicalByName, (const String&), (const, override));
};

} // namespace kanji_tools
