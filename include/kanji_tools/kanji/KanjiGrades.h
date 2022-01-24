#ifndef KANJI_TOOLS_KANJI_KANJI_GRADES_H
#define KANJI_TOOLS_KANJI_KANJI_GRADES_H

#include <array>
#include <iostream>

namespace kanji_tools {

// 'KanjiGrades' represents the official school grade for all Jouyou kanji
enum class KanjiGrades { G1, G2, G3, G4, G5, G6, S, None }; // S=secondary school, None=not Jouyou
constexpr std::array AllKanjiGrades{KanjiGrades::G1, KanjiGrades::G2, KanjiGrades::G3, KanjiGrades::G4,
                                    KanjiGrades::G5, KanjiGrades::G6, KanjiGrades::S,  KanjiGrades::None};

constexpr auto toBool(KanjiGrades x) { return x != KanjiGrades::None; }

constexpr auto toString(KanjiGrades x) {
  switch (x) {
  case KanjiGrades::G1: return "G1";
  case KanjiGrades::G2: return "G2";
  case KanjiGrades::G3: return "G3";
  case KanjiGrades::G4: return "G4";
  case KanjiGrades::G5: return "G5";
  case KanjiGrades::G6: return "G6";
  case KanjiGrades::S: return "S";
  default: return "None";
  }
}

inline auto& operator<<(std::ostream& os, KanjiGrades x) { return os << toString(x); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_KANJI_GRADES_H
