#include <kanji_tools/kanji/UcdData.h>
#include <tests/kanji_tools/TestData.h>
#include <tests/kanji_tools/TestKanji.h>
#include <tests/kanji_tools/TestUcd.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path TestFile{TestDir / "ucd.txt"};

const std::string FileMsg{" - file: ucd.txt, row: 1"};

class UcdDataTest : public TestData {
protected:
  void SetUp() override {
    write("Code\tName\tBlock\tVersion\tRadical\tStrokes\tVStrokes\tPinyin\t"
          "Morohashi\tNelsonIds\tSources\tJSource\tJoyo\tJinmei\tLinkCodes\t"
          "LinkNames\tLinkType\tMeaning\tOn\tKun");
  }

  void write(const std::string& s) {
    std::ofstream of{TestFile, std::ios_base::app};
    of << s << '\n';
    of.close();
  }

  void writeOne(bool includeOn = true, bool includeKun = true) {
    static const auto Tab{'\t'};
    auto r{"4E00\t" + _testName + "\tCJK\t1.1\t" + _testRadical + Tab +
           _testStrokes + Tab + _testVStrokes + "\tyī\t" + _testMorohashi +
           "\t1\tGHJKTV\t" + _testJSource + Tab + _testJouyou + Tab +
           _testJinmei + Tab + _testLinkCodes + Tab + _testLinkNames + Tab +
           _testLinkType + Tab + _testMeaning + Tab};
    r += includeOn ? "ICHI ITSU\t" : "\t";
    write(r += includeKun ? "HITOTSU HITOTABI HAJIME" : "");
  }

  const Ucd& loadOne(bool includeOn = true, bool includeKun = true) {
    writeOne(includeOn, includeKun);
    _ucd.load(TestFile);
    return *_ucd.find("一");
  }

  const Ucd& loadLinkedJinmei() {
    // write the Jōyō Kanji
    write("50E7\t僧\tCJK\t1.1\t9\t13\t\tsēng\t1076\t536,538\tGHJKTV\tJ0-414E\t"
          "Y\t\t\t\t\tBuddhist priest, monk; san of Sanskrit sangha\tSOU\t"
          "BOUZU");
    // write the 'linked' Jinmeiyō Kanji
    write("FA31\t僧\tCJK_Compat_Ideographs\t3.2\t9\t14\t\t\t\t\tJ\tJ3-2E49\t\t"
          "Y\t50E7\t僧\tJinmei*\tBuddhist priest, monk; san of Sanskrit sangha"
          "\tSOU\tBOUZU");
    _ucd.load(TestFile);
    return *_ucd.find("僧");
  }

  std::string _testName{"一"}, _testRadical{"1"}, _testStrokes{"1"},
      _testVStrokes{}, _testJouyou{"Y"}, _testJinmei{}, _testMorohashi{"1"},
      _testJSource{"J0-306C"}, _testMeaning{"one; a, an; alone"},
      _testLinkCodes{}, _testLinkNames{}, _testLinkType{};
};

} // namespace

TEST_F(UcdDataTest, LoadOneEntry) {
  auto& u{loadOne()};
  EXPECT_EQ(u.code(), U'\x4e00');
  EXPECT_EQ(u.name(), "一");
  EXPECT_EQ(u.block(), "CJK");
  EXPECT_EQ(u.version(), "1.1");
  EXPECT_EQ(u.strokes(), 1);
  EXPECT_EQ(u.variantStrokes(), 0);
  EXPECT_EQ(u.pinyin(), "yī");
  EXPECT_EQ(u.morohashiId(), "1");
  EXPECT_EQ(u.nelsonIds(), "1");
  EXPECT_EQ(u.sources(), "GHJKTV");
  EXPECT_EQ(u.jSource(), "J0-306C");
  EXPECT_TRUE(u.joyo());
  EXPECT_FALSE(u.jinmei());
  EXPECT_TRUE(u.links().empty());
  EXPECT_EQ(u.linkType(), UcdLinkTypes::None);
  EXPECT_FALSE(u.linkedReadings());
  EXPECT_EQ(u.meaning(), "one; a, an; alone");
  // readings get converted to Kana during Kanji creation (when required) by
  // 'getReadingAsKana' method (tested below)
  EXPECT_EQ(u.onReading(), "ICHI ITSU");
  EXPECT_EQ(u.kunReading(), "HITOTSU HITOTABI HAJIME");
  // has methods
  EXPECT_FALSE(u.hasLinks());
  EXPECT_FALSE(u.hasTraditionalLinks());
  EXPECT_FALSE(u.hasNonTraditionalLinks());
  EXPECT_FALSE(u.hasVariantStrokes());
}

