#pragma once

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// Kanji Kentei (漢字検定) Kyū (級), K = Kanken (漢検), J=Jun (準)
enum class KenteiKyus {
  K10,
  K9,
  K8,
  K7,
  K6,
  K5,
  K4,
  K3,
  KJ2,
  K2,
  KJ1,
  K1,
  None
};
template<> inline constexpr auto is_enumarray_with_none<KenteiKyus>{true};
inline const auto AllKenteiKyus{BaseEnumArray<KenteiKyus>::create(
  "K10", "K9", "K8", "K7", "K6", "K5", "K4", "K3", "KJ2", "K2", "KJ1", "K1")};

} // namespace kanji_tools
