#pragma once

#include <kanji_tools/utils/String.h>

#include <iostream>
#include <vector>

namespace kanji_tools {

class Radical {
public:
  using AltForms = std::vector<String>;
  using Number = uint16_t;
  // some type aliases to help make parameter lists shorter and clearer
  using Name = const String&;
  using Reading = const String&;

  inline static constexpr Number MaxRadicals{214};

  Radical(Number, Name, const AltForms&, const String& longName, Reading);

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
  const String _name;
  const AltForms _altForms;
  const String _longName;
  const String _reading;
};

using RadicalRef = const Radical&;

std::ostream& operator<<(std::ostream&, RadicalRef);

} // namespace kanji_tools
