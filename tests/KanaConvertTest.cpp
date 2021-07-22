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
  std::string romajiToHiragana(const std::string& s, int flags = 0) const {
    return _converter.convert(s, CharType::Romaji, CharType::Hiragana, flags);
  }
  std::string romajiToKatakana(const std::string& s, int flags = 0) const {
    return _converter.convert(s, CharType::Romaji, CharType::Katakana, flags);
  }
  std::string hiraganaToRomaji(const std::string& s, int flags = 0) const {
    return _converter.convert(s, CharType::Hiragana, CharType::Romaji, flags);
  }
  std::string hiraganaToKatakana(const std::string& s) const {
    return _converter.convert(s, CharType::Hiragana, CharType::Katakana);
  }
  std::string katakanaToRomaji(const std::string& s, int flags = 0) const {
    return _converter.convert(s, CharType::Katakana, CharType::Romaji, flags);
  }
  std::string katakanaToHiragana(const std::string& s) const {
    return _converter.convert(s, CharType::Katakana, CharType::Hiragana);
  }
  void kanaConvertCheck(const std::string& hiragana, const std::string& katakana) const {
    auto r = hiraganaToRomaji(hiragana);
    EXPECT_EQ(katakanaToRomaji(katakana), r);
    EXPECT_EQ(romajiToHiragana(r), hiragana);
    EXPECT_EQ(romajiToKatakana(r), katakana);
    EXPECT_EQ(hiraganaToKatakana(hiragana), katakana);
    EXPECT_EQ(katakanaToHiragana(katakana), hiragana);
  }
  void check(const char* hiragana, const char* katakana, const char* romaji, const char* hepburn = nullptr,
             const char* kunrei = nullptr) const {
    EXPECT_EQ(hiraganaToRomaji(hiragana), romaji);
    EXPECT_EQ(katakanaToRomaji(katakana), romaji);
    EXPECT_EQ(hiraganaToRomaji(hiragana, KanaConvert::Hepburn), hepburn ? hepburn : romaji);
    EXPECT_EQ(katakanaToRomaji(katakana, KanaConvert::Hepburn), hepburn ? hepburn : romaji);
    EXPECT_EQ(hiraganaToRomaji(hiragana, KanaConvert::Kunrei), kunrei ? kunrei : romaji);
    EXPECT_EQ(katakanaToRomaji(katakana, KanaConvert::Kunrei), kunrei ? kunrei : romaji);
    const char* preferHepburnIfBoth = hepburn ? hepburn : kunrei ? kunrei : romaji;
    EXPECT_EQ(hiraganaToRomaji(hiragana, KanaConvert::Hepburn | KanaConvert::Kunrei), preferHepburnIfBoth);
    EXPECT_EQ(katakanaToRomaji(katakana, KanaConvert::Hepburn | KanaConvert::Kunrei), preferHepburnIfBoth);
  }
  void checkKunrei(const char* hiragana, const char* katakana, const char* romaji, const char* kunrei) const {
    check(hiragana, katakana, romaji, nullptr, kunrei);
  }
  enum Values { KanaSize = 177, Variants = 32 };
  const KanaConvert _converter;
};

