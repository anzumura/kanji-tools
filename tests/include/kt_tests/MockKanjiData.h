#pragma once

#include <gmock/gmock.h>
#include <kt_kanji/KanjiData.h>

namespace kanji_tools {

class MockKanjiData final : public KanjiData {
public:
  explicit MockKanjiData() : KanjiData{{}, KanjiData::DebugMode::None} {}
  MOCK_METHOD(Kanji::Frequency, frequency, (const String&), (const, final));
  MOCK_METHOD(JlptLevels, level, (const String&), (const, final));
  MOCK_METHOD(KenteiKyus, kyu, (const String&), (const, final));
  MOCK_METHOD(RadicalRef, ucdRadical, (const String&, UcdPtr), (const, final));
  MOCK_METHOD(Strokes, ucdStrokes, (const String&, UcdPtr), (const, final));
  MOCK_METHOD(RadicalRef, getRadicalByName, (const String&), (const, final));
};

} // namespace kanji_tools
