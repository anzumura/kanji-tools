#include <gtest/gtest.h>
#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/utils/DisplaySize.h>

#include <type_traits>

namespace kanji_tools {

namespace {

constexpr auto Arg0 = "program-name", DebugArg = "-debug", DataArg = "-data",
               DataDir = "some-dir";

} // namespace

namespace fs = std::filesystem;

TEST(DataTest, NextArgWithJustArg0) {
  // call without final 'currentArg' parameter increments to 1
  EXPECT_EQ(Data::nextArg(1, &Arg0), 1);
}

TEST(DataTest, NextArgWithCurrentArg) {
  auto arg1 = "arg1", arg2 = "arg2";
  const char* argv[] = {Arg0, arg1, arg2};
  EXPECT_EQ(Data::nextArg(std::size(argv), argv, 1), 2);
  EXPECT_EQ(Data::nextArg(std::size(argv), argv, 2), 3);
}

TEST(DataTest, NextArgWithDebugArg) {
  const char* argv[] = {Arg0, DebugArg};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 2);
}

TEST(DataTest, NextArgWithDataArg) {
  const char* argv[] = {Arg0, DataArg, DataDir};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 3);
}

TEST(DataTest, NextArgWithDebugAndDataArgs) {
  const char* argv[] = {Arg0, DebugArg, DataArg, DataDir};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 4);
}

TEST(DataTest, NextArgWithMultipleArgs) {
  auto arg1 = "arg1", arg3 = "arg3", arg6 = "arg6";
  const char* argv[] = {Arg0, arg1, DebugArg, arg3, DataArg, DataDir, arg6};
  auto argc = std::size(argv);
  std::vector<const char*> actualArgs;
  for (auto i = Data::nextArg(argc, argv); i < argc;
       i = Data::nextArg(argc, argv, i))
    actualArgs.push_back(argv[i]);
  EXPECT_EQ(actualArgs, std::vector<const char*>({arg1, arg3, arg6}));
}

class KanjiDataTest : public ::testing::Test {
protected:
  [[nodiscard]] static const char** argv() {
    static auto arg2 = "../../../data";
    static const char* args[] = {Arg0, DataArg, arg2};
    return args;
  }

  static void SetUpTestCase() {
    // Contructs KanjiData using the real data files
    _data = std::make_shared<KanjiData>(3, argv());
  }

  KanjiDataTest() {}

  [[nodiscard]] auto checkKanji(const Data::List& l) const {
    auto variants = 0;
    for (auto& i : l) {
      if (i->variant()) {
        EXPECT_NE(i->name(), i->nonVariantName());
        EXPECT_NE(i->name(), i->compatibilityName());
        auto j = _data->findKanjiByName(i->compatibilityName());
        EXPECT_TRUE(j);
        if (j) {
          EXPECT_EQ((**j).type(), i->type());
          EXPECT_EQ((**j).name(), i->name());
        }
        ++variants;
      }
      if (!Kanji::hasLink(i->type()))
        EXPECT_TRUE(_data->getStrokes(i->name()))
          << i->type() << ", " << i->name() << ", " << toUnicode(i->name());
      EXPECT_EQ(MBChar::size(i->name()), 1)
        << i->type() << ", " << i->name() << ", " << toUnicode(i->name());
      EXPECT_TRUE(isKanji(i->name()))
        << i->type() << ", " << i->name() << ", " << toUnicode(i->name());
    }
    return variants;
  }

  inline static DataPtr _data;
};

TEST_F(KanjiDataTest, BasicChecks) {
  EXPECT_EQ(_data->kanjiNameMap().size(), 23715);
  EXPECT_EQ(_data->level("院"), JlptLevels::N4);
  EXPECT_EQ(_data->frequency("蝦"), 2501);
  EXPECT_EQ(_data->getStrokes("廳"), 25);
}

TEST_F(KanjiDataTest, FrequencyKanjiChecks) {
  const auto yeast = _data->findKanjiByName("麹");
  ASSERT_TRUE(yeast);
  EXPECT_EQ((**yeast).type(), KanjiTypes::Frequency);
  EXPECT_FALSE((**yeast).hasGrade());
  EXPECT_FALSE((**yeast).hasLevel());
  EXPECT_EQ((**yeast).kyu(), KenteiKyus::KJ1);
  EXPECT_EQ((**yeast).frequency(), 1988);
  EXPECT_FALSE((**yeast).newName());
  EXPECT_EQ((**yeast).oldNames(), Kanji::LinkNames({"麴"}));
  EXPECT_EQ((**yeast).reading(), "キク、こうじ");
  EXPECT_EQ((**yeast).meaning(), "yeast, leaven; surname");
}