TEST_F(KanaConvertTest, CheckHiragana) {
  EXPECT_EQ(_converter.hiraganaMap().size(), KanaSize);
  for (auto& i : _converter.hiraganaMap()) {
    MBChar s(i.first);
    std::string c;
    auto check = [&i, &c](const std::string& a, const std::string& b = "") {
      EXPECT_TRUE(c == a || (!b.empty() && c == b)) << c << " != " << a << (b.empty() ? "" : " or ") << b << " for '"
                                                    << i.second->romaji() << "', hiragana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    EXPECT_TRUE(isHiragana(c)) << c;
    if (s.next(c)) {
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
                                                    << i.second->romaji() << "', katakana " << i.first;
    };
    EXPECT_TRUE(s.next(c));
    EXPECT_TRUE(isKatakana(c)) << c;
    if (s.next(c)) {
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
    }
  }
}

TEST_F(KanaConvertTest, CheckRomaji) {
  EXPECT_EQ(_converter.romajiMap().size(), KanaSize + Variants);
  int aCount = 0, iCount = 0, uCount = 0, eCount = 0, oCount = 0, nCount = 0;
  oCount = 0, nCount = 0;
  std::set<std::string> variants;
  for (auto& i : _converter.romajiMap()) {
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
  EXPECT_EQ(aCount, 47);
  EXPECT_EQ(iCount, 37);
  EXPECT_EQ(uCount, 44);
  EXPECT_EQ(eCount, 37);
  EXPECT_EQ(oCount, 43);
  EXPECT_EQ(nCount, 1);
  EXPECT_EQ(variants.size(), Variants);
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
  // so in order to preserve round-trip a macron is mapped to a prolonged mark (ー). This
  // is not standard and can be turned off by a flag (see KanaConvert.h for more details).
  // 'ou' can be used instead to avoid ambiguity.
  EXPECT_EQ(romajiToHiragana("tōkyō"), "とーきょー");
  EXPECT_EQ(romajiToHiragana("toukyou"), "とうきょう");
  // This next case is of course incorrect, but it's the standard mapping for modern Hepburn romanization.
  EXPECT_EQ(romajiToHiragana("tōkyō", KanaConvert::NoProlongMark), "とおきょお");
  EXPECT_EQ(romajiToHiragana("rāmen da", KanaConvert::NoProlongMark | KanaConvert::RemoveSpaces), "らあめんだ");
  EXPECT_EQ(romajiToHiragana("no"), "の");
  EXPECT_EQ(romajiToHiragana("ken"), "けん");
  EXPECT_EQ(romajiToHiragana("kannon"), "かんのん");
  EXPECT_EQ(romajiToHiragana("jun'ichi"), "じゅんいち");
  EXPECT_EQ(romajiToHiragana("kani"), "かに");
  EXPECT_EQ(romajiToHiragana("kan-i"), "かんい");
  EXPECT_EQ(romajiToHiragana("ninja samurai"), "にんじゃ　さむらい");
  // case insensitive
  EXPECT_EQ(romajiToHiragana("Dare desu ka? ngya!"), "だれ　です　か？　んぎゃ！");
  EXPECT_EQ(romajiToHiragana("Dare dESu ka? kyaa!!", KanaConvert::RemoveSpaces), "だれですか？きゃあ！！");
  // don't convert non-romaji
  EXPECT_EQ(romajiToHiragana("店じまいdesu."), "店じまいです。");
  EXPECT_EQ(romajiToHiragana("[サメはkowai!]"), "「サメはこわい！」");
}

TEST_F(KanaConvertTest, ConvertRomajiToKatakana) {
  EXPECT_EQ(romajiToKatakana("i"), "イ");
  EXPECT_EQ(romajiToKatakana("ke"), "ケ");
  // both the standard way (t+chi) as well as the wāpuro way (c+chi) are supported
  EXPECT_EQ(romajiToKatakana("matchi"), "マッチ");
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
  EXPECT_EQ(hiraganaToRomaji("かつ　さんど！"), "katsu sando!");
  EXPECT_EQ(hiraganaToRomaji("うぃき"), "wiki");
  EXPECT_EQ(hiraganaToRomaji("おんな"), "onna");
  // Small letters that don't form part of a digraph are output in 'wāpuro' style favoring
  // 'l' instead of 'x' as the first letter (note, small tsu is 'ltu').
  EXPECT_EQ(hiraganaToRomaji("ぁぃぅぇぉゃゅょっ"), "lalilulelolyalyulyoltu");
  EXPECT_EQ(hiraganaToRomaji("きょうと"), "kyouto");
  EXPECT_EQ(hiraganaToRomaji("にいがた"), "niigata");
  EXPECT_EQ(hiraganaToRomaji("かんけいない"), "kankeinai");
  // add apostrophe before a vowel or 'y' as per Hepburn standard to avoid ambiguity
  EXPECT_EQ(hiraganaToRomaji("かんい"), "kan'i");
  EXPECT_EQ(hiraganaToRomaji("しんよう"), "shin'you");
  // here are the same examples without the apostrophes
  EXPECT_EQ(hiraganaToRomaji("かに"), "kani");
  EXPECT_EQ(hiraganaToRomaji("しにょう"), "shinyou");
  // Sokuon handling
  EXPECT_EQ(hiraganaToRomaji("きっと"), "kitto");
  EXPECT_EQ(hiraganaToRomaji("べっぴん"), "beppin");
  EXPECT_EQ(hiraganaToRomaji("こっきょう"), "kokkyou");
  // not sure what to do with a final or repeated small tsu ... for now it falls back to 'wāpuro',
  // i.e., exactly what you would need to type on a keyboard to reproduce the Hiragana.
  EXPECT_EQ(hiraganaToRomaji("いてっ"), "iteltu");
  EXPECT_EQ(hiraganaToRomaji("いっって"), "iltutte");
  // prolonged sound mark is mainly for Katakana, but also works for Hiragana, for now using this
  // mark is the only way to get a macron (bar over letter) in Romaji output.
  EXPECT_EQ(hiraganaToRomaji("らーめん"), "rāmen");
  EXPECT_EQ(hiraganaToRomaji("きゃー"), "kyā");
  // ー not following a vowel is left unchanged
  EXPECT_EQ(hiraganaToRomaji("ーぶ"), "ーbu");
  EXPECT_EQ(hiraganaToRomaji("はんーぶ"), "hanーbu");
  // Hepburn examples
  EXPECT_EQ(hiraganaToRomaji("ちぢむ"), "chidimu");
  EXPECT_EQ(hiraganaToRomaji("ちぢむ", KanaConvert::Hepburn), "chijimu");
  EXPECT_EQ(hiraganaToRomaji("つづき"), "tsuduki");
  EXPECT_EQ(hiraganaToRomaji("つづき", KanaConvert::Hepburn), "tsuzuki");
  EXPECT_EQ(hiraganaToRomaji("ぢゃ"), "dya");
  EXPECT_EQ(hiraganaToRomaji("ぢゃ", KanaConvert::Hepburn), "ja");
  EXPECT_EQ(hiraganaToRomaji("ぢゅ"), "dyu");
  EXPECT_EQ(hiraganaToRomaji("ぢゅ", KanaConvert::Hepburn), "ju");
  EXPECT_EQ(hiraganaToRomaji("ぢょ"), "dyo");
  EXPECT_EQ(hiraganaToRomaji("ぢょ", KanaConvert::Hepburn), "jo");
  EXPECT_EQ(hiraganaToRomaji("を"), "wo");
  EXPECT_EQ(hiraganaToRomaji("を", KanaConvert::Hepburn), "o");
}

TEST_F(KanaConvertTest, ConvertKatakanaToRomaji) {
  EXPECT_EQ(katakanaToRomaji("エ"), "e");
  EXPECT_EQ(katakanaToRomaji("アカ　サカ！"), "aka saka!");
  EXPECT_EQ(katakanaToRomaji("イェビス"), "yebisu");
  // Small letters that don't form part of a digraph are output in 'wāpuro' style favoring
  // 'l' instead of 'x' as the first letter (note, small tsu is 'ltu').
  EXPECT_EQ(katakanaToRomaji("ァィゥェォャュョッ"), "lalilulelolyalyulyoltu");
  EXPECT_EQ(katakanaToRomaji("テニス"), "tenisu");
  EXPECT_EQ(katakanaToRomaji("カナダ"), "kanada");
  // add apostrophe before a vowel or 'y' as per Hepburn standard to avoid ambiguity
  EXPECT_EQ(katakanaToRomaji("タンイ"), "tan'i");
  EXPECT_EQ(katakanaToRomaji("ポンヨ"), "pon'yo");
  // here are the same examples without the apostrophes
  EXPECT_EQ(katakanaToRomaji("タニ"), "tani");
  EXPECT_EQ(katakanaToRomaji("ポニョ"), "ponyo"); // BTW, this is the correct name of the movie
  // Sokuon handling
  EXPECT_EQ(katakanaToRomaji("アッパ"), "appa");
  EXPECT_EQ(katakanaToRomaji("マッチ"), "matchi");
  EXPECT_EQ(katakanaToRomaji("ジョッキ"), "jokki");
  // not sure what to do with a final or repeated small tsu ... for now it falls back to 'wāpuro',
  // i.e., exactly what you would need to type on a keyboard to reproduce the Hiragana.
  EXPECT_EQ(katakanaToRomaji("イテッ"), "iteltu");
  EXPECT_EQ(katakanaToRomaji("イッッテ"), "iltutte");
  // prolonged sound mark is mainly for Katakana, but also works for Hiragana, for now using this
  // mark is the only way to get a macron (bar over letter) in Romaji output.
  EXPECT_EQ(katakanaToRomaji("ラーメン"), "rāmen");
  EXPECT_EQ(katakanaToRomaji("キャー"), "kyā");
  EXPECT_EQ(katakanaToRomaji("ファーザー"), "fāzā");
  // ー not following a vowel is left unchanged
  EXPECT_EQ(katakanaToRomaji("ーカ"), "ーka");
  EXPECT_EQ(katakanaToRomaji("ホンート"), "honーto");
  // Hepburn examples
  EXPECT_EQ(katakanaToRomaji("チヂム"), "chidimu");
  EXPECT_EQ(katakanaToRomaji("チヂム", KanaConvert::Hepburn), "chijimu");
  EXPECT_EQ(katakanaToRomaji("ツヅキ"), "tsuduki");
  EXPECT_EQ(katakanaToRomaji("ツヅキ", KanaConvert::Hepburn), "tsuzuki");
  EXPECT_EQ(katakanaToRomaji("ヂャ"), "dya");
  EXPECT_EQ(katakanaToRomaji("ヂャ", KanaConvert::Hepburn), "ja");
  EXPECT_EQ(katakanaToRomaji("ヂュ"), "dyu");
  EXPECT_EQ(katakanaToRomaji("ヂュ", KanaConvert::Hepburn), "ju");
  EXPECT_EQ(katakanaToRomaji("ヂョ"), "dyo");
  EXPECT_EQ(katakanaToRomaji("ヂョ", KanaConvert::Hepburn), "jo");
  EXPECT_EQ(katakanaToRomaji("ヲ"), "wo");
  EXPECT_EQ(katakanaToRomaji("ヲ", KanaConvert::Hepburn), "o");
}

TEST_F(KanaConvertTest, ConvertBetweenKana) {
  for (auto& i : _converter.hiraganaMap()) {
    auto r = _converter.convert(i.first, CharType::Hiragana, CharType::Katakana);
    EXPECT_EQ(r, i.second->katakana());
    EXPECT_EQ(_converter.convert(r, CharType::Katakana, CharType::Hiragana), i.second->hiragana());
  }
  for (auto& i : _converter.katakanaMap()) {
    auto r = _converter.convert(i.first, CharType::Katakana, CharType::Hiragana);
    EXPECT_EQ(r, i.second->hiragana());
    EXPECT_EQ(_converter.convert(r, CharType::Hiragana, CharType::Katakana), i.second->katakana());
  }
  kanaConvertCheck("きょうはいいてんきです。", "キョウハイイテンキデス。");
  // try mixing sokuon and long vowels
  kanaConvertCheck("らーめん！", "ラーメン！");
  kanaConvertCheck("びっぐ　ばあど、すまーる　はっまー？", "ビッグ　バアド、スマール　ハッマー？");
}

TEST_F(KanaConvertTest, ConvertAllToOneType) {
  EXPECT_EQ(_converter.convert("ima クリスマス　です。", CharType::Romaji), "ima kurisumasu desu.");
  EXPECT_EQ(_converter.convert("ima クリスマス　です。", CharType::Hiragana), "いま　くりすます　です。");
  EXPECT_EQ(_converter.convert("ima クリスマス　です。", CharType::Katakana), "イマ　クリスマス　デス。");
  EXPECT_EQ(_converter.convert("rāmenらーめんラーメン!!", CharType::Romaji), "rāmenrāmenrāmen!!");
  EXPECT_EQ(_converter.convert("rāmenらーめんラーメン!!", CharType::Hiragana), "らーめんらーめんらーめん！！");
  EXPECT_EQ(_converter.convert("rāmenらーめんラーメン!!", CharType::Katakana), "ラーメンラーメンラーメン！！");
}

TEST_F(KanaConvertTest, HepburnVersusKunrei) {
  // Romaji output is usually Modern Hepburn by default, but will be Nihon Shiki sometimes in
  // order to be unique for round-trips (plus there are a lot of extra wāpuro entries). Below
  // are the entries from the Differences among romanizations table from:
  // https://en.wikipedia.org/wiki/Romanization_of_Japanese
  // -- A
  check("あ", "ア", "a");
  check("い", "イ", "i");
  check("う", "ウ", "u");
  check("え", "エ", "e");
  check("お", "オ", "o");
  // -- KA
  check("か", "カ", "ka");
  check("き", "キ", "ki");
  check("く", "ク", "ku");
  check("け", "ケ", "ke");
  check("こ", "コ", "ko");
  check("きゃ", "キャ", "kya");
  check("きゅ", "キュ", "kyu");
  check("きょ", "キョ", "kyo");
  // -- SA
  check("さ", "サ", "sa");
  checkKunrei("し", "シ", "shi", "si");
  check("す", "ス", "su");
  check("せ", "セ", "se");
  check("そ", "ソ", "so");
  checkKunrei("しゃ", "シャ", "sha", "sya");
  checkKunrei("しゅ", "シュ", "shu", "syu");
  checkKunrei("しょ", "ショ", "sho", "syo");
  // -- TA
  check("た", "タ", "ta");
  checkKunrei("ち", "チ", "chi", "ti");
  checkKunrei("つ", "ツ", "tsu", "tu");
  check("て", "テ", "te");
  check("と", "ト", "to");
  checkKunrei("ちゃ", "チャ", "cha", "tya");
  checkKunrei("ちゅ", "チュ", "chu", "tyu");
  checkKunrei("ちょ", "チョ", "cho", "tyo");
  // -- NA
  check("な", "ナ", "na");
  check("に", "ニ", "ni");
  check("ぬ", "ヌ", "nu");
  check("ね", "ネ", "ne");
  check("の", "ノ", "no");
  check("にゃ", "ニャ", "nya");
  check("にゅ", "ニュ", "nyu");
  check("にょ", "ニョ", "nyo");
  // -- HA
  check("は", "ハ", "ha");
  check("ひ", "ヒ", "hi");
  checkKunrei("ふ", "フ", "fu", "hu");
  check("へ", "ヘ", "he");
  check("ほ", "ホ", "ho");
  check("ひゃ", "ヒャ", "hya");
  check("ひゅ", "ヒュ", "hyu");
  check("ひょ", "ヒョ", "hyo");
  // -- MA
  check("ま", "マ", "ma");
  check("み", "ミ", "mi");
  check("む", "ム", "mu");
  check("め", "メ", "me");
  check("も", "モ", "mo");
  check("みゃ", "ミャ", "mya");
  check("みゅ", "ミュ", "myu");
  check("みょ", "ミョ", "myo");
  // -- YA
  check("や", "ヤ", "ya");
  check("ゆ", "ユ", "yu");
  check("よ", "ヨ", "yo");
  // -- RA, WA and N
  check("ら", "ラ", "ra");
  check("り", "リ", "ri");
  check("る", "ル", "ru");
  check("れ", "レ", "re");
  check("ろ", "ロ", "ro");
  check("りゃ", "リャ", "rya");
  check("りゅ", "リュ", "ryu");
  check("りょ", "リョ", "ryo");
  check("わ", "ワ", "wa");
  // both Hepburn and Kunrei use 'o' for を, but program (and Nihon Shiki) uses 'wo' for uniqueness
  check("を", "ヲ", "wo", "o", "o");
  check("ん", "ン", "n");
  // -- GA
  check("が", "ガ", "ga");
  check("ぎ", "ギ", "gi");
  check("ぐ", "グ", "gu");
  check("げ", "ゲ", "ge");
  check("ご", "ゴ", "go");
  check("ぎゃ", "ギャ", "gya");
  check("ぎゅ", "ギュ", "gyu");
  check("ぎょ", "ギョ", "gyo");
  // -- ZA
  check("ざ", "ザ", "za");
  checkKunrei("じ", "ジ", "ji", "zi");
  check("ず", "ズ", "zu");
  check("ぜ", "ゼ", "ze");
  check("ぞ", "ゾ", "zo");
  checkKunrei("じゃ", "ジャ", "ja", "zya");
  checkKunrei("じゅ", "ジュ", "ju", "zyu");
  checkKunrei("じょ", "ジョ", "jo", "zyo");
  // -- DA
  // Lots of differences for this group, for example the mapping for ヂ in Nihon Shiki style
  // (and default for this program) is 'di', whereas Hepburn is 'ji' and Kunrei is 'zi'.
  check("だ", "ダ", "da");
  check("ぢ", "ヂ", "di", "ji", "zi");
  check("づ", "ヅ", "du", "zu", "zu");
  check("で", "デ", "de");
  check("ど", "ド", "do");
  check("ぢゃ", "ヂャ", "dya", "ja", "zya");
  check("ぢゅ", "ヂュ", "dyu", "ju", "zyu");
  check("ぢょ", "ヂョ", "dyo", "jo", "zyo");
  // -- BA
  check("ば", "バ", "ba");
  check("び", "ビ", "bi");
  check("ぶ", "ブ", "bu");
  check("べ", "ベ", "be");
  check("ぼ", "ボ", "bo");
  check("びゃ", "ビャ", "bya");
  check("びゅ", "ビュ", "byu");
  check("びょ", "ビョ", "byo");
  // -- PA
  check("ぱ", "パ", "pa");
  check("ぴ", "ピ", "pi");
  check("ぷ", "プ", "pu");
  check("ぺ", "ペ", "pe");
  check("ぽ", "ポ", "po");
  check("ぴゃ", "ピャ", "pya");
  check("ぴゅ", "ピュ", "pyu");
  check("ぴょ", "ピョ", "pyo");
  // -- VU
  check("ゔ", "ヴ", "vu");
}

} // namespace kanji
