#ifndef KANJI_TOOLS_KANA_MBCHAR_H
#define KANJI_TOOLS_KANA_MBCHAR_H

#include <kanji_tools/utils/MBUtils.h>

#include <optional>

namespace kanji_tools {

// MBChar is a helper class for working with UTF-8 strings. Create MBChar from a string
// and then call 'next' to get one 'character' at a time. 'next' will return false
// once the end of the original string is reached. Use 'reset' to iterate again.
//
// Note on UTF-8 structure:
// - UTF-8 uses 1 to 4 bytes per character, depending on the Unicode symbol
// - if high bit is 0 then it's a single byte value (so normal case)
// - if two high bits are 10 then it's a continuation byte (of a multi-byte sequence)
// - Otherwise it's the first byte of a multi-byte sequence. The number of leading '1's indicates
//   how many bytes are in the sequence, i.e.: 110 means 2 bytes, 1110 means 3, etc.
class MBChar {
public:
  // 'isVariationSelector' returns true if s points to a UTF-8 variation selector, this
  // method is used by 'length', 'next' and 'doPeek'.
  [[nodiscard]] static auto isVariationSelector(const unsigned char* s) {
    // Checking for variation selectors would be easier if 'i' was char32_t, but that would involve
    // calling more expensive conversion functions (like fromUtf8). Note, variation selectors are
    // range 'fe00' to 'fe0f' in Unicode which is '0xef 0xb8 0x80' to '0xef 0xb8 0x8f' in UTF-8.
    return s && *s++ == 0xef && *s++ == 0xb8 && *s >= 0x80 && *s <= 0x8f;
  }
  [[nodiscard]] static auto isVariationSelector(const char* s) {
    return isVariationSelector(reinterpret_cast<const unsigned char*>(s));
  }
  [[nodiscard]] static auto isVariationSelector(const std::string& s) { return isVariationSelector(s.c_str()); }

  inline static const auto CombiningVoiced = std::string("\xe3\x82\x99");     // U+3099
  inline static const auto CombiningSemiVoiced = std::string("\xe3\x82\x9a"); // U+309A

  [[nodiscard]] static auto isCombiningMark(const unsigned char* s) {
    return s && *s++ == 0xe3 && *s++ == 0x82 && (*s == 0x99 || *s == 0x9a);
  }
  [[nodiscard]] static auto isCombiningMark(const char* s) {
    return isCombiningMark(reinterpret_cast<const unsigned char*>(s));
  }
  [[nodiscard]] static auto isCombiningMark(const std::string& s) { return isCombiningMark(s.c_str()); }

  // 'length' with onlyMB=true only counts multi-byte 'sequence start' bytes, otherwise length
  // includes both multi-byte sequence starts as well as regular single byte values, i.e.,
  // simply don't add 'continuation' bytes to length (this done by using '11 00 00 00' to grab the
  // first two bits and only adding to length if the result is not binary '10 00 00 00'). Examples:
  // - length("abc") = 0
  // - length("abc", false) = 3
  // - length("大blue空") = 2
  // - length("大blue空", false) = 6
  // Note: some Kanji can be followed by a 'variation selector' or 'combining mark' - these are not
  // counted since they are considered part of the previous 'MB character' (as a modifier).
  [[nodiscard]] static auto length(const char* s, bool onlyMB = true) {
    int len = 0;
    // doing one 'reinterpret_cast' at the beginning saves doing a bunch of static_casts when checking
    // if the next 3 bytes represent a 'variation selector'
    if (auto i = reinterpret_cast<const unsigned char*>(s); i) {
      while (*i)
        if (isCombiningMark(i) || isVariationSelector(i))
          i += 3;
        else if (onlyMB)
          len += (*i++ & TwoBits) == TwoBits;
        else
          len += (*i++ & TwoBits) != Bit1;
    }
    return len;
  }
  [[nodiscard]] static auto length(const std::string& s, bool onlyMB = true) { return length(s.c_str(), onlyMB); }

