#include <gtest/gtest.h>
#include <kanji_tools/kana/Converter.h>
#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

namespace {

class ConverterTest : public ::testing::Test {
protected:
  [[nodiscard]] auto romajiToHiragana(
      const String& s, ConvertFlags flags = ConvertFlags::None) {
    return _converter.convert(CharType::Romaji, s, CharType::Hiragana, flags);
  }

  [[nodiscard]] auto romajiToKatakana(
      const String& s, ConvertFlags flags = ConvertFlags::None) {
    return _converter.convert(CharType::Romaji, s, CharType::Katakana, flags);
  }

  [[nodiscard]] auto hiraganaToRomaji(
      const String& s, ConvertFlags flags = ConvertFlags::None) {
    return _converter.convert(CharType::Hiragana, s, CharType::Romaji, flags);
  }

  [[nodiscard]] auto hiraganaToKatakana(const String& s) {
    return _converter.convert(CharType::Hiragana, s, CharType::Katakana);
  }

  [[nodiscard]] auto katakanaToRomaji(
      const String& s, ConvertFlags flags = ConvertFlags::None) {
    return _converter.convert(CharType::Katakana, s, CharType::Romaji, flags);
  }

  [[nodiscard]] auto katakanaToHiragana(const String& s) {
    return _converter.convert(CharType::Katakana, s, CharType::Hiragana);
  }

