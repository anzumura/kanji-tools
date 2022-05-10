#pragma once

#include <kanji_tools/utils/String.h>

#include <compare>
#include <iostream>
#include <numeric>

namespace kanji_tools {

// Unicode 14.0 has ~18K distinct values in 'kMorohashi' property with most
// entries being plain numbers (max is 49,867). 379 are numbers followed by a
// single quote (referred to as 'Prime'). There is a proposal to add several
// hundred 'DoublePrime' and 'Supplemental' types as well as add most of the
// missing plain entries (Dai Kan-Wa Jiten has 51,284 total entries). This class
// encapsulates this by internally storing an unsigned 'id' and an 'IdType'.
class MorohashiId {
public:
  using Id = uint16_t;

  enum class IdType : uint8_t { Regular, Prime, DoublePrime, Supplemental };

  static constexpr Id MaxId{std::numeric_limits<Id>::max()};

  constexpr MorohashiId() noexcept = default;

  // ctor expects a string that's a positive number (up to MaxId) optionally
  // followed by a single quote or a 'P' for Prime, two single quotes or 'PP'
  // for DoublePrime or prefixed with a 'H' for Supplemental (補巻). Note, only
  // plain and Prime values are in the current version of Unicode.
  explicit MorohashiId(const String&);

  [[nodiscard]] constexpr auto id() const noexcept { return _id; }
  [[nodiscard]] constexpr auto idType() const noexcept { return _idType; }
  [[nodiscard]] explicit constexpr operator bool() const noexcept {
    return _id;
  }
  [[nodiscard]] constexpr auto operator<=>(
      const MorohashiId&) const noexcept = default; // NOLINT: nullptr

  [[nodiscard]] String toString() const;
private:
  // helper functions used by ctor to validate and populate '_id' and '_idType'
  [[nodiscard]] static Id getId(const String&);
  [[nodiscard]] static IdType getIdType(const String&);
  [[nodiscard]] static Id validate(const String&, size_t = 0, size_t = 0);

  const Id _id{};
  const IdType _idType{IdType::Regular};
};

std::ostream& operator<<(std::ostream&, const MorohashiId&);

} // namespace kanji_tools