TEST_F(KanjiDataTest, ExtraKanjiChecks) {
  const auto grab = _data->findKanjiByName("掴");
  ASSERT_TRUE(grab);
  EXPECT_EQ((**grab).type(), KanjiTypes::Extra);
  EXPECT_FALSE((**grab).hasGrade());
  EXPECT_FALSE((**grab).hasLevel());
  EXPECT_EQ((**grab).kyu(), KenteiKyus::KJ1);
  EXPECT_FALSE((**grab).frequency());
  EXPECT_FALSE((**grab).newName());
  EXPECT_EQ((**grab).oldNames(), Kanji::LinkNames({"摑"}));
  EXPECT_EQ((**grab).reading(), "カク、つか-む、つか-まえる、つか-まる");
  EXPECT_EQ((**grab).meaning(), "catch");
}

TEST_F(KanjiDataTest, KenteiKanjiChecks) {
  const auto apple = _data->findKanjiByName("蘋");
  ASSERT_TRUE(apple);
  EXPECT_EQ((**apple).type(), KanjiTypes::Kentei);
  EXPECT_FALSE((**apple).hasGrade());
  EXPECT_FALSE((**apple).hasLevel());
  EXPECT_EQ((**apple).kyu(), KenteiKyus::K1);
  EXPECT_EQ((**apple).reading(), "ヒン、ビン、うきくさ、でんじそ");
  EXPECT_EQ((**apple).meaning(), "apple");
  EXPECT_EQ((**apple).newName(), "苹");
  EXPECT_FALSE((**apple).linkedReadings());
}

TEST_F(KanjiDataTest, UcdKanjiChecks) {
  const auto complete = _data->findKanjiByName("侭");
  ASSERT_TRUE(complete);
  EXPECT_EQ((**complete).type(), KanjiTypes::Ucd);
  EXPECT_FALSE((**complete).hasGrade());
  EXPECT_FALSE((**complete).hasLevel());
  EXPECT_FALSE((**complete).hasKyu());
  EXPECT_EQ((**complete).reading(), "ジン、ことごとく、まま");
  EXPECT_EQ((**complete).meaning(), "complete, utmost");
  EXPECT_FALSE((**complete).linkedReadings());
  const auto shape = _data->findKanjiByName("檨");
  ASSERT_TRUE(shape);
  EXPECT_EQ((**shape).type(), KanjiTypes::Ucd);
  EXPECT_TRUE((**shape).linkedReadings());
}

TEST_F(KanjiDataTest, RadicalChecks) {
  const auto radical = _data->getRadicalByName("鹿");
  EXPECT_EQ(radical.number(), 198);
  EXPECT_EQ(radical.name(), "鹿");
  EXPECT_EQ(radical.longName(), "鹿部（ろくぶ）");
  EXPECT_EQ(radical.reading(), "しか");
}

TEST_F(KanjiDataTest, TotalsChecks) {
  EXPECT_EQ(_data->gradeTotal(KanjiGrades::G1), 80);
  EXPECT_EQ(_data->gradeTotal(KanjiGrades::G2), 160);
  EXPECT_EQ(_data->gradeTotal(KanjiGrades::G3), 200);
  EXPECT_EQ(_data->gradeTotal(KanjiGrades::G4), 200);
  EXPECT_EQ(_data->gradeTotal(KanjiGrades::G5), 185);
  EXPECT_EQ(_data->gradeTotal(KanjiGrades::G6), 181);
  EXPECT_EQ(_data->gradeTotal(KanjiGrades::S), 1130);
  EXPECT_EQ(_data->gradeTotal(KanjiGrades::None), 0);
  EXPECT_EQ(_data->levelTotal(JlptLevels::N5), 103);
  EXPECT_EQ(_data->levelTotal(JlptLevels::N4), 181);
  EXPECT_EQ(_data->levelTotal(JlptLevels::N3), 361);
  EXPECT_EQ(_data->levelTotal(JlptLevels::N2), 415);
  EXPECT_EQ(_data->levelTotal(JlptLevels::N1), 1162);
  EXPECT_EQ(_data->levelTotal(JlptLevels::None), 0);
  EXPECT_EQ(_data->frequencyTotal(0), 500);
  EXPECT_EQ(_data->frequencyTotal(1), 500);
  EXPECT_EQ(_data->frequencyTotal(2), 500);
  EXPECT_EQ(_data->frequencyTotal(3), 500);
  EXPECT_EQ(_data->frequencyTotal(4), 501);
  EXPECT_EQ(_data->frequencyTotal(5), 0);
}

