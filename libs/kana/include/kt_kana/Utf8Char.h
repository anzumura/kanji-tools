#pragma once

#include <kt_utils/Utf8.h>

#include <optional>

namespace kanji_tools { /// \kana_group{Utf8Char}
/// Utf8Char class for working with UTF-8 strings

/// provide functions for iterating, tracking variant and error counts and
/// converting %Kana Combining Marks in UTF-8 strings \kana{Utf8Char}
///
/// Create Utf8Char from a String and call next() to get one 'character' at a
/// time (with support for variation selectors and combining marks). Use reset()
/// to iterate again and also reset tracking counts.
///
/// Note on UTF-8 structure:
/// \li UTF-8 uses 1 to 4 bytes per character, depending on the Unicode symbol
/// \li if high bit is 0 then it's a single byte value (so normal case)
/// \li if high bits are 10 then it's a continuation byte (of a multi-byte seq)
/// \li otherwise it's the first byte of a multi-byte sequence. The number of
///   leading '1's indicates how many bytes follow, i.e.: 110 means 2 bytes,
///   1110 means 3, etc.
class Utf8Char final {
public:
  using OptString = std::optional<String>;

  /// return true if the first UTF-8 value in `s` is a Variation Selector (used
  /// by size(), next() and peek() methods) @{
  [[nodiscard]] static bool isVariationSelector(const uint8_t* s);
  [[nodiscard]] static bool isVariationSelector(const char* s);
  [[nodiscard]] static bool isVariationSelector(const String& s); ///@}

  /// return true if the fist UTF-8 value in `s` is a Kana Combining Mark (used
  /// by size(), next() and peek() methods) @{
  [[nodiscard]] static bool isCombiningMark(const uint8_t* s);
  [[nodiscard]] static bool isCombiningMark(const char* s);
  [[nodiscard]] static bool isCombiningMark(const String& s); ///@}

  /// return number of UTF-8 characters in a string
  /// \param s input string
  /// \param onlyMB if true (the default) then only count 'multi-byte' UTF-8
  ///     characters, otherwise single-byte UTF-8 characters are also counted
  /// \return the logical size of a UTF-8 string
  /// \note variation selectors and combining marks are not counted since they
  ///     are considered to be modifiers of the previous character @{
  /// \details Examples:
  /// \li `size("abc")` returns `0`
  /// \li `size("abc", false)` returns `3`
  /// \li `size("大blue空")` returns `2`
  /// \li `size("大blue空", false)` returns `6`
  [[nodiscard]] static size_t size(const char* s, bool onlyMB = true);
  [[nodiscard]] static size_t size(const String& s, bool onlyMB = true); ///@}

  /// return true if `s` is a single Utf8Char (so 2-4 bytes) followed by a
  /// recognized variation selector (which are always 3 bytes).
  [[nodiscard]] static bool isCharWithVariationSelector(const String& s);

  /// return a copy of `s` with variation selector removed (if it has one)
  [[nodiscard]] static String noVariationSelector(const String& s);

  /// return one UTF-8 character from the start of `s` (including any variation
  /// selector that may follow) or empty string if `s` doesn't start with a
  /// 'multi-byte' UTF-8 character
  [[nodiscard]] static String getFirst(const String&);

  /// create a Utf8Char object from a String
  explicit Utf8Char(const String&);

  Utf8Char(const Utf8Char&) = delete; ///< deleted copy ctor

  /// resets location and counters (call to allow iterating again)
  void reset();

  /// get the next UTF-8 character
  /// \param[out] result populated with the next character potentially including
  ///     a variation selector
  /// \param onlyMB if true (the default) then skip single-byte UTF-8 characters
  /// \return true if `result` was populated
  /// \note when a UTF-8 character is added to `result` the next character is
  /// also inspected and if it's a variation selector it will be added as well.
  /// Plain %Kana followed 'Combining Marks' (U+3099, U+309A) are converted to
  /// single values, i.e., U+306F (は) + U+3099 maps to U+3070 (ば).
  bool next(String& result, bool onlyMB = true);

  /// works like next(), but doesn't update internal state
  [[nodiscard]] bool peek(String& result, bool onlyMB = true) const;

  /// number of errors (invalid UTF-8 sequences) found in calls to next() method
  [[nodiscard]] auto errors() const { return _errors; }

  /// number of Variation Selectors found in calls to next() method
  [[nodiscard]] auto variants() const { return _variants; }

  /// number of Kana Combining Marks found in calls to next() method
  [[nodiscard]] auto combiningMarks() const { return _combiningMarks; }

  /// return size (of string passed to ctor), see static size() function
  [[nodiscard]] size_t size(bool onlyMB = true) const;

  /// validate string passed to ctor by calling validateMBUtf8()
  [[nodiscard]] MBUtf8Result valid(bool sizeOne = true) const;

  /// return true if string passed to ctor is a valid UTF-8 string
  [[nodiscard]] bool isValid(bool sizeOne = true) const;

private:
  /// return one multi-byte UTF-8 character starting at `loc`, `loc` is moved
  /// forward by 2 to 4 bytes (depending on the size of the value returned)
  [[nodiscard]] static String getMBUtf8(const char*& loc);

  /// called from next() and peek() after determining `loc` points to a valid
  /// multi-byte UTF-8 sequence
  /// \param[out] result next UTF-8 character
  /// \param loc location to pass to getMBUtf8()
  /// \return true if `result` is not a variation selector or combining mark
  [[nodiscard]] static bool validResult(String& result, const char*& loc);

  /// called from next() and peek() methods to check if the next value (after
  /// the one being processed) is a variation selector
  /// \param[out] result next UTF-8 character (only set if `loc` points to a
  ///     valid UTF-8 character)
  /// \param loc location to validate
  /// \return true if `loc` points to a valid multi-byte UTF-8 character and
  ///    the character is a variation selector
  [[nodiscard]] static bool peekVariant(String& result, const char* loc);

  /// returns a String containing a single %Kana character if `next` is a
  /// combining mark, otherwise return `cur` (calls combiningMark() functions)
  /// \tparam T Utf8Char type (either const or non-const)
  template <typename T>
  [[nodiscard]] static String processOne(
      T&, const String& cur, const String& next);

  /// return `accented` if it's defined, otherwise return 'base'
  [[nodiscard]] String combiningMark(
      const String& base, const OptString& accented) const;

  /// \doc combiningMark
  /// also increments combining mark count if `accented` is defined, otherwise
  /// increments error count
  [[nodiscard]] String combiningMark(
      const String& base, const OptString& accented);

  const String _data;
  const char* _curLocation{_data.c_str()};
  // counts of errors, variants and combiningMarks found
  size_t _errors{}, _variants{}, _combiningMarks{};
};

/// \end_group
} // namespace kanji_tools
