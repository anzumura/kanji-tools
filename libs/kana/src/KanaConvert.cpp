#include <kanji_tools/kana/KanaConvert.h>
#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <sstream>

namespace kanji_tools {

KanaConvert::Tokens::Tokens() : _narrowDelimList{Apostrophe, Dash} {
  for (auto& i : Kana::getMap(CharType::Hiragana))
    if (auto& r{i.second->romaji()}; !r.starts_with("n")) {
      if (r.size() == 1 || r == "ya" || r == "yu" || r == "yo") {
        insertUnique(_afterNHiragana, i.second->hiragana());
        insertUnique(_afterNKatakana, i.second->katakana());
      } else if (r.starts_with("l")) {
        if (*i.second != Kana::SmallTsu && !r.starts_with("lk")) {
          insertUnique(_smallHiragana, i.second->hiragana());
          insertUnique(_smallKatakana, i.second->katakana());
        }
      } else
        _repeatingConsonents.insert(r[0]);
    }
  struct D {
    const char narrow;
    const char (&wide)[Kana::OneKanaArraySize]; // wide delims are 3 byte UTF-8
  };
  // add delims in Ascii order (skipping alphanum, Apostrophe and Dash)
  for (auto& i : {D{' ', "　"}, D{'!', "！"}, D{'"', "”"}, D{'#', "＃"},
           D{'$', "＄"}, D{'%', "％"}, D{'&', "＆"}, D{'(', "（"}, D{')', "）"},
           D{'*', "＊"}, D{'+', "＋"}, D{',', "、"}, D{'.', "。"}, D{'/', "・"},
           // Ascii 0-9
           D{':', "："}, D{';', "；"}, D{'<', "＜"}, D{'=', "＝"}, D{'>', "＞"},
           D{'?', "？"}, D{'@', "＠"},
           // Ascii A-Z
           D{'[', "「"}, D{'\\', "￥"}, D{']', "」"}, D{'^', "＾"},
           D{'_', "＿"}, D{'`', "｀"},
           // Ascii a-z
           D{'{', "『"}, D{'|', "｜"}, D{'}', "』"}, D{'~', "〜"}}) {
    _narrowDelimList += i.narrow;
    _narrowDelims[i.narrow] = i.wide;
    _wideDelims[i.wide] = i.narrow;
  }
  verifyData();
}

void KanaConvert::Tokens::verifyData() const {
  assert(Kana::N.romaji() == "n");
  assert(Kana::SmallTsu.romaji() == "ltu");
  assert(_repeatingConsonents.size() == 18);
  for ([[maybe_unused]] const auto i : {'a', 'i', 'u', 'e', 'o', 'l', 'n', 'x'})
    assert(_repeatingConsonents.contains(i) == false);
  assert(_afterNHiragana.size() == 8); // 5 vowels plus 3 y's
  assert(_afterNHiragana.size() == _afterNKatakana.size());
  // 5 small vowels plus 3 small y's plus small 'wa'
  assert(_smallHiragana.size() == 9);
  assert(_smallHiragana.size() == _smallKatakana.size());
  for ([[maybe_unused]] auto& i : _afterNHiragana) assert(isHiragana(i));
  for ([[maybe_unused]] auto& i : _afterNKatakana) assert(isKatakana(i));
  for ([[maybe_unused]] auto& i : _smallHiragana) assert(isHiragana(i));
  for ([[maybe_unused]] auto& i : _smallKatakana) assert(isKatakana(i));
  // make sure there are no duplicate narrow or wide delims, i.e., both maps
  // must have the same size (which is 2 less than _narrowDelimList)
  assert(_wideDelims.size() == _narrowDelimList.size() - 2);
  assert(_narrowDelims.size() == _wideDelims.size());
}

const KanaConvert::Tokens& KanaConvert::tokens() {
  static const Tokens tokens;
  return tokens;
}

KanaConvert::KanaConvert(CharType target, ConvertFlags flags)
    : _target{target}, _flags{flags} {}

std::string KanaConvert::flagString() const {
  if (_flags == ConvertFlags::None) return "None";
  std::string result;
  const auto flag{[this, &result](auto f, const char* v) {
    if (hasValue(_flags & f)) {
      if (!result.empty()) result += '|';
      result += v;
    }
  }};
  flag(ConvertFlags::Hepburn, "Hepburn");
  flag(ConvertFlags::Kunrei, "Kunrei");
  flag(ConvertFlags::NoProlongMark, "NoProlongMark");
  flag(ConvertFlags::RemoveSpaces, "RemoveSpaces");
  return result;
}

std::string KanaConvert::convert(const std::string& input) const {
  std::string result{input};
  for (const auto i : CharTypes)
    if (_target != i) result = convert(i, result);
  return result;
}

std::string KanaConvert::convert(
    const std::string& input, CharType target, ConvertFlags flags) {
  _target = target;
  _flags = flags;
  return convert(input);
}

std::string KanaConvert::convert(CharType source, const std::string& input,
    CharType target, ConvertFlags flags) {
  _target = target;
  _flags = flags;
  return convert(source, input);
}

std::string KanaConvert::convert(
    CharType source, const std::string& input) const {
  if (source == _target) return input;
  if (source == CharType::Hiragana) return convertFromKana(input, source);
  if (source == CharType::Katakana) return convertFromKana(input, source);
  // For Romaji source, break into words separated by any of _narrowDelimList
  // and process each word. This helps deal with words ending in 'n'.
  std::string result;
  size_t oldPos{};
  for (const auto keepSpaces{!(_flags & ConvertFlags::RemoveSpaces)};;) {
    const auto pos{input.find_first_of(tokens().narrowDelimList(), oldPos)};
    if (pos == std::string::npos) {
      result += convertToKana(input.substr(oldPos));
      break;
    }
    result += convertToKana(input.substr(oldPos, pos - oldPos));
    if (const auto delim{input[pos]};
        delim != Apostrophe && delim != Dash && (keepSpaces || delim != ' '))
      result += narrowDelims().at(delim);
    oldPos = pos + 1;
  }
  return result;
}

std::string KanaConvert::convertFromKana(
    const std::string& kanaInput, CharType source) const {
  std::string result, kanaGroup, kana;
  auto hasSmallTsu{false}, groupDone{false};
  const Kana* prevKana{};
  const auto done{
      [this, source, &prevKana, &result, &hasSmallTsu, &groupDone, &kanaGroup,
          &kana](bool startNewGroup = true, bool prolong = false) {
        result += processKana(kanaGroup, source, prevKana, prolong);
        if (romajiTarget() && Kana::N.containsKana(kanaGroup) &&
            afterN(source).contains(kana))
          result += Apostrophe;
        hasSmallTsu = false;
        groupDone = false;
        // if 'startNewGroup' is false then drop the current letter instead of
        // using it to start a new group
        kanaGroup = startNewGroup ? kana : EmptyString;
      }};
  for (MBChar s{kanaInput}; s.next(kana, false);) {
    // check prolong and repeating marks first since they aren't in 'sourceMap'
    if (kana == Kana::ProlongMark)
      // prolong is 'katakana', but it can appear in (non-standard) Hiragana.
      done(false, true);
    else if (Kana::RepeatPlain.matches(source, kana)) {
      done(false);
      result += Kana::RepeatPlain.get(_target, _flags, prevKana);
    } else if (Kana::RepeatAccented.matches(source, kana)) {
      done(false);
      result += Kana::RepeatAccented.get(_target, _flags, prevKana);
    } else if (Kana::getMap(source).contains(kana)) {
      if (Kana::SmallTsu.containsKana(kana)) {
        // getting a small tsu should cause any stored letters to be processed
        done();
        hasSmallTsu = true;
      } else if (Kana::N.containsKana(kana)) {
        // getting an 'n' should cause any stored letters to be processed
        done();
        groupDone = true; // mark the new group as 'done' for an 'n'
      } else if (groupDone)
        done();
      else if (smallKana(source).contains(kana)) {
        // a small letter should cause letters to be processed including the
        // small letter so mark group as done, but continue the loop in case
        // there's a 'prolong' mark.
        kanaGroup += kana;
        groupDone = true;
      } else if (kanaGroup.size() > (hasSmallTsu ? Kana::OneKanaSize : 0))
        // a normal (non-n, non-small) letter can't form the second part of a
        // digraph so process any stored previous letter and hold processing of
        // the new letter in case it forms the first part of a new digraph.
        done();
      else
        kanaGroup += kana;
    } else {
      // got non-hiragana letter so flush any letters and preserve the new
      // letter unconverted
      done(false);
      if (romajiTarget()) {
        if (const auto i{wideDelims().find(kana)}; i != wideDelims().end())
          result += i->second;
        else
          result += kana;
      } else
        result += kana;
    }
  }
  return result + processKana(kanaGroup, source, prevKana);
}

std::string KanaConvert::processKana(const std::string& kanaGroup,
    CharType source, const Kana*& prevKana, bool prolong) const {
  auto& sourceMap{Kana::getMap(source)};
  const auto macron{[this, prolong, &prevKana](
                        const Kana* k, bool sokuon = false) {
    const auto& s{sokuon ? k->getSokuonRomaji(_flags) : get(*k)};
    if (prolong) {
      if (_target != CharType::Romaji) return s + Kana::ProlongMark;
      switch (s[s.size() - 1]) {
      case 'a': return s.substr(0, s.size() - 1) + "ā";
      case 'i': return s.substr(0, s.size() - 1) + "ī";
      case 'u': return s.substr(0, s.size() - 1) + "ū";
      case 'e': return s.substr(0, s.size() - 1) + "ē";
      case 'o': return s.substr(0, s.size() - 1) + "ō";
      default:
        return s + Kana::ProlongMark; // shouldn't happen - output unconverted
      }
    }
    prevKana = k;
    return s;
  }};
  if (!kanaGroup.empty()) {
    prevKana = nullptr;
    if (const auto i{sourceMap.find(kanaGroup)}; i != sourceMap.end())
      return macron(i->second);
    // if letter group is an unknown, split it up and try processing each part
    if (kanaGroup.size() > Kana::OneKanaSize) {
      const auto firstKana{kanaGroup.substr(0, Kana::OneKanaSize)};
      if (const auto i{sourceMap.find(kanaGroup.substr(Kana::OneKanaSize))};
          i != sourceMap.end())
        return romajiTarget() && Kana::SmallTsu.containsKana(firstKana) &&
                       repeatingConsonents().contains(i->second->romaji()[0])
                   ? macron(i->second, true)
                   : processKana(firstKana, source, prevKana) +
                         macron(i->second);
      // error: couldn't convert second part
      return processKana(firstKana, source, prevKana) +
             kanaGroup.substr(Kana::OneKanaSize);
    }
  } else if (prolong)
    // got 'prolong mark' at the start of a group which isn't valid so just
    // return the symbol unchanged
    return Kana::ProlongMark;
  return kanaGroup;
}

std::string KanaConvert::convertToKana(const std::string& romajiInput) const {
  std::string result, letters, letter;
  for (MBChar s{romajiInput}; s.next(letter, false);)
    if (isSingleByte(letter)) {
      if (const auto lowerLetter{static_cast<char>(std::tolower(letter[0]))};
          lowerLetter != 'n') {
        letters += lowerLetter;
        processRomaji(letters, result);
      } else if (letters.empty())
        letters += lowerLetter;
      else if (letters == "n")
        // got two 'n's in a row so output one, but don't clear letters
        result += getN();
      else {
        // error: partial romaji followed by n - output uncoverted partial group
        result += letters;
        letters = lowerLetter; // 'n' starts a new group
      }
    } else if (!processRomajiMacron(letter, letters, result)) {
      processRomaji(letters, result);
      result += letter;
    }
  while (!letters.empty())
    if (letters == "n") {
      result += getN(); // normal case for a word ending in 'n'
      letters.clear();
    } else {
      result += letters[0]; // error: output the unprocessed letter
      letters = letters.substr(1);
      processRomaji(letters, result);
    }
  return result;
}

void KanaConvert::processRomaji(
    std::string& letters, std::string& result) const {
  auto& sourceMap{Kana::getMap(CharType::Romaji)};
  if (const auto i{sourceMap.find(letters)}; i != sourceMap.end()) {
    result += get(*i->second);
    letters.clear();
  } else if (letters.size() == Kana::RomajiStringMaxSize) {
    // convert first letter to small tsu if letter repeats and is a valid
    // consonant (also allow 'tc' combination) otherwise output the first letter
    // unconverted since no valid romaji can be longer than 3 letters
    result +=
        letters[0] == 'n' ? getN()
        : letters[0] == letters[1] || letters[0] == 't' && letters[1] == 'c'
            ? repeatingConsonents().contains(letters[0])
                  ? getSmallTsu()
                  : letters.substr(0, 1) // error: first letter not valid
            : letters.substr(0, 1);      // error: first letter not valid
    letters = letters.substr(1);
    // try converting the shortened letters
    processRomaji(letters, result);
  }
}

bool KanaConvert::processRomajiMacron(const std::string& letter,
    std::string& letters, std::string& result) const {
  static const std::map<std::string, std::pair<char, std::string>> Macrons{
      {"ā", {'a', "あ"}}, {"ī", {'i', "い"}}, {"ū", {'u', "う"}},
      {"ē", {'e', "え"}}, {"ō", {'o', "お"}}};

  if (const auto i{Macrons.find(letter)}; i != Macrons.end()) {
    processRomaji(letters += i->second.first, result);
    if (letters.empty())
      result +=
          hiraganaTarget() && hasValue(_flags & ConvertFlags::NoProlongMark)
              ? i->second.second
              : Kana::ProlongMark;
    else
      result += i->second.first; // should never happen ...
    return true;
  }
  return false;
}

} // namespace kanji_tools
