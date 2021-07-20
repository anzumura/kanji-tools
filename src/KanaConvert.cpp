#include <kanji/KanaConvert.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

#include <array>
#include <iostream>
#include <sstream>

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
  Kana{"va", "ゔぁ", "ヴァ"},
  Kana{"kya", "きゃ", "キャ"},
  Kana{"gya", "ぎゃ", "ギャ"},
  Kana{"kwa", "くぁ", "クァ"},
  Kana{"qa", "くぁ", "クァ", true},
  Kana{"qwa", "くゎ", "クヮ"},
  Kana{"sha", "しゃ", "シャ"},
  Kana{"sya", "しゃ", "シャ", true},
  Kana{"ja", "じゃ", "ジャ"},
  Kana{"jya", "じゃ", "ジャ", true},
  Kana{"zya", "じゃ", "ジャ", true},
  Kana{"swa", "すぁ", "スァ"},
  Kana{"cha", "ちゃ", "チャ"},
  Kana{"tya", "ちゃ", "チャ", true},
  Kana{"dya", "ぢゃ", "ヂャ"},
  Kana{"twa", "とぁ", "トァ"},
  Kana{"nya", "にゃ", "ニャ"},
  Kana{"hya", "ひゃ", "ヒャ"},
  Kana{"bya", "びゃ", "ビャ"},
  Kana{"pya", "ぴゃ", "ピャ"},
  Kana{"fa", "ふぁ", "ファ"},
  Kana{"fya", "ふゃ", "フャ"},
  Kana{"mya", "みゃ", "ミャ"},
  Kana{"rya", "りゃ", "リャ"},
  // い 行 Monographs
  Kana{"i", "い", "イ"},
  Kana{"ki", "き", "キ"},
  Kana{"gi", "ぎ", "ギ"},
  Kana{"shi", "し", "シ"},
  Kana{"si", "し", "シ", true}, 
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
  Kana{"zyi", "じぃ", "ジィ"},
  Kana{"thi", "てぃ", "ティ"},
  Kana{"twi", "とぃ", "トィ"},
  Kana{"tyi", "ちぃ", "チィ"},
  Kana{"nyi", "にぃ", "ニィ"},
  Kana{"hyi", "ひぃ", "ヒィ"},
  Kana{"byi", "びぃ", "ビィ"},
  Kana{"pyi", "ぴぃ", "ピィ"},
  Kana{"fi", "ふぃ", "フィ"},
  Kana{"fyi", "ふぃ", "フィ", true},
  Kana{"myi", "みぃ", "ミィ"},
  Kana{"ryi", "りぃ", "リィ"},
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
  Kana{"qu", "くぅ", "クゥ"},
  Kana{"shu", "しゅ", "シュ"},
  Kana{"syu", "しゅ", "シュ", true},
  Kana{"ju", "じゅ", "ジュ"},
  Kana{"jyu", "じゅ", "ジュ", true},
  Kana{"zyu", "じゅ", "ジュ", true},
  Kana{"chu", "ちゅ", "チュ"},
  Kana{"tyu", "ちゅ", "チュ", true},
  Kana{"dyu", "ぢゅ", "ヂュ"},
  Kana{"thu", "てゅ", "テュ"},
  Kana{"twu", "とぅ", "トゥ"},
  Kana{"nyu", "にゅ", "ニュ"},
  Kana{"hyu", "ひゅ", "ヒュ"},
  Kana{"byu", "びゅ", "ビュ"},
  Kana{"pyu", "ぴゅ", "ピュ"},
  Kana{"fyu", "ふゅ", "フュ"},
  Kana{"myu", "みゅ", "ミュ"},
  Kana{"ryu", "りゅ", "リュ"},
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
  Kana{"je", "じぇ", "ジェ"},
  Kana{"zye", "じぇ", "ジェ", true},
  Kana{"the", "てぇ", "テェ"},
  Kana{"twe", "とぇ", "トェ"},
  Kana{"nye", "にぇ", "ニェ"},
  Kana{"fe", "ふぇ", "フェ"},
  Kana{"mye", "みぇ", "ミェ"},
  Kana{"rye", "りぇ", "リェ"},
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
  Kana{"vo", "ゔぉ", "ヴォ"},
  Kana{"kyo", "きょ", "キョ"},
  Kana{"gyo", "ぎょ", "ギョ"},
  Kana{"qo", "くぉ", "クォ"},
  Kana{"sho", "しょ", "ショ"},
  Kana{"syo", "しょ", "ショ", true},
  Kana{"jo", "じょ", "ジョ"},
  Kana{"jyo", "じょ", "ジョ", true},
  Kana{"zyo", "じょ", "ジョ", true},
  Kana{"cho", "ちょ", "チョ"},
  Kana{"tyo", "ちょ", "チョ", true},
  Kana{"dyo", "ぢょ", "ヂョ"},
  Kana{"tho", "てょ", "テョ"},
  Kana{"two", "とぉ", "トォ"},
  Kana{"nyo", "にょ", "ニョ"},
  Kana{"hyo", "ひょ", "ヒョ"},
  Kana{"byo", "びょ", "ビョ"},
  Kana{"pyo", "ぴょ", "ピョ"},
  Kana{"fo", "ふぉ", "フォ"},
  Kana{"fyo", "ふょ", "フョ"},
  Kana{"myo", "みょ", "ミョ"},
  Kana{"ryo", "りょ", "リョ"},
  // ん - keep this entry at the end of the list
  Kana{"n", "ん", "ン"}
};
// clang-format on

