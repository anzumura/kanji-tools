#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace kanji_tools {

class Radical {
public:
  using AltForms = std::vector<std::string>;

  Radical(u_int8_t number, const std::string& name, const AltForms& altForms,
      const std::string& longName, const std::string& reading)
      : _number(number), _name(name), _altForms(altForms), _longName(longName),
        _reading(reading) {}

  Radical(const Radical&) = default;
  Radical& operator=(const Radical&) = default;

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
  u_int8_t _number;
  std::string _name;
  AltForms _altForms;
  std::string _longName;
  std::string _reading;
};

inline auto& operator<<(std::ostream& os, const Radical& r) {
  return os << '[' << std::right << std::setfill('0') << std::setw(3)
            << r.number() << "] " << r.name();
}

} // namespace kanji_tools
