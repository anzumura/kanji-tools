#include <gtest/gtest.h>
#include <kanji_tools/kana/Kana.h>
#include <kanji_tools/kana/MBChar.h>

namespace kanji_tools {

namespace {

enum Values {
  HanDakuten = 5,       // both mono- and di-graphs have the same number
  SmallMonographs = 12, // digraphs end (but don't start) with small kana
  DakutenMonographs = 21,
  DakutenDigraphs = 42,
  PlainMonographs = 48,
  PlainDigraphs = 71,
  RomajiVariants = 55
};
constexpr auto TotalMonographs{
    HanDakuten + SmallMonographs + DakutenMonographs + PlainMonographs},
    TotalDigraphs{HanDakuten + PlainDigraphs + DakutenDigraphs},
    TotalKana{TotalMonographs + TotalDigraphs},
    TotalRomaji{TotalKana + RomajiVariants};

} // namespace

TEST(CharTypeTest, CheckStrings) {
  EXPECT_EQ(toString(CharType::Hiragana), "Hiragana");
  EXPECT_EQ(toString(CharType::Katakana), "Katakana");
  EXPECT_EQ(toString(CharType::Romaji), "Romaji");
}

TEST(CharTypeTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(CharTypes[i], CharType::Hiragana);
  EXPECT_EQ(CharTypes[++i], CharType::Katakana);
  EXPECT_EQ(CharTypes[++i], CharType::Romaji);
}

TEST(KanaTest, FindDakuten) {
  EXPECT_EQ(Kana::findDakuten("か"), Kana::OptString("が"));
  EXPECT_EQ(Kana::findDakuten("シ"), Kana::OptString("ジ"));
  EXPECT_EQ(Kana::findDakuten("う"), Kana::OptString("ゔ"));
  EXPECT_FALSE(Kana::findDakuten("ま"));
}

TEST(KanaTest, FindHanDakuten) {
  EXPECT_EQ(Kana::findHanDakuten("は"), Kana::OptString("ぱ"));
  EXPECT_EQ(Kana::findHanDakuten("ホ"), Kana::OptString("ポ"));
  EXPECT_FALSE(Kana::findHanDakuten("さ"));
}

TEST(KanaTest, CheckHiragana) {
  auto& sourceMap{Kana::getMap(CharType::Hiragana)};
  EXPECT_EQ(sourceMap.size(), TotalKana);
  // count various types including smallDigraphs (which should be 0)
  u_int8_t hanDakutenMonographs{}, smallMonographs{}, plainMonographs{},
      dakutenMonographs{}, plainDigraphs{}, hanDakutenDigraphs{},
      dakutenDigraphs{}, smallDigraphs{};
  for (auto& i : sourceMap) {
    MBChar s{i.first};
    std::string c;
    const auto checkDigraph{
        [&i, &c](const std::string& a, const std::string& b = {}) {
          EXPECT_TRUE(c == a || (!b.empty() && c == b))
              << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
              << i.second->romaji() << "', hiragana " << i.first;
        }};
    EXPECT_TRUE(s.next(c));
    if (s.next(c)) {
      EXPECT_FALSE(i.second->isMonograph());
      EXPECT_TRUE(i.second->isDigraph());
      if (i.second->isSmall())
        ++smallDigraphs;
      else if (i.second->isDakuten())
        ++dakutenDigraphs;
      else if (i.second->isHanDakuten())
        ++hanDakutenDigraphs;
      else
        ++plainDigraphs;
      // if there's a second character it must be a small symbol matching the
      // final romaji letter
      const auto romajiLen{i.second->romaji().size()};
      ASSERT_GT(romajiLen, 1);
      if (i.second->romaji() == "qwa") // only digraph that ends with small 'wa'
        EXPECT_EQ(i.first, "くゎ");
      else
        switch (i.second->romaji()[romajiLen - 1]) {
        case 'a': checkDigraph("ぁ", "ゃ"); break;
        case 'i': checkDigraph("ぃ"); break;
        case 'u': checkDigraph("ぅ", "ゅ"); break;
        case 'e': checkDigraph("ぇ"); break;
        case 'o': checkDigraph("ぉ", "ょ"); break;
        default: checkDigraph("");
        }
      // can't be longer than 2 characters
      EXPECT_FALSE(s.next(c));
    } else {
      EXPECT_TRUE(i.second->isMonograph());
      EXPECT_FALSE(i.second->isDigraph());
      if (i.second->isSmall())
        ++smallMonographs;
      else if (i.second->isDakuten())
        ++dakutenMonographs;
      else if (i.second->isHanDakuten())
        ++hanDakutenMonographs;
      else
        ++plainMonographs;
    }
  }
  EXPECT_EQ(smallMonographs, SmallMonographs);
  EXPECT_EQ(plainMonographs, PlainMonographs);
  EXPECT_EQ(dakutenMonographs, DakutenMonographs);
  EXPECT_EQ(hanDakutenMonographs, HanDakuten);
  EXPECT_EQ(smallDigraphs, 0);
  EXPECT_EQ(plainDigraphs, PlainDigraphs);
  EXPECT_EQ(dakutenDigraphs, DakutenDigraphs);
  EXPECT_EQ(hanDakutenDigraphs, HanDakuten);
}

TEST(KanaTest, CheckKatakana) {
  auto& sourceMap{Kana::getMap(CharType::Katakana)};
  auto& hiraganaMap{Kana::getMap(CharType::Hiragana)};
  EXPECT_EQ(sourceMap.size(), TotalKana);
  for (auto& i : sourceMap) {
    MBChar s{i.first};
    // As long as all entries in katakana map are also be in hiragana map (and
    // the maps are the same size) then there's no need to checkDigraph the
    // various counts again.
    EXPECT_TRUE(hiraganaMap.contains(i.second->hiragana()));
    std::string c;
    const auto checkDigraph{
        [&i, &c](const std::string& a, const std::string& b = {}) {
          EXPECT_TRUE(c == a || (!b.empty() && c == b))
              << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
              << i.second->romaji() << "', katakana " << i.first;
        }};
    EXPECT_TRUE(s.next(c));
    if (s.next(c)) {
      // if there's a second character it must be a small symbol matching the
      // final romaji letter
      const auto romajiLen{i.second->romaji().size()};
      ASSERT_GT(romajiLen, 1);
      if (i.second->romaji() == "qwa") // only digraph that ends with small 'wa'
        EXPECT_EQ(i.first, "クヮ");
      else
        switch (i.second->romaji()[romajiLen - 1]) {
        case 'a': checkDigraph("ァ", "ャ"); break;
        case 'i': checkDigraph("ィ"); break;
        case 'u': checkDigraph("ゥ", "ュ"); break;
        case 'e': checkDigraph("ェ"); break;
        case 'o': checkDigraph("ォ", "ョ"); break;
        default: checkDigraph("");
        }
      // can't be longer than 2 characters
      EXPECT_FALSE(s.next(c));
    }
  }
}

TEST(KanaTest, CheckRomaji) {
  auto& sourceMap{Kana::getMap(CharType::Romaji)};
  EXPECT_EQ(sourceMap.size(), TotalRomaji);
  u_int8_t aNum{}, vaNum{}, iNum{}, viNum{}, uNum{}, vuNum{}, eNum{}, veNum{},
      oNum{}, voNum{}, nNum{};
  std::set<std::string> romajiVariants;
  for (auto& i : sourceMap) {
    const auto count{[&i](auto& normal, auto& variant) {
      i.second->romaji() == i.first ? ++normal : ++variant;
    }};
    ASSERT_FALSE(i.first.empty());
    EXPECT_LT(i.first.size(), 4);
    for (auto& j : i.second->romajiVariants()) romajiVariants.insert(j);
    if (i.first == "n")
      ++nNum;
    else
      switch (i.first[i.first.size() - 1]) {
      case 'a': count(aNum, vaNum); break;
      case 'i': count(iNum, viNum); break;
      case 'u': count(uNum, vuNum); break;
      case 'e': count(eNum, veNum); break;
      case 'o': count(oNum, voNum); break;
      default:
        FAIL() << "romaji " << i.first << " doesn't end with expected letter\n";
      }
  }
  // test romaji counts per last letter
  EXPECT_EQ(aNum, 44);
  EXPECT_EQ(iNum, 38);
  EXPECT_EQ(uNum, 40);
  EXPECT_EQ(eNum, 40);
  EXPECT_EQ(oNum, 41);
  // test romaji variant counts per last letter
  EXPECT_EQ(vaNum, 11);
  EXPECT_EQ(viNum, 10);
  EXPECT_EQ(vuNum, 12);
  EXPECT_EQ(veNum, 12);
  EXPECT_EQ(voNum, 10);
  EXPECT_EQ(nNum, 1);
  EXPECT_EQ(aNum + vaNum + iNum + viNum + uNum + vuNum + eNum + veNum + oNum +
                voNum + nNum,
      TotalRomaji);
  EXPECT_EQ(romajiVariants.size(), RomajiVariants);
}

} // namespace kanji_tools
