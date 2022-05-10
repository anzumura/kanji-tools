#include <gtest/gtest.h>
#include <kanji_tools/kana/DisplaySize.h>
#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/kanji/LinkedKanji.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <type_traits>

namespace kanji_tools {

namespace {

class KanjiDataTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    // Constructs KanjiData using the real data files
    _data = std::make_shared<KanjiData>();
  }

  [[nodiscard]] static auto checkKanji(const Data::KanjiList& l) { // NOLINT
    size_t variants{};
    for (auto& i : l) {
      if (i->variant()) {
        EXPECT_NE(i->name(), i->nonVariantName());
        EXPECT_NE(i->name(), i->compatibilityName());
        const auto j{_data->findKanjiByName(i->compatibilityName())};
        EXPECT_TRUE(j);
        if (j) {
          EXPECT_EQ(j->type(), i->type());
          EXPECT_EQ(j->name(), i->name());
        }
        ++variants;
      }
      EXPECT_EQ(MBChar::size(i->name()), 1)
          << i->type() << ", " << i->name() << ", " << toUnicode(i->name());
      EXPECT_TRUE(isKanji(i->name()))
          << i->type() << ", " << i->name() << ", " << toUnicode(i->name());
    }
    return variants;
  }

  inline static DataPtr _data;
};

} // namespace

TEST_F(KanjiDataTest, BasicChecks) {
  EXPECT_EQ(_data->kanjiNameMap().size(), 23715);
  EXPECT_EQ(_data->level("院"), JlptLevels::N4);
  EXPECT_EQ(_data->frequency("蝦"), 2501);
  // Ucd data related
  EXPECT_EQ(_data->ucd().map().size(), _data->kanjiNameMap().size());
  EXPECT_EQ(Pinyin::size(), 1337);
  EXPECT_EQ(UcdBlock::size(), 8);
  EXPECT_EQ(UcdVersion::size(), 10);
}

TEST_F(KanjiDataTest, FrequencyKanjiChecks) {
  const auto yeast{_data->findKanjiByName("麹")};
  ASSERT_TRUE(yeast);
  EXPECT_EQ(yeast->type(), KanjiTypes::Frequency);
  EXPECT_FALSE(yeast->hasGrade());
  EXPECT_FALSE(yeast->hasLevel());
  EXPECT_EQ(yeast->kyu(), KenteiKyus::KJ1);
  EXPECT_EQ(yeast->frequency(), 1988);
  EXPECT_FALSE(yeast->newName());
  EXPECT_EQ(yeast->oldNames(), Kanji::LinkNames{"麴"});
  EXPECT_EQ(yeast->reading(), "キク、こうじ");
  EXPECT_EQ(yeast->meaning(), "yeast, leaven; surname");
}

TEST_F(KanjiDataTest, ExtraKanjiChecks) {
  const auto grab{_data->findKanjiByName("掴")};
  ASSERT_TRUE(grab);
  EXPECT_EQ(grab->type(), KanjiTypes::Extra);
  EXPECT_FALSE(grab->hasGrade());
  EXPECT_FALSE(grab->hasLevel());
  EXPECT_EQ(grab->kyu(), KenteiKyus::KJ1);
  EXPECT_FALSE(grab->frequency());
  EXPECT_FALSE(grab->newName());
  EXPECT_EQ(grab->oldNames(), Kanji::LinkNames{"摑"});
  EXPECT_EQ(grab->reading(), "カク、つか-む、つか-まえる、つか-まる");
  EXPECT_EQ(grab->meaning(), "catch");
}

TEST_F(KanjiDataTest, KenteiKanjiChecks) {
  const auto apple{_data->findKanjiByName("蘋")};
  ASSERT_TRUE(apple);
  EXPECT_EQ(apple->type(), KanjiTypes::Kentei);
  EXPECT_FALSE(apple->hasGrade());
  EXPECT_FALSE(apple->hasLevel());
  EXPECT_EQ(apple->kyu(), KenteiKyus::K1);
  EXPECT_EQ(apple->reading(), "ヒン、ビン、うきくさ、でんじそ");
  EXPECT_EQ(apple->meaning(), "apple");
  EXPECT_EQ(apple->newName(), "苹");
  EXPECT_FALSE(apple->linkedReadings());
}

