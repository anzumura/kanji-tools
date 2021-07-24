#include <gtest/gtest.h>

#include <kanji/Kana.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

namespace kanji {

namespace {

enum Values { Monographs = 86, Digraphs = 106, Variants = 35 };
constexpr int TotalKanaCombinations = Monographs + Digraphs + Variants;

} // namespace

TEST(KanaTest, CheckHiragana) {
  auto& sourceMap = Kana::getMap(CharType::Hiragana);
  EXPECT_EQ(sourceMap.size(), Monographs + Digraphs);
  int monographs = 0, digraphs = 0;
  for (auto& i : sourceMap) {
    MBChar s(i.first);
    std::string c;
    auto check = [&i, &c](const std::string& a, const std::string& b = "") {
      EXPECT_TRUE(c == a || (!b.empty() && c == b)) << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
                                                    << i.second->romaji() << "', hiragana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    EXPECT_TRUE(isHiragana(c)) << c;
    if (s.next(c)) {
      ++digraphs;
      EXPECT_TRUE(isHiragana(c)) << c;
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
    } else
      ++monographs;
  }
  EXPECT_EQ(monographs, Monographs);
  EXPECT_EQ(digraphs, Digraphs);
}

TEST(KanaTest, CheckKatakana) {
  auto& sourceMap = Kana::getMap(CharType::Katakana);
  EXPECT_EQ(sourceMap.size(), Monographs + Digraphs);
  int monographs = 0, digraphs = 0;
  for (auto& i : sourceMap) {
    MBChar s(i.first);
    std::string c;
    auto check = [&i, &c](const std::string& a, const std::string& b = "") {
      EXPECT_TRUE(c == a || (!b.empty() && c == b)) << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
                                                    << i.second->romaji() << "', katakana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    EXPECT_TRUE(isKatakana(c)) << c;
    if (s.next(c)) {
      ++digraphs;
      EXPECT_TRUE(isKatakana(c)) << c;
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
    } else
      ++monographs;
  }
  EXPECT_EQ(monographs, Monographs);
  EXPECT_EQ(digraphs, Digraphs);
}

TEST(KanaTest, CheckRomaji) {
  auto& sourceMap = Kana::getMap(CharType::Romaji);
  EXPECT_EQ(sourceMap.size(), TotalKanaCombinations);
  int aCount = 0, iCount = 0, uCount = 0, eCount = 0, oCount = 0, nCount = 0;
  oCount = 0, nCount = 0;
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
  EXPECT_EQ(aCount, 51);
  EXPECT_EQ(iCount, 41);
  EXPECT_EQ(uCount, 45);
  EXPECT_EQ(eCount, 43);
  EXPECT_EQ(oCount, 46);
  EXPECT_EQ(nCount, 1);
  EXPECT_EQ(aCount + iCount + uCount + eCount + oCount + nCount, TotalKanaCombinations);
  EXPECT_EQ(variants.size(), Variants);
}

} // namespace kanji
