#include <kanji_tools/kana/Kana.h>
#include <kanji_tools/kana/MBChar.h>

namespace kanji_tools {

bool MBChar::next(std::string& result, bool onlyMB) {
  const auto combiningMark = [this](const auto& r, const auto& i) {
    _location += 3;
    if (i) {
      ++_combiningMarks;
      return *i;
    }
    ++_errors;
    return r;
  };
  while (*_location) {
    const unsigned char firstOfGroup = *_location;
    if (unsigned char x = firstOfGroup & TwoBits; !x || x == Bit2) { // not a multi byte character
      if (!onlyMB) {
        result = *_location++;
        return true;
      }
      ++_location; // skip regular ascii when onlyMB is true
    } else if (isValidMBUtf8(_location, false)) {
      std::string r({*_location++});
      for (x = Bit2; x && firstOfGroup & x; x >>= 1) r += *_location++;
      if (isVariationSelector(r) || isCombiningMark(r))
        ++_errors; // can't start with a variation selector or a combining mark
      else {
        if (std::string s; doPeek(s, onlyMB, _location, true) && isVariationSelector(s)) {
          _location += 3;
          ++_variants;
          result = r + s;
        } else
          result = s == CombiningVoiced ? combiningMark(r, Kana::findDakuten(r))
            : s == CombiningSemiVoiced  ? combiningMark(r, Kana::findHanDakuten(r))
                                        : r;
        return true;
      }
    } else { // _location doesn't start a valid utf8 sequence so try next byte
      ++_location;
      ++_errors;
    }
  }
  return false;
}

bool MBChar::doPeek(std::string& result, bool onlyMB, const char* location, bool internalCall) const {
  const auto combiningMark = [](const auto& r, const auto& i) { return i ? *i : r; };
  while (*location) {
    const unsigned char firstOfGroup = *location;
    if (unsigned char x = firstOfGroup & TwoBits; !x || x == Bit2) { // not a multi byte character
      if (internalCall) return false;
      if (!onlyMB) {
        result = *location;
        return true;
      }
      ++location;
    } else if (isValidMBUtf8(location, false)) {
      // only modify 'result' if 'location' is the start of a valid UTF-8 group
      std::string r({*location++});
      for (x = Bit2; x && firstOfGroup & x; x >>= 1) r += *location++;
      if (internalCall) {
        result = r;
        return true;
      } else if (!isVariationSelector(r) && !isCombiningMark(r)) {
        if (std::string s; doPeek(s, onlyMB, location, true) && isVariationSelector(s))
          result = r + s;
        else
          result = s == CombiningVoiced ? combiningMark(r, Kana::findDakuten(r))
            : s == CombiningSemiVoiced  ? combiningMark(r, Kana::findHanDakuten(r))
                                        : r;
        return true;
      }
    } else
      ++location;
  }
  return false;
}

} // namespace kanji_tools