TEST_F(KanjiDataTest, UcdKanjiChecks) {
  const auto complete{_data->findKanjiByName("侭")};
  ASSERT_TRUE(complete);
  EXPECT_EQ(complete->type(), KanjiTypes::Ucd);
  EXPECT_FALSE(complete->hasGrade());
  EXPECT_FALSE(complete->hasLevel());
  EXPECT_FALSE(complete->hasKyu());
  EXPECT_EQ(complete->reading(), "ジン、ことごとく、まま");
  EXPECT_EQ(complete->meaning(), "complete, utmost");
  EXPECT_FALSE(complete->linkedReadings());
  const auto shape{_data->findKanjiByName("檨")};
  ASSERT_TRUE(shape);
  EXPECT_EQ(shape->type(), KanjiTypes::Ucd);
  EXPECT_TRUE(shape->linkedReadings());
}

TEST_F(KanjiDataTest, RadicalChecks) {
  const auto radical{_data->getRadicalByName("鹿")};
  EXPECT_EQ(radical.number(), 198);
  EXPECT_EQ(radical.name(), "鹿");
  EXPECT_EQ(radical.longName(), "鹿部（ろくぶ）");
  EXPECT_EQ(radical.reading(), "しか");
}

TEST_F(KanjiDataTest, TotalsChecks) {
  EXPECT_EQ(_data->gradeSize(KanjiGrades::G1), 80);
  EXPECT_EQ(_data->gradeSize(KanjiGrades::G2), 160);
  EXPECT_EQ(_data->gradeSize(KanjiGrades::G3), 200);
  EXPECT_EQ(_data->gradeSize(KanjiGrades::G4), 200);
  EXPECT_EQ(_data->gradeSize(KanjiGrades::G5), 185);
  EXPECT_EQ(_data->gradeSize(KanjiGrades::G6), 181);
  EXPECT_EQ(_data->gradeSize(KanjiGrades::S), 1130);
  EXPECT_EQ(_data->gradeSize(KanjiGrades::None), 0);
  EXPECT_EQ(_data->levelSize(JlptLevels::N5), 103);
  EXPECT_EQ(_data->levelSize(JlptLevels::N4), 181);
  EXPECT_EQ(_data->levelSize(JlptLevels::N3), 361);
  EXPECT_EQ(_data->levelSize(JlptLevels::N2), 415);
  EXPECT_EQ(_data->levelSize(JlptLevels::N1), 1162);
  EXPECT_EQ(_data->levelSize(JlptLevels::None), 0);
  EXPECT_EQ(_data->frequencySize(0), 500);
  EXPECT_EQ(_data->frequencySize(1), 500);
  EXPECT_EQ(_data->frequencySize(2), 500);
  EXPECT_EQ(_data->frequencySize(3), 500);
  EXPECT_EQ(_data->frequencySize(4), 501);
  EXPECT_EQ(_data->frequencySize(5), 0);
}

TEST_F(KanjiDataTest, SortingAndPrintingQualifiedName) {
  std::vector<String> list{"弓", "弖", "窮", "弼", "穹", "躬"};
  Data::KanjiList kanjis;
  for (auto& i : list) {
    const auto k{_data->findKanjiByName(i)};
    ASSERT_TRUE(k);
    kanjis.emplace_back(k);
  }
  std::sort(kanjis.begin(), kanjis.end(), Data::OrderByQualifiedName);
  String sorted;
  for (auto& i : kanjis) {
    if (!sorted.empty()) sorted += ' ';
    sorted += i->qualifiedName();
  }
  EXPECT_EQ(sorted, "弓. 窮. 穹^ 弼@ 弖# 躬#");
  // Make sure all Kanji are in Kanji related Unicode blocks
  EXPECT_EQ(checkKanji(_data->types(KanjiTypes::Jouyou)), 0);
  EXPECT_EQ(checkKanji(_data->types(KanjiTypes::Jinmei)), 0);
  // 52 LinkedJinmei type Kanji use the Unicode 'Variation Selector'
  EXPECT_EQ(checkKanji(_data->types(KanjiTypes::LinkedJinmei)), 52);
  EXPECT_EQ(checkKanji(_data->types(KanjiTypes::LinkedOld)), 0);
  EXPECT_EQ(checkKanji(_data->types(KanjiTypes::Extra)), 0);
  EXPECT_EQ(checkKanji(_data->types(KanjiTypes::Frequency)), 0);
}

