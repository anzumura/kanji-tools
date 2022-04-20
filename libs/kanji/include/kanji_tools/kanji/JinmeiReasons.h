#pragma once

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// JinmeiReasons represents reason kanji was added to Jinmei list:
// - Names: for use in names
// - Print: for use in publications
// - Variant: allowed variant form (異体字)
// - Moved: moved out of Jouyou into Jinmei
// - Simple: simplified form (表外漢字字体表の簡易慣用字体)
// - Other: reason listed as その他
// - None: all JinmeiKanji have one of the above reasons, None is used for base
//   class virtual function return value (similar to other Kanji related enums)
enum class JinmeiReasons : BaseEnum::Size {
  Names,
  Print,
  Variant,
  Moved,
  Simple,
  Other,
  None
};

template<> inline constexpr auto is_enumarray_with_none<JinmeiReasons>{true};

inline const auto AllJinmeiReasons{TypedEnumArray<JinmeiReasons>::create(
    "Names", "Print", "Variant", "Moved", "Simple", "Other")};

} // namespace kanji_tools
