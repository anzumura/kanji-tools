#pragma once

#include <gtest/gtest.h>
#include <kanji_tools/kanji/Data.h>

#include <fstream>

namespace kanji_tools {

inline constexpr auto TestDirArg{"testDir"};
inline const std::filesystem::path TestDir{TestDirArg};
inline const std::filesystem::path TestFile{TestDir / "testFile.txt"};

class TestData : public ::testing::Test, public Data {
public:
  [[nodiscard]] Kanji::Frequency frequency(const std::string&) const override {
    return Kanji::Frequency{};
  }
  [[nodiscard]] JlptLevels level(const std::string&) const override {
    return JlptLevels::None;
  }
  [[nodiscard]] KenteiKyus kyu(const std::string&) const override {
    return KenteiKyus::None;
  }
protected:
  TestData() : Data{TestDir, DebugMode::None, _os, _es} {}

  ~TestData() override { clear(); }

  static void clear() {
    _os.str({});
    _os.clear();
    _es.str({});
    _es.clear();
    std::filesystem::remove_all(TestDir);
  }

  static void write(const std::string& s) {
    if (!std::filesystem::exists(TestDir))
      std::filesystem::create_directory(TestDir);
    std::ofstream of{TestFile, std::ios_base::app};
    of << s << '\n';
    of.close();
  }

  inline static std::stringstream _os, _es;
};

} // namespace kanji_tools