TEST_F(KanjiDataTest, FindByName) {
  const auto result{_data->findKanjiByName("響︀")};
  ASSERT_TRUE(result);
  EXPECT_EQ(result->type(), KanjiTypes::LinkedJinmei);
  EXPECT_EQ(result->name(), "響︀");
  EXPECT_EQ(result->radical(), _data->getRadicalByName("音"));
  EXPECT_FALSE(result->hasLevel());
  EXPECT_FALSE(result->hasGrade());
  EXPECT_FALSE(result->frequency());
  EXPECT_TRUE(result->variant());
  const auto result2{_data->findKanjiByName("逸︁")};
  EXPECT_TRUE(result2->variant());
  EXPECT_EQ(result2->type(), KanjiTypes::LinkedJinmei);
  EXPECT_EQ(result2->nonVariantName(), "逸");
}

TEST_F(KanjiDataTest, FindKanjiByFrequency) {
  ASSERT_FALSE(_data->findKanjiByFrequency(0));
  ASSERT_FALSE(_data->findKanjiByFrequency(2502));
  for (Kanji::Frequency i{1}; i < Data::maxFrequency(); ++i)
    ASSERT_TRUE(_data->findKanjiByFrequency(i));
  EXPECT_EQ(_data->findKanjiByFrequency(1)->name(), "日");
  EXPECT_EQ(_data->findKanjiByFrequency(2001)->name(), "炒");
  EXPECT_EQ(_data->findKanjiByFrequency(2501)->name(), "蝦");
}

TEST_F(KanjiDataTest, FindKanjisByMorohashiId) {
  auto& morohashi{_data->findByMorohashiId("4138")};
  ASSERT_EQ(morohashi.size(), 1);
  EXPECT_EQ(morohashi[0]->name(), "嗩");
  auto& morohashiPrime{_data->findByMorohashiId("4138P")};
  ASSERT_EQ(morohashiPrime.size(), 1);
  EXPECT_EQ(morohashiPrime[0]->name(), "嘆");
  auto& multiMorohashi{_data->findByMorohashiId("3089")};
  ASSERT_EQ(multiMorohashi.size(), 2);
  EXPECT_EQ(multiMorohashi[0]->name(), "叁"); // Unicode 53C1
  EXPECT_EQ(multiMorohashi[1]->name(), "叄"); // Unicode 53C4
}

TEST_F(KanjiDataTest, FindKanjisByNelsonId) {
  constexpr Kanji::NelsonId totalNelsonIds{5447};
  ASSERT_TRUE(_data->findByNelsonId(0).empty());
  ASSERT_TRUE(_data->findByNelsonId(totalNelsonIds).empty());
  Kanji::NelsonIds missingNelsonIds;
  for (Kanji::NelsonId i{1}; i < totalNelsonIds; ++i)
    if (_data->findByNelsonId(i).empty()) missingNelsonIds.push_back(i);
  // There are a few Nelson IDs that are missing from UCD data
  EXPECT_EQ(missingNelsonIds, (Kanji::NelsonIds{125, 149, 489, 1639}));
  EXPECT_EQ(_data->findByNelsonId(1)[0]->name(), "一");
  EXPECT_EQ(_data->findByNelsonId(5446)[0]->name(), "龠");
}

TEST_F(KanjiDataTest, KanjiWithMultipleOldNames) {
  // kanji with 3 old names
  auto result3{_data->findKanjiByName("弁")};
  ASSERT_TRUE(result3);
  EXPECT_EQ(result3->oldNames(), (Kanji::LinkNames{"辨", "瓣", "辯"}));
  EXPECT_EQ(result3->info(KanjiInfo::Old), "Old 辨／瓣／辯");
  for (auto& i : result3->oldNames()) {
    const auto old{_data->findKanjiByName(i)};
    ASSERT_TRUE(old);
    ASSERT_EQ(old->type(), KanjiTypes::LinkedOld);
    EXPECT_EQ(old->link(), result3);
  }
}

