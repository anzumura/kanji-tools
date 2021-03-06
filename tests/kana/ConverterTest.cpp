#include <gtest/gtest.h>
#include <kt_kana/Converter.h>
#include <kt_kana/Kana.h>
#include <kt_utils/UnicodeBlock.h>

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
    // small letters that don't form part of a digraph are output in 'w??puro'
    // style favoring 'l' instead of 'x' as first letter (so small tsu is 'ltu')
    String romaji{"lalilulelolkalkelyalyulyoltulwa"}; // cSpell:disable-line
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
  String s{"atatakai??????????????????????????????"};
  EXPECT_EQ(converter().convert(CharType::Romaji, s, CharType::Romaji), s);
  EXPECT_EQ(converter().convert(CharType::Hiragana, s, CharType::Hiragana), s);
  EXPECT_EQ(converter().convert(CharType::Katakana, s, CharType::Katakana), s);
}

TEST_F(ConverterTest, ConvertRomajiToHiragana) {
  EXPECT_EQ(romajiToHiragana("a"), "???");
  EXPECT_EQ(romajiToHiragana("ka"), "???");
  EXPECT_EQ(romajiToHiragana("kitte"), "?????????");
  EXPECT_EQ(romajiToHiragana("burikko"), "????????????");
  EXPECT_EQ(romajiToHiragana("tte"), "??????");
  EXPECT_EQ(romajiToHiragana("ryo"), "??????");
  // ?? or other macrons map to the same vowel in hiragana which is of course not
  // always correct so to preserve round-trip a macron is mapped to a prolonged
  // mark (???). This isn't standard and can be turned off by a flag (see
  // Converter.h for details). 'ou' can be used instead to avoid ambiguity.
  EXPECT_EQ(romajiToHiragana("t??ky??"), "???????????????");
  EXPECT_EQ(romajiToHiragana("toukyou"), "???????????????");
  // This next case is of course incorrect, but it's the standard mapping for
  // modern Hepburn romanization.
  EXPECT_EQ(
      romajiToHiragana("t??ky??", ConvertFlags::NoProlongMark), "???????????????");
  EXPECT_EQ(romajiToHiragana("r??men da",
                ConvertFlags::NoProlongMark | ConvertFlags::RemoveSpaces),
      "???????????????");
  EXPECT_EQ(romajiToHiragana("no"), "???");
  EXPECT_EQ(romajiToHiragana("ken"), "??????");
  EXPECT_EQ(romajiToHiragana("kannon"), "????????????");
  EXPECT_EQ(romajiToHiragana("jun'ichi"), "???????????????");
  EXPECT_EQ(romajiToHiragana("kani"), "??????");
  EXPECT_EQ(romajiToHiragana("kan-i"), "?????????");
  EXPECT_EQ(romajiToHiragana("ninja samurai"), "???????????????????????????");
  // case insensitive - cSpell:disable
  EXPECT_EQ(
      romajiToHiragana("Dare desu ka? ngya!"), "???????????????????????????????????????");
  EXPECT_EQ(
      romajiToHiragana("Dare dESu ka? kyaa!!", ConvertFlags::RemoveSpaces),
      "?????????????????????????????????"); // cSpell:enable
  // don't convert non-romaji
  EXPECT_EQ(romajiToHiragana("????????????desu."), "?????????????????????");
  EXPECT_EQ(romajiToHiragana("[?????????kowai!]"), "???????????????????????????");
}

TEST_F(ConverterTest, ConvertRomajiToKatakana) {
  EXPECT_EQ(romajiToKatakana("i"), "???");
  EXPECT_EQ(romajiToKatakana("ke"), "???");
  // support both standard way (t+chi) as well as the w??puro way (c+chi)
  EXPECT_EQ(romajiToKatakana("matchi"), "?????????");
  EXPECT_EQ(romajiToKatakana("macchi"), "?????????");
  // use macrons to get a katakana '???' - cSpell:disable
  EXPECT_EQ(romajiToKatakana("s??r??"), "????????????");
  EXPECT_EQ(romajiToKatakana("p??th??"), "???????????????");
  EXPECT_EQ(romajiToKatakana("ch??zu"), "?????????");
  EXPECT_EQ(romajiToKatakana("chiizu"), "?????????");
  // don't convert non-romaji - cSpell:enable
  EXPECT_EQ(romajiToKatakana("????????????desu."), "?????????????????????");
  EXPECT_EQ(romajiToKatakana("[?????????kowai!]"), "???????????????????????????");
  // don't convert invalid romaji
  EXPECT_EQ(romajiToKatakana("(hello world)"), "??????l?????????rld???");
}

