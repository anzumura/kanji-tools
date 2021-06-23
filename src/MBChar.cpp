#include <kanji/MBChar.h>

namespace kanji {

bool MBChar::getNext(std::string& result, bool onlyMB) {
  for (; *_location; ++_location) {
    const unsigned char firstOfGroup = *_location;
    unsigned char x = firstOfGroup & HighTwoBitsMask;
    if (!x || x == HighSecondBit) { // not a multi byte character
      if (!onlyMB) {
        result = *_location++;
        return true;
      }
    } else if (x != ContinuationPattern) { // shouldn't be a continuation, but check to be safe
      result = *_location++;
      for (x = HighSecondBit; x && firstOfGroup & x; x >>= 1)
        result += *_location++;
      return true;
    }
  }
  return false;
}

} // namespace kanji