TEST_F(KanjiDataTest, SortingAndPrintingQualifiedName) {
  std::vector<std::string> list({"弓", "弖", "窮", "弼", "穹", "躬"});
  Data::List kanjis;
  for (auto& i : list) {
    auto k = _data->findKanjiByName(i);
    ASSERT_TRUE(k);
    kanjis.push_back(*k);
  }
  std::sort(kanjis.begin(), kanjis.end(), Data::orderByQualifiedName);
  std::string sorted;
  for (auto& i : kanjis) {
    if (!sorted.empty()) sorted += ' ';
    sorted += i->qualifiedName();
  }
  EXPECT_EQ(sorted, "弓. 窮. 穹^ 弼@ 弖# 躬#");
  // Make sure all Kanji are in Kanji related Unicode blocks
  EXPECT_EQ(checkKanji(_data->jouyouKanji()), 0);
  EXPECT_EQ(checkKanji(_data->jinmeiKanji()), 0);
  // 52 LinkedJinmei type Kanji use the Unicode 'Variation Selector'
  EXPECT_EQ(checkKanji(_data->linkedJinmeiKanji()), 52);
  EXPECT_EQ(checkKanji(_data->linkedOldKanji()), 0);
  EXPECT_EQ(checkKanji(_data->extraKanji()), 0);
  EXPECT_EQ(checkKanji(_data->frequencyKanji()), 0);
}

TEST_F(KanjiDataTest, FindByName) {
  auto result = _data->findKanjiByName("響︀");
  ASSERT_TRUE(result);
  auto& k = **result;
  EXPECT_EQ(k.type(), KanjiTypes::LinkedJinmei);
  EXPECT_EQ(k.name(), "響︀");
  EXPECT_EQ(k.radical(), _data->getRadicalByName("音"));
  EXPECT_FALSE(k.hasLevel());
  EXPECT_FALSE(k.hasGrade());
  EXPECT_FALSE(k.frequency());
  EXPECT_TRUE(k.variant());
  auto result2 = _data->findKanjiByName("逸︁");
  EXPECT_TRUE((**result2).variant());
  EXPECT_EQ((**result2).type(), KanjiTypes::LinkedJinmei);
  EXPECT_EQ((**result2).nonVariantName(), "逸");
}

TEST_F(KanjiDataTest, FindKanjiByFrequency) {
  ASSERT_FALSE(_data->findKanjiByFrequency(0));
  ASSERT_FALSE(_data->findKanjiByFrequency(2502));
  for (size_t i = 1; i < 2502; ++i) ASSERT_TRUE(_data->findKanjiByFrequency(i));
  EXPECT_EQ((**_data->findKanjiByFrequency(1)).name(), "日");
  EXPECT_EQ((**_data->findKanjiByFrequency(2001)).name(), "炒");
  EXPECT_EQ((**_data->findKanjiByFrequency(2501)).name(), "蝦");
}

TEST_F(KanjiDataTest, FindKanjisByMorohashiId) {
  auto& morohashi = _data->findKanjisByMorohashiId("4138");
  ASSERT_EQ(morohashi.size(), 1);
  EXPECT_EQ(morohashi[0]->name(), "嗩");
  auto& morohashiPrime = _data->findKanjisByMorohashiId("4138P");
  ASSERT_EQ(morohashiPrime.size(), 1);
  EXPECT_EQ(morohashiPrime[0]->name(), "嘆");
  auto& multiMorohashi = _data->findKanjisByMorohashiId("3089");
  ASSERT_EQ(multiMorohashi.size(), 2);
  EXPECT_EQ(multiMorohashi[0]->name(), "叁"); // Unicode 53C1
  EXPECT_EQ(multiMorohashi[1]->name(), "叄"); // Unicode 53C4
}

TEST_F(KanjiDataTest, FindKanjisByNelsonId) {
  ASSERT_TRUE(_data->findKanjisByNelsonId(-1).empty());
  ASSERT_TRUE(_data->findKanjisByNelsonId(0).empty());
  ASSERT_TRUE(_data->findKanjisByNelsonId(5447).empty());
  std::vector<int> missingNelsonIds;
  for (auto i = 1; i < 5447; ++i)
    if (_data->findKanjisByNelsonId(i).empty()) missingNelsonIds.push_back(i);
  // There are a few Nelson IDs that are missing from UCD data
  EXPECT_EQ(missingNelsonIds, std::vector({125, 149, 489, 1639}));
  EXPECT_EQ(_data->findKanjisByNelsonId(1)[0]->name(), "一");
  EXPECT_EQ(_data->findKanjisByNelsonId(5446)[0]->name(), "龠");
}

