#pragma once

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// JLPT (Japanese Language Proficiency Test) Levels: None=not a JLPT kanji
enum class JlptLevels : BaseEnum::Size { N5, N4, N3, N2, N1, None };

template<> inline constexpr auto is_enumarray_with_none<JlptLevels>{true};

inline const auto AllJlptLevels{
    TypedEnumArray<JlptLevels>::create("N5", "N4", "N3", "N2", "N1")};

} // namespace kanji_tools
