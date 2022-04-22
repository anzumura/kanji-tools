#include <kanji_tools/kanji/Strokes.h>

namespace kanji_tools {

namespace {

void rangeError(Strokes::Size value, const std::string& msg = {}) {
  throw std::range_error{
      msg + "strokes '" + std::to_string(value) + "' out of range"};
}

} // namespace

Strokes::Strokes(Size value, Size variant) : _value{value}, _variant{variant} {
  if (!value || value > Max) rangeError(value);
  if (variant == 1 || variant > MaxVariant) rangeError(variant, "variant ");
  if (value == variant)
    throw std::domain_error{"strokes and variant strokes are the same '" +
                            std::to_string(value) + "'"};
}

std::string Strokes::toString(bool includeVariant) const {
  std::string result{std::to_string(_value)};
  if (includeVariant && _variant) {
    result += '/';
    result += std::to_string(_variant);
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const Strokes& s) {
  return os << s.toString();
}

} // namespace kanji_tools
