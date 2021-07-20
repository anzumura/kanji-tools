#include <kanji/KanaConvert.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

#include <array>
#include <iostream>
#include <sstream>

namespace kanji {

namespace {

using K = KanaConvert::Kana;
using V = KanaConvert::VariantKana;

// 'KanaList' has the standard mappings for kana. Non-standard mappings are handled separately
// such as repeat symbols, small 'tsu' and rare kana like ( ゐ, ゑ, ヰ, and ヱ)
const std::array KanaList = {
  // --- あ 行 ---
  K{"a", "あ", "ア"}, K{"ka", "か", "カ"}, K{"ga", "が", "ガ"}, K{"sa", "さ", "サ"}, K{"za", "ざ", "ザ"},
  K{"ta", "た", "タ"}, K{"da", "だ", "ダ"}, K{"na", "な", "ナ"}, K{"ha", "は", "ハ"}, K{"ba", "ば", "バ"},
  K{"pa", "ぱ", "パ"}, K{"ma", "ま", "マ"}, K{"ya", "や", "ヤ"}, K{"ra", "ら", "ラ"}, K{"wa", "わ", "ワ"},
  // Small letters - prefer 'l' versions for Romaji output
  K{"la", "ぁ", "ァ"}, K{"lya", "ゃ", "ャ"}, K{"lwa", "ゎ", "ヮ"},
  // Digraphs
  K{"va", "ゔぁ", "ヴァ"}, K{"kya", "きゃ", "キャ"}, K{"gya", "ぎゃ", "ギャ"}, K{"qa", "くぁ", "クァ"},
  K{"qwa", "くゎ", "クヮ"}, K{"sha", "しゃ", "シャ"}, K{"ja", "じゃ", "ジャ"}, K{"swa", "すぁ", "スァ"},
  K{"cha", "ちゃ", "チャ"}, K{"dya", "ぢゃ", "ヂャ"}, K{"twa", "とぁ", "トァ"}, K{"nya", "にゃ", "ニャ"},
  K{"hya", "ひゃ", "ヒャ"}, K{"bya", "びゃ", "ビャ"}, K{"pya", "ぴゃ", "ピャ"}, K{"fa", "ふぁ", "ファ"},
  K{"fya", "ふゃ", "フャ"}, K{"mya", "みゃ", "ミャ"}, K{"rya", "りゃ", "リャ"},
  // --- い 行 ---
  K{"i", "い", "イ"}, K{"ki", "き", "キ"}, K{"gi", "ぎ", "ギ"}, K{"shi", "し", "シ"}, K{"ji", "じ", "ジ"},
  K{"chi", "ち", "チ"}, K{"di", "ぢ", "ヂ"}, K{"ni", "に", "ニ"}, K{"hi", "ひ", "ヒ"}, K{"bi", "び", "ビ"},
  K{"pi", "ぴ", "ピ"}, K{"mi", "み", "ミ"}, K{"ri", "り", "リ"},
  // Small letters - prefer 'l' versions for Romaji output
  K{"li", "ぃ", "ィ"},
  // Digraphs
  K{"wi", "うぃ", "ウィ"}, K{"vi", "ゔぃ", "ヴィ"}, K{"kyi", "きぃ", "キィ"}, K{"qi", "くぃ", "クィ"},
  K{"zyi", "じぃ", "ジィ"}, K{"thi", "てぃ", "ティ"}, K{"twi", "とぃ", "トィ"}, K{"tyi", "ちぃ", "チィ"},
  K{"nyi", "にぃ", "ニィ"}, K{"hyi", "ひぃ", "ヒィ"}, K{"byi", "びぃ", "ビィ"}, K{"pyi", "ぴぃ", "ピィ"},
  K{"fi", "ふぃ", "フィ"}, K{"myi", "みぃ", "ミィ"}, K{"ryi", "りぃ", "リィ"},
  // --- う 行 ---
  K{"u", "う", "ウ"}, K{"vu", "ゔ", "ヴ"}, K{"ku", "く", "ク"}, K{"gu", "ぐ", "グ"}, K{"su", "す", "ス"},
  K{"zu", "ず", "ズ"}, K{"tsu", "つ", "ツ"}, K{"du", "づ", "ヅ"}, K{"nu", "ぬ", "ヌ"}, K{"fu", "ふ", "フ"},
  K{"bu", "ぶ", "ブ"}, K{"pu", "ぷ", "プ"}, K{"mu", "む", "ム"}, K{"yu", "ゆ", "ユ"}, K{"ru", "る", "ル"},
  // Small letters - prefer 'l' versions for Romaji output
  K{"lu", "ぅ", "ゥ"}, K{"lyu", "ゅ", "ュ"},
  // Digraphs
  K{"kyu", "きゅ", "キュ"}, K{"gyu", "ぎゅ", "ギュ"}, K{"qu", "くぅ", "クゥ"}, K{"shu", "しゅ", "シュ"},
  K{"ju", "じゅ", "ジュ"}, K{"chu", "ちゅ", "チュ"}, K{"dyu", "ぢゅ", "ヂュ"}, K{"thu", "てゅ", "テュ"},
  K{"twu", "とぅ", "トゥ"}, K{"nyu", "にゅ", "ニュ"}, K{"hyu", "ひゅ", "ヒュ"}, K{"byu", "びゅ", "ビュ"},
  K{"pyu", "ぴゅ", "ピュ"}, K{"fyu", "ふゅ", "フュ"}, K{"myu", "みゅ", "ミュ"}, K{"ryu", "りゅ", "リュ"},
  // --- え 行 ---
  K{"e", "え", "エ"}, K{"ke", "け", "ケ"}, K{"ge", "げ", "ゲ"}, K{"se", "せ", "セ"}, K{"ze", "ぜ", "ゼ"},
  K{"te", "て", "テ"}, K{"de", "で", "デ"}, K{"ne", "ね", "ネ"}, K{"he", "へ", "ヘ"}, K{"be", "べ", "ベ"},
  K{"pe", "ぺ", "ペ"}, K{"me", "め", "メ"}, K{"re", "れ", "レ"},
  // Small letters - prefer 'l' versions for Romaji output
  K{"le", "ぇ", "ェ"},
  // Digraphs
  K{"ye", "いぇ", "イェ"}, K{"we", "うぇ", "ウェ"}, K{"ve", "ゔぇ", "ヴェ"}, K{"kye", "きぇ", "キェ"},
  K{"qe", "くぇ", "クェ"}, K{"je", "じぇ", "ジェ"}, K{"the", "てぇ", "テェ"}, K{"twe", "とぇ", "トェ"},
  K{"nye", "にぇ", "ニェ"}, K{"fe", "ふぇ", "フェ"}, K{"mye", "みぇ", "ミェ"}, K{"rye", "りぇ", "リェ"},
  // --- お 行 ---
  K{"o", "お", "オ"}, K{"ko", "こ", "コ"}, K{"go", "ご", "ゴ"}, K{"so", "そ", "ソ"}, K{"zo", "ぞ", "ゾ"},
  K{"to", "と", "ト"}, K{"do", "ど", "ド"}, K{"no", "の", "ノ"}, K{"ho", "ほ", "ホ"}, K{"bo", "ぼ", "ボ"},
  K{"po", "ぽ", "ポ"}, K{"mo", "も", "モ"}, K{"yo", "よ", "ヨ"}, K{"ro", "ろ", "ロ"}, K{"wo", "を", "ヲ"},
  // Small letters - prefer 'l' versions for Romaji output
  K{"lo", "ぉ", "ォ"}, K{"lyo", "ょ", "ョ"},
  // Digraphs
  K{"vo", "ゔぉ", "ヴォ"}, K{"kyo", "きょ", "キョ"}, K{"gyo", "ぎょ", "ギョ"}, K{"qo", "くぉ", "クォ"},
  K{"sho", "しょ", "ショ"}, K{"jo", "じょ", "ジョ"}, K{"cho", "ちょ", "チョ"}, K{"dyo", "ぢょ", "ヂョ"},
  K{"tho", "てょ", "テョ"}, K{"two", "とぉ", "トォ"}, K{"nyo", "にょ", "ニョ"}, K{"hyo", "ひょ", "ヒョ"},
  K{"byo", "びょ", "ビョ"}, K{"pyo", "ぴょ", "ピョ"}, K{"fo", "ふぉ", "フォ"}, K{"fyo", "ふょ", "フョ"},
  K{"myo", "みょ", "ミョ"}, K{"ryo", "りょ", "リョ"},
  // Small 'tsu' amd ん - keep these entries at the end of the list
  K{"ltu", "っ", "ッ"}, K{"n", "ん", "ン"}};

// 'KanaVariantList' contains Romaji combinations that are supported when converting from Romaji
// to kana, but are not used when converting from kana to Romaji. All entries in this list should
// have new unique Romaji values, but be duplicates of Hiragana and Katakana values in the above
// standard 'KanaList' (these assumptions are verified by 'asserts' in 'KanaConvert' constructor).
const std::array KanaVariantList = {
  // --- あ 行 ---
  // Small letters
  V{"xa", "ぁ", "ァ"}, V{"xya", "ゃ", "ャ"}, V{"xwa", "ゎ", "ヮ"},
  // Digraphs
  V{"kwa", "くぁ", "クァ"}, V{"sya", "しゃ", "シャ"}, V{"jya", "じゃ", "ジャ"}, V{"zya", "じゃ", "ジャ"},
  V{"tya", "ちゃ", "チャ"},
  // --- い 行 ---
  V{"si", "し", "シ"}, V{"zi", "じ", "ジ"}, V{"ti", "ち", "チ"},
  // Small letters
  V{"xi", "ぃ", "ィ"},
  // Digraphs
  V{"fyi", "ふぃ", "フィ"},
  // --- う 行 ---
  V{"tu", "つ", "ツ"}, V{"hu", "ふ", "フ"},
  // Small letters
  V{"xu", "ぅ", "ゥ"}, V{"xtu", "っ", "ッ"}, V{"xyu", "ゅ", "ュ"},
  // Digraphs
  V{"syu", "しゅ", "シュ"}, V{"jyu", "じゅ", "ジュ"}, V{"zyu", "じゅ", "ジュ"}, V{"tyu", "ちゅ", "チュ"},
  // --- え 行 ---
  // Small letters
  V{"xe", "ぇ", "ェ"}, V{"lye", "ぇ", "ェ"}, V{"xye", "ぇ", "ェ"},
  // Digraphs
  V{"zye", "じぇ", "ジェ"},
  // --- お 行 ---
  // Small letters
  V{"xo", "ぉ", "ォ"}, V{"xyo", "ょ", "ョ"},
  // Digraphs
  V{"syo", "しょ", "ショ"}, V{"jyo", "じょ", "ジョ"}, V{"zyo", "じょ", "ジョ"}, V{"tyo", "ちょ", "チョ"}};

std::ostream& operator<<(std::ostream& os, const K& k) {
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
    auto i = result.insert(std::make_pair(k, &v));
    if (!i.second) {
      std::cerr << "key '" << k << "' already in " << toString(t) << " map: " << i.first->second << '\n';
      ++duplicates;
    }
  };
  auto checkList = [](auto& l) {
    for (auto& i : l) {
      assert(!i.romaji.empty() && i.romaji.length() < 4);           // must be 1 to 3 chars
      assert(i.hiragana.length() == 3 || i.hiragana.length() == 6); // 3 bytes per character
      assert(i.katakana.length() == 3 || i.katakana.length() == 6); // 3 bytes per characer
      assert(isAllSingleByte(i.romaji));
      assert(isAllHiragana(i.hiragana));
      assert(isAllKatakana(i.katakana));
    }
  };
  static bool firstTime = true;
  if (firstTime) {
    checkList(KanaList);
    checkList(KanaVariantList);
    firstTime = false;
  }
  for (auto& i : KanaList)
    switch (t) {
    case CharType::Romaji: insert(i.romaji, i); break;
    case CharType::Hiragana: insert(i.hiragana, i); break;
    case CharType::Katakana: insert(i.katakana, i); break;
    }
  if (t == CharType::Romaji)
    for (auto& i : KanaVariantList)
      insert(i.romaji, i);
  assert(duplicates == 0);
  return result;
}

KanaConvert::KanaConvert()
  : _romajiMap(populate(CharType::Romaji)), _hiraganaMap(populate(CharType::Hiragana)),
    _katakanaMap(populate(CharType::Katakana)), _smallTsu(KanaList[KanaList.size() - 2]),
    _n(KanaList[KanaList.size() - 1]), _markHiraganaAfterN({"あ", "い", "う", "え", "お", "や", "ゆ", "よ"}),
    _markKatakanaAfterN({"ア", "イ", "ウ", "エ", "オ", "ヤ", "ユ", "オ"}),
    _smallHiragana({"ぁ", "ぃ", "ぅ", "ぇ", "ぉ", "ゃ", "ゅ", "ょ", "ゎ"}),
    _smallKatakana({"ァ", "ィ", "ゥ", "ェ", "ォ", "ャ", "ュ", "ョ", "ヮ"}) {
  assert(_n.romaji == "n");
  // make sure variants also have a 'non-variant' entry in _hiraganaMap and _katakanaMap;
  for (auto& i : _romajiMap)
    if (i.second->variant) {
      assert(_hiraganaMap.find(i.second->hiragana) != _hiraganaMap.end());
      assert(_katakanaMap.find(i.second->katakana) != _katakanaMap.end());
    }
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
  if (source == CharType::Hiragana) return convertFromHiragana(input, target);
  if (source == CharType::Katakana) return convertFromKatakana(input, target);
  // When source is Romaji break input up into words separated by any of _narrowDelims and process
  // each word. This helps deal with words ending in 'n'.
  std::string result;
  size_t oldPos = 0;
  do {
    const size_t pos = input.find_first_of(_narrowDelims, oldPos);
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
  return result;
}

std::string KanaConvert::convertFromHiragana(const std::string& input, CharType target) const {
  std::string result, letterGroup, c;
  MBChar s(input);
  while (s.next(c, false)) {
    if (isHiragana(c)) {
      if (_n.contains(c)) {
        // getting an 'n' should cause any stored letters to be processed
        if (_n.contains(letterGroup))
          result += _n.get(target);
        else {
          if (!letterGroup.empty()) result += hiraganaLetters(letterGroup, target);
          letterGroup = c;
        }
      } else if (_smallHiragana.contains(c)) {
        // getting a small letter should cause letters to be processed
        letterGroup += c;
        result += hiraganaLetters(letterGroup, target);
        letterGroup.clear();
      } else {
        // a normal (non-n, non-small) letter can't form the second part of a digraph
        // so process any stored previous letter and hold processing of the new letter
        // in case it forms the first part of a digraph.
        if (!letterGroup.empty()) {
          result += hiraganaLetters(letterGroup, target);
          if (target == CharType::Romaji && _n.contains(letterGroup) && _markHiraganaAfterN.contains(c))
            result += _apostrophe;
        }
        letterGroup = c;
      }
    }
  }
  if (!letterGroup.empty()) result += hiraganaLetters(letterGroup, target);
  return result;
}

std::string KanaConvert::hiraganaLetters(const std::string& letterGroup, CharType target) const {
  auto i = _hiraganaMap.find(letterGroup);
  if (i != _hiraganaMap.end()) return i->second->get(target);
  // if letter group is an unknown combination, split it up and try processing each part
  if (letterGroup.length() > 1)
    return hiraganaLetters(letterGroup.substr(0, 1), target) + hiraganaLetters(letterGroup.substr(1), target);
  return letterGroup; // should never happen
}

std::string KanaConvert::convertFromKatakana(const std::string& input, CharType target) const { return input; }

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
  while (!letterGroup.empty()) {
    if (letterGroup == "n") {
      result += _n.get(target); // normal case for a word ending in 'n'
      letterGroup.clear();
    } else {
      result += letterGroup[0]; // error: output the unprocessed letter
      letterGroup = letterGroup.substr(1);
      convertRomajiLetters(letterGroup, result, target);
    }
  }
  return result;
}

void KanaConvert::convertRomajiLetters(std::string& letterGroup, std::string& result, CharType target) const {
  auto i = _romajiMap.find(letterGroup);
  if (i != _romajiMap.end()) {
    result += i->second->get(target);
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
      case 'z': result += _smallTsu.get(target); break;
      default: result += letterGroup[0]; // error: first letter not valid
      }
    } else // error: no romaji is longer than 3 chars so output the first letter unconverted
      result += letterGroup[0];
    letterGroup = letterGroup.substr(1);
    convertRomajiLetters(letterGroup, result, target); // try converting the shortened letterGroup
  }
}

} // namespace kanji
