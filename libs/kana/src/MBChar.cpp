#include <kanji_tools/kana/Kana.h>
#include <kanji_tools/kana/MBChar.h>

namespace kanji_tools {

bool MBChar::isVariationSelector(const unsigned char* s) {
  // Checking for variation selectors would be easier if 'i' was char32_t, but
  // that would involve calling more expensive conversion functions (like
  // fromUtf8). Note, variation selectors are range 'fe00' to 'fe0f' in
  // Unicode which is '0xef 0xb8 0x80' to '0xef 0xb8 0x8f' in UTF-8.
  return s && *s++ == 0xef && *s++ == 0xb8 && *s >= 0x80 && *s <= 0x8f;
}

bool MBChar::isVariationSelector(const char* s) {
  return isVariationSelector(reinterpret_cast<const unsigned char*>(s));
}

bool MBChar::isVariationSelector(const std::string& s) {
  return isVariationSelector(s.c_str());
}

bool MBChar::isCombiningMark(const unsigned char* s) {
  return s && *s++ == 0xe3 && *s++ == 0x82 && (*s == 0x99 || *s == 0x9a);
}

bool MBChar::isCombiningMark(const char* s) {
  return isCombiningMark(reinterpret_cast<const unsigned char*>(s));
}

bool MBChar::isCombiningMark(const std::string& s) {
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
        i += 3;
      else if (onlyMB)
        result += (*i++ & TwoBits) == TwoBits;
      else
        result += (*i++ & TwoBits) != Bit1;
  }
  return result;
}

size_t MBChar::size(const std::string& s, bool onlyMB) {
  return size(s.c_str(), onlyMB);
}

bool MBChar::isMBCharWithVariationSelector(const std::string& s) {
  return s.size() > 4 && s.size() < 8 &&
         isVariationSelector(s.substr(s.size() - 3));
}

std::string MBChar::noVariationSelector(const std::string& s) {
  return isMBCharWithVariationSelector(s) ? s.substr(0, s.size() - 3) : s;
}

MBChar::OptString MBChar::optNoVariationSelector(const std::string& s) {
  return isMBCharWithVariationSelector(s)
             ? std::optional(s.substr(0, s.size() - 3))
             : std::nullopt;
}

std::string MBChar::getFirst(const std::string& s) {
  std::string result;
  MBChar c{s};
  c.next(result);
  return result;
}

void MBChar::reset() {
  _loc = _data.c_str();
  _errors = _variants = _combiningMarks = 0;
}

bool MBChar::next(std::string& result, bool onlyMB) {
  const auto combiningMark{[this](const auto& r, const auto& i) {
    _loc += 3;
    if (i) {
      ++_combiningMarks;
      return *i;
    }
    ++_errors;
    return r;
  }};
  while (*_loc) {
    switch (validateMBUtf8(_loc)) {
    case MBUtf8Result::NotMultiByte: // LCOV_EXCL_LINE - missed by Clang
      if (!onlyMB) {
        result = *_loc++;
        return true;
      }
      ++_loc; // skip regular ascii when onlyMB is true
      break;
    case MBUtf8Result::Valid:
      if (std::string r; validResult(r, _loc)) {
        if (std::string s; peekVariant(s, _loc)) {
          _loc += 3;
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
      // _loc doesn't start a valid utf8 sequence so try next byte
      ++_loc;
      ++_errors;
    }
  }
  return false;
}

bool MBChar::peek(std::string& result, bool onlyMB) const {
  const auto combiningMark{
      [](const auto& r, const auto& i) { return i ? *i : r; }};
  for (auto location{_loc}; *location;) {
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

size_t MBChar::size(bool onlyMB) const { return size(_data, onlyMB); }

MBUtf8Result MBChar::valid(bool sizeOne) const {
  return validateMBUtf8(_data, sizeOne);
}

bool MBChar::isValid(bool sizeOne) const {
  return valid(sizeOne) == MBUtf8Result::Valid;
}

std::string MBChar::getMBUtf8(const char*& loc) {
  const auto firstOfGroup{static_cast<unsigned char>(*loc)};
  std::string result{*loc++};
  for (unsigned char x{Bit2}; x && firstOfGroup & x; x >>= 1) result += *loc++;
  return result;
}

bool MBChar::validResult(std::string& result, const char*& loc) {
  return !isVariationSelector(result = getMBUtf8(loc)) &&
         !isCombiningMark(result);
}

bool MBChar::peekVariant(std::string& result, const char* loc) {
  return isValidMBUtf8(loc) && isVariationSelector(result = getMBUtf8(loc));
}

} // namespace kanji_tools
