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
  enum class State { New, SmallTsu, Done };
  State state{State::New};
  std::string result, kanaGroup, kana;
  const Kana* prevKana{};

  enum class DoneType { NewGroup, NewEmptyGroup, Prolong };
  // 'done' is called to process the Kana built up in 'kanaGroup'. By default it
  // starts a new group containing 'kana' (the current symbol being processed).
  const auto done{[this, source, &state, &result, &kanaGroup, &kana, &prevKana](
                      DoneType dt = DoneType::NewGroup, State ns = State::New) {
    result += processKana(kanaGroup, source, prevKana, dt == DoneType::Prolong);
    if (romajiTarget() && Kana::N.containsKana(kanaGroup) &&
        afterN(source).contains(kana))
      result += Apostrophe;
    kanaGroup = dt == DoneType::NewGroup ? kana : EmptyString;
    state = ns;
  }};

  for (MBChar s{kanaInput}; s.next(kana, false);) {
    // check prolong and repeating marks first since they aren't in 'sourceMap'
    if (kana == Kana::ProlongMark)
      // prolong is 'katakana', but it can appear in (non-standard) Hiragana.
      done(DoneType::Prolong);
    else if (Kana::RepeatPlain.matches(source, kana)) {
      done(DoneType::NewEmptyGroup);
      result += Kana::RepeatPlain.get(_target, _flags, prevKana);
    } else if (Kana::RepeatAccented.matches(source, kana)) {
      done(DoneType::NewEmptyGroup);
      result += Kana::RepeatAccented.get(_target, _flags, prevKana);
    } else if (Kana::getMap(source).contains(kana)) {
      if (Kana::SmallTsu.containsKana(kana))
        // getting a small tsu causes any stored kana to be processed
        done(DoneType::NewGroup, State::SmallTsu);
      else if (Kana::N.containsKana(kana))
        // getting an 'n' causes any stored kana to be processed
        done(DoneType::NewGroup, State::Done); // new group marked as 'Done'
      else if (state == State::Done)
        done();
      else if (smallKana(source).contains(kana)) {
        // a small letter should cause letters to be processed including the
        // small letter so mark group as done, but continue the loop in case
        // there's a 'prolong' mark.
        kanaGroup += kana;
        state = State::Done;
      } else if (kanaGroup.size() >
                 (state == State::SmallTsu ? Kana::OneKanaSize : 0))
        // a normal (non-n, non-small) letter can't form the second part of a
        // digraph so process any stored previous letter and hold processing of
        // the new letter in case it forms the first part of a new digraph.
        done();
      else
        kanaGroup += kana;
    } else {
      // got non-hiragana letter so flush any letters and preserve the new
      // letter unconverted
      done(DoneType::NewEmptyGroup);
      if (romajiTarget())
        if (const auto i{wideDelims().find(kana)}; i != wideDelims().end()) {
          result += i->second;
          continue;
        }
      result += kana;
    }
  }
  return result + processKana(kanaGroup, source, prevKana);
}

std::string KanaConvert::processKana(const std::string& kanaGroup,
    CharType source, const Kana*& prevKana, bool prolong) const {
  if (!kanaGroup.empty()) {
    prevKana = nullptr;
    auto& sourceMap{Kana::getMap(source)};
    if (const auto i{sourceMap.find(kanaGroup)}; i != sourceMap.end())
      return processKanaMacron(prolong, prevKana, i->second);
    // if letter group is an unknown, split it up and try processing each part
    if (kanaGroup.size() > Kana::OneKanaSize) {
      const auto firstKana{kanaGroup.substr(0, Kana::OneKanaSize)};
      if (const auto i{sourceMap.find(kanaGroup.substr(Kana::OneKanaSize))};
          i != sourceMap.end())
        return romajiTarget() && Kana::SmallTsu.containsKana(firstKana) &&
                       repeatingConsonents().contains(i->second->romaji()[0])
                   ? processKanaMacron(prolong, prevKana, i->second, true)
                   : processKana(firstKana, source, prevKana) +
                         processKanaMacron(prolong, prevKana, i->second);
      // LCOV_EXCL_START
      // return second part unconverted - this should be impossible by design
      // since only Kana that exists in sourceMap are added to 'kanaGroup'
      return processKana(firstKana, source, prevKana) +
             kanaGroup.substr(Kana::OneKanaSize);
      // LCOV_EXCL_STOP
    }
  } else if (prolong)
    // a 'prolong mark' at the start of a group isn't valid so in this case just
    // return the symbol unchanged
    return Kana::ProlongMark;
  return kanaGroup;
}

std::string KanaConvert::processKanaMacron(
    bool prolong, const Kana*& prevKana, const Kana* kana, bool sokuon) const {
  const auto& result{sokuon ? kana->getSokuonRomaji(_flags) : get(*kana)};
  if (prolong) {
    if (_target != CharType::Romaji) return result + Kana::ProlongMark;
    switch (result[result.size() - 1]) {
    case 'a': return result.substr(0, result.size() - 1) + "ā";
    case 'i': return result.substr(0, result.size() - 1) + "ī";
    case 'u': return result.substr(0, result.size() - 1) + "ū";
    case 'e': return result.substr(0, result.size() - 1) + "ē";
    case 'o': return result.substr(0, result.size() - 1) + "ō";
    default:
      return result + Kana::ProlongMark; // shouldn't happen, return unconverted
    }
  }
  prevKana = kana;
  return result;
}

std::string KanaConvert::convertToKana(const std::string& romajiInput) const {
  std::string result, letters, letter;
  for (MBChar s{romajiInput}; s.next(letter, false);)
    if (isSingleByte(letter)) {
      if (!isN(letter)) {
        letters += letter;
        processRomaji(letters, result);
      } else if (letters.empty())
        letters += letter;
      else if (isN(letters))
        // got two 'n's in a row so output one, but don't clear letters
        result += getN();
      else {
        // error: partial romaji followed by n - output uncoverted partial group
        result += letters;
        letters = letter; // 'n' starts a new group
      }
    } else if (!processRomajiMacron(letter, letters, result)) {
      processRomaji(letters, result);
      result += letter;
    }
  while (!letters.empty())
    if (isN(letters)) {
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
  std::string lower{toLower(letters)};
  if (const auto i{sourceMap.find(lower)}; i != sourceMap.end()) {
    result += get(*i->second);
    letters.clear();
  } else if (letters.size() == Kana::RomajiStringMaxSize) {
    // convert first letter to small tsu if letter repeats and is a valid
    // consonant (also allow 'tc' combination) otherwise output the first letter
    // unconverted since no valid romaji can be longer than 3 letters
    result += lower[0] == 'n' ? getN()
              : lower[0] == lower[1] || lower[0] == 't' && lower[1] == 'c'
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
    else {
      // non-empty 'letters' (after the above call to 'processRomaji') can only
      // happen for bad input like 'vyī', i.e., a bad Rōmaji group followed by a
      // macron. In this case add unconverted letters and treat the macron as a
      // single vowel (above case would become 'vyい' for Hiragana target)
      letters.pop_back();
      result += letters;
      processRomaji(letters = i->second.first, result);
    }
    return true;
  }
  // not being found in 'Macrons' map can happen when processing Rōmaji input
  // and a non-macron multi-byte character is found (like Kana for example)
  return false;
}

} // namespace kanji_tools
