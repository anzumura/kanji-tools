#pragma once

#include <kt_kanji/Radical.h>

#include <filesystem>
#include <map>
#include <memory>

namespace kanji_tools { /// \kanji_group{RadicalData}
/// RadicalData class for loading data from 'radicals.txt'

/// load, store, find and print Radical objects \kanji{RadicalData}
class RadicalData final {
public:
  using Map = std::map<String, Radical::Number>;
  using List = std::vector<Radical>;

  RadicalData() = default; ///< default ctor

  RadicalData(const RadicalData&) = delete;    ///< deleted copy ctor
  auto operator=(const RadicalData&) = delete; ///< deleted operator=

  /// find Radical by `name`
  /// \param name Japanese Kanji 'name' of the Radical
  /// \details For example, Radical '30 (口)' is found using "口" (U+53E3) from
  /// the usual "CJK Unified" Unicode block as opposed to using "⼝" (U+2F1D)
  /// from the "Kangxi Radicals" block.
  /// \return const reference to Radical
  /// \throw DomainError if not found
  [[nodiscard]] RadicalRef find(Radical::Name name) const;

  /// find Radical by `number`
  /// \param number the official Radical number (1 - 214)
  /// \return const reference to Radical
  /// \throw DomainError if not found
  [[nodiscard]] RadicalRef find(Radical::Number number) const;

  /// load radicals from `file`
  void load(const std::filesystem::path& file);

  /// print example 'Common Kanji' from `data` for each Radical (sorted by
  /// ascending stroke count)
  void print(const class KanjiData& data) const;

private:
  /// used by print() to control the max number of examples printed per Radical
  static constexpr auto MaxExamples{12};

  using KanjiList = std::vector<std::shared_ptr<class Kanji>>;
  using RadicalLists = std::map<Radical, KanjiList>;

  static void printRadicalLists(const KanjiData&, RadicalLists&);

  void checkLoaded() const;

  void printMissingRadicals(const KanjiData&, const RadicalLists&) const;

  /// when populated from 'radicals.txt' the index in the vector is one less
  /// than the actual Radical.number().
  List _radicals;

  /// maps from the Radical name (ideograph) to the index in #_radicals.
  Map _map;
};

/// \end_group
} // namespace kanji_tools
