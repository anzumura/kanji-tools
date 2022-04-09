#pragma once

#include <kanji_tools/kanji/Data.h>

#include <gtest/gtest.h>

namespace kanji_tools {

inline constexpr auto TestDataDir{"dir"};
inline const std::filesystem::path TestDataPath{TestDataDir};

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
  TestData(bool create = true) : Data{TestDataPath, DebugMode::None, _os, _es} {
    clear();
    if (create) std::filesystem::create_directory(TestDataPath);
  }

  ~TestData() override { clear(); }

  void clear() {
    _os.str({});
    _os.clear();
    _es.str({});
    _es.clear();
    std::filesystem::remove_all(TestDataPath);
  }

  inline static std::stringstream _os, _es;
};

} // namespace kanji_tools
