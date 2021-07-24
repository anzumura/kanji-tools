#include <kanji/Kana.h>
#include <kanji/KanaConvert.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

#include <iostream>
#include <sstream>

namespace kanji {

namespace {

using K = Kana;
using V = Kana::List;
// 'KanaList' has mappings for all monographs (single kana) with no 'dakuten' or 'han-dakuten' versions
// and regularly used digraphs (normal kana followed by a small kana 'vowel', 'y' or 'wa'). See
// comments for 'Kana' class for a description of the fields.
const std::array KanaList{
  // --- あ 行 ---
  K{"a", "あ", "ア"}, K{"na", "な", "ナ"}, K{"ma", "ま", "マ"}, K{"ya", "や", "ヤ"}, K{"ra", "ら", "ラ"},
  K{"wa", "わ", "ワ"},
  // Digraphs
  K{"qwa", "くゎ", "クヮ"}, K{"swa", "すぁ", "スァ"}, K{"tsa", "つぁ", "ツァ"}, K{"nya", "にゃ", "ニャ"},
  K{"fa", "ふぁ", "ファ"}, K{"fya", "ふゃ", "フャ"}, K{"mya", "みゃ", "ミャ"}, K{"rya", "りゃ", "リャ"},
  // --- い 行 ---
  K{"i", "い", "イ"}, K{"ni", "に", "ニ"}, K{"mi", "み", "ミ"}, K{"ri", "り", "リ"}, K{"wyi", "ゐ", "ヰ", "i", "i"},
  // Digraphs
  K{"tsi", "つぃ", "ツィ"}, K{"nyi", "にぃ", "ニィ"}, K{"fi", "ふぃ", "フィ", V{"fyi"}}, K{"myi", "みぃ", "ミィ"},
  K{"ryi", "りぃ", "リィ"},
  // --- う 行 ---
  K{"nu", "ぬ", "ヌ"}, K{"mu", "む", "ム"}, K{"yu", "ゆ", "ユ"}, K{"ru", "る", "ル"},
  // Digraphs
  K{"nyu", "にゅ", "ニュ"}, K{"fyu", "ふゅ", "フュ"}, K{"myu", "みゅ", "ミュ"}, K{"ryu", "りゅ", "リュ"},
  // --- え 行 ---
  K{"e", "え", "エ"}, K{"ne", "ね", "ネ"}, K{"me", "め", "メ"}, K{"re", "れ", "レ"}, K{"wye", "ゑ", "ヱ", "e", "e"},
  // Digraphs
  K{"ye", "いぇ", "イェ"}, K{"che", "ちぇ", "チェ"}, K{"tse", "つぇ", "ツェ"}, K{"nye", "にぇ", "ニェ"},
  K{"fe", "ふぇ", "フェ"}, K{"mye", "みぇ", "ミェ"}, K{"rye", "りぇ", "リェ"},
  // --- お 行 ---
  K{"o", "お", "オ"}, K{"no", "の", "ノ"}, K{"mo", "も", "モ"}, K{"yo", "よ", "ヨ"}, K{"ro", "ろ", "ロ"},
  K{"wo", "を", "ヲ", "o", "o"},
  // Digraphs
  K{"tso", "つぉ", "ツォ"}, K{"nyo", "にょ", "ニョ"}, K{"fo", "ふぉ", "フォ"}, K{"fyo", "ふょ", "フョ"},
  K{"myo", "みょ", "ミョ"}, K{"ryo", "りょ", "リョ"},
  // Digraphs that only have a dakuten version
  K{"va", "ゔぁ", "ヴァ"}, K{"vo", "ゔぉ", "ヴォ"}, K{"vya", "ゔゃ", "ヴャ"}, K{"vyu", "ゔゅ", "ヴュ"},
  K{"vyo", "ゔょ", "ヴョ"},
  // 12 Small letters (5 vowels, 2 k's, 3 y's, small 'wa' and small 'tsu') - prefer 'l' versions for Romaji output
  K{"la", "ぁ", "ァ", V{"xa"}}, K{"li", "ぃ", "ィ", V{"xi"}}, K{"lu", "ぅ", "ゥ", V{"xu"}},
  K{"le", "ぇ", "ェ", V{"xe", "lye", "xye"}}, K{"lo", "ぉ", "ォ", V{"xo"}}, K{"lka", "ゕ", "ヵ", V{"xka"}},
  K{"lke", "ゖ", "ヶ", V{"xke"}}, K{"lya", "ゃ", "ャ", V{"xya"}}, K{"lyu", "ゅ", "ュ", V{"xyu"}},
  K{"lyo", "ょ", "ョ", V{"xyo"}}, K{"lwa", "ゎ", "ヮ", V{"xwa"}}, K{"ltu", "っ", "ッ", V{"xtu"}},
  // ん - keep 'n' as well as the previous small 'tsu' at the end of the list
  K{"n", "ん", "ン"}};

using D = DakutenKana;
// 'DakutenKanaList' contains kana that have a 'dakuten' version, but not 'h'
std::array DakutenKanaList = {
  // --- あ 行 ---
  D{"ka", "か", "カ", K{"ga", "が", "ガ"}}, D{"sa", "さ", "サ", K{"za", "ざ", "ザ"}},
  D{"ta", "た", "タ", K{"da", "だ", "ダ"}}, D{"kya", "きゃ", "キャ", K{"gya", "ぎゃ", "ギャ"}},
  // Diagraphs
  D{"qa", "くぁ", "クァ", K{"gwa", "ぐぁ", "グァ"}, V{"kwa"}},
  D{"sha", "しゃ", "シャ", K{"ja", "じゃ", "ジャ", V{"zya", "jya"}, true}, V{"sya"}, true},
  D{"cha", "ちゃ", "チャ", K{"dya", "ぢゃ", "ヂャ", "ja", "zya"}, V{"tya"}, true},
  D{"twa", "とぁ", "トァ", K{"dwa", "どぁ", "ドァ"}},
  // --- い 行 ---
  D{"ki", "き", "キ", K{"gi", "ぎ", "ギ"}}, D{"shi", "し", "シ", K{"ji", "じ", "ジ", V{"zi"}, true}, V{"si"}, true},
  D{"chi", "ち", "チ", K{"di", "ぢ", "ヂ", "ji", "zi"}, V{"ti"}, true},
  // Digraphs
  D{"kyi", "きぃ", "キィ", K{"gyi", "ぎぃ", "ギィ"}}, D{"syi", "しぃ", "シィ", K{"zyi", "じぃ", "ジィ"}},
  D{"tyi", "ちぃ", "チィ", K{"dyi", "ぢぃ", "ヂィ"}}, D{"thi", "てぃ", "ティ", K{"dhi", "でぃ", "ディ"}},
  // --- う 行 ---
  D{"u", "う", "ウ", K{"vu", "ゔ", "ヴ"}}, D{"ku", "く", "ク", K{"gu", "ぐ", "グ"}},
  D{"su", "す", "ス", K{"zu", "ず", "ズ"}}, D{"tsu", "つ", "ツ", K{"du", "づ", "ヅ", "zu", "zu"}, V{"tu"}, true},
  // Digraphs
  D{"wi", "うぃ", "ウィ", K{"vi", "ゔぃ", "ヴィ"}}, D{"kyu", "きゅ", "キュ", K{"gyu", "ぎゅ", "ギュ"}},
  D{"qi", "くぃ", "クィ", K{"gwi", "ぐぃ", "グィ"}, V{"kwi"}}, D{"qu", "くぅ", "クゥ", K{"gwu", "ぐぅ", "グゥ"}},
  D{"shu", "しゅ", "シュ", K{"ju", "じゅ", "ジュ", V{"zyu", "jyu"}, true}, V{"syu"}, true},
  D{"chu", "ちゅ", "チュ", K{"dyu", "ぢゅ", "ヂュ", "ju", "zyu"}, V{"tyu"}, true},
  D{"twu", "とぅ", "トゥ", K{"dwu", "どぅ", "ドゥ"}},
  // --- え 行 ---
  D{"ke", "け", "ケ", K{"ge", "げ", "ゲ"}}, D{"kye", "きぇ", "キェ", K{"gye", "ぎぇ", "ギェ"}},
  D{"se", "せ", "セ", K{"ze", "ぜ", "ゼ"}}, D{"te", "て", "テ", K{"de", "で", "デ"}},
  // Digraphs
  D{"we", "うぇ", "ウェ", K{"ve", "ゔぇ", "ヴェ"}}, D{"qe", "くぇ", "クェ", K{"gwe", "ぐぇ", "グェ"}},
  D{"she", "しぇ", "シェ", K{"je", "じぇ", "ジェ", V{"zye"}}}, D{"the", "てぇ", "テェ", K{"dhe", "でぇ", "デェ"}},
  D{"twe", "とぇ", "トェ", K{"dwe", "どぇ", "ドェ"}},
  // --- お 行 ---
  D{"ko", "こ", "コ", K{"go", "ご", "ゴ"}}, D{"so", "そ", "ソ", K{"zo", "ぞ", "ゾ"}},
  D{"to", "と", "ト", K{"do", "ど", "ド"}},
  // Digraphs
  D{"kyo", "きょ", "キョ", K{"gyo", "ぎょ", "ギョ"}}, D{"qo", "くぉ", "クォ", K{"gwo", "ぐぉ", "グォ"}},
  D{"sho", "しょ", "ショ", K{"jo", "じょ", "ジョ", V{"zyo", "jyo"}, true}, V{"syo"}, true},
  D{"cho", "ちょ", "チョ", K{"dyo", "ぢょ", "ヂョ", "jo", "zyo"}, V{"tyo"}, true},
  D{"tho", "てょ", "テョ", K{"dho", "でょ", "デョ"}}, D{"two", "とぉ", "トォ", K{"dwo", "どぉ", "ドォ"}}};

using H = HanDakutenKana;
// 'HanDakutenKanaList' contains kana that have both a 'dakuten' and a 'han-dakuten' (so 'h' row)
std::array HanDakutenKanaList = {H{"ha", "は", "ハ", K{"ba", "ば", "バ"}, K{"pa", "ぱ", "パ"}},
                                 H{"hi", "ひ", "ヒ", K{"bi", "び", "ビ"}, K{"pi", "ぴ", "ピ"}},
                                 H{"fu", "ふ", "フ", K{"bu", "ぶ", "ブ"}, K{"pu", "ぷ", "プ"}, V{"hu"}, true},
                                 H{"he", "へ", "ヘ", K{"be", "べ", "ベ"}, K{"pe", "ぺ", "ペ"}},
                                 H{"ho", "ほ", "ホ", K{"bo", "ぼ", "ボ"}, K{"po", "ぽ", "ポ"}},
                                 H{"hya", "ひゃ", "ヒャ", K{"bya", "びゃ", "ビャ"}, K{"pya", "ぴゃ", "ピャ"}},
                                 H{"hyi", "ひぃ", "ヒィ", K{"byi", "びぃ", "ビィ"}, K{"pyi", "ぴぃ", "ピィ"}},
                                 H{"hyu", "ひゅ", "ヒュ", K{"byu", "びゅ", "ビュ"}, K{"pyu", "ぴゅ", "ピュ"}},
                                 H{"hye", "ひぇ", "ヒェ", K{"bye", "びぇ", "ビェ"}, K{"pye", "ぴぇ", "ピェ"}},
                                 H{"hyo", "ひょ", "ヒョ", K{"byo", "びょ", "ビョ"}, K{"pyo", "ぴょ", "ピョ"}}};

std::ostream& operator<<(std::ostream& os, const K& k) {
  return os << '[' << k.romaji() << ", " << k.hiragana() << ", " << k.katakana() << ']';
}

using P = std::pair<char, const char*>;
// Support converting other non-letter ascii from narrow to wide values. These values are also used as
// delimiters for splitting up input strings when converting from Rõmaji to Kana. Use a '*' for katakana
// middle dot '・' to keep round-trip translations as non-lossy as possible. For now, don't include '-'
// (minus) or apostrophe since these could get mixed up with _prolongMark 'ー' and special
// separation handling after 'n' in Romaji output. Backslash maps to ￥ as per the usual keyboard input.
constexpr std::array Delimiters{P(' ', "　"), P('.', "。"), P(',', "、"), P(':', "："), P(';', "；"), P('/', "／"),
                                P('!', "！"), P('?', "？"), P('(', "（"), P(')', "）"), P('[', "「"), P(']', "」"),
                                P('*', "・"), P('~', "〜"), P('=', "＝"), P('+', "＋"), P('@', "＠"), P('#', "＃"),
                                P('$', "＄"), P('%', "％"), P('^', "＾"), P('&', "＆"), P('{', "『"), P('}', "』"),
                                P('|', "｜"), P('"', "”"),  P('`', "｀"), P('<', "＜"), P('>', "＞"), P('\\', "￥")};

} // namespace

std::string KanaConvert::RepeatMark::getRomaji(const std::string& prevKana, int flags) const { return prevKana; }

KanaConvert::Map KanaConvert::populate(CharType t) {
  Map result;
  int duplicates = 0;
  auto insert = [&result, &duplicates, t](auto& k, auto& v) {
    auto i = result.insert(std::make_pair(k, &v));
    if (!i.second) {
      std::cerr << "key '" << k << "' already in " << toString(t) << " map: " << i.first->second << '\n';
      ++duplicates;
    }
  };
  auto processKana = [&insert, t](auto& k) {
    switch (t) {
    case CharType::Romaji:
      insert(k.romaji(), k);
      for (auto& i : k.variants())
        insert(i, k);
      break;
    case CharType::Hiragana: insert(k.hiragana(), k); break;
    case CharType::Katakana: insert(k.katakana(), k); break;
    }
  };
  // process lists (inserting into 'result')
  for (auto& i : KanaList)
    processKana(i);
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

KanaConvert::KanaConvert()
  : _romajiMap(populate(CharType::Romaji)), _hiraganaMap(populate(CharType::Hiragana)),
    _katakanaMap(populate(CharType::Katakana)), _smallTsu(KanaList[KanaList.size() - 2]),
    _n(KanaList[KanaList.size() - 1]), _prolongMark("ー") {
  for (auto& i : _hiraganaMap) {
    auto r = i.second->romaji();
    if (!r.starts_with("n")) {
      if (r.length() == 1 || r == "ya" || r == "yu" || r == "yo") {
        assert(_markAfterNHiragana.insert(i.second->hiragana()).second);
        assert(_markAfterNKatakana.insert(i.second->katakana()).second);
      } else if (r.starts_with("l")) {
        if (*i.second != _smallTsu && !r.starts_with("lk")) {
          assert(_digraphSecondHiragana.insert(i.second->hiragana()).second);
          assert(_digraphSecondKatakana.insert(i.second->katakana()).second);
        }
      } else
        _repeatingConsonents.insert(r[0]);
    }
  }
  for (auto& i : Delimiters) {
    _narrowDelims += i.first;
    _narrowToWideDelims[i.first] = i.second;
    _wideToNarrowDelims[i.second] = i.first;
  }
  _narrowDelims += _apostrophe;
  _narrowDelims += _dash;
  verifyData();
}

void KanaConvert::verifyData() const {
  assert(_n.romaji() == "n");
  assert(_smallTsu.romaji() == "ltu");
  assert(_repeatingConsonents.size() == 18); // 26 - 8 where '8' is 5 vowels + 3 consonents (l, n and x)
  for (auto i : {'a', 'i', 'u', 'e', 'o', 'l', 'n', 'x'})
    assert(_repeatingConsonents.contains(i) == false);
  assert(_markAfterNHiragana.size() == 8); // 5 vowels plus 3 y's
  assert(_markAfterNHiragana.size() == _markAfterNKatakana.size());
  assert(_digraphSecondHiragana.size() == 9); // 5 small vowels plus 3 small y's plus small 'wa'
  assert(_digraphSecondHiragana.size() == _digraphSecondKatakana.size());
  for (auto& i : _markAfterNHiragana)
    assert(isHiragana(i));
  for (auto& i : _markAfterNKatakana)
    assert(isKatakana(i));
  for (auto& i : _digraphSecondHiragana)
    assert(isHiragana(i));
  for (auto& i : _digraphSecondKatakana)
    assert(isKatakana(i));
  assert(_wideToNarrowDelims.size() == Delimiters.size());
  assert(_narrowToWideDelims.size() == Delimiters.size());
  assert(_narrowDelims.length() == Delimiters.size() + 2);
}

std::string KanaConvert::convert(const std::string& input, CharType target, int flags) const {
  std::string result(input);
  for (auto i : CharTypes)
    if (target != i) result = convert(result, i, target, flags);
  return result;
}

std::string KanaConvert::convert(const std::string& input, CharType source, CharType target, int flags) const {
  if (source == target) return input;
  if (source == CharType::Hiragana)
    return convertFromKana(input, target, flags, _hiraganaMap, _markAfterNHiragana, _digraphSecondHiragana);
  if (source == CharType::Katakana)
    return convertFromKana(input, target, flags, _katakanaMap, _markAfterNKatakana, _digraphSecondKatakana);
  // When source is Romaji break input up into words separated by any of _narrowDelims and process
  // each word. This helps deal with words ending in 'n'.
  std::string result;
  size_t oldPos = 0;
  const bool keepSpaces = !(flags & RemoveSpaces);
  do {
    const size_t pos = input.find_first_of(_narrowDelims, oldPos);
    if (pos != std::string::npos) {
      result += convertFromRomaji(input.substr(oldPos, pos - oldPos), target, flags);
      const char delim = input[pos];
      if (delim != _apostrophe && delim != _dash && (keepSpaces || delim != ' '))
        result += _narrowToWideDelims.at(delim);
      oldPos = pos + 1;
    } else {
      result += convertFromRomaji(input.substr(oldPos), target, flags);
      break;
    }
  } while (true);
  return result;
}

std::string KanaConvert::convertFromKana(const std::string& input, CharType target, int flags, const Map& sourceMap,
                                         const Set& afterN, const Set& smallKana) const {
  std::string result, letterGroup, c;
  int count = 0;
  bool hasSmallTsu = false, groupDone = false;
  auto done = [this, target, flags, &result, &count, &hasSmallTsu, &groupDone, &letterGroup, &c, &sourceMap, &afterN] {
    result += kanaLetters(sourceMap, letterGroup, count, target, flags);
    if (target == CharType::Romaji && _n.containsKana(letterGroup) && afterN.contains(c)) result += _apostrophe;
    count = 1;
    hasSmallTsu = false;
    groupDone = false;
    letterGroup = c;
  };
  MBChar s(input);
  while (s.next(c, false)) {
    if (c == _prolongMark) {
      // this is actually a katakana symbol, but it can also appear in (non-standard) Hiragana.
      result += kanaLetters(sourceMap, letterGroup, count, target, flags, true);
      letterGroup.clear();
      count = 0;
      hasSmallTsu = false;
      groupDone = false;
    } else if (sourceMap.contains(c)) {
      if (_smallTsu.containsKana(c)) {
        // getting a small tsu should cause any stored letters to be processed
        result += kanaLetters(sourceMap, letterGroup, count, target, flags);
        count = 1;
        hasSmallTsu = true;
        letterGroup = c;
        groupDone = false;
      } else if (_n.containsKana(c)) {
        // getting an 'n' should cause any stored letters to be processed
        result += kanaLetters(sourceMap, letterGroup, count, target, flags);
        // start a new group that just contains 'n' and is marked as done
        count = 1;
        hasSmallTsu = false;
        letterGroup = c;
        groupDone = true;
      } else if (groupDone)
        done();
      else if (smallKana.contains(c)) {
        // a small letter should cause letters to be processed including the small letter
        // so mark group as done, but continue the loop in case there's a 'prolong' mark.
        letterGroup += c;
        ++count;
        groupDone = true;
      } else if (count > (hasSmallTsu ? 1 : 0))
        // a normal (non-n, non-small) letter can't form the second part of a digraph so
        // process any stored previous letter and hold processing of the new letter in
        // case it forms the first part of a new digraph.
        done();
      else {
        letterGroup += c;
        ++count;
      }
    } else {
      // got non-hiragana letter so flush any letters and preserve the new letter unconverted
      result += kanaLetters(sourceMap, letterGroup, count, target, flags);
      letterGroup.clear();
      count = 0;
      hasSmallTsu = false;
      groupDone = false;
      if (target == CharType::Romaji) {
        auto i = _wideToNarrowDelims.find(c);
        if (i != _wideToNarrowDelims.end())
          result += i->second;
        else
          result += c;
      } else
        result += c;
    }
  }
  result += kanaLetters(sourceMap, letterGroup, count, target, flags);
  return result;
}

std::string KanaConvert::kanaLetters(const Map& sourceMap, const std::string& letterGroup, int count, CharType target,
                                     int flags, bool prolonged) const {
  auto macron = [this, target, prolonged](const auto& s) {
    if (prolonged) {
      if (target != CharType::Romaji) return s + _prolongMark;
      switch (s[s.length() - 1]) {
      case 'a': return s.substr(0, s.length() - 1) + "ā";
      case 'i': return s.substr(0, s.length() - 1) + "ī";
      case 'u': return s.substr(0, s.length() - 1) + "ū";
      case 'e': return s.substr(0, s.length() - 1) + "ē";
      case 'o': return s.substr(0, s.length() - 1) + "ō";
      default: return s + _prolongMark; // shouldn't happen - output mark unconverted
      }
    }
    return s;
  };
  if (!letterGroup.empty()) {
    auto i = sourceMap.find(letterGroup);
    if (i != sourceMap.end()) return macron(i->second->get(target, flags));
    // if letter group is an unknown combination, split it up and try processing each part
    if (count > 1) {
      const auto firstLetter = letterGroup.substr(0, 3);
      i = sourceMap.find(letterGroup.substr(3));
      if (i != sourceMap.end()) {
        if (target == CharType::Romaji && _smallTsu.containsKana(firstLetter) &&
            _repeatingConsonents.contains(i->second->romaji()[0]))
          return macron(i->second->getSokuonRomaji(flags));
        return kanaLetters(sourceMap, firstLetter, 1, target, flags) + macron(i->second->get(target, flags));
      }
      // error: couldn't convert second part
      return kanaLetters(sourceMap, firstLetter, 1, target, flags) + letterGroup.substr(3);
    }
  } else if (prolonged)
    // got 'prolonged' at the start of a group which isn't valid so just return the symbol unchanged
    return _prolongMark;
  return letterGroup;
}

std::string KanaConvert::convertFromRomaji(const std::string& input, CharType target, int flags) const {
  std::string result, letterGroup, c;
  auto macron = [this, &letterGroup, &result, target, flags](char x, const auto& s) {
    letterGroup += x;
    romajiLetters(letterGroup, result, target, flags);
    if (letterGroup.empty())
      result += target == CharType::Hiragana && (flags & NoProlongMark) ? s : _prolongMark;
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
        romajiLetters(letterGroup, result, target, flags);
      } else {
        if (letterGroup.empty())
          letterGroup += letter;
        else if (letterGroup == "n") // got two 'n's in a row so output one, but don't need to clear letterGroup
          result += _n.get(target, flags);
        else {
          // error: partial romaji followed by n - output uncoverted partial group
          result += letterGroup;
          letterGroup = c; // 'n' starts a new group
        }
      }
    } else {
      romajiLetters(letterGroup, result, target, flags);
      result += c;
    }
  }
  while (!letterGroup.empty()) {
    if (letterGroup == "n") {
      result += _n.get(target, flags); // normal case for a word ending in 'n'
      letterGroup.clear();
    } else {
      result += letterGroup[0]; // error: output the unprocessed letter
      letterGroup = letterGroup.substr(1);
      romajiLetters(letterGroup, result, target, flags);
    }
  }
  return result;
}

void KanaConvert::romajiLetters(std::string& letterGroup, std::string& result, CharType target, int flags) const {
  auto i = _romajiMap.find(letterGroup);
  if (i != _romajiMap.end()) {
    result += i->second->get(target, flags);
    letterGroup.clear();
  } else if (letterGroup.length() == 3) {
    if (letterGroup[0] == 'n')
      result += _n.get(target, flags);
    else if (letterGroup[0] == letterGroup[1] || letterGroup[0] == 't' && letterGroup[1] == 'c') {
      // convert first letter to small tsu if letter repeats and is a valid consonant
      if (_repeatingConsonents.contains(letterGroup[0]))
        result += _smallTsu.get(target, flags);
      else // error: first letter not valid
        result += letterGroup[0];
    } else // error: no romaji is longer than 3 chars so output the first letter unconverted
      result += letterGroup[0];
    letterGroup = letterGroup.substr(1);
    romajiLetters(letterGroup, result, target, flags); // try converting the shortened letterGroup
  }
}

} // namespace kanji