TEST_F(ConverterTest, ConvertHiraganaToRomaji) {
  EXPECT_EQ(hiraganaToRomaji("???"), "u");
  EXPECT_EQ(hiraganaToRomaji("?????????????????????"), "katsu sando!");
  EXPECT_EQ(hiraganaToRomaji("????????????"), "4wiki");
  EXPECT_EQ(hiraganaToRomaji("?????????"), "onna");
  checkSmallKana(CharType::Hiragana, "????????????????????????????????????");
  EXPECT_EQ(hiraganaToRomaji("????????????"), "kyouto");
  EXPECT_EQ(hiraganaToRomaji("????????????"), "niigata");
  EXPECT_EQ(hiraganaToRomaji("??????????????????"), "kankeinai");
  EXPECT_EQ(hiraganaToRomaji("???????????????"), "naka/guro");
  // add apostrophe before a vowel or 'y' as per Hepburn standard
  EXPECT_EQ(hiraganaToRomaji("?????????"), "kan'i");
  EXPECT_EQ(hiraganaToRomaji("????????????"), "shin'you");
  // here are the same examples without the apostrophes
  EXPECT_EQ(hiraganaToRomaji("??????"), "kani");
  EXPECT_EQ(hiraganaToRomaji("????????????"), "shinyou");
  // Sokuon handling
  EXPECT_EQ(hiraganaToRomaji("?????????"), "kitto");
  EXPECT_EQ(hiraganaToRomaji("????????????"), "beppin");
  EXPECT_EQ(hiraganaToRomaji("???????????????"), "kokkyou");
  // not sure what to do with a final or repeated small tsu ... for now it falls
  // back to 'w??puro', i.e., exactly what you would need to type on a keyboard
  // to reproduce the Hiragana.
  EXPECT_EQ(hiraganaToRomaji("?????????"), "iteltu");    // cSpell:disable-line
  EXPECT_EQ(hiraganaToRomaji("????????????"), "iltutte"); // cSpell:disable-line
}

TEST_F(ConverterTest, ConvertKatakanaToRomaji) {
  EXPECT_EQ(katakanaToRomaji("???"), "e");
  EXPECT_EQ(katakanaToRomaji("??????????????????"), "aka saka!");
  EXPECT_EQ(katakanaToRomaji("????????????"), "yebisu");
  checkSmallKana(CharType::Katakana, "????????????????????????????????????");
  EXPECT_EQ(katakanaToRomaji("?????????"), "tenisu");
  EXPECT_EQ(katakanaToRomaji("?????????"), "kanada");
  EXPECT_EQ(katakanaToRomaji("???????????????"), "naka/guro");
  // add apostrophe before a vowel or 'y' as per Hepburn standard
  EXPECT_EQ(katakanaToRomaji("?????????"), "tan'i");
  EXPECT_EQ(katakanaToRomaji("?????????"), "pon'yo");
  // here are the same examples without the apostrophes
  EXPECT_EQ(katakanaToRomaji("??????"), "tani");
  EXPECT_EQ(katakanaToRomaji("?????????"), "ponyo"); // the correct movie name
  // Sokuon handling
  EXPECT_EQ(katakanaToRomaji("?????????"), "appa");
  EXPECT_EQ(katakanaToRomaji("?????????"), "matchi");
  EXPECT_EQ(katakanaToRomaji("????????????"), "jokki");
  // not sure what to do with a final or repeated small tsu ... for now it falls
  // back to 'w??puro', i.e., exactly what you would need to type on a keyboard
  // to reproduce the Hiragana.
  EXPECT_EQ(katakanaToRomaji("?????????"), "iteltu");    // cSpell:disable-line
  EXPECT_EQ(katakanaToRomaji("????????????"), "iltutte"); // cSpell:disable-line
}

