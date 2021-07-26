#include <gtest/gtest.h>

#include <kanji/Kana.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

namespace kanji {

namespace {

enum Values {
  HanDakuten = 5,       // both mono- and di-graphs have the same number
  SmallMonographs = 12, // no digraphs start with a small kana (but they all end with one)
  DakutenMonographs = 21,
  DakutenDigraphs = 42,
  PlainMonographs = 48,
  PlainDigraphs = 67,
  Variants = 46
};
constexpr int TotalMonographs = HanDakuten + SmallMonographs + DakutenMonographs + PlainMonographs;
constexpr int TotalDigraphs = HanDakuten + PlainDigraphs + DakutenDigraphs;
constexpr int TotalKana = TotalMonographs + TotalDigraphs;
constexpr int TotalRomaji = TotalKana + Variants;

} // namespace

TEST(KanaTest, CheckHiragana) {
  auto& sourceMap = Kana::getMap(CharType::Hiragana);
  EXPECT_EQ(sourceMap.size(), TotalKana);
  // count various types including smallDigraphs (which should be 0)
  int hanDakutenMonographs = 0, smallMonographs = 0, plainMonographs = 0, dakutenMonographs = 0, plainDigraphs = 0,
      hanDakutenDigraphs = 0, dakutenDigraphs = 0, smallDigraphs = 0;
  for (auto& i : sourceMap) {
    MBChar s(i.first);
    std::string c;
    auto check = [&i, &c](const std::string& a, const std::string& b = "") {
      EXPECT_TRUE(c == a || (!b.empty() && c == b)) << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
                                                    << i.second->romaji() << "', hiragana " << i.first;
    };
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
      // if there's a second character it must be a small symbol matching the final romaji letter
      auto romajiLen = i.second->romaji().length();
      ASSERT_GT(romajiLen, 1);
      if (i.second->romaji() == "qwa") // the only digraph that ends with small 'wa'
        EXPECT_EQ(i.first, "くゎ");
      else
        switch (i.second->romaji()[romajiLen - 1]) {
        case 'a': check("ぁ", "ゃ"); break;
        case 'i': check("ぃ"); break;
        case 'u': check("ぅ", "ゅ"); break;
        case 'e': check("ぇ"); break;
        case 'o': check("ぉ", "ょ"); break;
        default: check("");
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
  auto& sourceMap = Kana::getMap(CharType::Katakana);
  auto& hiraganaMap = Kana::getMap(CharType::Hiragana);
  EXPECT_EQ(sourceMap.size(), TotalKana);
  for (auto& i : sourceMap) {
    MBChar s(i.first);
    // all entries in katakana map should also be in hiragana map so no need to check
    // various counts again.
    EXPECT_TRUE(hiraganaMap.contains(i.second->hiragana()));
    std::string c;
    auto check = [&i, &c](const std::string& a, const std::string& b = "") {
      EXPECT_TRUE(c == a || (!b.empty() && c == b)) << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
                                                    << i.second->romaji() << "', katakana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    if (s.next(c)) {
      // if there's a second character it must be a small symbol matching the final romaji letter
      auto romajiLen = i.second->romaji().length();
      ASSERT_GT(romajiLen, 1);
      if (i.second->romaji() == "qwa") // the only digraph that ends with small 'wa'
        EXPECT_EQ(i.first, "クヮ");
      else
        switch (i.second->romaji()[romajiLen - 1]) {
        case 'a': check("ァ", "ャ"); break;
        case 'i': check("ィ"); break;
        case 'u': check("ゥ", "ュ"); break;
        case 'e': check("ェ"); break;
        case 'o': check("ォ", "ョ"); break;
        default: check("");
        }
      // can't be longer than 2 characters
      EXPECT_FALSE(s.next(c));
    }
  }
}

TEST(KanaTest, CheckRomaji) {
  auto& sourceMap = Kana::getMap(CharType::Romaji);
  EXPECT_EQ(sourceMap.size(), TotalRomaji);
  int aCount = 0, iCount = 0, uCount = 0, eCount = 0, oCount = 0, nCount = 0;
  std::set<std::string> variants;
  for (auto& i : sourceMap) {
    ASSERT_FALSE(i.first.empty());
    EXPECT_LT(i.first.length(), 4);
    for (auto& j : i.second->variants())
      variants.insert(j);
    if (i.first == "n")
      ++nCount;
    else
      switch (i.first[i.first.length() - 1]) {
      case 'a': ++aCount; break;
      case 'i': ++iCount; break;
      case 'u': ++uCount; break;
      case 'e': ++eCount; break;
      case 'o': ++oCount; break;
      default: FAIL() << "romaji " << i.first << " doesn't end with expected letter\n";
      }
  }
  EXPECT_EQ(aCount, 55);
  EXPECT_EQ(iCount, 46);
  EXPECT_EQ(uCount, 48);
  EXPECT_EQ(eCount, 48);
  EXPECT_EQ(oCount, 48);
  EXPECT_EQ(nCount, 1);
  EXPECT_EQ(aCount + iCount + uCount + eCount + oCount + nCount, TotalRomaji);
  EXPECT_EQ(variants.size(), Variants);
}

} // namespace kanji
