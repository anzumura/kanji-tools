#include <kanji_tools/kana/Kana.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <iostream>

namespace kanji_tools {

namespace {

using K = Kana;
using V = Kana::List;

// 'KanaList' has mappings for all monographs (single kana) with no 'dakuten' or
// 'han-dakuten' versions and regularly used digraphs (normal kana followed by a
// small kana 'vowel', 'y' or 'wa'). See comments for 'Kana' class for a
// description of the fields.
const std::array KanaList{
  // --- あ 行 ---
  K{"a", "あ", "ア"}, K{"na", "な", "ナ"}, K{"ma", "ま", "マ"},
  K{"ya", "や", "ヤ"}, K{"ra", "ら", "ラ"}, K{"wa", "わ", "ワ"},
  // Digraphs
  K{"qwa", "くゎ", "クヮ"}, K{"swa", "すぁ", "スァ"}, K{"tsa", "つぁ", "ツァ"},
  K{"nya", "にゃ", "ニャ"}, K{"fa", "ふぁ", "ファ", V{"fwa", "hwa"}},
  K{"fya", "ふゃ", "フャ"}, K{"mya", "みゃ", "ミャ"}, K{"rya", "りゃ", "リャ"},
  // --- い 行 ---
  K{"i", "い", "イ"}, K{"ni", "に", "ニ"}, K{"mi", "み", "ミ"},
  K{"ri", "り", "リ"}, K{"wyi", "ゐ", "ヰ", "i", "i"},
  // Digraphs
  K{"swi", "すぃ", "スィ"}, K{"tsi", "つぃ", "ツィ"}, K{"nyi", "にぃ", "ニィ"},
  K{"fi", "ふぃ", "フィ", V{"fyi", "fwi", "hwi"}}, K{"myi", "みぃ", "ミィ"},
  K{"ryi", "りぃ", "リィ"},
  // --- う 行 ---
  K{"nu", "ぬ", "ヌ"}, K{"mu", "む", "ム"}, K{"yu", "ゆ", "ユ"},
  K{"ru", "る", "ル"},
  // Digraphs
  K{"swu", "すぅ", "スゥ"}, K{"nyu", "にゅ", "ニュ"}, K{"fwu", "ふぅ", "フゥ"},
  K{"fyu", "ふゅ", "フュ"}, K{"myu", "みゅ", "ミュ"}, K{"ryu", "りゅ", "リュ"},
  // --- え 行 ---
  K{"e", "え", "エ"}, K{"ne", "ね", "ネ"}, K{"me", "め", "メ"},
  K{"re", "れ", "レ"}, K{"wye", "ゑ", "ヱ", "e", "e"},
  // Digraphs
  K{"ye", "いぇ", "イェ"}, K{"swe", "すぇ", "スェ"}, K{"tse", "つぇ", "ツェ"},
  K{"nye", "にぇ", "ニェ"}, K{"fe", "ふぇ", "フェ", V{"fye", "fwe", "hwe"}},
  K{"mye", "みぇ", "ミェ"}, K{"rye", "りぇ", "リェ"},
  // --- お 行 ---
  K{"o", "お", "オ"}, K{"no", "の", "ノ"}, K{"mo", "も", "モ"},
  K{"yo", "よ", "ヨ"}, K{"ro", "ろ", "ロ"}, K{"wo", "を", "ヲ", "o", "o"},
  // Digraphs
  K{"swo", "すぉ", "スォ"}, K{"tso", "つぉ", "ツォ"}, K{"nyo", "にょ", "ニョ"},
  K{"fo", "ふぉ", "フォ", V{"fwo", "hwo"}}, K{"fyo", "ふょ", "フョ"},
  K{"myo", "みょ", "ミョ"}, K{"ryo", "りょ", "リョ"},
  // Digraphs that only have a dakuten version
  K{"va", "ゔぁ", "ヴァ"}, K{"vo", "ゔぉ", "ヴォ"}, K{"vya", "ゔゃ", "ヴャ"},
  K{"vyu", "ゔゅ", "ヴュ"}, K{"vyo", "ゔょ", "ヴョ"},
  // 12 Small letters (5 vowels, 2 k's, 3 y's, small 'wa' and small 'tsu') -
  // prefer 'l' versions for Romaji output
  K{"la", "ぁ", "ァ", V{"xa"}}, K{"li", "ぃ", "ィ", V{"xi"}},
  K{"lu", "ぅ", "ゥ", V{"xu"}}, K{"le", "ぇ", "ェ", V{"xe", "lye", "xye"}},
  K{"lo", "ぉ", "ォ", V{"xo"}}, K{"lka", "ゕ", "ヵ", V{"xka"}},
  K{"lke", "ゖ", "ヶ", V{"xke"}}, K{"lya", "ゃ", "ャ", V{"xya"}},
  K{"lyu", "ゅ", "ュ", V{"xyu"}}, K{"lyo", "ょ", "ョ", V{"xyo"}},
  K{"lwa", "ゎ", "ヮ", V{"xwa"}}, K{"ltu", "っ", "ッ", V{"xtu"}},
  // ん - keep 'n' as well as the previous small 'tsu' at the end of the list
  K{"n", "ん", "ン"}};

using D = DakutenKana;
// 'DakutenKanaList' contains kana that have a 'dakuten' version, but not 'h'
std::array DakutenKanaList = {
  // --- あ 行 ---
  D{"ka", "か", "カ", K{"ga", "が", "ガ"}},
  D{"sa", "さ", "サ", K{"za", "ざ", "ザ"}},
  D{"ta", "た", "タ", K{"da", "だ", "ダ"}},
  D{"kya", "きゃ", "キャ", K{"gya", "ぎゃ", "ギャ"}},
  // Diagraphs
  D{"qa", "くぁ", "クァ", K{"gwa", "ぐぁ", "グァ"}, V{"kwa"}},
  D{"sha", "しゃ", "シャ", K{"ja", "じゃ", "ジャ", V{"zya", "jya"}, true},
    V{"sya"}, true},
  D{"cha", "ちゃ", "チャ", K{"dya", "ぢゃ", "ヂャ", "ja", "zya"}, V{"tya"},
    true},
  D{"tha", "てゃ", "テャ", K{"dha", "でゃ", "デャ"}},
  D{"twa", "とぁ", "トァ", K{"dwa", "どぁ", "ドァ"}},
  // --- い 行 ---
  D{"ki", "き", "キ", K{"gi", "ぎ", "ギ"}},
  D{"shi", "し", "シ", K{"ji", "じ", "ジ", V{"zi"}, true}, V{"si"}, true},
  D{"chi", "ち", "チ", K{"di", "ぢ", "ヂ", "ji", "zi"}, V{"ti"}, true},
  // Digraphs
  D{"wi", "うぃ", "ウィ", K{"vi", "ゔぃ", "ヴィ"}},
  D{"qi", "くぃ", "クィ", K{"gwi", "ぐぃ", "グィ"}, V{"kwi", "qwi"}},
  D{"kyi", "きぃ", "キィ", K{"gyi", "ぎぃ", "ギィ"}},
  D{"syi", "しぃ", "シィ", K{"jyi", "じぃ", "ジィ", V{"zyi"}}},
  D{"tyi", "ちぃ", "チィ", K{"dyi", "ぢぃ", "ヂィ"}},
  D{"twi", "とぃ", "トィ", K{"dwi", "どぃ", "ドィ"}},
  D{"thi", "てぃ", "ティ", K{"dhi", "でぃ", "ディ"}},
  // --- う 行 ---
  D{"u", "う", "ウ", K{"vu", "ゔ", "ヴ"}, V{"wu"}},
  D{"ku", "く", "ク", K{"gu", "ぐ", "グ"}},
  D{"su", "す", "ス", K{"zu", "ず", "ズ"}},
  D{"tsu", "つ", "ツ", K{"du", "づ", "ヅ", "zu", "zu"}, V{"tu"}, true},
  // Digraphs
  D{"kyu", "きゅ", "キュ", K{"gyu", "ぎゅ", "ギュ"}},
  D{"qu", "くぅ", "クゥ", K{"gwu", "ぐぅ", "グゥ"}, V{"kwu", "qwu"}},
  D{"shu", "しゅ", "シュ", K{"ju", "じゅ", "ジュ", V{"zyu", "jyu"}, true},
    V{"syu"}, true},
  D{"chu", "ちゅ", "チュ", K{"dyu", "ぢゅ", "ヂュ", "ju", "zyu"}, V{"tyu"},
    true},
  D{"thu", "てゅ", "テュ", K{"dhu", "でゅ", "デュ"}},
  D{"twu", "とぅ", "トゥ", K{"dwu", "どぅ", "ドゥ"}},
  // --- え 行 ---
  D{"ke", "け", "ケ", K{"ge", "げ", "ゲ"}},
  D{"kye", "きぇ", "キェ", K{"gye", "ぎぇ", "ギェ"}},
  D{"se", "せ", "セ", K{"ze", "ぜ", "ゼ"}},
  D{"te", "て", "テ", K{"de", "で", "デ"}},
  // Digraphs
  D{"we", "うぇ", "ウェ", K{"ve", "ゔぇ", "ヴェ"}},
  D{"qe", "くぇ", "クェ", K{"gwe", "ぐぇ", "グェ"}, V{"kwe", "qwe"}},
  D{"she", "しぇ", "シェ", K{"je", "じぇ", "ジェ", V{"zye", "jye"}}},
  D{"che", "ちぇ", "チェ", K{"dye", "ぢぇ", "ヂェ"}, V{"tye"}},
  D{"the", "てぇ", "テェ", K{"dhe", "でぇ", "デェ"}},
  D{"twe", "とぇ", "トェ", K{"dwe", "どぇ", "ドェ"}},
  // --- お 行 ---
  D{"ko", "こ", "コ", K{"go", "ご", "ゴ"}},
  D{"so", "そ", "ソ", K{"zo", "ぞ", "ゾ"}},
  D{"to", "と", "ト", K{"do", "ど", "ド"}},
  // Digraphs
  D{"kyo", "きょ", "キョ", K{"gyo", "ぎょ", "ギョ"}},
  D{"qo", "くぉ", "クォ", K{"gwo", "ぐぉ", "グォ"}, V{"kwo", "qwo"}},
  D{"sho", "しょ", "ショ", K{"jo", "じょ", "ジョ", V{"zyo", "jyo"}, true},
    V{"syo"}, true},
  D{"cho", "ちょ", "チョ", K{"dyo", "ぢょ", "ヂョ", "jo", "zyo"}, V{"tyo"},
    true},
  D{"tho", "てょ", "テョ", K{"dho", "でょ", "デョ"}},
  D{"two", "とぉ", "トォ", K{"dwo", "どぉ", "ドォ"}}};

using H = HanDakutenKana;
// 'HanDakutenKanaList' contains kana that have both a 'dakuten' and a
// 'han-dakuten' (so 'h' row)
std::array HanDakutenKanaList = {
  H{"ha", "は", "ハ", K{"ba", "ば", "バ"}, K{"pa", "ぱ", "パ"}},
  H{"hi", "ひ", "ヒ", K{"bi", "び", "ビ"}, K{"pi", "ぴ", "ピ"}},
  H{"fu", "ふ", "フ", K{"bu", "ぶ", "ブ"}, K{"pu", "ぷ", "プ"}, V{"hu"}, true},
  H{"he", "へ", "ヘ", K{"be", "べ", "ベ"}, K{"pe", "ぺ", "ペ"}},
  H{"ho", "ほ", "ホ", K{"bo", "ぼ", "ボ"}, K{"po", "ぽ", "ポ"}},
  H{"hya", "ひゃ", "ヒャ", K{"bya", "びゃ", "ビャ"}, K{"pya", "ぴゃ", "ピャ"}},
  H{"hyi", "ひぃ", "ヒィ", K{"byi", "びぃ", "ビィ"}, K{"pyi", "ぴぃ", "ピィ"}},
  H{"hyu", "ひゅ", "ヒュ", K{"byu", "びゅ", "ビュ"}, K{"pyu", "ぴゅ", "ピュ"}},
  H{"hye", "ひぇ", "ヒェ", K{"bye", "びぇ", "ビェ"}, K{"pye", "ぴぇ", "ピェ"}},
  H{"hyo", "ひょ", "ヒョ", K{"byo", "びょ", "ビョ"}, K{"pyo", "ぴょ", "ピョ"}}};

} // namespace

const std::string& Kana::getRomaji(ConvertFlags flags) const {
  return hasValue(flags & ConvertFlags::Hepburn) && _hepburn ? *_hepburn
         : hasValue(flags & ConvertFlags::Kunrei) && _kunreiVariant
           ? _romajiVariants[0]
         : hasValue(flags & ConvertFlags::Kunrei) && _kunrei ? *_kunrei
                                                             : _romaji;
}

const std::string& Kana::get(CharType t, ConvertFlags flags) const {
  switch (t) {
  case CharType::Romaji: return getRomaji(flags);
  case CharType::Hiragana: return _hiragana;
  case CharType::Katakana: return _katakana;
  }
  __builtin_unreachable(); // prevent gcc 'control reaches end ...' warning¥
}

void Kana::validate() const {
  for ([[maybe_unused]] auto& i : _romajiVariants)
    assert(!i.empty() && i.size() < 4);           // must be 1 to 3 chars
  assert(!_romaji.empty() && _romaji.size() < 4); // must be 1 to 3 chars
  assert(_hiragana.size() == 3 || _hiragana.size() == 6);
  assert(_katakana.size() == 3 || _katakana.size() == 6);
  assert(isAllSingleByte(_romaji));
  assert(isAllHiragana(_hiragana));
  assert(isAllKatakana(_katakana));
}

Kana::Map Kana::populate(CharType t) {
  Kana::Map result;
  size_t duplicates{};
  const auto insert = [&result, &duplicates, t](auto& k, auto& v) {
    if (const auto i = result.emplace(k, &v); !i.second) {
      std::cerr << "key '" << k << "' already in " << toString(t)
                << " map: " << i.first->second << '\n';
      ++duplicates;
    }
  };
  const auto processKana = [&insert, t](auto& k) {
    switch (t) {
    case CharType::Romaji:
      insert(k.romaji(), k);
      for (auto& i : k.romajiVariants()) insert(i, k);
      break;
    case CharType::Hiragana: insert(k.hiragana(), k); break;
    case CharType::Katakana: insert(k.katakana(), k); break;
    }
  };
  // process lists (inserting into 'result')
  for (auto& i : KanaList) processKana(i);
  for (auto& i : DakutenKanaList) {
    processKana(i);
    processKana(*i.dakutenKana());
  }
  for (auto& i : HanDakutenKanaList) {
    processKana(i);
    processKana(*i.dakutenKana());
    processKana(*i.hanDakutenKana());
  }
  assert(duplicates == 0);
  return result;
}

std::string Kana::RepeatMark::get(CharType target, ConvertFlags flags,
                                  const Kana* prevKana) const {
  switch (target) {
  case CharType::Hiragana: return _hiragana;
  case CharType::Katakana: return _katakana;
  case CharType::Romaji: break;
  }
  if (!prevKana) return "";
  const Kana* k = prevKana;
  if (_dakuten) {
    if (const auto accented = prevKana->dakutenKana(); accented) k = accented;
  } else if (const auto unaccented = prevKana->plainKana(); unaccented)
    k = unaccented;
  return k->getRomaji(flags);
}

const Kana::Map Kana::_romajiMap(Kana::populate(CharType::Romaji));
const Kana::Map Kana::_hiraganaMap(Kana::populate(CharType::Hiragana));
const Kana::Map Kana::_katakanaMap(Kana::populate(CharType::Katakana));

const Kana& Kana::SmallTsu(KanaList[KanaList.size() - 2]);
const Kana& Kana::N(KanaList[KanaList.size() - 1]);

} // namespace kanji_tools
