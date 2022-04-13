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

TEST(KanaTest, CheckN) {
  EXPECT_EQ(Kana::N.hiragana(), "ん");
  EXPECT_EQ(Kana::N.katakana(), "ン");
  EXPECT_EQ(Kana::N.romaji(), "n");
  EXPECT_TRUE(Kana::N.isMonograph());
  EXPECT_FALSE(Kana::N.isDigraph());
  EXPECT_FALSE(Kana::N.isDakuten());
  EXPECT_FALSE(Kana::N.isHanDakuten());
  EXPECT_TRUE(Kana::N.romajiVariants().empty());
  EXPECT_FALSE(Kana::N.kunreiVariant());
}

TEST(KanaTest, CheckSmallTsu) {
  EXPECT_EQ(Kana::SmallTsu.hiragana(), "っ");
  EXPECT_EQ(Kana::SmallTsu.katakana(), "ッ");
  EXPECT_EQ(Kana::SmallTsu.romaji(), "ltu");
  EXPECT_TRUE(Kana::SmallTsu.isMonograph());
  EXPECT_FALSE(Kana::SmallTsu.isDigraph());
  EXPECT_FALSE(Kana::SmallTsu.isDakuten());
  EXPECT_FALSE(Kana::SmallTsu.isHanDakuten());
  EXPECT_EQ(Kana::SmallTsu.romajiVariants(), Kana::RomajiVariants::List{"xtu"});
  EXPECT_FALSE(Kana::SmallTsu.kunreiVariant());
}

TEST(KanaTest, RepeatPlain) {
  EXPECT_EQ(Kana::RepeatPlain.hiragana(), "ゝ");
  EXPECT_EQ(Kana::RepeatPlain.katakana(), "ヽ");
}

TEST(KanaTest, RepeatAccented) {
  EXPECT_EQ(Kana::RepeatAccented.hiragana(), "ゞ");
  EXPECT_EQ(Kana::RepeatAccented.katakana(), "ヾ");
}

TEST(KanaTest, RepeatMarkMatches) {
  for (auto i : std::array{&Kana::RepeatPlain, &Kana::RepeatAccented}) {
    EXPECT_TRUE(i->matches(CharType::Hiragana, i->hiragana()));
    EXPECT_TRUE(i->matches(CharType::Katakana, i->katakana()));
    EXPECT_FALSE(i->matches(CharType::Hiragana, i->katakana()));
    EXPECT_FALSE(i->matches(CharType::Katakana, i->hiragana()));
    EXPECT_FALSE(i->matches(CharType::Romaji, {}));
  }
}

TEST(KanaTest, RepeatMarkGet) {
  for (auto flags : std::array{ConvertFlags::None, ConvertFlags::Hepburn})
    for (auto i : std::array{&Kana::RepeatPlain, &Kana::RepeatAccented}) {
      // get with CharType 'Hiragana' or 'Katakana' always return underlying
      // 'hiragana' or 'katakana' respectively regardless of 'flags' or prevKana
      EXPECT_EQ(i->get(CharType::Hiragana, flags, {}), i->hiragana());
      EXPECT_EQ(i->get(CharType::Katakana, flags, {}), i->katakana());
      // get CharType 'Romaji' always returns EmptyString if prevKana is nullptr
      // see other tests below for getting with non-empty prevKana
      EXPECT_EQ(i->get(CharType::Romaji, flags, {}), "");
    }
}

TEST(KanaTest, RepeatMarkGetRomaji) {
  auto& m{Kana::getMap(CharType::Romaji)};
  const auto i{m.find("tsu")};
  ASSERT_TRUE(i != m.end());
  auto* prev{i->second};
  auto flags{ConvertFlags::None};
  EXPECT_EQ(Kana::RepeatPlain.get(CharType::Romaji, flags, prev), "tsu");
  EXPECT_EQ(Kana::RepeatAccented.get(CharType::Romaji, flags, prev), "du");
  // 'tsu' has 'Kunrei' of 'tu' and accented value of 'du' by default (the
  // Wāpuro value), but the accented value is 'zu' if either 'Hepburn' or
  // 'Kunrei' standard is requested
  flags = ConvertFlags::Hepburn;
  EXPECT_EQ(Kana::RepeatPlain.get(CharType::Romaji, flags, prev), "tsu");
  EXPECT_EQ(Kana::RepeatAccented.get(CharType::Romaji, flags, prev), "zu");
  flags = ConvertFlags::Kunrei;
  EXPECT_EQ(Kana::RepeatPlain.get(CharType::Romaji, flags, prev), "tu");
  EXPECT_EQ(Kana::RepeatAccented.get(CharType::Romaji, flags, prev), "zu");
}

TEST(KanaTest, FindDakuten) {
  EXPECT_EQ(Kana::findDakuten("か"), Kana::OptString{"が"});
  EXPECT_EQ(Kana::findDakuten("シ"), Kana::OptString{"ジ"});
  EXPECT_EQ(Kana::findDakuten("う"), Kana::OptString{"ゔ"});
  EXPECT_FALSE(Kana::findDakuten("ま"));
  EXPECT_FALSE(Kana::findDakuten("マ"));
  EXPECT_FALSE(Kana::findDakuten("bad"));
}

TEST(KanaTest, FindHanDakuten) {
  EXPECT_EQ(Kana::findHanDakuten("は"), Kana::OptString{"ぱ"});
  EXPECT_EQ(Kana::findHanDakuten("ホ"), Kana::OptString{"ポ"});
  EXPECT_FALSE(Kana::findHanDakuten("さ"));
  EXPECT_FALSE(Kana::findHanDakuten("サ"));
  EXPECT_FALSE(Kana::findHanDakuten("bad"));
}

TEST(KanaTest, CheckHiragana) {
  auto& sourceMap{Kana::getMap(CharType::Hiragana)};
  EXPECT_EQ(sourceMap.size(), TotalKana);
  // count various types including smallDigraphs (which should be 0)
  u_int16_t hanDakutenMonographs{}, smallMonographs{}, plainMonographs{},
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
  u_int16_t aNum{}, vaNum{}, iNum{}, viNum{}, uNum{}, vuNum{}, eNum{}, veNum{},
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
