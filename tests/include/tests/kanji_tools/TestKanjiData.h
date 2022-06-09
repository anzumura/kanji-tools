#pragma once

#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiData.h>

#include <fstream>

namespace kanji_tools {

inline constexpr auto TestDirArg{"testDir"};
inline const std::filesystem::path TestDir{TestDirArg};
inline const std::filesystem::path TestFile{TestDir / "testFile.txt"};

class TestKanjiData : public ::testing::Test, public KanjiData {
public:
  ~TestKanjiData() override { clear(); }

  [[nodiscard]] Kanji::Frequency frequency(const String&) const final {
    return Kanji::Frequency{};
  }
  [[nodiscard]] JlptLevels level(const String&) const final {
    return JlptLevels::None;
  }
  [[nodiscard]] KenteiKyus kyu(const String&) const final {
    return KenteiKyus::None;
  }

  static void write(const String& s, bool append = true) {
    if (!std::filesystem::exists(TestDir))
      std::filesystem::create_directory(TestDir);
    std::ofstream of{
        TestFile, append ? std::ios_base::app : std::ios_base::ate};
    of << s << '\n';
    of.close();
  }
protected:
  TestKanjiData() : KanjiData{TestDir, DebugMode::None, _os, _es} {}

  static void clear() {
    _os.str({});
    _os.clear();
    _es.str({});
    _es.clear();
    std::filesystem::remove_all(TestDir);
  }

  inline static std::stringstream _os, _es;
};

} // namespace kanji_tools
