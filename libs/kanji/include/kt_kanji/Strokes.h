#pragma once

#include <kt_utils/String.h>

#include <compare>
#include <iostream>

namespace kanji_tools { /// \kanji_group{Strokes}
/// Strokes class

/// class for Kanji stroke counts (画数) \kanji{Strokes}
class Strokes {
public:
  using Size = uint8_t;

  /// max number of strokes and variant strokes found in current 'ucd.txt' data
  /// for example, 9F98 (龘) has 48 strokes and 2C6A9 has 53 strokes @{
  static constexpr Size Max{53}, MaxVariant{33};
  ///@}

  /// ctor for Strokes with a one stroke count
  /// \param value stroke count
  /// \throw RangeError if values is 0 or greater than #Max
  explicit Strokes(Size value);

  /// ctor for Strokes with two stroke counts
  /// \param value main (or 'more common') stroke count
  /// \param variant a secondary stroke count (comes from UCD data and is only
  ///     set for some Kanji with stroke counts loaded from 'ucd.txt'
  /// \throw RangeError if `value` isn't between 2 and #Max or `variant` isn't
  ///     between 3 and #MaxVariant
  /// \throw DomainError if `value` is the same as `variant`
  Strokes(Size value, Size variant);

  [[nodiscard]] constexpr Size value() const noexcept { return _value; }
  [[nodiscard]] constexpr Size variant() const noexcept { return _variant; }
  [[nodiscard]] constexpr bool hasVariant() const noexcept { return _variant; }

  [[nodiscard]] constexpr auto operator<=>(
      const Strokes&) const noexcept = default; // NOLINT: nullptr

  /// by default return a string containing value(), but if `includeVariant` is
  /// true and variant() is non-zero then return 'value()/variant()'
  [[nodiscard]] String toString(bool includeVariant = false) const;
private:
  const Size _value, _variant;
};

std::ostream& operator<<(std::ostream&, const Strokes&);

/// \end_group
} // namespace kanji_tools
