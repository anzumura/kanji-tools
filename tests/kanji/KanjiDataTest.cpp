#include <gtest/gtest.h>

#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <type_traits>

namespace kanji_tools {

namespace fs = std::filesystem;

TEST(DataTest, NextArgWithJustArg0) {
  const char* arg0 = "program-name";
  // call without final 'currentArg' parameter increments to 1
  EXPECT_EQ(Data::nextArg(1, &arg0), 1);
}

TEST(DataTest, NextArgWithCurrentArg) {
  const char* arg0 = "program-name";
  const char* arg1 = "arg1";
  const char* arg2 = "arg2";
  const char* argv[] = {arg0, arg1, arg2};
  EXPECT_EQ(Data::nextArg(std::size(argv), argv, 1), 2);
  EXPECT_EQ(Data::nextArg(std::size(argv), argv, 2), 3);
}

TEST(DataTest, NextArgWithDebugArg) {
  const char* arg0 = "program-name";
  const char* debugArg = "-debug";
  const char* argv[] = {arg0, debugArg};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 2);
}

TEST(DataTest, NextArgWithDataArg) {
  const char* arg0 = "program-name";
  const char* dataArg = "-data";
  const char* dataDir = "some-dir";
  const char* argv[] = {arg0, dataArg, dataDir};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 3);
}

TEST(DataTest, NextArgWithDebugAndDataArgs) {
  const char* arg0 = "program-name";
  const char* debugArg = "-debug";
  const char* dataArg = "-data";
  const char* dataDir = "some-dir";
  const char* argv[] = {arg0, debugArg, dataArg, dataDir};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 4);
}

TEST(DataTest, NextArgWithMultipleArgs) {
  const char* arg0 = "program-name";
  const char* arg1 = "arg1";
  const char* debugArg = "-debug";
  const char* arg3 = "arg3";
  const char* dataArg = "-data";
  const char* dataDir = "some-dir";
  const char* arg6 = "arg6";
  const char* argv[] = {arg0, arg1, debugArg, arg3, dataArg, dataDir, arg6};
  int argc = std::size(argv);
  std::vector<const char*> actualArgs;
  for (int i = Data::nextArg(argc, argv); i < argc; i = Data::nextArg(argc, argv, i))
    actualArgs.push_back(argv[i]);
  EXPECT_EQ(actualArgs, std::vector<const char*>({arg1, arg3, arg6}));
}

class KanjiDataTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "-data";
    static const char* arg2 = "../../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }
  // Contructs KanjiData using the real data files
  KanjiDataTest() : _data(3, argv()) {}
  int checkKanji(const Data::List& l) const {
    int variants = 0;
    for (auto& i : l) {
      if (i->variant()) {
        EXPECT_NE(i->name(), i->nonVariantName());
        EXPECT_NE(i->name(), i->compatibilityName());
        auto j = _data.findKanji(i->compatibilityName());
        EXPECT_TRUE(j.has_value());
        if (j.has_value()) {
          EXPECT_EQ((**j).type(), i->type());
          EXPECT_EQ((**j).name(), i->name());
        }
        ++variants;
      }
      if (!Kanji::hasLink(i->type()))
        EXPECT_TRUE(_data.getStrokes(i->name())) << i->type() << ", " << i->name() << ", " << toUnicode(i->name());
      EXPECT_EQ(MBChar::length(i->name()), 1) << i->type() << ", " << i->name() << ", " << toUnicode(i->name());
      EXPECT_TRUE(isKanji(i->name())) << i->type() << ", " << i->name() << ", " << toUnicode(i->name());
    }
    return variants;
  }
  KanjiData _data;
};