TEST_F(ConverterTest, ProlongMark) {
  // prolonged sound mark is mainly for Katakana, but also works for Hiragana,
  // for now using this mark is the only way to get a macron (bar over letter)
  // in Romaji output.
  check("????????????", "????????????", "r??men");
  check("?????????", "?????????", "ky??");
  EXPECT_EQ(katakanaToRomaji("???????????????"), "f??z??");
  EXPECT_EQ(katakanaToRomaji("????????????"), "k??h??");
  EXPECT_EQ(katakanaToRomaji("??????"), "ts??");
  EXPECT_EQ(katakanaToRomaji("?????????"), "p??ji");
  // ??? not following a vowel is left unchanged
  EXPECT_EQ(hiraganaToRomaji("??????"), "???bu");
  EXPECT_EQ(hiraganaToRomaji("????????????"), "han???bu");
  EXPECT_EQ(katakanaToRomaji("??????"), "???ka");
  EXPECT_EQ(katakanaToRomaji("????????????"), "hon???to");
}

TEST_F(ConverterTest, HepburnAndKunrei) {
  // third param is 'Hepburn', fourth is 'Kunrei', fifth is both flags enabled
  check("?????????", "?????????", "chidimu", "chijimu", "tizimu", "tijimu");
  check("?????????", "?????????", "tsuduki", "tsuzuki", "tuzuki", "tuzuki");
  // explanation of 'tijimu':
  // - when both Hepburn and Kunrei are set then the Hepburn value is preferred
  // - this leads to '???' mapping to 'ji' (instead of 'zi' or the default 'di')
  // - but '???' maps to 'ti' (the 'Kunrei' value) since there is no 'Hepburn'
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
  kanaConvertCheck("???????????????????????????????????????", "???????????????????????????????????????");
  // try mixing sokuon and long vowels
  kanaConvertCheck("???????????????", "???????????????");
  kanaConvertCheck("??????????????????????????????????????????????????????",
      "??????????????????????????????????????????????????????");
  kanaConvertCheck("??????????????????", "??????????????????");
}

TEST_F(ConverterTest, RepeatSymbol) { // cSpell:disable
  kanaConvertCheck("??????", "??????", "kaka");
  kanaConvertCheck("??????", "??????", "kaga");
  kanaConvertCheck("??????", "??????", "gaka");
  kanaConvertCheck("??????", "??????", "gaga");
  kanaConvertCheck("?????????", "?????????", "kokoro");
  kanaConvertCheck("????????????", "????????????", "hahahaha");
  // examples with h, b and p
  kanaConvertCheck("??????", "??????", "hihi");
  kanaConvertCheck("??????", "??????", "hibi");
  kanaConvertCheck("??????", "??????", "bihi");
  kanaConvertCheck("??????", "??????", "bibi");
  kanaConvertCheck("??????", "??????", "pihi");
  kanaConvertCheck("??????", "??????", "pipi");
  // don't convert a repeat symbol if it's not part of 'source' type
  EXPECT_EQ(hiraganaToKatakana("?????????"), "?????????");
  EXPECT_EQ(hiraganaToKatakana("?????????"), "?????????");
  EXPECT_EQ(hiraganaToRomaji("?????????"), "ko???ro");
  EXPECT_EQ(hiraganaToRomaji("?????????"), "ko???ro");
  EXPECT_EQ(katakanaToHiragana("?????????"), "?????????");
  EXPECT_EQ(katakanaToHiragana("?????????"), "?????????");
  EXPECT_EQ(katakanaToRomaji("?????????"), "ko???ro");
  EXPECT_EQ(katakanaToRomaji("?????????"), "ko???ro");
  // currently a digraph is also be repeated - this might not be correct
  kanaConvertCheck("?????????", "?????????", "kyokyo");
  kanaConvertCheck("?????????", "?????????", "kyogyo");
  // repeating symbol is ignored after 'prolong' mark when target is Romaji
  kanaConvertCheck("???????????????", "???????????????", "h??r??");
  kanaConvertCheck("???????????????", "???????????????", "b??r??");
  // repeating symbol at the beginning is an error so drop for romaji, but can
  // still convert for kana
  kanaConvertCheck("??????", "??????", "ro");
} // cSpell:enable

