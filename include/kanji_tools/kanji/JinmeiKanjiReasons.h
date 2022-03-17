#pragma once

#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

// JinmeiKanjiReasons represents reason kanji was added to Jinmei list:
// - Names: for use in names
// - Print: for use in publications
// - Variant: allowed variant form (異体字)
// - Moved: moved out of Jouyou into Jinmei
// - Simple: simplified form (表外漢字字体表の簡易慣用字体)
// - Other: reason listed as その他
enum class JinmeiKanjiReasons { Names, Print, Variant, Moved, Simple, Other };

template<> inline constexpr auto is_enumarray<JinmeiKanjiReasons>{true};

inline const auto AllJinmeiKanjiReasons{
    BaseEnumArray<JinmeiKanjiReasons>::create(
        "Names", "Print", "Variant", "Moved", "Simple", "Other")};

} // namespace kanji_tools