  // pass in 'romaji' when round trip is lossy (like repeat symbols)
  void kanaConvertCheck(const String& hiragana, // NOLINT
      const String& katakana, const String& romaji = {}) {
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

  void check(const String& hiragana, const String& katakana, // NOLINT
      const String& romaji, const char* hepburn = {}, const char* kunrei = {},
      const char* both = {}) {
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
    auto result{both ? both : hepburn ? hepburn : kunrei ? kunrei : romaji};
    EXPECT_EQ(hiraganaToRomaji(
                  hiragana, ConvertFlags::Hepburn | ConvertFlags::Kunrei),
        result);
    EXPECT_EQ(katakanaToRomaji(
                  katakana, ConvertFlags::Hepburn | ConvertFlags::Kunrei),
        result);
  }

  void checkKunrei(const String& hiragana, const String& katakana,
      const String& romaji, const char* kunrei) {
    check(hiragana, katakana, romaji, nullptr, kunrei);
  }

  void checkSmallKana(CharType source, const String& s) {
    // small letters that don't form part of a digraph are output in 'wāpuro'
    // style favoring 'l' instead of 'x' as first letter (so small tsu is 'ltu')
    String romaji{"lalilulelolkalkelyalyulyoltulwa"};
    EXPECT_EQ(_converter.convert(source, s, CharType::Romaji), romaji);
    EXPECT_EQ(_converter.convert(CharType::Romaji, romaji, source), s);
    // also test small letters starting with 'x'
    std::replace(romaji.begin(), romaji.end(), 'l', 'x');
    EXPECT_EQ(_converter.convert(CharType::Romaji, romaji, source), s);
  }

  [[nodiscard]] auto& converter() { return _converter; }
private:
  Converter _converter;
};

} // namespace

TEST_F(ConverterTest, FlagString) {
  EXPECT_EQ(converter().flagString(), "None");
  converter().flags(ConvertFlags::Hepburn);
  EXPECT_EQ(converter().flagString(), "Hepburn");
  converter().flags(converter().flags() | ConvertFlags::Kunrei);
  EXPECT_EQ(converter().flagString(), "Hepburn|Kunrei");
  converter().flags(converter().flags() | ConvertFlags::NoProlongMark);
  EXPECT_EQ(converter().flagString(), "Hepburn|Kunrei|NoProlongMark");
  converter().flags(ConvertFlags::Kunrei | ConvertFlags::RemoveSpaces);
  EXPECT_EQ(converter().flagString(), "Kunrei|RemoveSpaces");
}

TEST_F(ConverterTest, CheckConvertTarget) {
  EXPECT_EQ(converter().target(), CharType::Hiragana); // check default ctor
  Converter converter(CharType::Katakana);
  EXPECT_EQ(converter.target(), CharType::Katakana); // check ctor
  converter.target(CharType::Romaji);
  EXPECT_EQ(converter.target(), CharType::Romaji); // check update
}

TEST_F(ConverterTest, CheckConvertFlags) {
  EXPECT_EQ(converter().flags(), ConvertFlags::None); // check default ctor
  Converter converter(CharType::Romaji, ConvertFlags::Hepburn);
  EXPECT_EQ(converter.flags(), ConvertFlags::Hepburn); // check ctor
  converter.flags(ConvertFlags::Kunrei);
  EXPECT_EQ(converter.flags(), ConvertFlags::Kunrei); // check update
}

TEST_F(ConverterTest, NoConversionIfSourceAndTargetAreTheSame) {
  String s{"atatakaiあたたかいアタタカイ"};
  EXPECT_EQ(converter().convert(CharType::Romaji, s, CharType::Romaji), s);
  EXPECT_EQ(converter().convert(CharType::Hiragana, s, CharType::Hiragana), s);
  EXPECT_EQ(converter().convert(CharType::Katakana, s, CharType::Katakana), s);
}

TEST_F(ConverterTest, ConvertRomajiToHiragana) {
  EXPECT_EQ(romajiToHiragana("a"), "あ");
  EXPECT_EQ(romajiToHiragana("ka"), "か");
  EXPECT_EQ(romajiToHiragana("kitte"), "きって");
  EXPECT_EQ(romajiToHiragana("burikko"), "ぶりっこ");
  EXPECT_EQ(romajiToHiragana("tte"), "って");
  EXPECT_EQ(romajiToHiragana("ryo"), "りょ");
  // ō or other macrons map to the same vowel in hiragana which is of course not
  // always correct so to preserve round-trip a macron is mapped to a prolonged
  // mark (ー). This isn't standard and can be turned off by a flag (see
  // Converter.h for details). 'ou' can be used instead to avoid ambiguity.
  EXPECT_EQ(romajiToHiragana("tōkyō"), "とーきょー");
  EXPECT_EQ(romajiToHiragana("toukyou"), "とうきょう");
  // This next case is of course incorrect, but it's the standard mapping for
  // modern Hepburn romanization.
  EXPECT_EQ(
      romajiToHiragana("tōkyō", ConvertFlags::NoProlongMark), "とおきょお");
  EXPECT_EQ(romajiToHiragana("rāmen da",
                ConvertFlags::NoProlongMark | ConvertFlags::RemoveSpaces),
      "らあめんだ");
  EXPECT_EQ(romajiToHiragana("no"), "の");
  EXPECT_EQ(romajiToHiragana("ken"), "けん");
  EXPECT_EQ(romajiToHiragana("kannon"), "かんのん");
  EXPECT_EQ(romajiToHiragana("jun'ichi"), "じゅんいち");
  EXPECT_EQ(romajiToHiragana("kani"), "かに");
  EXPECT_EQ(romajiToHiragana("kan-i"), "かんい");
  EXPECT_EQ(romajiToHiragana("ninja samurai"), "にんじゃ　さむらい");
  // case insensitive
  EXPECT_EQ(
      romajiToHiragana("Dare desu ka? ngya!"), "だれ　です　か？　んぎゃ！");
  EXPECT_EQ(
      romajiToHiragana("Dare dESu ka? kyaa!!", ConvertFlags::RemoveSpaces),
      "だれですか？きゃあ！！");
  // don't convert non-romaji
  EXPECT_EQ(romajiToHiragana("店じまいdesu."), "店じまいです。");
  EXPECT_EQ(romajiToHiragana("[サメはkowai!]"), "「サメはこわい！」");
}

TEST_F(ConverterTest, ConvertRomajiToKatakana) {
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

TEST_F(ConverterTest, ConvertHiraganaToRomaji) {
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
}

TEST_F(ConverterTest, ConvertKatakanaToRomaji) {
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
}

TEST_F(ConverterTest, ProlongMark) {
  // prolonged sound mark is mainly for Katakana, but also works for Hiragana,
  // for now using this mark is the only way to get a macron (bar over letter)
  // in Romaji output.
  check("らーめん", "ラーメン", "rāmen");
  check("きゃー", "キャー", "kyā");
  EXPECT_EQ(katakanaToRomaji("ファーザー"), "fāzā");
  EXPECT_EQ(katakanaToRomaji("コーヒー"), "kōhī");
  EXPECT_EQ(katakanaToRomaji("ツー"), "tsū");
  EXPECT_EQ(katakanaToRomaji("ページ"), "pēji");
  // ー not following a vowel is left unchanged
  EXPECT_EQ(hiraganaToRomaji("ーぶ"), "ーbu");
  EXPECT_EQ(hiraganaToRomaji("はんーぶ"), "hanーbu");
  EXPECT_EQ(katakanaToRomaji("ーカ"), "ーka");
  EXPECT_EQ(katakanaToRomaji("ホンート"), "honーto");
}

TEST_F(ConverterTest, HepburnAndKunrei) {
  // third param is 'Hepburn', fourth is 'Kunrei', fifth is both flags enabled
  check("ちぢむ", "チヂム", "chidimu", "chijimu", "tizimu", "tijimu");
  check("つづき", "ツヅキ", "tsuduki", "tsuzuki", "tuzuki", "tuzuki");
  // explanation of 'tijimu':
  // - when both Hepburn and Kunrei are set then the Hepburn value is preferred
  // - this leads to 'ぢ' mapping to 'ji' (instead of 'zi' or the default 'di')
  // - but 'ち' maps to 'ti' (the 'Kunrei' value) since there is no 'Hepburn'
  //   value override, i.e., just the 'Hepburn' flag would produce 'chi'
  // best idea is to only set 'flags' to one or the other (or neither) to avoid
  // surprising results (see 'HepburnVersusKunrei' test below to see all values)
}

TEST_F(ConverterTest, ConvertBetweenKana) {
  for (auto& i : Kana::getMap(CharType::Hiragana)) {
    const auto r{
        converter().convert(CharType::Hiragana, i.first, CharType::Katakana)};
    EXPECT_EQ(r, i.second->katakana());
    EXPECT_EQ(converter().convert(CharType::Katakana, r, CharType::Hiragana),
        i.second->hiragana());
  }
  for (auto& i : Kana::getMap(CharType::Katakana)) {
    const auto r{
        converter().convert(CharType::Katakana, i.first, CharType::Hiragana)};
    EXPECT_EQ(r, i.second->hiragana());
    EXPECT_EQ(converter().convert(CharType::Hiragana, r, CharType::Katakana),
        i.second->katakana());
  }
  kanaConvertCheck("きょうはいいてんきです。", "キョウハイイテンキデス。");
  // try mixing sokuon and long vowels
  kanaConvertCheck("らーめん！", "ラーメン！");
  kanaConvertCheck("びっぐ　ばあど、すまーる　はっまー？",
      "ビッグ　バアド、スマール　ハッマー？");
  kanaConvertCheck("じょん・どー", "ジョン・ドー");
}

TEST_F(ConverterTest, RepeatSymbol) {
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

TEST_F(ConverterTest, ConvertAllToOneType) {
  EXPECT_EQ(converter().convert("ima クリスマス　です。", CharType::Romaji),
      "ima kurisumasu desu.");
  EXPECT_EQ(converter().convert("ima クリスマス　です。", CharType::Hiragana),
      "いま　くりすます　です。");
  EXPECT_EQ(converter().convert("ima クリスマス　です。", CharType::Katakana),
      "イマ　クリスマス　デス。");
  EXPECT_EQ(converter().convert("rāmenらーめんラーメン!!", CharType::Romaji),
      "rāmenrāmenrāmen!!");
  EXPECT_EQ(converter().convert("rāmenらーめんラーメン!!", CharType::Hiragana),
      "らーめんらーめんらーめん！！");
  EXPECT_EQ(converter().convert("rāmenらーめんラーメン!!", CharType::Katakana),
      "ラーメンラーメンラーメン！！");
}

TEST_F(ConverterTest, UnsupportedKana) {
  // leave unsupported Kana symbols unconverted
  EXPECT_EQ(hiraganaToRomaji("かゟこ"), "kaゟko"); // Hiragana 'yori'
  EXPECT_EQ(katakanaToRomaji("カヿコ"), "kaヿko"); // Katakana 'koto'
}

TEST_F(ConverterTest, UnsupportedRomaji) {
  // leave unsupported Rōmaji combinations unconverted
  EXPECT_EQ(romajiToHiragana("TGIF"), "TぎF");
  EXPECT_EQ(romajiToKatakana("Alba"), "アlバ");
  // incorrect 'n'
  EXPECT_EQ(romajiToHiragana("sHni"), "sHに");
  // incorrect macron
  EXPECT_EQ(romajiToKatakana("Vyī"), "Vyイ");
}

TEST_F(ConverterTest, HepburnVersusKunrei) {
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

TEST_F(ConverterTest, CheckDelims) {
  using P = std::pair<char, const char*>;
  for (const auto& i : {P{' ', "　"}, P{'.', "。"}, P{',', "、"}, P{':', "："},
           P{';', "；"}, P{'/', "・"}, P{'!', "！"}, P{'?', "？"}, P{'(', "（"},
           P{')', "）"}, P{'[', "「"}, P{']', "」"}, P{'*', "＊"}, P{'~', "〜"},
           P{'=', "＝"}, P{'+', "＋"}, P{'@', "＠"}, P{'#', "＃"}, P{'$', "＄"},
           P{'%', "％"}, P{'^', "＾"}, P{'&', "＆"}, P{'{', "『"}, P{'}', "』"},
           P{'|', "｜"}, P{'"', "”"}, P{'`', "｀"}, P{'<', "＜"}, P{'>', "＞"},
           P{'_', "＿"}, P{'\\', "￥"}}) {
    const String romaji{i.first}, kana{i.second};
    check(kana, kana, romaji);
  }
}

} // namespace kanji_tools