std::ostream& operator<<(std::ostream& os, const Kana& k) {
  return os << '[' << k.romaji << (k.variant ? "*" : "") << ", " << k.hiragana << ", " << k.katakana << ']';
}

// Support converting some punctuation from narrow to wide values. These values are also used
// as delimiters for splitting up input strings when converting from Rõmaji to Kana.
constexpr std::array Delimiters = {std::make_pair(' ', "　"), std::make_pair('.', "。"), std::make_pair(',', "、"),
                                   std::make_pair(':', "："), std::make_pair(';', "；"), std::make_pair('/', "／"),
                                   std::make_pair('!', "！"), std::make_pair('?', "？"), std::make_pair('(', "（"),
                                   std::make_pair(')', "）"), std::make_pair('[', "「"), std::make_pair(']', "」")};

} // namespace

KanaConvert::Map KanaConvert::populate(KanaConvert::CharType t) {
  Map result;
  int duplicates = 0;
  auto insert = [&result, &duplicates, t](auto& k, auto& v) {
    auto i = result.insert(std::make_pair(k, v));
    if (!i.second) {
      std::cerr << "key '" << k << "' already in " << toString(t) << " map: " << i.first->second << '\n';
      ++duplicates;
    }
  };
  static bool firstTime = true;
  if (firstTime) {
    for (auto& i : KanaList) {
      assert(!i.romaji.empty() && i.romaji.length() < 4);           // must be 1 to 3 chars
      assert(i.hiragana.length() == 3 || i.hiragana.length() == 6); // 3 bytes per character
      assert(i.katakana.length() == 3 || i.katakana.length() == 6); // 3 bytes per characer
      assert(isAllSingleByte(i.romaji));
      assert(isAllHiragana(i.hiragana));
      assert(isAllKatakana(i.katakana));
    }
    firstTime = false;
  }
  for (auto& i : KanaList) {
    switch (t) {
    case CharType::Romaji: insert(i.romaji, i); break;
    case CharType::Hiragana:
      if (!i.variant) insert(i.hiragana, i);
      break;
    case CharType::Katakana:
      if (!i.variant) insert(i.katakana, i);
      break;
    }
  }
  assert(duplicates == 0);
  return result;
}

