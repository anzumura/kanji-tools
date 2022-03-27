#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace kanji_tools {

class Radical {
public:
  using AltForms = std::vector<std::string>;
  using Number = u_int8_t;

  Radical(Number number, const std::string& name, const AltForms& altForms,
      const std::string& longName, const std::string& reading)
      : _number{number}, _name{name}, _altForms{altForms}, _longName{longName},
        _reading{reading} {}

  Radical(const Radical&) = default;
  // operator= is not generated since there are const members

  [[nodiscard]] auto operator==(const Radical& rhs) const {
    return _number == rhs._number;
  }
  [[nodiscard]] auto operator<(const Radical& rhs) const {
    return _number < rhs._number;
  }

  [[nodiscard]] auto number() const { return _number; }
  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto& altForms() const { return _altForms; }
  [[nodiscard]] auto& longName() const { return _longName; }
  [[nodiscard]] auto& reading() const { return _reading; }
private:
  const Number _number;
  const std::string _name;
  const AltForms _altForms;
  const std::string _longName;
  const std::string _reading;
};

inline auto& operator<<(std::ostream& os, const Radical& r) {
  return os << '[' << std::right << std::setfill('0') << std::setw(3)
            << r.number() << "] " << r.name();
}

} // namespace kanji_tools
