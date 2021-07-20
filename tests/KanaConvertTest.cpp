#include <gtest/gtest.h>

#include <kanji/KanaConvert.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

namespace kanji {

using CharType = KanaConvert::CharType;

class KanaConvertTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "-data";
    static const char* arg2 = "../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }
  std::string romajiToHiragana(const std::string& s, bool keepSpaces = true) const {
    return _converter.convert(s, CharType::Romaji, CharType::Hiragana, keepSpaces);
  }
  std::string romajiToKatakana(const std::string& s, bool keepSpaces = true) const {
    return _converter.convert(s, CharType::Romaji, CharType::Katakana, keepSpaces);
  }
  std::string hiraganaToRomaji(const std::string& s) const {
    return _converter.convert(s, CharType::Hiragana, CharType::Romaji);
  }
  enum Values { KanaSize = 162, Variants = 32 };
  const KanaConvert _converter;
};

TEST_F(KanaConvertTest, CheckHiragana) {
  EXPECT_EQ(_converter.hiraganaMap().size(), KanaSize);
  for (auto& i : _converter.hiraganaMap()) {
    MBChar s(i.first);
    std::string c;
    auto check = [&i, &c](const std::string& a, const std::string& b = "") {
      EXPECT_TRUE(c == a || (!b.empty() && c == b)) << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
                                                    << i.second->romaji << "', hiragana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    EXPECT_TRUE(isHiragana(c)) << c;
    if (s.next(c)) {
      EXPECT_TRUE(isHiragana(c)) << c;
      // if there's a second character it must be a small symbol matching the final romaji letter
      auto romajiLen = i.second->romaji.length();
      ASSERT_GT(romajiLen, 1);
      if (i.second->romaji == "qwa") // the only digraph that ends with small 'wa'
        EXPECT_EQ(i.first, "くゎ");
      else
        switch (i.second->romaji[romajiLen - 1]) {
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
                                                    << i.second->romaji << "', katakana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    EXPECT_TRUE(isKatakana(c)) << c;
    if (s.next(c)) {
      EXPECT_TRUE(isKatakana(c)) << c;
      // if there's a second character it must be a small symbol matching the final romaji letter
      auto romajiLen = i.second->romaji.length();
      ASSERT_GT(romajiLen, 1);
      if (i.second->romaji == "qwa") // the only digraph that ends with small 'wa'
        EXPECT_EQ(i.first, "クヮ");
      else
        switch (i.second->romaji[romajiLen - 1]) {
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
    if (i.second->variant) ++variantCount;
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
  EXPECT_EQ(aCount, 45);
  EXPECT_EQ(iCount, 34);
  EXPECT_EQ(uCount, 43);
  EXPECT_EQ(eCount, 30);
  EXPECT_EQ(oCount, 41);
  EXPECT_EQ(nCount, 1);
  EXPECT_EQ(variantCount, Variants);
}

TEST_F(KanaConvertTest, NoConversionIfSourceAndTargetAreTheSame) {
  std::string s("atatakaiあたたかいアタタカイ");
  EXPECT_EQ(_converter.convert(s, CharType::Romaji, CharType::Romaji), s);
  EXPECT_EQ(_converter.convert(s, CharType::Hiragana, CharType::Hiragana), s);
  EXPECT_EQ(_converter.convert(s, CharType::Katakana, CharType::Katakana), s);
}

TEST_F(KanaConvertTest, ConvertRomajiToHiragana) {
  EXPECT_EQ(romajiToHiragana("a"), "あ");
  EXPECT_EQ(romajiToHiragana("ka"), "か");
  EXPECT_EQ(romajiToHiragana("kitte"), "きって");
  EXPECT_EQ(romajiToHiragana("burikko"), "ぶりっこ");
  EXPECT_EQ(romajiToHiragana("tte"), "って");
  EXPECT_EQ(romajiToHiragana("ryo"), "りょ");
  // ō or other macrons map to the same vowel in hiragana which is of course not correct
  // in many cases; 'ou' can be used instead.
  EXPECT_EQ(romajiToHiragana("tōkyō"), "とおきょお");
  EXPECT_EQ(romajiToHiragana("toukyou"), "とうきょう");
  EXPECT_EQ(romajiToHiragana("no"), "の");
  EXPECT_EQ(romajiToHiragana("ken"), "けん");
  EXPECT_EQ(romajiToHiragana("kannon"), "かんのん");
  EXPECT_EQ(romajiToHiragana("jun'ichi"), "じゅんいち");
  EXPECT_EQ(romajiToHiragana("kani"), "かに");
  EXPECT_EQ(romajiToHiragana("kan-i"), "かんい");
  EXPECT_EQ(romajiToHiragana("ninja samurai"), "にんじゃ　さむらい");
  // case insensitive
  EXPECT_EQ(romajiToHiragana("Dare desu ka? ngya!"), "だれ　です　か？　んぎゃ！");
  EXPECT_EQ(romajiToHiragana("Dare dESu ka? kyaa!!", false), "だれですか？きゃあ！！");
  // don't convert non-romaji
  EXPECT_EQ(romajiToHiragana("店じまいdesu."), "店じまいです。");
  EXPECT_EQ(romajiToHiragana("[サメはkowai!]"), "「サメはこわい！」");
}

TEST_F(KanaConvertTest, ConvertRomajiToKatakana) {
  EXPECT_EQ(romajiToKatakana("i"), "イ");
  EXPECT_EQ(romajiToKatakana("ke"), "ケ");
  EXPECT_EQ(romajiToKatakana("macchi"), "マッチ");
  // use macrons to get a katakana 'ー'
  EXPECT_EQ(romajiToKatakana("sērā"), "セーラー");
  EXPECT_EQ(romajiToKatakana("pāthī"), "パーティー");
  EXPECT_EQ(romajiToKatakana("chīzu"), "チーズ");
  EXPECT_EQ(romajiToKatakana("chiizu"), "チイズ");
  // don't convert non-romaji
  EXPECT_EQ(romajiToKatakana("店じまいdesu."), "店じまいデス。");
  EXPECT_EQ(romajiToKatakana("[サメはkowai!]"), "「サメはコワイ！」");
  // don't convert invalid romaji
  EXPECT_EQ(romajiToKatakana("(hello world)"), "（ヘlォ　ヲrld）");
}

TEST_F(KanaConvertTest, ConvertHiraganaToRomaji) {
  EXPECT_EQ(hiraganaToRomaji("う"), "u");
  EXPECT_EQ(hiraganaToRomaji("きょうと"), "kyouto");
  EXPECT_EQ(hiraganaToRomaji("にいがた"), "niigata");
  EXPECT_EQ(hiraganaToRomaji("かんけいない"), "kankeinai");
  EXPECT_EQ(hiraganaToRomaji("かんい"), "kan'i");
  EXPECT_EQ(hiraganaToRomaji("しんよう"), "shin'you");
}

} // namespace kanji