TEST_F(ConverterTest, ConvertAllToOneType) {
  EXPECT_EQ(converter().convert("ima ???????????????????????????", CharType::Romaji),
      "ima kurisumasu desu.");
  EXPECT_EQ(converter().convert("ima ???????????????????????????", CharType::Hiragana),
      "????????????????????????????????????");
  EXPECT_EQ(converter().convert("ima ???????????????????????????", CharType::Katakana),
      "????????????????????????????????????");
  EXPECT_EQ(converter().convert("r??men????????????????????????!!", CharType::Romaji),
      "r??menr??menr??men!!"); // cSpell:disable-line
  EXPECT_EQ(converter().convert("r??men????????????????????????!!", CharType::Hiragana),
      "??????????????????????????????????????????");
  EXPECT_EQ(converter().convert("r??men????????????????????????!!", CharType::Katakana),
      "??????????????????????????????????????????");
}

TEST_F(ConverterTest, UnsupportedKana) {
  // leave unsupported Kana symbols unconverted
  EXPECT_EQ(hiraganaToRomaji("?????????"), "ka???ko"); // Hiragana 'yori'
  EXPECT_EQ(katakanaToRomaji("?????????"), "ka???ko"); // Katakana 'koto'
}

TEST_F(ConverterTest, UnsupportedRomaji) {
  // leave unsupported R??maji combinations unconverted
  EXPECT_EQ(romajiToHiragana("TGIF"), "T???F");
  EXPECT_EQ(romajiToKatakana("Alba"), "???l???");
  // incorrect 'n'
  EXPECT_EQ(romajiToHiragana("sHni"), "sH???");
  // incorrect macron
  EXPECT_EQ(romajiToKatakana("Vy??"), "Vy???");
}

