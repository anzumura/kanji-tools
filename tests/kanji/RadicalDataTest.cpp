#include <kanji_tools/kanji/RadicalData.h>
#include <tests/kanji_tools/TestKanjiData.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

namespace {

class RadicalDataTest : public TestKanjiData {
protected:
  void SetUp() override { write("Number\tName\tLongName\tReading"); }

  void loadOne() {
    write("001\t一\t一部（いちぶ）\tイチ");
    radicals().load(TestFile);
  }
};

} // namespace

TEST_F(RadicalDataTest, LoadOneRadical) {
  loadOne();
  const auto& r{radicals().find(1)};
  EXPECT_EQ(r.number(), 1);
  EXPECT_EQ(r.name(), "一");
  EXPECT_EQ(r.longName(), "一部（いちぶ）");
  EXPECT_EQ(r.reading(), "イチ");
  EXPECT_TRUE(r.altForms().empty());
  EXPECT_EQ(radicals().find("一"), r);
}

TEST_F(RadicalDataTest, FindBeforeLoad) {
  const String msg{"must call 'load' before calling 'find'"};
  EXPECT_THROW(call([this] { return radicals().find(1); }, msg), DomainError);
  EXPECT_THROW(
      call([this] { return radicals().find("一"); }, msg), DomainError);
}

TEST_F(RadicalDataTest, NotFound) {
  loadOne();
  EXPECT_THROW(
      call([this] { return radicals().find("二"); }, "name not found: 二"),
      DomainError);
  EXPECT_THROW(call([this] { return radicals().find(0); },
                   "'0' is not a valid radical number"),
      DomainError);
  EXPECT_THROW(call([this] { return radicals().find(2); },
                   "'2' is not a valid radical number"),
      DomainError);
}

TEST_F(RadicalDataTest, InvalidNumbering) {
  write("003\t一\t一部（いちぶ）\tイチ");
  EXPECT_THROW(
      call([this] { radicals().load(TestFile); },
          "radicals must be ordered by 'number' - file: testFile.txt, row: 1"),
      DomainError);
}

TEST_F(RadicalDataTest, AltForms) {
  write("001\t水 氵 氺\t水部（すいぶ）\tみず さんずい したみず");
  radicals().load(TestFile);
  const auto& r{radicals().find(1)};
  EXPECT_EQ(r.name(), "水");
  EXPECT_EQ(r.altForms(), (Radical::AltForms{"氵", "氺"}));
}

TEST_F(RadicalDataTest, PrintWithOneMissing) {
  loadOne();
  radicals().print(*this);
  EXPECT_EQ(_os.str(),
      R"(>>> Common Kanji Radicals (Jouyou Jinmei LinkedJinmei LinkedOld Frequency Extra Kentei Ucd):
>>>   Total for 0 radicals:    0 (0 0 0 0 0 0 0 0)
>>>   Found 1 radical with no Kanji: [001] 一
)");
}

TEST_F(RadicalDataTest, PrintWithMultipleMissing) {
  write("001\t一\t一部（いちぶ）\tイチ");
  write("002\t水 氵 氺\t水部（すいぶ）\tみず さんずい したみず");
  radicals().load(TestFile);
  radicals().print(*this);
  EXPECT_EQ(_os.str(),
      R"(>>> Common Kanji Radicals (Jouyou Jinmei LinkedJinmei LinkedOld Frequency Extra Kentei Ucd):
>>>   Total for 0 radicals:    0 (0 0 0 0 0 0 0 0)
>>>   Found 2 radicals with no Kanji: [001] 一 [002] 水
)");
}

} // namespace kanji_tools
