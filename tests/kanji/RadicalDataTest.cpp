#include <kanji_tools/kanji/RadicalData.h>
#include <tests/kanji_tools/TestData.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path TestFile{TestDir / "radicals.txt"};

class RadicalDataTest : public TestData {
protected:
  void SetUp() override { write("Number\tName\tLongName\tReading"); }

  void write(const std::string& s) {
    std::ofstream of{TestFile, std::ios_base::app};
    of << s << '\n';
    of.close();
  }

  void loadOne() {
    write("001\t一\t一部（いちぶ）\tイチ");
    _radicals.load(TestFile);
  }
};

} // namespace

TEST_F(RadicalDataTest, LoadOneRadical) {
  loadOne();
  const auto& r{_radicals.find(1)};
  EXPECT_EQ(r.number(), 1);
  EXPECT_EQ(r.name(), "一");
  EXPECT_EQ(r.longName(), "一部（いちぶ）");
  EXPECT_EQ(r.reading(), "イチ");
  EXPECT_TRUE(r.altForms().empty());
  EXPECT_EQ(_radicals.find("一"), r);
}

TEST_F(RadicalDataTest, FindBeforeLoad) {
  const std::string msg{"must call 'load' before calling 'find'"};
  EXPECT_THROW(
      call([this] { return _radicals.find(1); }, msg), std::domain_error);
  EXPECT_THROW(
      call([this] { return _radicals.find("一"); }, msg), std::domain_error);
}

TEST_F(RadicalDataTest, NotFound) {
  loadOne();
  EXPECT_THROW(
      call([this] { return _radicals.find("二"); }, "name not found: 二"),
      std::domain_error);
  EXPECT_THROW(call([this] { return _radicals.find(0); },
                   "'0' is not a valid radical number"),
      std::domain_error);
  EXPECT_THROW(call([this] { return _radicals.find(2); },
                   "'2' is not a valid radical number"),
      std::domain_error);
}

TEST_F(RadicalDataTest, InvalidNumbering) {
  write("003\t一\t一部（いちぶ）\tイチ");
  EXPECT_THROW(
      call([this] { _radicals.load(TestFile); },
          "radicals must be ordered by 'number' - file: radicals.txt, row: 1"),
      std::domain_error);
}

TEST_F(RadicalDataTest, AltForms) {
  write("001\t水 氵 氺\t水部（すいぶ）\tみず さんずい したみず");
  _radicals.load(TestFile);
  const auto& r{_radicals.find(1)};
  EXPECT_EQ(r.name(), "水");
  EXPECT_EQ(r.altForms(), (Radical::AltForms{"氵", "氺"}));
}

TEST_F(RadicalDataTest, PrintWithOneMissing) {
  loadOne();
  _radicals.print(*this);
  EXPECT_EQ(_os.str(),
      R"(>>> Common Kanji Radicals (Jouyou Jinmei LinkedJinmei LinkedOld Frequency Extra Kentei Ucd):
>>>   Total for 0 radicals:    0 (0 0 0 0 0 0 0 0)
>>>   Found 1 radical with no Kanji: [001] 一
)");
}

TEST_F(RadicalDataTest, PrintWithMultipleMissing) {
  write("001\t一\t一部（いちぶ）\tイチ");
  write("002\t水 氵 氺\t水部（すいぶ）\tみず さんずい したみず");
  _radicals.load(TestFile);
  _radicals.print(*this);
  EXPECT_EQ(_os.str(),
      R"(>>> Common Kanji Radicals (Jouyou Jinmei LinkedJinmei LinkedOld Frequency Extra Kentei Ucd):
>>>   Total for 0 radicals:    0 (0 0 0 0 0 0 0 0)
>>>   Found 2 radicals with no Kanji: [001] 一 [002] 水
)");
}

} // namespace kanji_tools
