#pragma once

#include <kanji_tools/utils/MBUtils.h>

#include <optional>

namespace kanji_tools {

// MBChar is a helper class for working with UTF-8 strings. Create MBChar from a
// string and then call 'next' to get one 'character' at a time. 'next' will
// return false once the end of the original string is reached. Use 'reset' to
// iterate again.
//
// Note on UTF-8 structure:
// - UTF-8 uses 1 to 4 bytes per character, depending on the Unicode symbol
// - if high bit is 0 then it's a single byte value (so normal case)
// - if high bits are 10 then it's a continuation byte (of a multi-byte seq)
// - Otherwise it's the first byte of a multi-byte sequence. The number of
//   leading '1's indicates how many bytes follow, i.e.: 110 means 2 bytes, 1110
//   means 3, etc.
class MBChar {
public:
  using OptString = std::optional<std::string>;

  // 'isVariationSelector' returns true if s points to a UTF-8 variation
  // selector, this method is used by 'size', 'next' and 'doPeek'.
  [[nodiscard]] static bool isVariationSelector(const unsigned char* s);
  [[nodiscard]] static bool isVariationSelector(const char* s);
  [[nodiscard]] static bool isVariationSelector(const std::string& s);

  [[nodiscard]] static bool isCombiningMark(const unsigned char* s);
  [[nodiscard]] static bool isCombiningMark(const char* s);
  [[nodiscard]] static bool isCombiningMark(const std::string& s);

  // 'size' with onlyMB=true only counts multi-byte 'sequence start' bytes,
  // otherwise it includes both multi-byte sequence starts as well as regular
  // single byte values.
  [[nodiscard]] static size_t size(const char* s, bool onlyMB = true);
  [[nodiscard]] static size_t size(const std::string& s, bool onlyMB = true);

  // 'isMBCharWithVariationSelector' returns true if 's' is a single MBChar (so
  // 2-4 bytes) followed by a variation selector (which are always 3 bytes).
  [[nodiscard]] static bool isMBCharWithVariationSelector(const std::string&);

  // return copy of given string with variation selector removed (if it has one)
  [[nodiscard]] static std::string noVariationSelector(const std::string&);

  // 'getFirst' returns the first MBChar from 's' (including any variation
  // selector that might follow). If 's' doesn't start with a multi-byte
  // sequence then empty string is returned.
  [[nodiscard]] static std::string getFirst(const std::string&);

  explicit MBChar(const std::string& data) : _data{data} {}

  MBChar(const MBChar&) = delete;

  // call reset in order to loop over the string again
  void reset();

  // 'next' populates 'result' with the full multi-byte character (so could be
  // more than one byte) returns true if result was populated. This function
  // also supports 'variation selectors', i.e., when a multi-byte character is
  // added to 'result' the next character is also inspected and if it's a
  // variation selector it will be added as well. Plain Kana followed by
  // 'Combining Marks' (U+3099, U+309A) are converted to single values, i.e.,
  // U+306F (は) + U+3099 maps to U+3070 (ば).
  bool next(std::string& result, bool onlyMB = true);

  // 'peek' works like 'next', but doesn't update state.
  [[nodiscard]] bool peek(std::string& result, bool onlyMB = true) const;

  [[nodiscard]] auto errors() const { return _errors; }
  [[nodiscard]] auto variants() const { return _variants; }
  [[nodiscard]] auto combiningMarks() const { return _combiningMarks; }
  [[nodiscard]] size_t size(bool onlyMB = true) const;
  [[nodiscard]] MBUtf8Result valid(bool sizeOne = true) const;
  [[nodiscard]] bool isValid(bool sizeOne = true) const;
private:
  // 'getMBUtf8' returns a string containing one multi-byte UTF-8 sequence
  // starting at 'loc'
  [[nodiscard]] static std::string getMBUtf8(const char*& loc);

  // 'validResult' is called from 'next' and 'peek' after determining 'location'
  // points to a valid multi-byte utf8 sequence. It sets 'result', increments
  // 'loc' and returns true if the result is valid, i.e., not a 'variation
  // selector' or a 'combining mark'.
  [[nodiscard]] static bool validResult(std::string& result, const char*& loc);

  // 'peekVariant' is called from 'next' and 'peek' methods. It populates
  // 'result' if 'location' starts a valid multi-byte utf8 sequence and returns
  // true if 'result' is a 'variation selector'.
  [[nodiscard]] static bool peekVariant(std::string& result, const char* loc);

  // 'processOne' returns a single Kana character if 'next' is a combining mark
  // otherwise returns 'cur' (it calls the below 'combiningMark' functions)
  template<typename T>
  [[nodiscard]] static std::string processOne(
      T&, const std::string& cur, const std::string& next);

  // return 'accented' Kana if it's defined, otherwise return 'base' (non-const
  // overload updates '_curLocation' as well as '_combiningMarks' or '_errors')
  [[nodiscard]] std::string combiningMark(
      const std::string& base, const OptString& accented) const;
  [[nodiscard]] std::string combiningMark(
      const std::string& base, const OptString& accented);

  const std::string _data;
  const char* _curLocation{_data.c_str()};
  // counts of errors, variants and combiningMarks found
  size_t _errors{}, _variants{}, _combiningMarks{};
};

} // namespace kanji_tools
