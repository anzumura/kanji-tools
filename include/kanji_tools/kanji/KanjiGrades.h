#ifndef KANJI_TOOLS_KANJI_KANJI_GRADES_H
#define KANJI_TOOLS_KANJI_KANJI_GRADES_H

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// 'KanjiGrades' represents the official school grade for all Jouyou kanji
enum class KanjiGrades { G1, G2, G3, G4, G5, G6, S, None }; // S=secondary school, None=not Jouyou
template<> inline constexpr bool is_enumarray<KanjiGrades> = true;
inline const auto AllKanjiGrades = BaseEnumArray<KanjiGrades>::create("G1", "G2", "G3", "G4", "G5", "G6", "S");

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_KANJI_GRADES_H
