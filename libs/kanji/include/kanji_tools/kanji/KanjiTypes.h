#pragma once

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// 'KanjiTypes' is used to identify which official group (Jouyou or Jinmei) a
// kanji belongs to (or has a link to) as well as a few more groups for less
// common kanji:
// - Jouyou: 2136 official Jouyou kanji
// - Jinmei: 633 official Jinmei kanji
// - LinkedJinmei: 230 more Jinmei kanji that are old/variant forms of Jouyou
//   (212) or Jinmei (18)
// - LinkedOld: old/variant Jouyou kanji that aren't in 'LinkedJinmei'
// - Frequency: kanji in top 2501 frequency list and in the above types
// - Extra: kanji loaded from 'extra.txt' - shouldn't be any of the above types
// - Kentei: kanji loaded from 'kentei/k*.txt' files that aren't included above
// - Ucd: kanji loaded from 'ucd.txt' file that aren't included above
// - None: used as a type for a kanji that haven't been loaded
enum class KanjiTypes {
  Jouyou,
  Jinmei,
  LinkedJinmei,
  LinkedOld,
  Frequency,
  Extra,
  Kentei,
  Ucd,
  None
};

template<> inline constexpr auto is_enumarray_with_none<KanjiTypes>{true};

inline const auto AllKanjiTypes{
    TypedEnumArray<KanjiTypes>::create("Jouyou", "Jinmei", "LinkedJinmei",
        "LinkedOld", "Frequency", "Extra", "Kentei", "Ucd")};

} // namespace kanji_tools
