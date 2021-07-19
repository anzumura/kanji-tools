#include <kanji/KanaConvert.h>

#include <array>
#include <iostream>

namespace kanji {

namespace {

using Kana = KanaConvert::Kana;
// clang-format off

// 'KanaList' has the standard mappings for kana. Non-standard mappings are handled separately
// such as repeat symbols, small 'tsu' and rare kana like ( ゐ, ゑ, ヰ, and ヱ)
const std::array KanaList = {
  // あ 行 Monographs
  Kana{"a", "あ", "ア"},
  Kana{"ka", "か", "カ"},
  Kana{"ga", "が", "ガ"},
  Kana{"sa", "さ", "サ"},
  Kana{"za", "ざ", "ザ"},
  Kana{"ta", "た", "タ"},
  Kana{"da", "だ", "ダ"},
  Kana{"na", "な", "ナ"},
  Kana{"ha", "は", "ハ"},
  Kana{"ba", "ば", "バ"},
  Kana{"pa", "ぱ", "パ"},
  Kana{"ma", "ま", "マ"},
  Kana{"ya", "や", "ヤ"},
  Kana{"ra", "ら", "ラ"},
  Kana{"wa", "わ", "ワ"},
  // Digraphs
  Kana{"kya", "きゃ", "キャ"},
  Kana{"gya", "ぎゃ", "ギャ"},
  Kana{"ja", "じゃ", "ジャ"},
  Kana{"jya", "じゃ", "ジャ", true},
  Kana{"zya", "じゃ", "ジャ", true},
  Kana{"cha", "ちゃ", "チャ"},
  Kana{"tya", "ちゃ", "チャ", true},
  Kana{"twa", "とぁ", "トァ"},
  Kana{"dya", "ぢゃ", "ヂャ"},
  Kana{"hya", "ひゃ", "ヒャ"},
  Kana{"bya", "びゃ", "ビャ"},
  Kana{"pya", "ぴゃ", "ピャ"},
  Kana{"rya", "りゃ", "リャ"},
  Kana{"va", "ゔぁ", "ヴァ"},
  Kana{"qa", "くぁ", "クァ"},
  Kana{"fa", "ふぁ", "ファ"},
  Kana{"fya", "ふゃ", "フャ"},
  // い 行 Monographs
  Kana{"i", "い", "イ"},
  Kana{"ki", "き", "キ"},
  Kana{"gi", "ぎ", "ギ"},
  Kana{"shi", "し", "シ"},
  Kana{"ji", "じ", "ジ"},
  Kana{"zi", "じ", "ジ", true},
  Kana{"chi", "ち", "チ"},
  Kana{"di", "ぢ", "ヂ"},
  Kana{"ti", "ち", "チ", true},
  Kana{"ni", "に", "ニ"},
  Kana{"hi", "ひ", "ヒ"},
  Kana{"bi", "び", "ビ"},
  Kana{"pi", "ぴ", "ピ"},
  Kana{"mi", "み", "ミ"},
  Kana{"ri", "り", "リ"},
  // Digraphs
  Kana{"wi", "うぃ", "ウィ"},
  Kana{"vi", "ゔぃ", "ヴィ"},
  Kana{"kyi", "きぃ", "キィ"},
  Kana{"qi", "くぃ", "クィ"},
  Kana{"thi", "てぃ", "ティ"},
  Kana{"twi", "とぃ", "トィ"},
  Kana{"tyi", "ちぃ", "チィ"},
  Kana{"fi", "ふぃ", "フィ"},
  // う 行 Monographs
  Kana{"u", "う", "ウ"},
  Kana{"vu", "ゔ", "ヴ"},
  Kana{"ku", "く", "ク"},
  Kana{"gu", "ぐ", "グ"},
  Kana{"su", "す", "ス"},
  Kana{"zu", "ず", "ズ"},
  Kana{"tsu", "つ", "ツ"},
  Kana{"tu", "つ", "ツ", true},
  Kana{"du", "づ", "ヅ"},
  Kana{"nu", "ぬ", "ヌ"},
  Kana{"fu", "ふ", "フ"},
  Kana{"hu", "ふ", "フ", true},
  Kana{"bu", "ぶ", "ブ"},
  Kana{"pu", "ぷ", "プ"},
  Kana{"mu", "む", "ム"},
  Kana{"yu", "ゆ", "ユ"},
  Kana{"ru", "る", "ル"},
  // Digraphs
  Kana{"kyu", "きゅ", "キュ"},
  Kana{"gyu", "ぎゅ", "ギュ"},
  Kana{"ju", "じゅ", "ジュ"},
  Kana{"jyu", "じゅ", "ジュ", true},
  Kana{"zyu", "じゅ", "ジュ", true},
  Kana{"chu", "ちゅ", "チュ"},
  Kana{"tyu", "ちゅ", "チュ", true},
  Kana{"dyu", "ぢゅ", "ヂュ"},
  Kana{"thu", "てゅ", "テュ"},
  Kana{"twu", "とぅ", "トゥ"},
  Kana{"hyu", "ひゅ", "ヒュ"},
  Kana{"byu", "びゅ", "ビュ"},
  Kana{"pyu", "ぴゅ", "ピュ"},
  Kana{"ryu", "りゅ", "リュ"},
  Kana{"qu", "くぅ", "クゥ"},
  Kana{"fyu", "ふゅ", "フュ"},
  // え 行 Monographs
  Kana{"e", "え", "エ"},
  Kana{"ke", "け", "ケ"},
  Kana{"ge", "げ", "ゲ"},
  Kana{"se", "せ", "セ"},
  Kana{"ze", "ぜ", "ゼ"},
  Kana{"te", "て", "テ"},
  Kana{"de", "で", "デ"},
  Kana{"ne", "ね", "ネ"},
  Kana{"he", "へ", "ヘ"},
  Kana{"be", "べ", "ベ"},
  Kana{"pe", "ぺ", "ペ"},
  Kana{"me", "め", "メ"},
  Kana{"re", "れ", "レ"},
  // Digraphs
  Kana{"ye", "いぇ", "イェ"},
  Kana{"we", "うぇ", "ウェ"},
  Kana{"ve", "ゔぇ", "ヴェ"},
  Kana{"kye", "きぇ", "キェ"},
  Kana{"qe", "くぇ", "クェ"},
  Kana{"the", "てぇ", "テェ"},
  Kana{"twe", "とぇ", "トェ"},
  Kana{"fe", "ふぇ", "フェ"},
  // お 行 Monographs
  Kana{"o", "お", "オ"},
  Kana{"ko", "こ", "コ"},
  Kana{"go", "ご", "ゴ"},
  Kana{"so", "そ", "ソ"},
  Kana{"zo", "ぞ", "ゾ"},
  Kana{"to", "と", "ト"},
  Kana{"do", "ど", "ド"},
  Kana{"no", "の", "ノ"},
  Kana{"ho", "ほ", "ホ"},
  Kana{"bo", "ぼ", "ボ"},
  Kana{"po", "ぽ", "ポ"},
  Kana{"mo", "も", "モ"},
  Kana{"yo", "よ", "ヨ"},
  Kana{"ro", "ろ", "ロ"},
  Kana{"wo", "を", "ヲ"},
  // Digraphs
  Kana{"kyo", "きょ", "キョ"},
  Kana{"gyo", "ぎょ", "ギョ"},
  Kana{"jo", "じょ", "ジョ"},
  Kana{"jyo", "じょ", "ジョ", true},
  Kana{"zyo", "じょ", "ジョ", true},
  Kana{"cho", "ちょ", "チョ"},
  Kana{"tyo", "ちょ", "チョ", true},
  Kana{"dyo", "ぢょ", "ヂョ"},
  Kana{"tho", "てょ", "テョ"},
  Kana{"two", "とぉ", "トォ"},
  Kana{"hyo", "ひょ", "ヒョ"},
  Kana{"byo", "びょ", "ビョ"},
  Kana{"pyo", "ぴょ", "ピョ"},
  Kana{"ryo", "りょ", "リョ"},
  Kana{"vo", "ゔぉ", "ヴォ"},
  Kana{"qo", "くぉ", "クォ"},
  Kana{"fo", "ふぉ", "フォ"},
  Kana{"fyo", "ふょ", "フョ"},
  // ん
  Kana{"n", "ん", "ン"}
};
// clang-format on

std::ostream& operator<<(std::ostream& os, const Kana& k) {
  return os << '[' << k.romaji << (k.variant ? "*" : "") << ", " << k.hiragana << ", " << k.katakana << ']';
}

} // namespace

KanaConvert::Map KanaConvert::populate(KanaConvert::Target t) {
  Map result;
  int duplicates = 0;
  auto insert = [&result, &duplicates, t](auto& k, auto& v) {
    auto i = result.insert(std::make_pair(k, v));
    if (!i.second) {
      std::cerr << "key '" << k << "' already in " << toString(t) << " map: " << i.first->second << '\n';
      ++duplicates;
    }
  };
  for (auto& i : KanaList) {
    switch (t) {
    case Target::Romaji: insert(i.romaji, i); break;
    case Target::Hiragana:
      if (!i.variant) insert(i.hiragana, i);
      break;
    case Target::Katakana:
      if (!i.variant) insert(i.katakana, i);
      break;
    }
  }
  assert(duplicates == 0);
  return result;
}

KanaConvert::KanaConvert()
  : _romajiMap(populate(Target::Romaji)), _hiraganaMap(populate(Target::Hiragana)),
    _katakanaMap(populate(Target::Katakana)) {}

} // namespace kanji
