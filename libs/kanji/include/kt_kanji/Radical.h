#pragma once

#include <kt_utils/String.h>

#include <iostream>
#include <vector>

namespace kanji_tools { /// \kanji_group{Radical}
/// Radical class for Kanji radicals

/// class that represents an official Kanji Radical (部首) \kanji{Radical}
///
/// Example code to create 'water' radical:
/// \code
///   Radical water{85, "水", {"氵", "氺"}, "水部（すいぶ）",
///       "みず さんずい したみず"};
/// \endcode
class Radical final {
public:
  using AltForms = std::vector<String>;
  using Number = uint16_t;
  // some type aliases to help make parameter lists shorter and clearer
  using Name = const String&;
  using Reading = const String&;

  inline static constexpr Number MaxRadicals{214};

  /// ctor for creating a Radical
  /// \param number official radical number (1 to 214)
  /// \param name standard radical name
  /// \param altForms list of alterative forms
  /// \param longName long name in Japanese
  /// \param reading space-separated Japanese readings
  Radical(Number number, Name name, const AltForms& altForms,
      const String& longName, Reading reading);

  Radical(const Radical&) = default; ///< copy ctor

  /// equal operator
  [[nodiscard]] bool operator==(const Radical&) const noexcept;

  /// less-than operator, compares 'number' field
  [[nodiscard]] bool operator<(const Radical&) const noexcept;

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

/// \end_group
} // namespace kanji_tools