TEST_F(KanjiDataTest, SanityChecks) {
  // basic checks
  EXPECT_EQ(_data.getLevel("院"), JlptLevels::N4);
  EXPECT_EQ(_data.getFrequency("蝦"), 2501);
  EXPECT_EQ(_data.getStrokes("廳"), 25);
  // radical
  auto radical = _data.getRadicalByName("鹿");
  EXPECT_EQ(radical.number(), 198);
  EXPECT_EQ(radical.name(), "鹿");
  EXPECT_EQ(radical.longName(), "鹿部（ろくぶ）");
  EXPECT_EQ(radical.reading(), "しか");
  // find
  auto result = _data.findKanji("響︀");
  ASSERT_TRUE(result.has_value());
  auto& k = **result;
  EXPECT_EQ(k.type(), KanjiTypes::LinkedJinmei);
  EXPECT_EQ(k.name(), "響︀");
  EXPECT_EQ(k.radical(), _data.getRadicalByName("音"));
  EXPECT_EQ(k.level(), JlptLevels::None);
  EXPECT_EQ(k.grade(), KanjiGrades::None);
  EXPECT_EQ(k.frequency(), 0);
  EXPECT_TRUE(k.variant());
  auto result2 = _data.findKanji("逸︁");
  EXPECT_TRUE((**result2).variant());
  EXPECT_EQ((**result2).type(), KanjiTypes::LinkedJinmei);
  EXPECT_EQ((**result2).nonVariantName(), "逸");
  // kanji with 3 old names
  auto result3 = _data.findKanji("弁");
  ASSERT_TRUE(result3.has_value());
  EXPECT_EQ((**result3).oldNames(), Kanji::OldNames({"辨", "瓣", "辯"}));
  EXPECT_EQ((**result3).info(Kanji::OldField), "Old 辨／瓣／辯");
  for (auto& i : (**result3).oldNames()) {
    auto old = _data.findKanji(i);
    ASSERT_TRUE(old.has_value());
    ASSERT_EQ((**old).type(), KanjiTypes::LinkedOld);
    EXPECT_EQ(static_cast<const LinkedKanji&>(**old).link(), result3);
  }
  // totals
  EXPECT_EQ(_data.gradeTotal(KanjiGrades::G1), 80);
  EXPECT_EQ(_data.gradeTotal(KanjiGrades::G2), 160);
  EXPECT_EQ(_data.gradeTotal(KanjiGrades::G3), 200);
  EXPECT_EQ(_data.gradeTotal(KanjiGrades::G4), 200);
  EXPECT_EQ(_data.gradeTotal(KanjiGrades::G5), 185);
  EXPECT_EQ(_data.gradeTotal(KanjiGrades::G6), 181);
  EXPECT_EQ(_data.gradeTotal(KanjiGrades::S), 1130);
  EXPECT_EQ(_data.gradeTotal(KanjiGrades::None), 0);
  EXPECT_EQ(_data.levelTotal(JlptLevels::N5), 103);
  EXPECT_EQ(_data.levelTotal(JlptLevels::N4), 181);
  EXPECT_EQ(_data.levelTotal(JlptLevels::N3), 361);
  EXPECT_EQ(_data.levelTotal(JlptLevels::N2), 415);
  EXPECT_EQ(_data.levelTotal(JlptLevels::N1), 1162);
  EXPECT_EQ(_data.levelTotal(JlptLevels::None), 0);
  EXPECT_EQ(_data.frequencyTotal(-1), 0);
  EXPECT_EQ(_data.frequencyTotal(0), 500);
  EXPECT_EQ(_data.frequencyTotal(1), 500);
  EXPECT_EQ(_data.frequencyTotal(2), 500);
  EXPECT_EQ(_data.frequencyTotal(3), 500);
  EXPECT_EQ(_data.frequencyTotal(4), 501);
  EXPECT_EQ(_data.frequencyTotal(5), 0);
  // Make sure all Kanji are in Kanji related Unicode blocks
  EXPECT_EQ(checkKanji(_data.jouyouKanji()), 0);
  EXPECT_EQ(checkKanji(_data.jinmeiKanji()), 0);
  // 52 LinkedJinmei type Kanji use the Unicode 'Variation Selector'
  EXPECT_EQ(checkKanji(_data.linkedJinmeiKanji()), 52);
  EXPECT_EQ(checkKanji(_data.linkedOldKanji()), 0);
  EXPECT_EQ(checkKanji(_data.extraKanji()), 0);
  EXPECT_EQ(checkKanji(_data.otherKanji()), 0);
}

