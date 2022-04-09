#pragma once

#include <kanji_tools/kanji/Data.h>

#include <gtest/gtest.h>

namespace kanji_tools {

inline constexpr auto TestDirArg{"testDir"};
inline const std::filesystem::path TestDir{TestDirArg};

class TestData : public ::testing::Test, public Data {
public:
  [[nodiscard]] Kanji::OptFreq frequency(const std::string&) const override {
    return {};
  }
  [[nodiscard]] JlptLevels level(const std::string&) const override {
    return JlptLevels::None;
  }
  [[nodiscard]] KenteiKyus kyu(const std::string&) const override {
    return KenteiKyus::None;
  }
protected:
  TestData(bool createDir = true) : Data{TestDir, DebugMode::None, _os, _es} {
    clear(createDir);
  }

  ~TestData() override { clear(); }

  void clear(bool createDir = false) {
    _os.str({});
    _os.clear();
    _es.str({});
    _es.clear();
    std::filesystem::remove_all(TestDir);
    if (createDir) std::filesystem::create_directory(TestDir);
  }

  inline static std::stringstream _os, _es;
};

} // namespace kanji_tools
