#pragma once

#include <kanji_tools/utils/String.h>

#include <compare>
#include <iostream>
#include <numeric>

namespace kanji_tools { /// \kanji_group{MorohashiId}
/// MorohashiId class

/// class representing a Morohashi IDs ('Dai Kan-Wa Jiten') \kanji{MorohashiId}
///
/// Unicode 14.0 has ~18K distinct values in 'kMorohashi' property. Most entries
/// are plain numbers and 379 are numbers followed by a single quote (these are
/// called 'Prime') - the max is 49,867. There's a proposal to add most of the
/// missing entries (Dai Kan-Wa Jiten has 51,284) which also includes several
/// hundred 'DoublePrime' and 'Supplemental' entries. This class supports the
/// new proposal by internally storing an unsigned #Id and an #IdType.
class MorohashiId {
public:
  using Id = uint16_t;

  enum class IdType : uint8_t { Plain, Prime, DoublePrime, Supplemental };

  static constexpr Id MaxId{std::numeric_limits<Id>::max()};

  /// default ctor creates an 'empty' MorohashiId (meaning 'doesn't have an id')
  constexpr MorohashiId() noexcept = default;

  /// create a MorohashiId from a String
  /// \param s positive number (up to #MaxId) optionally followed by a single
  ///     quote or a 'P' for Prime, two single quotes or 'PP' for DoublePrime or
  ///     prefixed with a 'H' for Supplemental (補巻)
  /// \details `s` can have leading zeroes (which are removed), but can't be all
  /// zeroes followed by a suffix. A 'zero' id is supported for now and treated
  /// as an empty (missing) id since UCD data does this for a few entries.
  /// \note Only Plain and Prime values are in the current version of Unicode.
  /// \throw DomainError if `s` is malformed
  explicit MorohashiId(const String& s);

  [[nodiscard]] constexpr auto id() const noexcept { return _id; }
  [[nodiscard]] constexpr auto idType() const noexcept { return _idType; }
  [[nodiscard]] explicit constexpr operator bool() const noexcept {
    return _id;
  }
  [[nodiscard]] constexpr auto operator<=>(
      const MorohashiId&) const noexcept = default; // NOLINT: nullptr

  [[nodiscard]] String toString() const;
private:
  /// functions used by ctor to validate and populate _id and _idType @{
  [[nodiscard]] static Id getId(const String&);
  [[nodiscard]] static IdType getIdType(const String&);
  [[nodiscard]] static Id validate(const String&, size_t = 0, size_t = 0); ///@}

  const Id _id{};
  const IdType _idType{IdType::Plain};
};

std::ostream& operator<<(std::ostream&, const MorohashiId&);

/// \end_group
} // namespace kanji_tools
