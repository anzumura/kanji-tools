#ifndef KANJI_TOOLS_UTILS_KENTEI_KYUS_H
#define KANJI_TOOLS_UTILS_KENTEI_KYUS_H

#include <array>
#include <iostream>

namespace kanji_tools {

// Kanji Kentei (漢字検定) Kyū (級), K = Kanken (漢検), J=Jun (準), None=not a Kentei kanji
enum class KenteiKyus { K10, K9, K8, K7, K6, K5, K4, K3, KJ2, K2, KJ1, K1, None };
constexpr std::array AllKenteiKyus{KenteiKyus::K10, KenteiKyus::K9, KenteiKyus::K8,  KenteiKyus::K7,  KenteiKyus::K6,
                                   KenteiKyus::K5,  KenteiKyus::K4, KenteiKyus::K3,  KenteiKyus::KJ2, KenteiKyus::K2,
                                   KenteiKyus::KJ1, KenteiKyus::K1, KenteiKyus::None};

constexpr bool toBool(KenteiKyus x) { return x != KenteiKyus::None; }

constexpr const char* toString(KenteiKyus x) {
  switch (x) {
  case KenteiKyus::K10: return "K10";
  case KenteiKyus::K9: return "K9";
  case KenteiKyus::K8: return "K8";
  case KenteiKyus::K7: return "K7";
  case KenteiKyus::K6: return "K6";
  case KenteiKyus::K5: return "K5";
  case KenteiKyus::K4: return "K4";
  case KenteiKyus::K3: return "K3";
  case KenteiKyus::KJ2: return "KJ2";
  case KenteiKyus::K2: return "K2";
  case KenteiKyus::KJ1: return "KJ1";
  case KenteiKyus::K1: return "K1";
  default: return "None";
  }
}

inline std::ostream& operator<<(std::ostream& os, KenteiKyus x) { return os << toString(x); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_KENTEI_KYUS_H
