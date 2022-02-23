#ifndef KANJI_TOOLS_KANJI_KANJI_TYPES_H
#define KANJI_TOOLS_KANJI_KANJI_TYPES_H

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// 'KanjiTypes' is used to identify which official group (Jouyou or Jinmei) a kanji belongs to (or has a
// link to) as well as a few more groups for less common kanji:
// - Jouyou: 2136 official Jouyou kanji
// - Jinmei: 633 official Jinmei kanji
// - LinkedJinmei: 230 more Jinmei kanji that are old/variant forms of Jouyou (212) or Jinmei (18)
// - LinkedOld: old/variant Jouyou kanji that aren't in 'LinkedJinmei'
// - Frequency: kanji that are in the top 2501 frequency list, but not one of the first 4 types
// - Extra: kanji loaded from 'extra.txt' - shouldn't be any of the above types
// - Kentei: kanji loaded from 'kentei/k*.txt' files that aren't in any of the above types
// - Ucd: kanji loaded from 'ucd.txt' file that aren't in any of the above types
// - None: used as a type for a kanji that hasn't been loaded
enum class KanjiTypes { Jouyou, Jinmei, LinkedJinmei, LinkedOld, Frequency, Extra, Kentei, Ucd, None };
template<> inline constexpr bool is_enumarray_with_none<KanjiTypes> = true;
inline const auto AllKanjiTypes = BaseEnumArray<KanjiTypes>::create("Jouyou", "Jinmei", "LinkedJinmei", "LinkedOld",
                                                                    "Frequency", "Extra", "Kentei", "Ucd");

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_KANJI_TYPES_H
