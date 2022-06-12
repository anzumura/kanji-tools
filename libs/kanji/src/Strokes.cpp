#include <kt_kanji/Strokes.h>
#include <kt_utils/Exception.h>

namespace kanji_tools {

namespace {

void rangeError(Strokes::Size value, const String& msg = {}) {
  throw RangeError{
      msg + "strokes '" + std::to_string(value) + "' out of range"};
}

} // namespace

Strokes::Strokes(Size value) : _value{value}, _variant{} {
  if (!value || value > Max) rangeError(value);
}

Strokes::Strokes(Size value, Size variant) : _value{value}, _variant{variant} {
  if (value < 2 || value > Max) rangeError(value);
  if (variant < 3 || variant > MaxVariant) rangeError(variant, "variant ");
  if (value == variant)
    throw DomainError{"strokes and variant strokes are the same '" +
                      std::to_string(value) + "'"};
}

String Strokes::toString(bool includeVariant) const {
  String result{std::to_string(_value)};
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
