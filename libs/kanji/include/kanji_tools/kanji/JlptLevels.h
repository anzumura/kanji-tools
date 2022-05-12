#pragma once

#include <kanji_tools/utils/EnumList.h>

namespace kanji_tools {

// JLPT (Japanese Language Proficiency Test) Levels: None=not a JLPT kanji
enum class JlptLevels : EnumContainer::Size { N5, N4, N3, N2, N1, None };

template<> inline constexpr auto is_enumlist_with_none<JlptLevels>{true};

inline const auto AllJlptLevels{
    BaseEnumList<JlptLevels>::create("N5", "N4", "N3", "N2", "N1")};

} // namespace kanji_tools
