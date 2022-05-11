#pragma once

#include <kanji_tools/utils/EnumList.h>

namespace kanji_tools {

// UcdLinkTypes represent the XML property from which the link was loaded - see
// parseUcdAllFlat.sh for details. '_R' means the link was used to also pull in
// readings. The script uses '*' for reading links so '*' has also been used in
// 'AllUcdLinkTypes' array). Put _R first to allow a '<' comparision to find all
// reading links. Note, there is no non-Reading 'Semantic' link type by design.
enum class UcdLinkTypes : EnumContainer::Size {
  Compatibility_R,
  Definition_R,
  Jinmei_R,
  Semantic_R,
  Simplified_R,
  Traditional_R,
  Compatibility,
  Definition,
  Jinmei,
  Simplified,
  Traditional,
  None
};

template<> inline constexpr auto is_enumlist_with_none<UcdLinkTypes>{true};

inline const auto AllUcdLinkTypes{
    BaseEnumList<UcdLinkTypes>::create("Compatibility*", "Definition*",
        "Jinmei*", "Semantic*", "Simplified*", "Traditional*", "Compatibility",
        "Definition", "Jinmei", "Simplified", "Traditional")};

} // namespace kanji_tools
