#ifndef KANJI_TOOLS_KANJI_KANJI_TYPES_H
#define KANJI_TOOLS_KANJI_KANJI_TYPES_H

#include <array>
#include <iostream>

namespace kanji_tools {

// 'KanjiTypes' is used to identify which official group (Jouyou or Jinmei) a kanji belongs to (or has a
// link to) as well as a few more groups for less common kanji:
// - Jouyou: 2136 official Jouyou kanji
// - Jinmei: 633 official Jinmei kanji
// - LinkedJinmei: 230 more Jinmei kanji that are old/variant forms of Jouyou (212) or Jinmei (18)
// - LinkedOld: old/variant Jouyou kanji that aren't in 'LinkedJinmei'
// - Other: kanji that are in the top 2501 frequency list, but not one of the first 4 types
// - Extra: kanji loaded from 'extra.txt' - shouldn't be any of the above types
// - Kentei: kanji loaded from 'kentei/k*.txt' files that aren't in any of the above types
// - None: used as a type for a kanji that hasn't been loaded
enum class KanjiTypes { Jouyou, Jinmei, LinkedJinmei, LinkedOld, Other, Extra, Kentei, None };
constexpr std::array AllKanjiTypes{KanjiTypes::Jouyou,    KanjiTypes::Jinmei, KanjiTypes::LinkedJinmei,
                                   KanjiTypes::LinkedOld, KanjiTypes::Other,  KanjiTypes::Extra,
                                   KanjiTypes::Kentei,    KanjiTypes::None};

constexpr const char* toString(KanjiTypes x) {
  switch (x) {
  case KanjiTypes::Jouyou: return "Jouyou";
  case KanjiTypes::Jinmei: return "Jinmei";
  case KanjiTypes::LinkedJinmei: return "LinkedJinmei";
  case KanjiTypes::LinkedOld: return "LinkedOld";
  case KanjiTypes::Other: return "Other";
  case KanjiTypes::Extra: return "Extra";
  case KanjiTypes::Kentei: return "Kentei";
  default: return "None";
  }
}

inline std::ostream& operator<<(std::ostream& os, const KanjiTypes& x) { return os << toString(x); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_KANJI_TYPES_H