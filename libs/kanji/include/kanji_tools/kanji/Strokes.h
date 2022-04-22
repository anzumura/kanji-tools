#pragma once

#include <compare>
#include <iostream>

namespace kanji_tools {

class Strokes {
public:
  using Size = u_int8_t;

  // max number of strokes and variant strokes found in current 'ucd.txt' data
  // for example, 9F98 (龘) has 48 strokes and 2C6A9 has 53 strokes
  static constexpr Size Max{53}, MaxVariant{33};

  // 'value' must be between 1 and 'Max'
  explicit Strokes(Size value);

  // 'value' must be between 2 and 'Max' and 'variant' should be between 3 and
  // 'MaxVariant' and must be different than 'value'
  Strokes(Size value, Size variant);

  [[nodiscard]] constexpr Size value() const noexcept { return _value; }
  [[nodiscard]] constexpr Size variant() const noexcept { return _variant; }
  [[nodiscard]] constexpr bool hasVariant() const noexcept { return _variant; }

  [[nodiscard]] constexpr auto operator<=>(
      const Strokes&) const noexcept = default;

  // by default return a string containing '_value', but if 'includeVariant' is
  // set to true and '_variant' is non-zero then return '_value/_variant'
  [[nodiscard]] std::string toString(bool includeVariant = false) const;
private:
  const Size _value, _variant;
};

std::ostream& operator<<(std::ostream&, const Strokes&);

} // namespace kanji_tools
