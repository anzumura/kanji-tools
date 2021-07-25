#include <kanji/Kana.h>
#include <kanji/KanaConvert.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

#include <iostream>
#include <sstream>

namespace kanji {

namespace {

using P = std::pair<char, const char*>;
constexpr std::array Delimiters{
  P(' ', "　"), P('.', "。"), P(',', "、"), P(':', "："), P(';', "；"), P('/', "・"), P('!', "！"), P('?', "？"),
  P('(', "（"), P(')', "）"), P('[', "「"), P(']', "」"), P('*', "＊"), P('~', "〜"),   P('=', "＝"), P('+', "＋"),
  P('@', "＠"), P('#', "＃"), P('$', "＄"), P('%', "％"), P('^', "＾"), P('&', "＆"),   P('{', "『"), P('}', "』"),
  P('|', "｜"), P('"', "”"),  P('`', "｀"), P('<', "＜"), P('>', "＞"), P('_', "＿"),   P('\\', "￥")};

} // namespace

KanaConvert::KanaConvert(CharType target, int flags) : _target(target), _flags(flags) {
  for (auto& i : Kana::getMap(CharType::Hiragana)) {
    auto r = i.second->romaji();
    if (!r.starts_with("n")) {
      if (r.length() == 1 || r == "ya" || r == "yu" || r == "yo") {
        assert(_markAfterNHiragana.insert(i.second->hiragana()).second);
        assert(_markAfterNKatakana.insert(i.second->katakana()).second);
      } else if (r.starts_with("l")) {
        if (*i.second != Kana::SmallTsu && !r.starts_with("lk")) {
          assert(_digraphSecondHiragana.insert(i.second->hiragana()).second);
          assert(_digraphSecondKatakana.insert(i.second->katakana()).second);
        }
      } else
        _repeatingConsonents.insert(r[0]);
    }
  }
  for (auto& i : Delimiters) {
    _narrowDelimList += i.first;
    _narrowDelims[i.first] = i.second;
    _wideDelims[i.second] = i.first;
  }
  _narrowDelimList += _apostrophe;
  _narrowDelimList += _dash;
  verifyData();
}

std::string KanaConvert::flagString() const {
  if (!_flags) return "none";
  std::string result;
  auto flag = [this, &result](int f, const char* v) {
    if (_flags & f) {
      if (!result.empty()) result += '|';
      result += v;
    }
  };
  flag(Hepburn, "Hepburn");
  flag(Kunrei, "Kunrei");
  flag(NoProlongMark, "NoProlongMark");
  flag(RemoveSpaces, "RemoveSpaces");
  return result;
}

void KanaConvert::verifyData() const {
  assert(Kana::N.romaji() == "n");
  assert(Kana::SmallTsu.romaji() == "ltu");
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
  assert(_wideDelims.size() == Delimiters.size());
  assert(_narrowDelims.size() == Delimiters.size());
  assert(_narrowDelimList.length() == Delimiters.size() + 2);
}

std::string KanaConvert::convert(const std::string& input) const {
  std::string result(input);
  for (auto i : CharTypes)
    if (_target != i) result = convert(i, result);
  return result;
}

std::string KanaConvert::convert(const std::string& input, CharType target, int flags) {
  _target = target;
  _flags = flags;
  return convert(input);
}

std::string KanaConvert::convert(CharType source, const std::string& input, CharType target, int flags) {
  _target = target;
  _flags = flags;
  return convert(source, input);
}

std::string KanaConvert::convert(CharType source, const std::string& input) const {
  if (source == _target) return input;
  if (source == CharType::Hiragana) return convertFromKana(input, source, _markAfterNHiragana, _digraphSecondHiragana);
  if (source == CharType::Katakana) return convertFromKana(input, source, _markAfterNKatakana, _digraphSecondKatakana);
  // When source is Romaji break input up into words separated by any of _narrowDelimList and process
  // each word. This helps deal with words ending in 'n'.
  std::string result;
  size_t oldPos = 0;
  const bool keepSpaces = !(_flags & RemoveSpaces);
  do {
    const size_t pos = input.find_first_of(_narrowDelimList, oldPos);
    if (pos != std::string::npos) {
      result += convertFromRomaji(input.substr(oldPos, pos - oldPos));
      const char delim = input[pos];
      if (delim != _apostrophe && delim != _dash && (keepSpaces || delim != ' ')) result += _narrowDelims.at(delim);
      oldPos = pos + 1;
    } else {
      result += convertFromRomaji(input.substr(oldPos));
      break;
    }
  } while (true);
  return result;
}

std::string KanaConvert::convertFromKana(const std::string& input, CharType source, const Set& afterN,
                                         const Set& smallKana) const {
  std::string result, letterGroup, c;
  int count = 0;
  bool hasSmallTsu = false, groupDone = false;
  const Kana* prevKana = nullptr;
  auto done = [this, source, &prevKana, &result, &count, &hasSmallTsu, &groupDone, &letterGroup, &c,
               &afterN](bool startNewGroup = true, bool prolong = false) {
    result += kanaLetters(letterGroup, source, count, prevKana, prolong);
    if (_target == CharType::Romaji && Kana::N.containsKana(letterGroup) && afterN.contains(c)) result += _apostrophe;
    hasSmallTsu = false;
    groupDone = false;
    // if 'startNewGroup' is false then drop the current letter instead of using it to start a new group
    if (startNewGroup) {
      count = 1;
      letterGroup = c;
    } else {
      count = 0;
      letterGroup.clear();
    }
  };
  MBChar s(input);
  while (s.next(c, false)) {
    // check prolong mark and repeating marks first since they aren't in 'sourceMap'
    if (c == Kana::ProlongMark) {
      // this is actually a katakana symbol, but it can also appear in (non-standard) Hiragana.
      done(false, true);
    } else if (Kana::RepeatUnaccented.matches(source, c)) {
      done(false);
      result += Kana::RepeatUnaccented.get(_target, _flags, prevKana);
    } else if (Kana::RepeatAccented.matches(source, c)) {
      done(false);
      result += Kana::RepeatAccented.get(_target, _flags, prevKana);
    } else if (Kana::getMap(source).contains(c)) {
      if (Kana::SmallTsu.containsKana(c)) {
        // getting a small tsu should cause any stored letters to be processed
        done();
        hasSmallTsu = true;
      } else if (Kana::N.containsKana(c)) {
        // getting an 'n' should cause any stored letters to be processed
        done();
        groupDone = true; // mark the new group as 'done' for an 'n'
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
      done(false);
      if (_target == CharType::Romaji) {
        auto i = _wideDelims.find(c);
        if (i != _wideDelims.end())
          result += i->second;
        else
          result += c;
      } else
        result += c;
    }
  }
  result += kanaLetters(letterGroup, source, count, prevKana);
  return result;
}

std::string KanaConvert::kanaLetters(const std::string& letterGroup, CharType source, int count, const Kana*& prevKana,
                                     bool prolong) const {
  auto& sourceMap = Kana::getMap(source);
  auto macron = [this, prolong, &prevKana](const Kana* k, bool sokuon = false) {
    const auto& s = sokuon ? k->getSokuonRomaji(_flags) : k->get(_target, _flags);
    if (prolong) {
      if (_target != CharType::Romaji) return s + Kana::ProlongMark;
      switch (s[s.length() - 1]) {
      case 'a': return s.substr(0, s.length() - 1) + "ā";
      case 'i': return s.substr(0, s.length() - 1) + "ī";
      case 'u': return s.substr(0, s.length() - 1) + "ū";
      case 'e': return s.substr(0, s.length() - 1) + "ē";
      case 'o': return s.substr(0, s.length() - 1) + "ō";
      default: return s + Kana::ProlongMark; // shouldn't happen - output mark unconverted
      }
    }
    prevKana = k;
    return s;
  };
  if (!letterGroup.empty()) {
    prevKana = nullptr;
    auto i = sourceMap.find(letterGroup);
    if (i != sourceMap.end()) return macron(i->second);
    // if letter group is an unknown combination, split it up and try processing each part
    if (count > 1) {
      const auto firstLetter = letterGroup.substr(0, 3);
      i = sourceMap.find(letterGroup.substr(3));
      if (i != sourceMap.end()) {
        if (_target == CharType::Romaji && Kana::SmallTsu.containsKana(firstLetter) &&
            _repeatingConsonents.contains(i->second->romaji()[0]))
          return macron(i->second, true);
        auto transformedFirst = kanaLetters(firstLetter, source, 1, prevKana);
        return transformedFirst + macron(i->second);
      }
      // error: couldn't convert second part
      return kanaLetters(firstLetter, source, 1, prevKana) + letterGroup.substr(3);
    }
  } else if (prolong)
    // got 'prolong mark' at the start of a group which isn't valid so just return the symbol unchanged
    return Kana::ProlongMark;
  return letterGroup;
}

std::string KanaConvert::convertFromRomaji(const std::string& input) const {
  std::string result, letterGroup, c;
  auto macron = [this, &letterGroup, &result](char x, const auto& s) {
    letterGroup += x;
    romajiLetters(letterGroup, result);
    if (letterGroup.empty())
      result += _target == CharType::Hiragana && (_flags & NoProlongMark) ? s : Kana::ProlongMark;
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
        romajiLetters(letterGroup, result);
      } else {
        if (letterGroup.empty())
          letterGroup += letter;
        else if (letterGroup == "n") // got two 'n's in a row so output one, but don't need to clear letterGroup
          result += Kana::N.get(_target, _flags);
        else {
          // error: partial romaji followed by n - output uncoverted partial group
          result += letterGroup;
          letterGroup = c; // 'n' starts a new group
        }
      }
    } else {
      romajiLetters(letterGroup, result);
      result += c;
    }
  }
  while (!letterGroup.empty()) {
    if (letterGroup == "n") {
      result += Kana::N.get(_target, _flags); // normal case for a word ending in 'n'
      letterGroup.clear();
    } else {
      result += letterGroup[0]; // error: output the unprocessed letter
      letterGroup = letterGroup.substr(1);
      romajiLetters(letterGroup, result);
    }
  }
  return result;
}

void KanaConvert::romajiLetters(std::string& letterGroup, std::string& result) const {
  auto& sourceMap = Kana::getMap(CharType::Romaji);
  auto i = sourceMap.find(letterGroup);
  if (i != sourceMap.end()) {
    result += i->second->get(_target, _flags);
    letterGroup.clear();
  } else if (letterGroup.length() == 3) {
    if (letterGroup[0] == 'n')
      result += Kana::N.get(_target, _flags);
    else if (letterGroup[0] == letterGroup[1] || letterGroup[0] == 't' && letterGroup[1] == 'c') {
      // convert first letter to small tsu if letter repeats and is a valid consonant
      if (_repeatingConsonents.contains(letterGroup[0]))
        result += Kana::SmallTsu.get(_target, _flags);
      else // error: first letter not valid
        result += letterGroup[0];
    } else // error: no romaji is longer than 3 chars so output the first letter unconverted
      result += letterGroup[0];
    letterGroup = letterGroup.substr(1);
    romajiLetters(letterGroup, result); // try converting the shortened letterGroup
  }
}

} // namespace kanji