TEST_F(KanjiDataTest, UcdChecks) {
  // 'shrimp' is a Jinmei kanji, but 'jinmei.txt' doesn't include a Meaning
  // column so the value is pulled from UCD.
  auto& shrimp{*_data->findKanjiByName("蝦")};
  EXPECT_EQ(shrimp.meaning(), "shrimp, prawn");
  // 'dull' is only in 'frequency.txt' so radical, strokes, meaning and reading
  // are all pulled from UCD (and readings are converted to Kana).
  auto& dull{*_data->findKanjiByName("呆")};
  EXPECT_EQ(dull.radical(), _data->getRadicalByName("口"));
  EXPECT_EQ(dull.strokes().value(), 7);
  EXPECT_EQ(dull.meaning(), "dull; dull-minded, simple, stupid");
  // Note: unlike official lists (and 'extra.txt'), 'kun' readings from UCD
  // unfortunately don't have a dash before the Okurigana.
  EXPECT_EQ(dull.reading(), "ボウ、ガイ、ホウ、おろか、あきれる");
}

TEST_F(KanjiDataTest, KanjiWithMultipleNelsonIds) {
  constexpr Kanji::NelsonId id{1491};
  const auto ucdNelson{_data->ucd().find("㡡")};
  ASSERT_NE(ucdNelson, nullptr);
  EXPECT_EQ(ucdNelson->nelsonIds(), "1487," + std::to_string(id));
  auto& kanjiNelson{*_data->findKanjiByName(ucdNelson->name())};
  EXPECT_EQ(kanjiNelson.nelsonIds(), (Kanji::NelsonIds{1487, id}));
  auto& ids{_data->findByNelsonId(id)};
  ASSERT_EQ(ids.size(), 3);
}

TEST_F(KanjiDataTest, UcdLinksMapToNewName) {
  const String north{"北"}, variantNorth{"北"};
  EXPECT_EQ(toUnicode(north), "5317");
  EXPECT_EQ(toUnicode(variantNorth), "F963");
  EXPECT_NE(north, variantNorth);
  const auto variantNorthKanji{_data->findKanjiByName(variantNorth)};
  ASSERT_TRUE(variantNorthKanji);
  EXPECT_EQ(variantNorthKanji->type(), KanjiTypes::Ucd);
  EXPECT_EQ(variantNorthKanji->name(), variantNorth);
  EXPECT_EQ(variantNorthKanji->newName(), north);
  const auto northKanji{_data->findKanjiByName(north)};
  ASSERT_TRUE(northKanji);
  EXPECT_EQ(northKanji->type(), KanjiTypes::Jouyou);
}

TEST_F(KanjiDataTest, UnicodeBlocksAndSources) {
  // Only some Ucd Kanji are in the 'rare' blocks. All other types (like Jouyou,
  // Jinmei Frequency, Kentei, etc.) should be in the 'common' blocks.
  uint32_t rareUcd{};
  std::map<String, uint32_t> rareMissingJSource;
  std::map<KanjiTypes, uint32_t> missingJSource;
  for (auto& i : _data->ucd().map()) {
    auto& u{i.second};
    // at least one of 'on', 'kun', 'jSource' or 'morohashiId' must have a value
    EXPECT_FALSE(u.onReading().empty() && u.kunReading().empty() &&
                 u.jSource().empty() && !u.morohashiId());
    if (isRareKanji(i.first)) {
      if (const auto t{_data->getType(i.first)}; t != KanjiTypes::Ucd)
        FAIL() << "rare kanji '" << i.first << "' has type: " << toString(t);
      // rare kanji have a jSource value (since that's how they got pulled in)
      EXPECT_FALSE(u.jSource().empty());
      ++rareUcd;
    } else if (!isCommonKanji(i.first))
      FAIL() << "kanji '" << i.first << "' not recognized";
    else if (u.jSource().empty()) {
      if (const auto t{_data->getType(i.first)}; t == KanjiTypes::LinkedOld)
        EXPECT_EQ(i.first, "絕"); // old form of 絶 doesn't have a jSource
      else
        ++missingJSource[t]; // other with empty jSource should be Kentei or Ucd
    } else
      // make sure 'J' is contained in 'sources' if 'jSource' is non-empty
      EXPECT_NE(u.sources().find('J'), String::npos);
  }
  EXPECT_EQ(rareUcd, 2534);
  // missing JSource for common Kanji are either 'Kentei' or 'Ucd' type
  EXPECT_EQ(missingJSource.size(), 2);
  EXPECT_EQ(missingJSource[KanjiTypes::Kentei], 16);
  EXPECT_EQ(missingJSource[KanjiTypes::Ucd], 7472);
}

