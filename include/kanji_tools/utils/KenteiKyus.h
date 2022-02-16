#ifndef KANJI_TOOLS_UTILS_KENTEI_KYUS_H
#define KANJI_TOOLS_UTILS_KENTEI_KYUS_H

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// Kanji Kentei (漢字検定) Kyū (級), K = Kanken (漢検), J=Jun (準), None=not a Kentei kanji
enum class KenteiKyus { K10, K9, K8, K7, K6, K5, K4, K3, KJ2, K2, KJ1, K1, None };
template<> inline constexpr bool is_enumarray<KenteiKyus> = true;
inline const auto AllKenteiKyus =
  BaseEnumArray<KenteiKyus>::create("K10", "K9", "K8", "K7", "K6", "K5", "K4", "K3", "KJ2", "K2", "KJ1", "K1");

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_KENTEI_KYUS_H