TEST_F(KanjiDataTest, KanjiWithMultipleOldNames) {
  // kanji with 3 old names
  auto result3 = _data->findKanjiByName("弁");
  ASSERT_TRUE(result3);
  EXPECT_EQ((**result3).oldNames(), Kanji::LinkNames({"辨", "瓣", "辯"}));
  EXPECT_EQ((**result3).info(KanjiInfo::Old), "Old 辨／瓣／辯");
  for (auto& i : (**result3).oldNames()) {
    auto old = _data->findKanjiByName(i);
    ASSERT_TRUE(old);
    ASSERT_EQ((**old).type(), KanjiTypes::LinkedOld);
    EXPECT_EQ(static_cast<const LinkedKanji&>(**old).link(), result3);
  }
}

TEST_F(KanjiDataTest, UcdChecks) {
  // 'shrimp' is a Jinmei kanji, but 'jinmei.txt' doesn't include a Meaning
  // column so the value is pulled from UCD.
  auto& shrimp = **_data->findKanjiByName("蝦");
  EXPECT_EQ(shrimp.meaning(), "shrimp, prawn");
  // 'dull' is only in 'frequency.txt' so radical, strokes, meaning and reading
  // are all pulled from UCD (and readings are converted to Kana).
  auto& dull = **_data->findKanjiByName("呆");
  EXPECT_EQ(dull.radical(), _data->getRadicalByName("口"));
  EXPECT_EQ(dull.strokes(), 7);
  EXPECT_EQ(dull.meaning(), "dull; dull-minded, simple, stupid");
  // Note: unlike official lists (and 'extra.txt'), 'kun' readings from UCD
  // unfortunately don't have a dash before the Okurigana.
  EXPECT_EQ(dull.reading(), "ボウ、ガイ、ホウ、おろか、あきれる");
}

TEST_F(KanjiDataTest, KanjiWithMultipleNelsonIds) {
  auto ucdNelson = _data->ucd().find("㡡");
  ASSERT_NE(ucdNelson, nullptr);
  EXPECT_EQ(ucdNelson->nelsonIds(), "1487,1491");
  auto& kanjiNelson = **_data->findKanjiByName(ucdNelson->name());
  EXPECT_EQ(kanjiNelson.nelsonIds(), Kanji::NelsonIds({1487, 1491}));
  auto& ids = _data->findKanjisByNelsonId(1491);
  ASSERT_EQ(ids.size(), 3);
}

TEST_F(KanjiDataTest, UcdLinksMapToNewName) {
  std::string north("北"), variantNorth("北");
  EXPECT_EQ(toUnicode(north), "5317");
  EXPECT_EQ(toUnicode(variantNorth), "F963");
  EXPECT_NE(north, variantNorth);
  auto variantNorthKanji = _data->findKanjiByName(variantNorth);
  ASSERT_TRUE(variantNorthKanji);
  EXPECT_EQ((**variantNorthKanji).type(), KanjiTypes::Ucd);
  EXPECT_EQ((**variantNorthKanji).name(), variantNorth);
  EXPECT_EQ((**variantNorthKanji).newName(), north);
  auto northKanji = _data->findKanjiByName(north);
  ASSERT_TRUE(northKanji);
  EXPECT_EQ((**northKanji).type(), KanjiTypes::Jouyou);
}