TEST_F(KanjiDataTest, UcdLinks) {
  auto& ucd{_data->ucd().map()};
  EXPECT_EQ(ucd.size(), _data->kanjiNameMap().size());
  uint32_t jouyou{}, jinmei{}, jinmeiLinks{}, jinmeiLinksToJouyou{},
      jinmeiLinksToJinmei{};
  std::map<KanjiTypes, uint32_t> otherLinks;
  // every 'linkName' should be different than 'name' and also exist in the map
  for (auto& i : ucd) {
    auto& u{i.second};
    // every Ucd entry should be a wide character, i.e., have 'display size' 2
    EXPECT_EQ(displaySize(u.name()), 2);
    // make sure MBUtils UCD characters are part of MBUtils unicode blocks
    if (u.joyo() || u.jinmei())
      EXPECT_TRUE(isCommonKanji(u.name())) << u.codeAndName();
    else
      EXPECT_TRUE(isKanji(u.name())) << u.codeAndName();
    // make sure links point to other valid UCD entries
    for (auto& j : u.links()) {
      EXPECT_NE(u.name(), j.name());
      const auto link{ucd.find(j.name())};
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
        auto& link{ucd.find(u.links()[0].name())->second};
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
  EXPECT_EQ(jouyou, _data->typeSize(KanjiTypes::Jouyou));
  EXPECT_EQ(jinmei - jinmeiLinks, _data->typeSize(KanjiTypes::Jinmei));
  EXPECT_EQ(jinmeiLinks, _data->typeSize(KanjiTypes::LinkedJinmei));
  EXPECT_EQ(otherLinks[KanjiTypes::Extra], 10);
  EXPECT_EQ(otherLinks[KanjiTypes::Frequency], 15);
  EXPECT_EQ(otherLinks[KanjiTypes::Kentei], 232);
  EXPECT_EQ(otherLinks[KanjiTypes::Ucd], 2838);
  EXPECT_EQ(otherLinks[KanjiTypes::LinkedJinmei], 0); // part of 'jinmeiLinks'
  EXPECT_EQ(otherLinks[KanjiTypes::LinkedOld], 90);
  uint32_t officialLinksToJinmei{}, officialLinksToJouyou{};
  for (auto& i : _data->types(KanjiTypes::LinkedJinmei)) {
    auto& link{*i->link()};
    if (link.type() == KanjiTypes::Jouyou)
      ++officialLinksToJouyou;
    else if (link.type() == KanjiTypes::Jinmei)
      ++officialLinksToJinmei;
    else
      FAIL() << "link from " << link.name() << " is type " << link.type();
  }
  EXPECT_EQ(jinmeiLinksToJouyou, officialLinksToJouyou);
  EXPECT_EQ(jinmeiLinksToJinmei, officialLinksToJinmei);
}

TEST_F(KanjiDataTest, SortByQualifiedName) {
  const auto find{[](const String& name, auto t, auto s, Kanji::Frequency f,
                      const String& u = EmptyString) {
    auto k{_data->findKanjiByName(name)};
    // can't use 'ASSERT' in a function returning non-void so throw an exception
    // if not found (which never happens by design of the rest of this test)
    if (!k) throw std::domain_error(name + " not found");
    // verify attributes of the Kanji found match expected values
    EXPECT_EQ(k->type(), t);
    EXPECT_EQ(k->strokes().value(), s);
    EXPECT_EQ(k->frequency(), f);
    if (!u.empty()) EXPECT_EQ(toUnicode(k->compatibilityName()), u);
    return k;
  }};
  // choose some existing Kanji with 'small', 'medium' and 'high' values for
  // Strokes and Frequency to help test sorting, i.e., small < medium < high
  constexpr Strokes::Size SmallS{4}, MediumS{7}, HighS{10};
  constexpr Kanji::Frequency SmallF{150}, MediumF{276}, HighF{771};
  auto jouyou7stroke1{find("位", KanjiTypes::Jouyou, MediumS, MediumF)};
  auto jouyou7stroke2{find("囲", KanjiTypes::Jouyou, MediumS, HighF)};
  auto jouyou10stroke{find("院", KanjiTypes::Jouyou, HighS, SmallF)};
  auto jinmei4stroke1{find("云", KanjiTypes::Jinmei, SmallS, {}, "4E91")};
  auto jinmei4stroke2{find("勿", KanjiTypes::Jinmei, SmallS, {}, "52FF")};

  const auto check{[](auto& x, auto& y) {
    EXPECT_TRUE(Data::OrderByQualifiedName(x, y));
    EXPECT_FALSE(Data::OrderByQualifiedName(y, x));
  }};
  // sort by qualified type first (so Jouyou is less then Jinmei)
  check(jouyou10stroke, jinmei4stroke1);
  check(jouyou10stroke, jinmei4stroke2);
  // if qualified type is the same then sort by stokes
  check(jouyou7stroke1, jouyou10stroke);
  check(jouyou7stroke2, jouyou10stroke);
  // if qualified type and strokes are the same then sort by frequency
  check(jouyou7stroke1, jouyou7stroke2);
  // if type and strokes are the same (and no frequency) then sort by unicode
  check(jinmei4stroke1, jinmei4stroke2);
}

TEST(KanjiDataPrintTest, Info) {
  const char* args[]{"", Data::InfoArg.c_str()};
  std::stringstream os;
  KanjiData data(args, os);
  const char* expected[]{
      (">>> Loaded 23715 Kanji (Jouyou 2136 Jinmei 633 LinkedJinmei 230 "
       "LinkedOld 163 Frequency 124 Extra 136 Kentei 2822 Ucd 17471)"),
      ">>> Grade breakdown:",
      ">>>   Total for grade G1: 80 (N5 57, N4 15, N3 8)",
      ">>>   Total for grade G2: 160 (N5 43, N4 74, N3 43)",
      ">>>   Total for grade G3: 200 (N5 3, N4 67, N3 130)",
      ">>>   Total for grade G4: 200 (N4 20, N3 180)",
      ">>>   Total for grade G5: 185 (N4 2, N2 149, N1 34)",
      ">>>   Total for grade G6: 181 (N4 3, N2 105, N1 73)",
      ">>>   Total for grade S: 1130 (nf 99) (N2 161, N1 804, None 165)",
      ">>>   Total for all grades: 2136"};
  int count{0}, maxLines{std::size(expected)};
  for (String line; std::getline(os, line); ++count) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST(KanjiDataPrintTest, Debug) {
  const char* args[]{"", Data::DebugArg.c_str()};
  std::stringstream os;
  KanjiData data(args, os);
  String lastLine;
  size_t count{}, found{};
  // output is really big so just check for a few examples
  for (String line; std::getline(os, line); ++count, lastLine = line) {
    if (count == 1)
      EXPECT_EQ(line, ">>> Begin Loading Data");
    else if (line.starts_with(">>> Found ")) {
      const String s{line.substr(10)};
      // check each line against all strings (to detect possible duplicates)
      if (s.starts_with("251 Jinmei in N1")) ++found;
      if (s.starts_with("2 Linked Old in Frequency")) ++found;
      if (s.starts_with("124 non-Jouyou/Jinmei/JLPT in Frequency")) ++found;
      if (s.starts_with("168 JLPT Jinmei in Frequency")) ++found;
      if (s.starts_with("158 non-JLPT Jinmei in Frequency")) ++found;
      if (s.starts_with("158 non-JLPT Jinmei in Frequency")) ++found;
      if (s.starts_with("12 non-JLPT Linked Jinmei in Frequency")) ++found;
      if (s.starts_with("12 Jouyou Kanji with differrent strokes")) ++found;
      if (s.starts_with("1 Extra Kanji with differrent strokes")) ++found;
    } else {
      if (line == ">>> Frequency Kanji with links 15:") ++found;
      if (line == ">>> Extra Kanji with links 10:") ++found;
      if (line.starts_with(">>>   Total for 214 radicals: 21181")) ++found;
    }
  }
  EXPECT_TRUE(lastLine.starts_with(">>>     52     [985E FE00] 類︀"));
  EXPECT_EQ(found, 12);
  EXPECT_EQ(count, 361);
}

} // namespace kanji_tools
