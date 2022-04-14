#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace kanji_tools {

class Radical {
public:
  using AltForms = std::vector<std::string>;
  using Number = u_int16_t;
  // some type aliases to help make parameter lists shorter and clearer
  using Name = const std::string&;
  using Reading = const std::string&;

  inline static constexpr Number MaxRadicals{214};

  Radical(Number, Name, const AltForms&, const std::string& longName, Reading);

  Radical(const Radical&) = default;

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

using RadicalRef = const Radical&;

std::ostream& operator<<(std::ostream&, RadicalRef);

} // namespace kanji_tools