TEST_F(UcdDataTest, LoadLinkedJinmeiEntries) {
  auto& u{loadLinkedJinmei()};
  EXPECT_EQ(u.code(), U'\xfa31');
  EXPECT_EQ(u.name(), "僧");
  EXPECT_EQ(u.block(), "CJK_Compat_Ideographs");
  EXPECT_EQ(u.version(), "3.2");
  EXPECT_EQ(u.strokes(), 14);
  EXPECT_EQ(u.variantStrokes(), 0);
  EXPECT_EQ(u.pinyin(), "");
  EXPECT_EQ(u.morohashiId(), "");
  EXPECT_EQ(u.nelsonIds(), "");
  EXPECT_EQ(u.sources(), "J");
  EXPECT_EQ(u.jSource(), "J3-2E49");
  EXPECT_FALSE(u.joyo());
  EXPECT_TRUE(u.jinmei());
  const Ucd::Links& links{u.links()};
  ASSERT_EQ(links.size(), 1);
  EXPECT_EQ(links[0].code(), U'\x50E7');
  EXPECT_EQ(links[0].name(), "僧");
  EXPECT_EQ(u.linkType(), UcdLinkTypes::Jinmei);
  EXPECT_TRUE(u.linkedReadings());
  EXPECT_EQ(u.meaning(), "Buddhist priest, monk; san of Sanskrit sangha");
  EXPECT_EQ(u.onReading(), "SOU");
  EXPECT_EQ(u.kunReading(), "BOUZU");
  // has methods
  EXPECT_TRUE(u.hasLinks());
  EXPECT_FALSE(u.hasTraditionalLinks());
  EXPECT_TRUE(u.hasNonTraditionalLinks());
  EXPECT_FALSE(u.hasVariantStrokes());
}

TEST_F(UcdDataTest, GetMeaning) {
  auto& u{loadOne()};
  EXPECT_EQ(_ucd.getMeaning(&u), "one; a, an; alone");
  EXPECT_EQ(_ucd.getMeaning({}), ""); // null ptr returns empty string
}

TEST_F(UcdDataTest, GetReadingAsKana) {
  auto& u{loadOne()};
  EXPECT_EQ(_ucd.getReadingsAsKana(&u), "イチ、イツ、ひとつ、ひとたび、はじめ");
  EXPECT_EQ(_ucd.getReadingsAsKana({}), ""); // null ptr returns empty string
}

TEST_F(UcdDataTest, GetReadingAsKanaForEntryWithoutOnReading) {
  auto& u{loadOne(false)};
  EXPECT_EQ(_ucd.getReadingsAsKana(&u), "ひとつ、ひとたび、はじめ");
}

TEST_F(UcdDataTest, GetReadingAsKanaForEntryWithoutKunReading) {
  auto& u{loadOne(true, false)};
  EXPECT_EQ(_ucd.getReadingsAsKana(&u), "イチ、イツ");
}

TEST_F(UcdDataTest, GetReadingAsKanaForEntryWithNoReadings) {
  auto& u{loadOne(false, false)};
  EXPECT_EQ(_ucd.getReadingsAsKana(&u), "");
}

TEST_F(UcdDataTest, NotFound) {
  loadOne();
  EXPECT_EQ(_ucd.find("虎"), nullptr);
}

TEST_F(UcdDataTest, FindIncludingVariations) {
  loadLinkedJinmei();
  const std::string jouyou{"僧"}, jinmei{"僧"}, jinmeiVariant{"僧︀"},
      otherVariant{"侮︀"};
  EXPECT_EQ(jouyou.size(), 3);
  EXPECT_EQ(jinmei.size(), 3);
  EXPECT_EQ(jinmeiVariant.size(), 6); // it has a 'variation selector'
  EXPECT_EQ(otherVariant.size(), 6);
  auto u{_ucd.find(jouyou)};
  ASSERT_TRUE(u);
  EXPECT_EQ(u->code(), U'\x50E7');
  ASSERT_TRUE(u = _ucd.find(jinmei));
  EXPECT_EQ(u->code(), U'\xfa31');
  ASSERT_TRUE(u = _ucd.find(jinmeiVariant));
  EXPECT_EQ(u->code(), U'\xfa31'); // returns the correct variant
  // should fail to find other variant since it hasn't been loaded
  EXPECT_FALSE(_ucd.find(otherVariant));
}

TEST_F(UcdDataTest, LoadWithNoReadingsOrMorohashiId) {
  _testMorohashi.clear();
  auto& u{loadOne(false, false)};
  EXPECT_TRUE(u.morohashiId().empty());
  EXPECT_TRUE(u.onReading().empty());
  EXPECT_TRUE(u.kunReading().empty());
  EXPECT_FALSE(u.jSource().empty());
}

