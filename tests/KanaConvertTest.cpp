#include <gtest/gtest.h>

#include <kanji/KanaConvert.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

namespace kanji {

class KanaConvertTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "-data";
    static const char* arg2 = "../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }
  enum Values { KanaSize = 130, Variants = 13 };
  const KanaConvert _converter;
};

TEST_F(KanaConvertTest, CheckHiragana) {
  EXPECT_EQ(_converter.hiraganaMap().size(), KanaSize);
  for (auto& i : _converter.hiraganaMap()) {
    MBChar s(i.first);
    std::string c;
    auto check = [&i, &c](const std::string& a, const std::string& b = "") {
      EXPECT_TRUE(c == a || (!b.empty() && c == b)) << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
                                                    << i.second.romaji << "', hiragana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    EXPECT_TRUE(isHiragana(c)) << c;
    if (s.next(c)) {
      EXPECT_TRUE(isHiragana(c)) << c;
      // if there's a second character it must be a small symbol matching the final romaji letter
      auto romajiLen = i.second.romaji.length();
      ASSERT_GT(romajiLen, 1);
      switch (i.second.romaji[romajiLen - 1]) {
      case 'a': check("ぁ", "ゃ"); break;
      case 'i': check("ぃ"); break;
      case 'u': check("ぅ", "ゅ"); break;
      case 'e': check("ぇ"); break;
      case 'o': check("ぉ", "ょ"); break;
      default: check("");
      }
      // can't be longer than 2 characters
      EXPECT_FALSE(s.next(c));
    }
  }
}

TEST_F(KanaConvertTest, CheckKatakana) {
  EXPECT_EQ(_converter.katakanaMap().size(), KanaSize);
  for (auto& i : _converter.katakanaMap()) {
    MBChar s(i.first);
    std::string c;
    auto check = [&i, &c](const std::string& a, const std::string& b = "") {
      EXPECT_TRUE(c == a || (!b.empty() && c == b)) << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
                                                    << i.second.romaji << "', katakana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    EXPECT_TRUE(isKatakana(c)) << c;
    if (s.next(c)) {
      EXPECT_TRUE(isKatakana(c)) << c;
      // if there's a second character it must be a small symbol matching the final romaji letter
      auto romajiLen = i.second.romaji.length();
      ASSERT_GT(romajiLen, 1);
      switch (i.second.romaji[romajiLen - 1]) {
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

TEST_F(KanaConvertTest, CheckRomaji) {
  EXPECT_EQ(_converter.romajiMap().size(), KanaSize + Variants);
  int variantCount = 0, aCount = 0, iCount = 0, uCount = 0, eCount = 0, oCount = 0, nCount = 0;
  oCount = 0, nCount = 0;
  for (auto& i : _converter.romajiMap()) {
    ASSERT_FALSE(i.first.empty());
    EXPECT_LT(i.first.length(), 4);
    if (i.second.variant) ++variantCount;
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
  EXPECT_EQ(aCount, 32);
  EXPECT_EQ(iCount, 23);
  EXPECT_EQ(uCount, 33);
  EXPECT_EQ(eCount, 21);
  EXPECT_EQ(oCount, 33);
  EXPECT_EQ(nCount, 1);
  EXPECT_EQ(variantCount, Variants);
}

} // namespace kanji
