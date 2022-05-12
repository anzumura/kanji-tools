#include <kanji_tools/kana/Kana.h>
#include <kanji_tools/kana/MBChar.h>

namespace kanji_tools {

bool MBChar::isVariationSelector(const unsigned char* s) {
  // Checking for variation selectors would be easier if 'i' was a wide char
  // type (like 'Code'), but that would involve calling expensive conversion
  // functions (like fromUtf8). Note, variation selectors are range 'fe00' to
  // 'fe0f' in Unicode which is '0xef 0xb8 0x80' to '0xef 0xb8 0x8f' in UTF-8.
  static constexpr auto B1{0xef}, B2{0xb8}, B3Start{0x80}, B3End{0x8f};
  return s && *s++ == B1 && *s++ == B2 && *s >= B3Start && *s <= B3End;
}

bool MBChar::isVariationSelector(const char* s) {
  return isVariationSelector(reinterpret_cast<const unsigned char*>(s));
}

bool MBChar::isVariationSelector(const String& s) {
  return isVariationSelector(s.c_str());
}

bool MBChar::isCombiningMark(const unsigned char* s) {
  static constexpr auto B1{0xe3}, B2{0x82}, B3_1{0x99}, B3_2{0x9a};
  return s && *s++ == B1 && *s++ == B2 && (*s == B3_1 || *s == B3_2);
}

bool MBChar::isCombiningMark(const char* s) {
  return isCombiningMark(reinterpret_cast<const unsigned char*>(s));
}

bool MBChar::isCombiningMark(const String& s) {
  return isCombiningMark(s.c_str());
}

size_t MBChar::size(const char* s, bool onlyMB) {
  // use '11 00 00 00' to grab the first two bits and only adding if the result
  // is not binary '10 00 00 00'). Examples:
  // - size("abc") = 0
  // - size("abc", false) = 3
  // - size("大blue空") = 2
  // - size("大blue空", false) = 6
  // Note: some Kanji can be followed by a 'variation selector' or 'combining
  // mark' - these are not counted since they are considered part of the
  // previous 'MB character' (as a modifier).

  size_t result{};
  // a 'reinterpret_cast' at the beginning saves a bunch of static_casts when
  // checking if the next 3 bytes represent a 'variation selector'
  if (auto i{reinterpret_cast<const unsigned char*>(s)}; i) {
    while (*i)
      if (isCombiningMark(i) || isVariationSelector(i))
        i += VarSelectorSize;
      else if (onlyMB)
        result += (*i++ & TwoBits) == TwoBits;
      else
        result += (*i++ & TwoBits) != Bit1;
  }
  return result;
}

size_t MBChar::size(const String& s, bool onlyMB) {
  return size(s.c_str(), onlyMB);
}

bool MBChar::isMBCharWithVariationSelector(const String& s) {
  // Variation selectors are 3 bytes and min UTF-8 multi-byte char is 2 bytes so
  // a character must be at least 5 long to include a variation selector. Also,
  // max UTF-8 char len is 4 so a single character with a selector can't be more
  // than 7 bytes.
  static constexpr auto MinSize{VarSelectorSize + MinMBSize},
      MaxSize{VarSelectorSize + MaxMBSize};
  return s.size() >= MinSize && s.size() <= MaxSize &&
         isVariationSelector(s.substr(s.size() - VarSelectorSize));
}

String MBChar::noVariationSelector(const String& s) {
  return isMBCharWithVariationSelector(s)
             ? s.substr(0, s.size() - VarSelectorSize)
             : s;
}

String MBChar::getFirst(const String& s) {
  String result;
  MBChar c{s};
  c.next(result);
  return result;
}

void MBChar::reset() {
  _curLocation = _data.c_str();
  _errors = _variants = _combiningMarks = 0;
}

bool MBChar::next(String& result, bool onlyMB) {
  while (*_curLocation) {
    switch (validateMBUtf8(_curLocation)) {
    case MBUtf8Result::NotMultiByte:
      if (!onlyMB) {
        result = *_curLocation++;
        return true;
      }
      ++_curLocation; // skip regular ascii when onlyMB is true
      break;
    case MBUtf8Result::Valid:
      if (String curChar; validResult(curChar, _curLocation)) {
        if (String nexChar; peekVariant(nexChar, _curLocation)) {
          _curLocation += VarSelectorSize;
          ++_variants;
          result = curChar + nexChar;
        } else
          result = processOne(*this, curChar, nexChar);
        return true;
      }
      ++_errors; // can't start with a variation selector or a combining mark
      break;
    case MBUtf8Result::NotValid:
      // _loc doesn't start a valid utf8 sequence so try next byte
      ++_curLocation;
      ++_errors;
    }
  }
  return false;
}

bool MBChar::peek(String& result, bool onlyMB) const {
  for (auto location{_curLocation}; *location;) {
    switch (validateMBUtf8(location)) {
    case MBUtf8Result::NotMultiByte:
      if (!onlyMB) {
        result = *location;
        return true;
      }
      ++location;
      break;
    case MBUtf8Result::Valid:
      if (String curChar; validResult(curChar, location)) {
        if (String nextChar; peekVariant(nextChar, location))
          result = curChar + nextChar;
        else
          result = processOne(*this, curChar, nextChar);
        return true;
      }
      break;
    case MBUtf8Result::NotValid: ++location;
    }
  }
  return false;
}

size_t MBChar::size(bool onlyMB) const { return size(_data, onlyMB); }

MBUtf8Result MBChar::valid(bool sizeOne) const {
  return validateMBUtf8(_data, sizeOne);
}

bool MBChar::isValid(bool sizeOne) const {
  return valid(sizeOne) == MBUtf8Result::Valid;
}

String MBChar::getMBUtf8(const char*& loc) {
  const auto firstOfGroup{toUChar(*loc)};
  String result{*loc++};
  for (unsigned char x{Bit2}; x && firstOfGroup & x; x >>= 1U) result += *loc++;
  return result;
}

bool MBChar::validResult(String& result, const char*& loc) {
  return !isVariationSelector(result = getMBUtf8(loc)) &&
         !isCombiningMark(result);
}

bool MBChar::peekVariant(String& result, const char* loc) {
  return isValidMBUtf8(loc) && isVariationSelector(result = getMBUtf8(loc));
}

template<typename T>
String MBChar::processOne(T& t, const String& cur, const String& next) {
  return next == CombiningVoiced ? t.combiningMark(cur, Kana::findDakuten(cur))
         : next == CombiningSemiVoiced
             ? t.combiningMark(cur, Kana::findHanDakuten(cur))
             : cur;
}

// NOLINTNEXTLINE: keep this non-static (for now) to allow overloading by const
String MBChar::combiningMark(
    const String& base, const OptString& accented) const {
  return accented.value_or(base);
}

String MBChar::combiningMark(const String& base, const OptString& accented) {
  _curLocation += VarSelectorSize;
  if (accented) {
    ++_combiningMarks;
    return *accented;
  }
  ++_errors;
  return base;
}

} // namespace kanji_tools
