#include <gtest/gtest.h>
#include <kanji_tools/kana/KanaConvert.h>

namespace kanji_tools {

class KanaConvertTest : public ::testing::Test {
protected:
  [[nodiscard]] auto romajiToHiragana(const std::string& s,
                                      ConvertFlags flags = ConvertFlags::None) {
    return _converter.convert(CharType::Romaji, s, CharType::Hiragana, flags);
  }
  [[nodiscard]] auto romajiToKatakana(const std::string& s,
                                      ConvertFlags flags = ConvertFlags::None) {
    return _converter.convert(CharType::Romaji, s, CharType::Katakana, flags);
  }
  [[nodiscard]] auto hiraganaToRomaji(const std::string& s,
                                      ConvertFlags flags = ConvertFlags::None) {
    return _converter.convert(CharType::Hiragana, s, CharType::Romaji, flags);
  }
  [[nodiscard]] auto hiraganaToKatakana(const std::string& s) {
    return _converter.convert(CharType::Hiragana, s, CharType::Katakana);
  }
  [[nodiscard]] auto katakanaToRomaji(const std::string& s,
                                      ConvertFlags flags = ConvertFlags::None) {
    return _converter.convert(CharType::Katakana, s, CharType::Romaji, flags);
  }
  [[nodiscard]] auto katakanaToHiragana(const std::string& s) {
    return _converter.convert(CharType::Katakana, s, CharType::Hiragana);
  }
  // populate 'romaji' when round trip is lossy (like repeat symbols)
  void kanaConvertCheck(const std::string& hiragana,
                        const std::string& katakana,
                        const std::string& romaji = {}) {
    if (romaji.empty()) {
      auto r{hiraganaToRomaji(hiragana)};
      EXPECT_EQ(katakanaToRomaji(katakana), r);
      EXPECT_EQ(romajiToHiragana(r), hiragana);
      EXPECT_EQ(romajiToKatakana(r), katakana);
    } else {
      EXPECT_EQ(hiraganaToRomaji(hiragana), romaji);
      EXPECT_EQ(katakanaToRomaji(katakana), romaji);
    }
    EXPECT_EQ(hiraganaToKatakana(hiragana), katakana);
    EXPECT_EQ(katakanaToHiragana(katakana), hiragana);
  }
  void check(const char* hiragana, const char* katakana, const char* romaji,
             const char* hepburn = {}, const char* kunrei = {}) {
    EXPECT_EQ(hiraganaToRomaji(hiragana), romaji);
    EXPECT_EQ(katakanaToRomaji(katakana), romaji);
    EXPECT_EQ(hiraganaToRomaji(hiragana, ConvertFlags::Hepburn),
              hepburn ? hepburn : romaji);
    EXPECT_EQ(katakanaToRomaji(katakana, ConvertFlags::Hepburn),
              hepburn ? hepburn : romaji);
    EXPECT_EQ(hiraganaToRomaji(hiragana, ConvertFlags::Kunrei),
              kunrei ? kunrei : romaji);
    EXPECT_EQ(katakanaToRomaji(katakana, ConvertFlags::Kunrei),
              kunrei ? kunrei : romaji);
    auto preferHepburnIfBoth{hepburn ? hepburn : kunrei ? kunrei : romaji};
    EXPECT_EQ(
      hiraganaToRomaji(hiragana, ConvertFlags::Hepburn | ConvertFlags::Kunrei),
      preferHepburnIfBoth);
    EXPECT_EQ(
      katakanaToRomaji(katakana, ConvertFlags::Hepburn | ConvertFlags::Kunrei),
      preferHepburnIfBoth);
  }
  void checkKunrei(const char* hiragana, const char* katakana,
                   const char* romaji, const char* kunrei) {
    check(hiragana, katakana, romaji, nullptr, kunrei);
  }
  void checkSmallKana(CharType source, const std::string& s) {
    // small letters that don't form part of a digraph are output in 'wāpuro'
    // style favoring 'l' instead of 'x' as first letter (so small tsu is 'ltu')
    std::string romaji{"lalilulelolkalkelyalyulyoltulwa"};
    EXPECT_EQ(_converter.convert(source, s, CharType::Romaji), romaji);
    EXPECT_EQ(_converter.convert(CharType::Romaji, romaji, source), s);
    // also test small letters starting with 'x'
    std::replace(romaji.begin(), romaji.end(), 'l', 'x');
    EXPECT_EQ(_converter.convert(CharType::Romaji, romaji, source), s);
  }
  KanaConvert _converter;
};

TEST_F(KanaConvertTest, NoConversionIfSourceAndTargetAreTheSame) {
  std::string s("atatakaiあたたかいアタタカイ");
  EXPECT_EQ(_converter.convert(CharType::Romaji, s, CharType::Romaji), s);
  EXPECT_EQ(_converter.convert(CharType::Hiragana, s, CharType::Hiragana), s);
  EXPECT_EQ(_converter.convert(CharType::Katakana, s, CharType::Katakana), s);
}

TEST_F(KanaConvertTest, ConvertRomajiToHiragana) {
  EXPECT_EQ(romajiToHiragana("a"), "あ");
  EXPECT_EQ(romajiToHiragana("ka"), "か");
  EXPECT_EQ(romajiToHiragana("kitte"), "きって");
  EXPECT_EQ(romajiToHiragana("burikko"), "ぶりっこ");
  EXPECT_EQ(romajiToHiragana("tte"), "って");
  EXPECT_EQ(romajiToHiragana("ryo"), "りょ");
  // ō or other macrons map to the same vowel in hiragana which is of course not
  // always correct so to preserve round-trip a macron is mapped to a prolonged
  // mark (ー). This isn't standard and can be turned off by a flag (see
  // KanaConvert.h for details). 'ou' can be used instead to avoid ambiguity.
  EXPECT_EQ(romajiToHiragana("tōkyō"), "とーきょー");
  EXPECT_EQ(romajiToHiragana("toukyou"), "とうきょう");
  // This next case is of course incorrect, but it's the standard mapping for
  // modern Hepburn romanization.
  EXPECT_EQ(romajiToHiragana("tōkyō", ConvertFlags::NoProlongMark),
            "とおきょお");
  EXPECT_EQ(romajiToHiragana("rāmen da", ConvertFlags::NoProlongMark |
                                           ConvertFlags::RemoveSpaces),
            "らあめんだ");
  EXPECT_EQ(romajiToHiragana("no"), "の");
  EXPECT_EQ(romajiToHiragana("ken"), "けん");
  EXPECT_EQ(romajiToHiragana("kannon"), "かんのん");
  EXPECT_EQ(romajiToHiragana("jun'ichi"), "じゅんいち");
  EXPECT_EQ(romajiToHiragana("kani"), "かに");
  EXPECT_EQ(romajiToHiragana("kan-i"), "かんい");
  EXPECT_EQ(romajiToHiragana("ninja samurai"), "にんじゃ　さむらい");
  // case insensitive
  EXPECT_EQ(romajiToHiragana("Dare desu ka? ngya!"),
            "だれ　です　か？　んぎゃ！");
  EXPECT_EQ(
    romajiToHiragana("Dare dESu ka? kyaa!!", ConvertFlags::RemoveSpaces),
    "だれですか？きゃあ！！");
  // don't convert non-romaji
  EXPECT_EQ(romajiToHiragana("店じまいdesu."), "店じまいです。");
  EXPECT_EQ(romajiToHiragana("[サメはkowai!]"), "「サメはこわい！」");
}

TEST_F(KanaConvertTest, ConvertRomajiToKatakana) {
  EXPECT_EQ(romajiToKatakana("i"), "イ");
  EXPECT_EQ(romajiToKatakana("ke"), "ケ");
  // support both standard way (t+chi) as well as the wāpuro way (c+chi)
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
  checkSmallKana(CharType::Hiragana, "ぁぃぅぇぉゕゖゃゅょっゎ");
  EXPECT_EQ(hiraganaToRomaji("きょうと"), "kyouto");
  EXPECT_EQ(hiraganaToRomaji("にいがた"), "niigata");
  EXPECT_EQ(hiraganaToRomaji("かんけいない"), "kankeinai");
  EXPECT_EQ(hiraganaToRomaji("なか・ぐろ"), "naka/guro");
  // add apostrophe before a vowel or 'y' as per Hepburn standard
  EXPECT_EQ(hiraganaToRomaji("かんい"), "kan'i");
  EXPECT_EQ(hiraganaToRomaji("しんよう"), "shin'you");
  // here are the same examples without the apostrophes
  EXPECT_EQ(hiraganaToRomaji("かに"), "kani");
  EXPECT_EQ(hiraganaToRomaji("しにょう"), "shinyou");
  // Sokuon handling
  EXPECT_EQ(hiraganaToRomaji("きっと"), "kitto");
  EXPECT_EQ(hiraganaToRomaji("べっぴん"), "beppin");
  EXPECT_EQ(hiraganaToRomaji("こっきょう"), "kokkyou");
  // not sure what to do with a final or repeated small tsu ... for now it falls
  // back to 'wāpuro', i.e., exactly what you would need to type on a keyboard
  // to reproduce the Hiragana.
  EXPECT_EQ(hiraganaToRomaji("いてっ"), "iteltu");
  EXPECT_EQ(hiraganaToRomaji("いっって"), "iltutte");
  // prolonged sound mark is mainly for Katakana, but also works for Hiragana,
  // for now using this mark is the only way to get a macron (bar over letter)
  // in Romaji output.
  EXPECT_EQ(hiraganaToRomaji("らーめん"), "rāmen");
  EXPECT_EQ(hiraganaToRomaji("きゃー"), "kyā");
  // ー not following a vowel is left unchanged
  EXPECT_EQ(hiraganaToRomaji("ーぶ"), "ーbu");
  EXPECT_EQ(hiraganaToRomaji("はんーぶ"), "hanーbu");
  // Hepburn examples
  EXPECT_EQ(hiraganaToRomaji("ちぢむ"), "chidimu");
  EXPECT_EQ(hiraganaToRomaji("ちぢむ", ConvertFlags::Hepburn), "chijimu");
  EXPECT_EQ(hiraganaToRomaji("つづき"), "tsuduki");
  EXPECT_EQ(hiraganaToRomaji("つづき", ConvertFlags::Hepburn), "tsuzuki");
  EXPECT_EQ(hiraganaToRomaji("ぢゃ"), "dya");
  EXPECT_EQ(hiraganaToRomaji("ぢゃ", ConvertFlags::Hepburn), "ja");
  EXPECT_EQ(hiraganaToRomaji("ぢゅ"), "dyu");
  EXPECT_EQ(hiraganaToRomaji("ぢゅ", ConvertFlags::Hepburn), "ju");
  EXPECT_EQ(hiraganaToRomaji("ぢょ"), "dyo");
  EXPECT_EQ(hiraganaToRomaji("ぢょ", ConvertFlags::Hepburn), "jo");
  EXPECT_EQ(hiraganaToRomaji("を"), "wo");
  EXPECT_EQ(hiraganaToRomaji("を", ConvertFlags::Hepburn), "o");
}

TEST_F(KanaConvertTest, ConvertKatakanaToRomaji) {
  EXPECT_EQ(katakanaToRomaji("エ"), "e");
  EXPECT_EQ(katakanaToRomaji("アカ　サカ！"), "aka saka!");
  EXPECT_EQ(katakanaToRomaji("イェビス"), "yebisu");
  checkSmallKana(CharType::Katakana, "ァィゥェォヵヶャュョッヮ");
  EXPECT_EQ(katakanaToRomaji("テニス"), "tenisu");
  EXPECT_EQ(katakanaToRomaji("カナダ"), "kanada");
  EXPECT_EQ(katakanaToRomaji("ナカ・グロ"), "naka/guro");
  // add apostrophe before a vowel or 'y' as per Hepburn standard
  EXPECT_EQ(katakanaToRomaji("タンイ"), "tan'i");
  EXPECT_EQ(katakanaToRomaji("ポンヨ"), "pon'yo");
  // here are the same examples without the apostrophes
  EXPECT_EQ(katakanaToRomaji("タニ"), "tani");
  EXPECT_EQ(katakanaToRomaji("ポニョ"), "ponyo"); // the correct movie name
  // Sokuon handling
  EXPECT_EQ(katakanaToRomaji("アッパ"), "appa");
  EXPECT_EQ(katakanaToRomaji("マッチ"), "matchi");
  EXPECT_EQ(katakanaToRomaji("ジョッキ"), "jokki");
  // not sure what to do with a final or repeated small tsu ... for now it falls
  // back to 'wāpuro', i.e., exactly what you would need to type on a keyboard
  // to reproduce the Hiragana.
  EXPECT_EQ(katakanaToRomaji("イテッ"), "iteltu");
  EXPECT_EQ(katakanaToRomaji("イッッテ"), "iltutte");
  // prolonged sound mark is mainly for Katakana, but also works for Hiragana,
  // for now using this mark is the only way to get a macron (bar over letter)
  // in Romaji output.
  EXPECT_EQ(katakanaToRomaji("ラーメン"), "rāmen");
  EXPECT_EQ(katakanaToRomaji("キャー"), "kyā");
  EXPECT_EQ(katakanaToRomaji("ファーザー"), "fāzā");
  // ー not following a vowel is left unchanged
  EXPECT_EQ(katakanaToRomaji("ーカ"), "ーka");
  EXPECT_EQ(katakanaToRomaji("ホンート"), "honーto");
  // Hepburn examples
  EXPECT_EQ(katakanaToRomaji("チヂム"), "chidimu");
  EXPECT_EQ(katakanaToRomaji("チヂム", ConvertFlags::Hepburn), "chijimu");
  EXPECT_EQ(katakanaToRomaji("ツヅキ"), "tsuduki");
  EXPECT_EQ(katakanaToRomaji("ツヅキ", ConvertFlags::Hepburn), "tsuzuki");
  EXPECT_EQ(katakanaToRomaji("ヂャ"), "dya");
  EXPECT_EQ(katakanaToRomaji("ヂャ", ConvertFlags::Hepburn), "ja");
  EXPECT_EQ(katakanaToRomaji("ヂュ"), "dyu");
  EXPECT_EQ(katakanaToRomaji("ヂュ", ConvertFlags::Hepburn), "ju");
  EXPECT_EQ(katakanaToRomaji("ヂョ"), "dyo");
  EXPECT_EQ(katakanaToRomaji("ヂョ", ConvertFlags::Hepburn), "jo");
  EXPECT_EQ(katakanaToRomaji("ヲ"), "wo");
  EXPECT_EQ(katakanaToRomaji("ヲ", ConvertFlags::Hepburn), "o");
}

TEST_F(KanaConvertTest, ConvertBetweenKana) {
  for (auto& i : Kana::getMap(CharType::Hiragana)) {
    const auto r{
      _converter.convert(CharType::Hiragana, i.first, CharType::Katakana)};
    EXPECT_EQ(r, i.second->katakana());
    EXPECT_EQ(_converter.convert(CharType::Katakana, r, CharType::Hiragana),
              i.second->hiragana());
  }
  for (auto& i : Kana::getMap(CharType::Katakana)) {
    const auto r{
      _converter.convert(CharType::Katakana, i.first, CharType::Hiragana)};
    EXPECT_EQ(r, i.second->hiragana());
    EXPECT_EQ(_converter.convert(CharType::Hiragana, r, CharType::Katakana),
              i.second->katakana());
  }
  kanaConvertCheck("きょうはいいてんきです。", "キョウハイイテンキデス。");
  // try mixing sokuon and long vowels
  kanaConvertCheck("らーめん！", "ラーメン！");
  kanaConvertCheck("びっぐ　ばあど、すまーる　はっまー？",
                   "ビッグ　バアド、スマール　ハッマー？");
  kanaConvertCheck("じょん・どー", "ジョン・ドー");
}

TEST_F(KanaConvertTest, RepeatSymbol) {
  kanaConvertCheck("かゝ", "カヽ", "kaka");
  kanaConvertCheck("かゞ", "カヾ", "kaga");
  kanaConvertCheck("がゝ", "ガヽ", "gaka");
  kanaConvertCheck("がゞ", "ガヾ", "gaga");
  kanaConvertCheck("こゝろ", "コヽロ", "kokoro");
  kanaConvertCheck("はゝゝゝ", "ハヽヽヽ", "hahahaha");
  // examples with h, b and p
  kanaConvertCheck("ひゝ", "ヒヽ", "hihi");
  kanaConvertCheck("ひゞ", "ヒヾ", "hibi");
  kanaConvertCheck("びゝ", "ビヽ", "bihi");
  kanaConvertCheck("びゞ", "ビヾ", "bibi");
  kanaConvertCheck("ぴゝ", "ピヽ", "pihi");
  kanaConvertCheck("ぴゞ", "ピヾ", "pipi");
  // don't convert a repeat symbol if it's not part of 'source' type
  EXPECT_EQ(hiraganaToKatakana("こヽろ"), "コヽロ");
  EXPECT_EQ(hiraganaToKatakana("こヾろ"), "コヾロ");
  EXPECT_EQ(hiraganaToRomaji("こヽろ"), "koヽro");
  EXPECT_EQ(hiraganaToRomaji("こヾろ"), "koヾro");
  EXPECT_EQ(katakanaToHiragana("コゝロ"), "こゝろ");
  EXPECT_EQ(katakanaToHiragana("コゞロ"), "こゞろ");
  EXPECT_EQ(katakanaToRomaji("コゝロ"), "koゝro");
  EXPECT_EQ(katakanaToRomaji("コゞロ"), "koゞro");
  // currently a digraph is also be repeated - this might not be correct
  kanaConvertCheck("きょゝ", "キョヽ", "kyokyo");
  kanaConvertCheck("きょゞ", "キョヾ", "kyogyo");
  // repeating symbol is ignored after 'prolong' mark when target is Romaji
  kanaConvertCheck("はーゝろー", "ハーヽロー", "hārō");
  kanaConvertCheck("ばーゞろー", "バーヾロー", "bārō");
  // repeating symbol at the begining is an error so drop for romaji, but can
  // still convert for kana
  kanaConvertCheck("ゝろ", "ヽロ", "ro");
}

TEST_F(KanaConvertTest, ConvertAllToOneType) {
  EXPECT_EQ(_converter.convert("ima クリスマス　です。", CharType::Romaji),
            "ima kurisumasu desu.");
  EXPECT_EQ(_converter.convert("ima クリスマス　です。", CharType::Hiragana),
            "いま　くりすます　です。");
  EXPECT_EQ(_converter.convert("ima クリスマス　です。", CharType::Katakana),
            "イマ　クリスマス　デス。");
  EXPECT_EQ(_converter.convert("rāmenらーめんラーメン!!", CharType::Romaji),
            "rāmenrāmenrāmen!!");
  EXPECT_EQ(_converter.convert("rāmenらーめんラーメン!!", CharType::Hiragana),
            "らーめんらーめんらーめん！！");
  EXPECT_EQ(_converter.convert("rāmenらーめんラーメン!!", CharType::Katakana),
            "ラーメンラーメンラーメン！！");
}

TEST_F(KanaConvertTest, HepburnVersusKunrei) {
  // Romaji output is usually Modern Hepburn by default, but will be Nihon Shiki
  // sometimes in order to be unique for round-trips (plus there are a lot of
  // extra wāpuro entries). Below are the entries from the Differences among
  // romanizations table: https://en.wikipedia.org/wiki/Romanization_of_Japanese
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
  // -- RA
  check("ら", "ラ", "ra");
  check("り", "リ", "ri");
  check("る", "ル", "ru");
  check("れ", "レ", "re");
  check("ろ", "ロ", "ro");
  check("りゃ", "リャ", "rya");
  check("りゅ", "リュ", "ryu");
  check("りょ", "リョ", "ryo");
  // -- WA and N
  check("わ", "ワ", "wa");
  // Nihon Shiki for the following rare kana are 'wi' and 'we' respectively, but
  // wāpuro values are used instead (since 'wi' and 'we' are already used for
  // the more common diagraphs ウィ and ウェ. Hepburn and Kunrei are both 'i'
  // and 'e' for these.
  check("ゐ", "ヰ", "wyi", "i", "i");
  check("ゑ", "ヱ", "wye", "e", "e");
  // both Hepburn and Kunrei use 'o' for を, but program (and Nihon Shiki) uses
  // 'wo' for uniqueness
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
  // Lots of differences for this group, for example the mapping for ヂ in Nihon
  // Shiki style (and default for this program) is 'di', whereas Hepburn is 'ji'
  // and Kunrei is 'zi'.
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

} // namespace kanji_tools
