#ifndef KANJI_RADICAL_H
#define KANJI_RADICAL_H

#include <iostream>
#include <string>
#include <vector>

namespace kanji {

class Radical {
public:
  using AltForms = std::vector<std::string>;
  Radical(int number, const std::string& radical, const AltForms& altForms, const std::string& name,
          const std::string& reading)
    : _number(number), _radical(radical), _altForms(altForms), _name(name), _reading(reading) {}
  Radical(const Radical&) = default;
  Radical& operator=(const Radical&) = default;
  bool operator==(const Radical& rhs) const { return _number == rhs._number; }
  bool operator<(const Radical& rhs) const { return _number < rhs._number; }

  int number() const { return _number; }
  const std::string& radical() const { return _radical; }
  const AltForms& altForms() const { return _altForms; }
  const std::string& name() const { return _name; }
  const std::string& reading() const { return _reading; }
private:
  int _number;
  std::string _radical;
  AltForms _altForms;
  std::string _name;
  std::string _reading;
};

inline std::ostream& operator<<(std::ostream& os, const Radical& r) {
  return os << '[' << std::right << std::setfill('0') << std::setw(3) << r.number() << "] " << r.radical();
}

} // namespace kanji

#endif // KANJI_RADICAL_H
