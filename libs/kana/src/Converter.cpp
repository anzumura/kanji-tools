#include <kanji_tools/kana/Converter.h>
#include <kanji_tools/kana/Utf8Char.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <sstream>

namespace kanji_tools {

Converter::Tokens::Tokens() : _narrowDelimList{Apostrophe, Dash} {
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
  populateDelimLists();
  verifyData();
}

void Converter::Tokens::populateDelimLists() {
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
           // Ascii A-Z - LCOV_EXCL_START
           D{'[', "「"}, D{'\\', "￥"}, D{']', "」"}, D{'^', "＾"},
           D{'_', "＿"}, D{'`', "｀"},
           // Ascii a-z - LCOV_EXCL_STOP
           D{'{', "『"}, D{'|', "｜"}, D{'}', "』"}, D{'~', "〜"}}) {
    _narrowDelimList += i.narrow;
    _narrowDelims[i.narrow] = i.wide;
    _wideDelims[i.wide] = i.narrow;
  }
}

void Converter::Tokens::verifyData() const {
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

const Converter::Tokens& Converter::tokens() {
  static const Tokens tokens;
  return tokens;
}

Converter::Converter(CharType target, ConvertFlags flags)
    : _target{target}, _flags{flags} {}

String Converter::flagString() const {
  if (_flags == ConvertFlags::None) return "None";
  String result;
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

String Converter::convert(const String& input) const {
  String result{input};
  for (const auto i : CharTypes)
    if (_target != i) result = convert(i, result);
  return result;
}

String Converter::convert(
    const String& input, CharType target, ConvertFlags flags) {
  _target = target;
  _flags = flags;
  return convert(input);
}

String Converter::convert(
    CharType source, const String& input, CharType target, ConvertFlags flags) {
  _target = target;
  _flags = flags;
  return convert(source, input);
}

String Converter::convert(CharType source, const String& input) const {
  if (source == _target) return input;
  if (source == CharType::Hiragana) return fromKana(input, source);
  if (source == CharType::Katakana) return fromKana(input, source);
  // For Romaji source, break into words separated by any of _narrowDelimList
  // and process each word. This helps deal with words ending in 'n'.
  String result;
  size_t oldPos{};
  for (const auto keepSpaces{!(_flags & ConvertFlags::RemoveSpaces)};;) {
    const auto pos{input.find_first_of(tokens().narrowDelimList(), oldPos)};
    if (pos == String::npos) {
      result += toKana(input.substr(oldPos));
      break;
    }
    result += toKana(input.substr(oldPos, pos - oldPos));
    if (const auto delim{input[pos]};
        delim != Apostrophe && delim != Dash && (keepSpaces || delim != ' '))
      result += narrowDelims().at(delim);
    oldPos = pos + 1;
  }
  return result;
}

String Converter::fromKana(const String& kanaInput, CharType source) const {
  State state{State::New};
  String result, kanaGroup, kana;
  const Kana* prevKana{};

  // 'done' is called to process the Kana built up in 'kanaGroup'. By default it
  // starts a new group containing 'kana' (the current symbol being processed).
  const auto done{[this, source, &state, &result, &kanaGroup, &kana, &prevKana](
                      DoneType dt = DoneType::NewGroup, State ns = State::New) {
    // NOLINTNEXTLINE: NonNullParamChecker
    result += processKana(kanaGroup, source, prevKana, dt == DoneType::Prolong);
    if (romajiTarget() && Kana::N.containsKana(kanaGroup) &&
        afterN(source).contains(kana))
      result += Apostrophe;
    kanaGroup = dt == DoneType::NewGroup ? kana : EmptyString;
    state = ns;
  }};

  for (Utf8Char s{kanaInput}; s.next(kana, false);) {
    // check prolong and repeating marks first since they aren't in 'sourceMap'
    if (kana == Kana::ProlongMark)
      // prolong is 'katakana', but it can appear in (non-standard) Hiragana.
      done(DoneType::Prolong);
    else if (const auto repeat{Kana::findRepeatMark(source, kana)}; repeat) {
      done(DoneType::NewEmptyGroup);
      result += repeat->get(_target, _flags, prevKana);
    } else if (Kana::getMap(source).contains(kana)) {
      if (!processOneKana(done, source, kana, kanaGroup, state))
        kanaGroup += kana;
    } else {
      // got non-kana so flush any letters and preserve new letter unconverted
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

template<typename T>
bool Converter::processOneKana(const T& done, CharType source,
    const String& kana, const String& kanaGroup, State& state) const {
  if (Kana::SmallTsu.containsKana(kana))
    // getting a small tsu causes any stored kana to be processed
    done(DoneType::NewGroup, State::SmallTsu);
  else if (Kana::N.containsKana(kana))
    // getting an 'n' causes any stored kana to be processed
    done(DoneType::NewGroup, State::Done); // new group marked as 'Done'
  else {
    if (state != State::Done) {
      if (smallKana(source).contains(kana)) {
        // a small letter (other than small tsu covered above) should cause
        // letters to be processed including the small letter so mark group as
        // done, but continue processing in case there's a 'prolong' mark.
        state = State::Done;
        return false;
      }
      if (kanaGroup.size() <=
          (state == State::SmallTsu ? Kana::OneKanaSize : 0))
        // keep processing for a normal (non-n non-small) letter if it's the
        // first part of a group (or the group starts with a small tsu)
        return false;
    }
    done();
  }
  return true;
}

String Converter::processKana(const String& kanaGroup, CharType source,
    const Kana*& prevKana, bool prolong) const {
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
      // return second part unconverted - this should be impossible by design
      // since only Kana that exists in sourceMap are added to 'kanaGroup'
      // XCOV_EXCL_START
      return processKana(firstKana, source, prevKana) +
             kanaGroup.substr(Kana::OneKanaSize);
      // XCOV_EXCL_STOP
    }
  } else if (prolong)
    // a 'prolong mark' at the start of a group isn't valid so in this case just
    // return the symbol unchanged
    return Kana::ProlongMark;
  return kanaGroup;
}

String Converter::processKanaMacron(
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

String Converter::toKana(const String& romajiInput) const {
  String result, letters, letter;
  for (Utf8Char s{romajiInput}; s.next(letter, false);)
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

void Converter::processRomaji(String& letters, String& result) const {
  auto& sourceMap{Kana::getMap(CharType::Romaji)};
  String lower{toLower(letters)};
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

bool Converter::processRomajiMacron(
    const String& letter, String& letters, String& result) const {
  static const std::map<String, std::pair<char, String>> Macrons{
      {"ā", {'a', "あ"}}, {"ī", {'i', "い"}}, // GCOV_EXCL_LINE
      {"ū", {'u', "う"}}, {"ē", {'e', "え"}}, {"ō", {'o', "お"}}};

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
