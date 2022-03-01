#pragma once

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// UcdLinkTypes represents how a Ucd link was loaded (from which XML property -
// see parse script for details)
enum class UcdLinkTypes {
  Compatibility,
  Definition,
  Jinmei,
  Semantic,
  Simplified,
  Traditional,
  None
};
template<> inline constexpr bool is_enumarray_with_none<UcdLinkTypes> = true;
inline const auto AllUcdLinkTypes =
  BaseEnumArray<UcdLinkTypes>::create("Compatibility", "Definition", "Jinmei",
                                      "Semantic", "Simplified", "Traditional");

} // namespace kanji_tools