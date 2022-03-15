#include <kanji_tools/kana/Kana.h>
#include <kanji_tools/kana/MBChar.h>

namespace kanji_tools {

bool MBChar::next(std::string& result, bool onlyMB) {
  const auto combiningMark{[this](const auto& r, const auto& i) {
    _location += 3;
    if (i) {
      ++_combiningMarks;
      return *i;
    }
    ++_errors;
    return r;
  }};
  while (*_location) {
    switch (validateMBUtf8(_location)) {
    case MBUtf8Result::NotMultiByte:
      if (!onlyMB) {
        result = *_location++;
        return true;
      }
      ++_location; // skip regular ascii when onlyMB is true
      break;
    case MBUtf8Result::Valid:
      if (std::string r; validResult(r, _location)) {
        if (std::string s; peekVariant(s, _location)) {
          _location += 3;
          ++_variants;
          result = r + s;
        } else
          result = s == CombiningVoiced ? combiningMark(r, Kana::findDakuten(r))
                   : s == CombiningSemiVoiced
                     ? combiningMark(r, Kana::findHanDakuten(r))
                     : r;
        return true;
      }
      ++_errors; // can't start with a variation selector or a combining mark
      break;
    case MBUtf8Result::NotValid:
      // _location doesn't start a valid utf8 sequence so try next byte
      ++_location;
      ++_errors;
    }
  }
  return false;
}

bool MBChar::peek(std::string& result, bool onlyMB) const {
  const auto combiningMark{
    [](const auto& r, const auto& i) { return i ? *i : r; }};
  for (auto location{_location}; *location;) {
    switch (validateMBUtf8(location)) {
    case MBUtf8Result::NotMultiByte:
      if (!onlyMB) {
        result = *location;
        return true;
      }
      ++location;
      break;
    case MBUtf8Result::Valid:
      if (std::string r; validResult(r, location)) {
        if (std::string s; peekVariant(s, location))
          result = r + s;
        else
          result = s == CombiningVoiced ? combiningMark(r, Kana::findDakuten(r))
                   : s == CombiningSemiVoiced
                     ? combiningMark(r, Kana::findHanDakuten(r))
                     : r;
        return true;
      }
      break;
    case MBUtf8Result::NotValid: ++location;
    }
  }
  return false;
}

} // namespace kanji_tools
