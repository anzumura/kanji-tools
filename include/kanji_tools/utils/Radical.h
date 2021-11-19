#ifndef KANJI_TOOLS_UTILS_RADICAL_H
#define KANJI_TOOLS_UTILS_RADICAL_H

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace kanji_tools {

class Radical {
public:
  using AltForms = std::vector<std::string>;
  Radical(int number, const std::string& name, const AltForms& altForms, const std::string& longName,
          const std::string& reading)
    : _number(number), _name(name), _altForms(altForms), _longName(longName), _reading(reading) {}
  Radical(const Radical&) = default;
  Radical& operator=(const Radical&) = default;
  bool operator==(const Radical& rhs) const { return _number == rhs._number; }
  bool operator<(const Radical& rhs) const { return _number < rhs._number; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }
  const AltForms& altForms() const { return _altForms; }
  const std::string& longName() const { return _longName; }
  const std::string& reading() const { return _reading; }
private:
  int _number;
  std::string _name;
  AltForms _altForms;
  std::string _longName;
  std::string _reading;
};

inline std::ostream& operator<<(std::ostream& os, const Radical& r) {
  return os << '[' << std::right << std::setfill('0') << std::setw(3) << r.number() << "] " << r.name();
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_RADICAL_H