TEST_F(ConverterTest, HepburnVersusKunrei) {
  // Romaji output is usually Modern Hepburn by default, but will be Nihon Shiki
  // sometimes in order to be unique for round-trips (plus there are a lot of
  // extra w??puro entries). Below are the entries from the Differences among
  // romanizations table: https://en.wikipedia.org/wiki/Romanization_of_Japanese
  // -- A
  check("???", "???", "a");
  check("???", "???", "i");
  check("???", "???", "u");
  check("???", "???", "e");
  check("???", "???", "o");
  // -- KA
  check("???", "???", "ka");
  check("???", "???", "ki");
  check("???", "???", "ku");
  check("???", "???", "ke");
  check("???", "???", "ko");
  check("??????", "??????", "kya");
  check("??????", "??????", "kyu");
  check("??????", "??????", "kyo");
  // -- SA
  check("???", "???", "sa");
  checkKunrei("???", "???", "shi", "si");
  check("???", "???", "su");
  check("???", "???", "se");
  check("???", "???", "so");
  checkKunrei("??????", "??????", "sha", "sya");
  checkKunrei("??????", "??????", "shu", "syu");
  checkKunrei("??????", "??????", "sho", "syo");
  // -- TA
  check("???", "???", "ta");
  checkKunrei("???", "???", "chi", "ti");
  checkKunrei("???", "???", "tsu", "tu");
  check("???", "???", "te");
  check("???", "???", "to");
  checkKunrei("??????", "??????", "cha", "tya");
  checkKunrei("??????", "??????", "chu", "tyu");
  checkKunrei("??????", "??????", "cho", "tyo");
  // -- NA
  check("???", "???", "na");
  check("???", "???", "ni");
  check("???", "???", "nu");
  check("???", "???", "ne");
  check("???", "???", "no");
  check("??????", "??????", "nya");
  check("??????", "??????", "nyu");
  check("??????", "??????", "nyo");
  // -- HA
  check("???", "???", "ha");
  check("???", "???", "hi");
  checkKunrei("???", "???", "fu", "hu");
  check("???", "???", "he");
  check("???", "???", "ho");
  check("??????", "??????", "hya");
  check("??????", "??????", "hyu");
  check("??????", "??????", "hyo");
  // -- MA
  check("???", "???", "ma");
  check("???", "???", "mi");
  check("???", "???", "mu");
  check("???", "???", "me");
  check("???", "???", "mo");
  check("??????", "??????", "mya");
  check("??????", "??????", "myu");
  check("??????", "??????", "myo");
  // -- YA
  check("???", "???", "ya");
  check("???", "???", "yu");
  check("???", "???", "yo");
  // -- RA
  check("???", "???", "ra");
  check("???", "???", "ri");
  check("???", "???", "ru");
  check("???", "???", "re");
  check("???", "???", "ro");
  check("??????", "??????", "rya");
  check("??????", "??????", "ryu");
  check("??????", "??????", "ryo");
  // -- WA and N
  check("???", "???", "wa");
  // Nihon Shiki for the following rare kana are 'wi' and 'we' respectively, but
  // w??puro values are used instead (since 'wi' and 'we' are already used for
  // the more common diagraphs ?????? and ??????. Hepburn and Kunrei are both 'i'
  // and 'e' for these.
  check("???", "???", "wyi", "i", "i");
  check("???", "???", "wye", "e", "e");
  // both Hepburn and Kunrei use 'o' for ???, but program (and Nihon Shiki) uses
  // 'wo' for uniqueness
  check("???", "???", "wo", "o", "o");
  check("???", "???", "n");
  // -- GA
  check("???", "???", "ga");
  check("???", "???", "gi");
  check("???", "???", "gu");
  check("???", "???", "ge");
  check("???", "???", "go");
  check("??????", "??????", "gya");
  check("??????", "??????", "gyu");
  check("??????", "??????", "gyo");
  // -- ZA
  check("???", "???", "za");
  checkKunrei("???", "???", "ji", "zi");
  check("???", "???", "zu");
  check("???", "???", "ze");
  check("???", "???", "zo");
  checkKunrei("??????", "??????", "ja", "zya");
  checkKunrei("??????", "??????", "ju", "zyu");
  checkKunrei("??????", "??????", "jo", "zyo");
  // -- DA
  // Lots of differences for this group, for example the mapping for ??? in Nihon
  // Shiki style (and default for this program) is 'di', whereas Hepburn is 'ji'
  // and Kunrei is 'zi'.
  check("???", "???", "da");
  check("???", "???", "di", "ji", "zi");
  check("???", "???", "du", "zu", "zu");
  check("???", "???", "de");
  check("???", "???", "do");
  check("??????", "??????", "dya", "ja", "zya");
  check("??????", "??????", "dyu", "ju", "zyu");
  check("??????", "??????", "dyo", "jo", "zyo");
  // -- BA
  check("???", "???", "ba");
  check("???", "???", "bi");
  check("???", "???", "bu");
  check("???", "???", "be");
  check("???", "???", "bo");
  check("??????", "??????", "bya");
  check("??????", "??????", "byu");
  check("??????", "??????", "byo");
  // -- PA
  check("???", "???", "pa");
  check("???", "???", "pi");
  check("???", "???", "pu");
  check("???", "???", "pe");
  check("???", "???", "po");
  check("??????", "??????", "pya");
  check("??????", "??????", "pyu");
  check("??????", "??????", "pyo");
  // -- VU
  check("???", "???", "vu");
}

TEST_F(ConverterTest, CheckDelims) {
  using P = std::pair<char, const char*>;
  for (const auto& i : {P{' ', "???"}, P{'.', "???"}, P{',', "???"}, P{':', "???"},
           P{';', "???"}, P{'/', "???"}, P{'!', "???"}, P{'?', "???"}, P{'(', "???"},
           P{')', "???"}, P{'[', "???"}, P{']', "???"}, P{'*', "???"}, P{'~', "???"},
           P{'=', "???"}, P{'+', "???"}, P{'@', "???"}, P{'#', "???"}, P{'$', "???"},
           P{'%', "???"}, P{'^', "???"}, P{'&', "???"}, P{'{', "???"}, P{'}', "???"},
           P{'|', "???"}, P{'"', "???"}, P{'`', "???"}, P{'<', "???"}, P{'>', "???"},
           P{'_', "???"}, P{'\\', "???"}, P{'0', "???"}, P{'1', "???"},
           P{'2', "???"}, P{'3', "???"}, P{'4', "???"}, P{'5', "???"}, P{'6', "???"},
           P{'7', "???"}, P{'8', "???"}, P{'9', "???"}}) {
    const String romaji{i.first}, kana{i.second};
    check(kana, kana, romaji);
  }
}

} // namespace kanji_tools
