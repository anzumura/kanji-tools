#include <kt_kanji/Radical.h>

#include <iomanip>

namespace kanji_tools {

Radical::Radical(Number number, Name name, const AltForms& altForms,
    const String& longName, Reading reading)
    : _number{number}, _name{name}, _altForms{altForms}, _longName{longName},
      _reading{reading} {}

bool Radical::operator==(RadicalRef rhs) const noexcept {
  return _number == rhs._number;
}

bool Radical::operator<(RadicalRef rhs) const noexcept {
  return _number < rhs._number;
}

std::ostream& operator<<(std::ostream& os, RadicalRef r) {
  return os << '[' << std::right << std::setfill('0') << std::setw(3)
            << r.number() << "] " << r.name();
}

} // namespace kanji_tools
