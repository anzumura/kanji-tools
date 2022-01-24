#ifndef KANJI_TOOLS_UTILS_JLPT_LEVELS_H
#define KANJI_TOOLS_UTILS_JLPT_LEVELS_H

#include <array>
#include <iostream>

namespace kanji_tools {

// JLPT (Japanese Language Proficiency Test) Levels: None=not a JLPT kanji
enum class JlptLevels { N5, N4, N3, N2, N1, None };
constexpr std::array AllJlptLevels{JlptLevels::N5, JlptLevels::N4, JlptLevels::N3,
                                   JlptLevels::N2, JlptLevels::N1, JlptLevels::None};

constexpr auto toBool(JlptLevels x) { return x != JlptLevels::None; }

constexpr auto toString(JlptLevels x) {
  switch (x) {
  case JlptLevels::N5: return "N5";
  case JlptLevels::N4: return "N4";
  case JlptLevels::N3: return "N3";
  case JlptLevels::N2: return "N2";
  case JlptLevels::N1: return "N1";
  default: return "None";
  }
}

inline auto& operator<<(std::ostream& os, JlptLevels x) { return os << toString(x); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_JLPT_LEVELS_H
