#include <kanji_tools/kanji/Radical.h>

#include <iomanip>

namespace kanji_tools {

Radical::Radical(Number number, const std::string& name,
    const AltForms& altForms, const std::string& longName,
    const std::string& reading)
    : _number{number}, _name{name}, _altForms{altForms}, _longName{longName},
      _reading{reading} {}

bool Radical::operator==(const Radical& rhs) const {
  return _number == rhs._number;
}

bool Radical::operator<(const Radical& rhs) const {
  return _number < rhs._number;
}

std::ostream& operator<<(std::ostream& os, const Radical& r) {
  return os << '[' << std::right << std::setfill('0') << std::setw(3)
            << static_cast<int>(r.number()) << "] " << r.name();
}

} // namespace kanji_tools