TEST_F(UcdDataTest, LoadFailsWithNoReadingsOrMorohashiIdOrJSource) {
  _testMorohashi.clear();
  _testJSource.clear();
  EXPECT_THROW(
      call([this] { loadOne(false, false); },
          "one of 'On', 'Kun', 'Morohashi' or 'JSource' must be populated" +
              FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, NameTooLong) {
  _testName = "一二";
  EXPECT_THROW(call([this] { loadOne(); }, "name more than 4 bytes" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, ZeroStrokes) {
  _testStrokes = "0";
  EXPECT_THROW(
      call([this] { loadOne(); }, "strokes '0' out of range" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, BigStrokes) {
  _testStrokes = "55";
  EXPECT_THROW(
      call([this] { loadOne(); }, "strokes '55' out of range" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, ZeroVStrokes) {
  _testVStrokes = "0";
  EXPECT_THROW(
      call([this] { loadOne(); }, "variant strokes shouldn't be 0" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, OneVStrokes) {
  _testVStrokes = "1";
  EXPECT_THROW(
      call([this] { loadOne(); }, "variant strokes '1' out of range" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, BigVStrokes) {
  _testVStrokes = "34";
  EXPECT_THROW(call([this] { loadOne(); },
                   "variant strokes '34' out of range" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, RadicalZeroOutOfRange) {
  _testRadical = "0";
  EXPECT_THROW(
      call([this] { loadOne(); }, "radical '0' out of range" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, RadicalOutOfRange) {
  _testRadical = "215";
  EXPECT_THROW(
      call([this] { loadOne(); }, "radical '215' out of range" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, BothJouyouAndJinmei) {
  _testJinmei = "Y";
  EXPECT_THROW(
      call([this] { loadOne(); }, "can't be both joyo and jinmei" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, MissingMeaningForJouyou) {
  _testMeaning.clear();
  EXPECT_THROW(
      call([this] { loadOne(); }, "meaning is empty for Jōyō Kanji" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, DuplicateEntry) {
  writeOne();
  EXPECT_THROW(call([this] { loadOne(); },
                   "duplicate entry '一' - file: ucd.txt, row: 2"),
      std::domain_error);
}

TEST_F(UcdDataTest, PrintWithMissingEntry) {
  // add an entry to 'Data' that doesn't exist in '_ucd' (should never happen
  // when loading from actual data files)
  auto testKanji{std::make_shared<TestKanji>("四")};
  _types[KanjiTypes::Frequency].emplace_back(testKanji);
  _ucd.print(*this);
  auto found{false};
  for (std::string line; std::getline(_os, line);)
    if (line == "  ERROR: 四 not found in UCD") found = true;
  EXPECT_TRUE(found);
}

TEST_F(UcdDataTest, PrintVariantWithMissingEntry) {
  // add an entry with a variation selector to 'Data' that doesn't exist in
  // '_ucd' (should never happen when loading from actual data files)
  auto testKanji{std::make_shared<TestKanji>("僧︀")};
  const Ucd u{TestUcd{testKanji->name()}};
  checkInsert(testKanji, &u);
  EXPECT_THROW(
      call([this] { _ucd.print(*this); }, "UCD not found for '僧︀'"),
      std::domain_error);
}

// link validation tests

TEST_F(UcdDataTest, MoreLinkNamesThanLinkCodes) {
  _testLinkCodes = "ABCD";
  _testLinkNames = "二,三";
  EXPECT_THROW(call([this] { loadOne(); },
                   "LinkNames has more values than LinkCodes" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, BadJouyouLink) {
  _testLinkCodes = "ABCD";
  _testLinkNames = "二";
  EXPECT_THROW(
      call([this] { loadOne(); }, "joyo shouldn't have links" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, LinkNamesButNoLinkType) {
  _testJouyou.clear();
  _testLinkCodes = "ABCD";
  _testLinkNames = "二";
  EXPECT_THROW(call([this] { loadOne(); },
                   "LinkNames has a value, but LinkType is empty" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, LinkTypeButNoLinkNames) {
  _testLinkType = "Jinmei";
  EXPECT_THROW(call([this] { loadOne(); },
                   "LinkType has a value, but LinkNames is empty" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, LinkCodesButNoLinkNames) {
  _testLinkCodes = "ABCD";
  EXPECT_THROW(call([this] { loadOne(); },
                   "LinkCodes has a value, but LinkNames is empty" + FileMsg),
      std::domain_error);
}

TEST_F(UcdDataTest, BadJinmeiLink) {
  _testJouyou.clear();
  _testJinmei = "Y";
  _testLinkCodes = "ABCD";
  _testLinkNames = "僧";
  _testLinkType = "Jinmei";
  writeOne(); // write an entry that mimics a Linked Jinmei Kanji
  const auto msg{"jinmei entry '僧' with link '" + _testLinkNames +
                 "' failed - link already points to '" + _testName +
                 "' - file: ucd.txt, row: 3"};
  EXPECT_THROW(call([this] { loadLinkedJinmei(); }, msg), std::domain_error);
}

} // namespace kanji_tools