TEST_F(KanjiDataTest, UnicodeBlocksAndSources) {
  // Only some Ucd Kanji are in the 'rare' blocks. All other types (like Jouyou,
  // Jinmei Frequency, Kentei, etc.) should be in the 'common' bloacks.
  auto rareUcd = 0;
  std::map<std::string, int> rareMissingJSource;
  std::map<KanjiTypes, int> missingJSource;
  for (auto& i : _data->ucd().map()) {
    auto& u = i.second;
    // at least one of 'on', 'kun', 'jSource' or 'morohashiId' must have a value
    EXPECT_FALSE(u.onReading().empty() && u.kunReading().empty() &&
                 u.jSource().empty() && u.morohashiId().empty());
    if (isRareKanji(i.first)) {
      if (auto t = _data->getType(i.first); t != KanjiTypes::Ucd)
        FAIL() << "rare kanji '" << i.first << "' has type: " << toString(t);
      // rare kanji have a jSource value (since that's how they got pulled in)
      EXPECT_FALSE(u.jSource().empty());
      ++rareUcd;
    } else if (!isCommonKanji(i.first))
      FAIL() << "kanji '" << i.first << "' not recognized";
    else if (u.jSource().empty()) {
      if (auto t = _data->getType(i.first); t == KanjiTypes::LinkedOld)
        EXPECT_EQ(i.first, "絕"); // old form of 絶 doesn't have a jSource
      else
        ++missingJSource[t]; // other with empty jSource should be Kentei or Ucd
    } else
      // make sure 'J' is contained in 'sources' if 'jSource' is non-empty
      EXPECT_NE(u.sources().find('J'), std::string::npos);
  }
  EXPECT_EQ(rareUcd, 2534);
  // missing JSource for common Kanji are either 'Kentei' or 'Ucd' type
  EXPECT_EQ(missingJSource.size(), 2);
  EXPECT_EQ(missingJSource[KanjiTypes::Kentei], 16);
  EXPECT_EQ(missingJSource[KanjiTypes::Ucd], 7472);
}

TEST_F(KanjiDataTest, UcdLinks) {
  auto& ucd = _data->ucd().map();
  EXPECT_EQ(ucd.size(), _data->kanjiNameMap().size());
  auto jouyou{0}, jinmei{0}, jinmeiLinks{0}, jinmeiLinksToJouyou{0},
    jinmeiLinksToJinmei{0};
  std::map<KanjiTypes, int> otherLinks;
  // every 'linkName' should be different than 'name' and also exist in the map
  for (auto& i : ucd) {
    auto& u = i.second;
    // every Ucd entry should be a wide character, i.e., have 'display size' 2
    EXPECT_EQ(displaySize(u.name()), 2);
    // if 'variantStrokes' is present it should be different than 'strokes'
    if (u.hasVariantStrokes())
      EXPECT_NE(u.strokes(), u.variantStrokes()) << u.codeAndName();
    // make sure MBUtils UCD characters are part of MBUtils unicode blocks
    if (u.joyo() || u.jinmei())
      EXPECT_TRUE(isCommonKanji(u.name())) << u.codeAndName();
    else
      EXPECT_TRUE(isKanji(u.name())) << u.codeAndName();
    // make sure links point to other valid UCD entries
    for (auto& j : u.links()) {
      EXPECT_NE(u.name(), j.name());
      const auto link = ucd.find(j.name());
      ASSERT_FALSE(link == ucd.end()) << j.name();
    }
    if (u.joyo()) {
      EXPECT_FALSE(u.jinmei()) << u.codeAndName() << " is both joyo and jinmei";
      EXPECT_FALSE(u.hasLinks()) << u.codeAndName() << " joyo has a link";
      ++jouyou;
    } else if (u.jinmei()) {
      ++jinmei;
      if (u.hasLinks()) {
        EXPECT_EQ(u.links().size(), 1) << u.name();
        ++jinmeiLinks;
        auto& link = ucd.find(u.links()[0].name())->second;
        if (link.joyo())
          ++jinmeiLinksToJouyou;
        else if (link.jinmei())
          ++jinmeiLinksToJinmei;
        else
          FAIL() << "jinmei '" << u.name()
                 << "' shouldn't have non-official link";
        if (link.hasLinks()) EXPECT_NE(link.links()[0].name(), u.name());
      }
    } else if (u.hasLinks())
      ++otherLinks[_data->getType(u.name())];
  }
  EXPECT_EQ(jouyou, _data->jouyouKanji().size());
  EXPECT_EQ(jinmei - jinmeiLinks, _data->jinmeiKanji().size());
  EXPECT_EQ(jinmeiLinks, _data->linkedJinmeiKanji().size());
  EXPECT_EQ(otherLinks[KanjiTypes::Extra], 10);
  EXPECT_EQ(otherLinks[KanjiTypes::Frequency], 15);
  EXPECT_EQ(otherLinks[KanjiTypes::Kentei], 232);
  EXPECT_EQ(otherLinks[KanjiTypes::Ucd], 2838);
  EXPECT_EQ(otherLinks[KanjiTypes::LinkedJinmei], 0); // part of 'jinmeiLinks'
  EXPECT_EQ(otherLinks[KanjiTypes::LinkedOld], 90);
  auto officialLinksToJinmei{0}, officialLinksToJouyou{0};
  for (auto& i : _data->linkedJinmeiKanji()) {
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
