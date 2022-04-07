#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace kanji_tools {

class Radical {
public:
  using AltForms = std::vector<std::string>;
  using Number = u_int8_t;

  Radical(Number number, const std::string& name, const AltForms& altForms,
      const std::string& longName, const std::string& reading);

  Radical(const Radical&) = default;
  // operator= is not generated since there are const members

  [[nodiscard]] bool operator==(const Radical&) const;
  [[nodiscard]] bool operator<(const Radical&) const;

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

std::ostream& operator<<(std::ostream&, const Radical&);

} // namespace kanji_tools
