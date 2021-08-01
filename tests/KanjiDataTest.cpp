#include <gtest/gtest.h>

#include <kanji/Kanji.h>
#include <kanji/KanjiData.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

#include <type_traits>

namespace kanji {

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
    static const char* arg2 = "../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }
  // Contructs KanjiData using the real data files
  KanjiDataTest() : _data(3, argv()) {}
  int checkKanji(const Data::List& l) const {
    int variants = 0;
    for (auto& i : l) {
      if (i->variant()) ++variants;
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
  EXPECT_EQ(_data.getLevel("院"), Levels::N4);
  EXPECT_EQ(_data.getFrequency("蝦"), 2501);
  EXPECT_EQ(_data.getStrokes("廳"), 25);
  // radical
  auto radical = _data.getRadical("鹿");
  EXPECT_EQ(radical.number(), 198);
  EXPECT_EQ(radical.name(), "鹿");
  EXPECT_EQ(radical.longName(), "鹿部（ろくぶ）");
  EXPECT_EQ(radical.reading(), "しか");
  // find
  auto result = _data.findKanji("響");
  ASSERT_TRUE(result.has_value());
  auto& k = **result;
  EXPECT_EQ(k.type(), Types::LinkedOld);
  EXPECT_EQ(k.name(), "響");
  EXPECT_EQ(k.radical(), _data.getRadical(180));
  EXPECT_EQ(k.level(), Levels::None);
  EXPECT_EQ(k.grade(), Grades::None);
  EXPECT_EQ(k.frequency(), 0);
  EXPECT_FALSE(k.variant());
  auto result2 = _data.findKanji("逸︁");
  EXPECT_TRUE((**result2).variant());
  EXPECT_EQ((**result2).type(), Types::LinkedJinmei);
  EXPECT_EQ((**result2).nonVariantName(), "逸");
  // totals
  EXPECT_EQ(_data.gradeTotal(Grades::G1), 80);
  EXPECT_EQ(_data.gradeTotal(Grades::G2), 160);
  EXPECT_EQ(_data.gradeTotal(Grades::G3), 200);
  EXPECT_EQ(_data.gradeTotal(Grades::G4), 200);
  EXPECT_EQ(_data.gradeTotal(Grades::G5), 185);
  EXPECT_EQ(_data.gradeTotal(Grades::G6), 181);
  EXPECT_EQ(_data.gradeTotal(Grades::S), 1130);
  EXPECT_EQ(_data.gradeTotal(Grades::None), 0);
  EXPECT_EQ(_data.levelTotal(Levels::N5), 103);
  EXPECT_EQ(_data.levelTotal(Levels::N4), 181);
  EXPECT_EQ(_data.levelTotal(Levels::N3), 361);
  EXPECT_EQ(_data.levelTotal(Levels::N2), 415);
  EXPECT_EQ(_data.levelTotal(Levels::N1), 1162);
  EXPECT_EQ(_data.levelTotal(Levels::None), 0);
  EXPECT_EQ(_data.frequencyTotal(-1), 0);
  EXPECT_EQ(_data.frequencyTotal(0), 500);
  EXPECT_EQ(_data.frequencyTotal(1), 500);
  EXPECT_EQ(_data.frequencyTotal(2), 500);
  EXPECT_EQ(_data.frequencyTotal(3), 500);
  EXPECT_EQ(_data.frequencyTotal(4), 501);
  EXPECT_EQ(_data.frequencyTotal(5), 0);
}

TEST_F(KanjiDataTest, UcdChecks) {
  // 'shrimp' is a Jinmei kanji, but 'jinmei.txt' doesn't include a Meaning column so the
  // value is pulled from UCD.
  auto& shrimp = **_data.findKanji("蝦");
  EXPECT_EQ(shrimp.meaning(), "shrimp, prawn");
  // 'dull' is only in 'frequency.txt' so radical, strokes, meaning and reading are all
  // pulled from UCD (and readings are converted to Kana).
  auto& dull = **_data.findKanji("呆");
  EXPECT_EQ(dull.radical(), _data.getRadical("口"));
  EXPECT_EQ(dull.strokes(), 7);
  EXPECT_EQ(dull.meaning(), "dull; dull-minded, simple, stupid");
  // Note: unlike official lists (and 'extra.txt'), 'kun' readings from UCD unfortunately
  // don't have a dash before the Okurigana.
  EXPECT_EQ(dull.reading(), "ボウ、ガイ、ホウ、おろか、あきれる");
  auto count = [this](const auto& p) { return std::count_if(_data.ucdMap().begin(), _data.ucdMap().end(), p); };
  EXPECT_EQ(_data.ucdMap().size(), 12460);
  EXPECT_EQ(count([](auto& i) { return i.second.joyo(); }), 2136);
  EXPECT_EQ(count([](auto& i) { return i.second.jinmei(); }), 863);
  EXPECT_EQ(count([](auto& i) { return i.second.jinmei() && i.second.hasLink(); }), 248);
  EXPECT_EQ(count([](auto& i) { return i.second.joyo() && i.second.hasLink(); }), 0);
  EXPECT_EQ(count([](auto& i) { return !i.second.jinmei() && i.second.hasLink(); }), 64);
  // every 'linkName' should be different than 'name' and also exist in the map
  for (auto& i : _data.ucdMap()) {
    const Ucd& k = i.second;
    if (k.joyo() || k.jinmei())
      EXPECT_TRUE(isCommonKanji(k.name())) << k.codeAndName();
    else
      EXPECT_TRUE(isKanji(k.name())) << k.codeAndName();
    if (k.hasLink()) {
      EXPECT_NE(k.name(), k.linkName());
      EXPECT_TRUE(_data.ucdMap().contains(k.linkName()));
    }
    EXPECT_FALSE(k.joyo() && k.jinmei()) << k.codeAndName() << " is both joyo and jinmei";
    // if 'variantStrokes' is present it should be different than 'strokes'
    if (k.hasVariantStrokes())
      EXPECT_NE(k.strokes(), k.variantStrokes()) << k.codeAndName();
  }
  // Make sure all Kanji are in Kanji related Unicode blocks
  EXPECT_EQ(checkKanji(_data.jouyouKanji()), 0);
  EXPECT_EQ(checkKanji(_data.jinmeiKanji()), 0);
  // 52 LinkedJinmei type Kanji use the Unicode 'Variation Selector'
  EXPECT_EQ(checkKanji(_data.linkedJinmeiKanji()), 52);
  EXPECT_EQ(checkKanji(_data.linkedOldKanji()), 0);
  EXPECT_EQ(checkKanji(_data.extraKanji()), 0);
  EXPECT_EQ(checkKanji(_data.otherKanji()), 0);
}

} // namespace kanji
