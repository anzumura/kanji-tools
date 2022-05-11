#pragma once

#include <kanji_tools/utils/EnumList.h>

namespace kanji_tools {

// 'KanjiGrades' represents the official school grade for all Jouyou kanji.
// S=secondary school, None=not Jouyou
enum class KanjiGrades : EnumContainer::Size {
  G1,
  G2,
  G3,
  G4,
  G5,
  G6,
  S,
  None
};

template<> inline constexpr auto is_enumlist_with_none<KanjiGrades>{true};

inline const auto AllKanjiGrades{
    BaseEnumList<KanjiGrades>::create("G1", "G2", "G3", "G4", "G5", "G6", "S")};

} // namespace kanji_tools