  // 'isMBCharWithVariationSelector' returns true if 's' is a single MBChar (so len 2-4) followed
  // by a variation selector (which are always len 3).
  [[nodiscard]] static auto isMBCharWithVariationSelector(const std::string& s) {
    return s.length() > 4 && s.length() < 8 && isVariationSelector(s.substr(s.length() - 3));
  }
  [[nodiscard]] static auto withoutVariationSelector(const std::string& s) {
    return isMBCharWithVariationSelector(s) ? s.substr(0, s.length() - 3) : s;
  }
  [[nodiscard]] static auto optionalWithoutVariationSelector(const std::string& s) {
    return isMBCharWithVariationSelector(s) ? std::optional(s.substr(0, s.length() - 3)) : std::nullopt;
  }

  // 'getFirst' returns the first MBChar from 's' (including any variation selector that might follow).
  // If 's' doesn't start with a multi-byte sequence then empty string is returned.
  [[nodiscard]] static auto getFirst(const std::string& s) {
    std::string result;
    MBChar c(s);
    c.next(result);
    return result;
  }

  explicit MBChar(const std::string& data) : _data(data) {}

  // call reset in order to loop over the string again
  void reset() {
    _location = _data.c_str();
    _errors = 0;
    _variants = 0;
    _combiningMarks = 0;
  }

  // 'next' populates 'result' with the full multi-byte character (so could be more than one byte)
  // returns true if result was populated. This function also supports 'variation selectors', i.e.,
  // when a multi-byte character is added to 'result' the next character is also inspected and if
  // it's a variation selector it will be added as well. Plain Kana followed by 'Combining Marks'
  // (U+3099, U+309A) are converted to single values, i.e., U+306F (は) + U+3099 maps to U+3070 (ば).
  bool next(std::string& result, bool onlyMB = true);

  // 'peek' works like 'next', but it doesn't update state (like _location or _errors).
  [[nodiscard]] bool peek(std::string& result, bool onlyMB = true) const;

  [[nodiscard]] auto errors() const { return _errors; }
  [[nodiscard]] auto variants() const { return _variants; }
  [[nodiscard]] auto combiningMarks() const { return _combiningMarks; }
  [[nodiscard]] auto length(bool onlyMB = true) const { return length(_data, onlyMB); }
  [[nodiscard]] auto valid(bool checkLengthOne = true) const { return validateMBUtf8(_data, checkLengthOne); }
  [[nodiscard]] auto isValid(bool checkLengthOne = true) const { return valid(checkLengthOne) == MBUtf8Result::Valid; }
private:
  // 'getMBUtf8' returns a string containing one multi-byte UTF-8 sequence starting at 'location'
  [[nodiscard]] static auto getMBUtf8(const char*& location) {
    const unsigned char firstOfGroup = *location;
    std::string result({*location++});
    for (unsigned char x = Bit2; x && firstOfGroup & x; x >>= 1) result += *location++;
    return result;
  }

  // 'validResult' is called from 'next' and 'peek' after determining 'location' points to a
  // valid multi-byte utf8 sequence. It sets 'result', increments 'location' and returns true
  // if the result is valid, i.e., not a 'variation selector' or a 'combining mark'.
  [[nodiscard]] static auto validResult(std::string& result, const char*& location) {
    return !isVariationSelector(result = getMBUtf8(location)) && !isCombiningMark(result);
  }

  // 'peekVariant' is called from 'next' and 'peek' methods. It populates 'result' if 'location'
  // starts a valid multi-byte utf8 sequence and returns true if 'result' is a 'variation selector'.
  [[nodiscard]] static auto peekVariant(std::string& result, const char* location) {
    return isValidMBUtf8(location) && isVariationSelector(result = getMBUtf8(location));
  }

  const std::string _data;
  const char* _location = _data.c_str();
  int _errors = 0;         // '_errors' keeps track of how many invalid bytes were encountered during iteration
  int _variants = 0;       // '_variants' keeps track of how many 'Variation Selector's were found
  int _combiningMarks = 0; // '_variants' keeps track of how many 'Combining Marks's were found
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANA_MBCHAR_H