TEST_F(KanjiDataTest, UcdChecks) {
  // 'shrimp' is a Jinmei kanji, but 'jinmei.txt' doesn't include a Meaning column so the
  // value is pulled from UCD.
  auto& shrimp = **_data.findKanji("蝦");
  EXPECT_EQ(shrimp.meaning(), "shrimp, prawn");
  // 'dull' is only in 'frequency.txt' so radical, strokes, meaning and reading are all
  // pulled from UCD (and readings are converted to Kana).
  auto& dull = **_data.findKanji("呆");
  EXPECT_EQ(dull.radical(), _data.getRadicalByName("口"));
  EXPECT_EQ(dull.strokes(), 7);
  EXPECT_EQ(dull.meaning(), "dull; dull-minded, simple, stupid");
  // Note: unlike official lists (and 'extra.txt'), 'kun' readings from UCD unfortunately
  // don't have a dash before the Okurigana.
  EXPECT_EQ(dull.reading(), "ボウ、ガイ、ホウ、おろか、あきれる");
}

TEST_F(KanjiDataTest, UcdLinks) {
  auto& ucd = _data.ucd().map();
  EXPECT_EQ(ucd.size(), 15646);
  int jouyou = 0, jinmei = 0, jinmeiLinks = 0, otherLinks = 0, jinmeiLinksToJouyou = 0, jinmeiLinksToJinmei = 0;
  // every 'linkName' should be different than 'name' and also exist in the map
  for (auto& i : ucd) {
    const Ucd& k = i.second;
    // if 'variantStrokes' is present it should be different than 'strokes'
    if (k.hasVariantStrokes()) EXPECT_NE(k.strokes(), k.variantStrokes()) << k.codeAndName();
    // make sure MBUtils UCD characters are part of MBUtils unicode blocks
    if (k.joyo() || k.jinmei())
      EXPECT_TRUE(isCommonKanji(k.name())) << k.codeAndName();
    else
      EXPECT_TRUE(isKanji(k.name())) << k.codeAndName();
    // if a link is present make sure it points to another valid UCD entry
    if (k.hasLink()) {
      EXPECT_NE(k.name(), k.linkName());
      auto link = ucd.find(k.linkName());
      ASSERT_NE(link, ucd.end()) << k.linkCodeAndName();
    }
    if (k.joyo()) {
      EXPECT_FALSE(k.jinmei()) << k.codeAndName() << " is both joyo and jinmei";
      EXPECT_FALSE(k.hasLink()) << k.codeAndName() << " joyo has a link";
      ++jouyou;
    } else if (k.jinmei()) {
      ++jinmei;
      if (k.hasLink()) {
        ++jinmeiLinks;
        auto& link = ucd.find(k.linkName())->second;
        if (link.joyo())
          ++jinmeiLinksToJouyou;
        else if (link.jinmei())
          ++jinmeiLinksToJinmei;
        else
          FAIL() << "jinmei '" << k.name() << "' shouldn't have non-official link";
        if (link.hasLink()) EXPECT_NE(link.linkName(), k.name());
      }
    } else if (k.hasLink())
      ++otherLinks;
  }
  EXPECT_EQ(jouyou, _data.jouyouKanji().size());
  EXPECT_EQ(jinmei - jinmeiLinks, _data.jinmeiKanji().size());
  EXPECT_EQ(jinmeiLinks, _data.linkedJinmeiKanji().size());
  EXPECT_EQ(otherLinks, 2190);
  int officialLinksToJinmei = 0, officialLinksToJouyou = 0;
  for (auto& i : _data.linkedJinmeiKanji()) {
    auto& link = *static_cast<const LinkedKanji&>(*i).link();
    if (link.type() == KanjiTypes::Jouyou)
      ++officialLinksToJouyou;
    else if (link.type() == KanjiTypes::Jinmei)
      ++officialLinksToJinmei;
    else
      FAIL() << "official link from " << link << " is type " << link.type();
  }
  EXPECT_EQ(jinmeiLinksToJouyou, officialLinksToJouyou);
  EXPECT_EQ(jinmeiLinksToJinmei, officialLinksToJinmei);
}

} // namespace kanji_tools