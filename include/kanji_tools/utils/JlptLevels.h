#ifndef KANJI_TOOLS_UTILS_JLPT_LEVELS_H
#define KANJI_TOOLS_UTILS_JLPT_LEVELS_H

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// JLPT (Japanese Language Proficiency Test) Levels: None=not a JLPT kanji
enum class JlptLevels { N5, N4, N3, N2, N1, None };
template<> inline constexpr bool is_enumarray_with_none<JlptLevels> = true;
inline const auto AllJlptLevels =
  BaseEnumArray<JlptLevels>::create("N5", "N4", "N3", "N2", "N1");

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_JLPT_LEVELS_H
