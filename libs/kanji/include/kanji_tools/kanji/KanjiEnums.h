#pragma once

#include <kanji_tools/utils/EnumList.h>

namespace kanji_tools {

// 'KanjiTypes' is used to identify which official group (Jouyou or Jinmei) a
// Kanji belongs to (or has a link to) as well as a few more groups for less
// common Kanji:
// - Jouyou: 2136 official Jouyou Kanji
// - Jinmei: 633 official Jinmei Kanji
// - LinkedJinmei: 230 more Jinmei Kanji that are old/variant forms of Jouyou
//   (212) or Jinmei (18)
// - LinkedOld: old/variant Jouyou Kanji that aren't in 'LinkedJinmei'
// - Frequency: Kanji in top 2501 frequency list and in the above types
// - Extra: Kanji loaded from 'extra.txt' - shouldn't be any of the above types
// - Kentei: Kanji loaded from 'kentei/k*.txt' files that aren't included above
// - Ucd: Kanji loaded from 'ucd.txt' file that aren't included above
// - None: used for Kanji that haven't been loaded from 'data/*.txt' files
enum class KanjiTypes : Enum::Size {
  Jouyou,
  Jinmei,
  LinkedJinmei,
  LinkedOld,
  Frequency,
  Extra,
  Kentei,
  Ucd,
  None
};
template<> inline constexpr auto is_enumlist_with_none<KanjiTypes>{true};
inline const auto AllKanjiTypes{
    BaseEnumList<KanjiTypes>::create("Jouyou", "Jinmei", "LinkedJinmei",
        "LinkedOld", "Frequency", "Extra", "Kentei", "Ucd")};

// 'KanjiGrades' represents the official school grade for all Jouyou Kanji.
// S=secondary school, None=not Jouyou
enum class KanjiGrades : Enum::Size { G1, G2, G3, G4, G5, G6, S, None };
template<> inline constexpr auto is_enumlist_with_none<KanjiGrades>{true};
inline const auto AllKanjiGrades{
    BaseEnumList<KanjiGrades>::create("G1", "G2", "G3", "G4", "G5", "G6", "S")};

// JLPT (Japanese Language Proficiency Test) Levels: None=not a JLPT Kanji
enum class JlptLevels : Enum::Size { N5, N4, N3, N2, N1, None };
template<> inline constexpr auto is_enumlist_with_none<JlptLevels>{true};
inline const auto AllJlptLevels{
    BaseEnumList<JlptLevels>::create("N5", "N4", "N3", "N2", "N1")};

// Kanji Kentei (漢字検定) Kyū (級), K = Kanken (漢検), J=Jun (準)
enum class KenteiKyus : Enum::Size {
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
template<> inline constexpr auto is_enumlist_with_none<KenteiKyus>{true};
inline const auto AllKenteiKyus{BaseEnumList<KenteiKyus>::create(
    "K10", "K9", "K8", "K7", "K6", "K5", "K4", "K3", "KJ2", "K2", "KJ1", "K1")};

// JinmeiReasons represents reason Kanji was added to Jinmei list:
// - Names: for use in names
// - Print: for use in publications
// - Variant: allowed variant form (異体字)
// - Moved: moved out of Jouyou into Jinmei
// - Simple: simplified form (表外漢字字体表の簡易慣用字体)
// - Other: reason listed as その他
// - None: all JinmeiKanji have one of the above reasons, None is used for base
//   class virtual function return value (similar to other Kanji related enums)
enum class JinmeiReasons : Enum::Size {
  Names,
  Print,
  Variant,
  Moved,
  Simple,
  Other,
  None
};
template<> inline constexpr auto is_enumlist_with_none<JinmeiReasons>{true};
inline const auto AllJinmeiReasons{BaseEnumList<JinmeiReasons>::create(
    "Names", "Print", "Variant", "Moved", "Simple", "Other")};

} // namespace kanji_tools
