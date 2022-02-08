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
  for (; *_location; ++_location) {
    const unsigned char firstOfGroup = *_location;
    if (unsigned char x = firstOfGroup & TwoBits; !x || x == Bit2) { // not a multi byte character
      if (!onlyMB) {
        result = *_location++;
        return true;
      }
    } else if (isValidMBUtf8(_location, false)) {
      // only modify 'result' if '_location' is the start of a valid UTF-8 group
      std::string r({*_location++});
      for (x = Bit2; x && firstOfGroup & x; x >>= 1) r += *_location++;
      if (!isVariationSelector(r)) {
        if (std::string s; doPeek(s, onlyMB, _location, true) && isVariationSelector(s)) {
          _location += 3;
          ++_variants;
          result = r + s;
        } else
          result = s == CombiningVoiced ? combiningMark(r, Kana::findDakuten(r))
            : s == CombiningSemiVoiced  ? combiningMark(r, Kana::findHanDakuten(r))
                                        : r;
      }
      return true;
    } else
      ++_errors;
  }
  return false;
}

bool MBChar::doPeek(std::string& result, bool onlyMB, const char* location, bool internalCall) const {
  for (; *location; ++location) {
    const unsigned char firstOfGroup = *location;
    if (unsigned char x = firstOfGroup & TwoBits; !x || x == Bit2) { // not a multi byte character
      if (internalCall) return false;
      if (!onlyMB) {
        result = *location;
        return true;
      }
    } else if (isValidMBUtf8(location, false)) {
      // only modify 'result' if 'location' is the start of a valid UTF-8 group
      result = *location++;
      for (x = Bit2; x && firstOfGroup & x; x >>= 1) result += *location++;
      if (!internalCall && !isVariationSelector(result))
        if (std::string s; doPeek(s, onlyMB, location, true) && isVariationSelector(s)) result += s;
      return true;
    }
  }
  return false;
}

} // namespace kanji_tools