KanaConvert::KanaConvert()
  : _romajiMap(populate(CharType::Romaji)), _hiraganaMap(populate(CharType::Hiragana)),
    _katakanaMap(populate(CharType::Katakana)), _n(KanaList[KanaList.size() - 1]) {
  assert(_n.romaji == "n");
  for (auto& i : Delimiters) {
    _narrowDelims += i.first;
    _narrowToWideDelims[i.first] = i.second;
    _wideToNarrowDelims[i.second] = i.first;
  }
  _narrowDelims += _apostrophe;
  _narrowDelims += _dash;
}

std::string KanaConvert::convert(const std::string& input, CharType target, bool keepSpaces) const { return input; }

std::string KanaConvert::convert(const std::string& input, CharType source, CharType target, bool keepSpaces) const {
  if (source == target) return input;
  std::string result, line;
  std::istringstream is(input);
  if (source == CharType::Romaji) {
    size_t oldPos = 0;
    do {
      size_t pos = input.find_first_of(_narrowDelims, oldPos);
      if (pos != std::string::npos) {
        result += convertFromRomaji(input.substr(oldPos, pos - oldPos), target);
        const char delim = input[pos];
        if (delim != _apostrophe && delim != _dash && (keepSpaces || delim != ' '))
          result += _narrowToWideDelims.at(delim);
        oldPos = pos + 1;
      } else {
        result += convertFromRomaji(input.substr(oldPos), target);
        break;
      }
    } while (true);
  }
  return result;
}

std::string KanaConvert::convertFromRomaji(const std::string& input, CharType target) const {
  std::string result, letterGroup, c;
  auto macron = [this, &letterGroup, &result, target](char x, const auto& s) {
    letterGroup += x;
    convertRomajiLetters(letterGroup, result, target);
    if (letterGroup.empty())
      result += target == CharType::Hiragana ? s : "ー";
    else
      result += x; // should never happen ...
  };
  MBChar s(input);
  while (s.next(c, false)) {
    if (c == "ā")
      macron('a', "あ");
    else if (c == "ī")
      macron('i', "い");
    else if (c == "ū")
      macron('u', "う");
    else if (c == "ē")
      macron('e', "え");
    else if (c == "ō")
      macron('o', "お");
    else if (isSingleByte(c)) {
      char letter = std::tolower(c[0]);
      if (letter != 'n') {
        letterGroup += letter;
        convertRomajiLetters(letterGroup, result, target);
      } else {
        if (letterGroup.empty())
          letterGroup += letter;
        else if (letterGroup == "n") // got two 'n's in a row so output one, but don't need to clear letterGroup
          result += _n.get(target);
        else {
          // error: partial romaji followed by n - output uncoverted partial group
          result += letterGroup;
          letterGroup = c; // 'n' starts a new group
        }
      }
    } else
      result += c;
  }
  if (letterGroup == "n") result += _n.get(target);
  return result;
}

void KanaConvert::convertRomajiLetters(std::string& letterGroup, std::string& result, CharType target) const {
  auto i = _romajiMap.find(letterGroup);
  if (i != _romajiMap.end()) {
    result += i->second.get(target);
    letterGroup.clear();
  } else if (letterGroup.length() == 3) {
    if (letterGroup[0] == 'n')
      result += _n.get(target);
    else if (letterGroup[0] == letterGroup[1]) {
      // convert first letter to small tsu if letter repeats and is a valid consonant
      switch (letterGroup[0]) {
      case 'b':
      case 'c':
      case 'd':
      case 'f':
      case 'g':
      case 'j':
      case 'k':
      case 'm':
      case 'p':
      case 'q':
      case 'r':
      case 's':
      case 't':
      case 'w':
      case 'y':
      case 'z': result += target == CharType::Hiragana ? "っ" : "ッ"; break;
      default: result += letterGroup[0]; // error: first letter not valid
      }
    } else // error: no romaji is longer than 3 chars so output the first letter unconverted
      result += letterGroup[0];
    letterGroup = letterGroup.substr(1);
    convertRomajiLetters(letterGroup, result, target); // try converting the shortened letterGroup
  }
}

} // namespace kanji
