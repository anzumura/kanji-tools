#pragma once

#include <kt_utils/EnumList.h>

namespace kanji_tools { /// \kanji_group{KanjiEnums}
/// Kanji related enums

/// used to identify which official group (Jouyou or Jinmei) a Kanji belongs to
/// (or has a link to) as well as a few more groups for less common Kanji:
enum class KanjiTypes : Enum::Size {
  Jouyou,       ///< 2,136 official Jōyō (常用) Kanji
  Jinmei,       ///< 633 official Jinmeiyō (人名用) Kanji
  LinkedJinmei, ///< 230 old/variant forms of Jouyou (212) and Jinmei 18)
  LinkedOld,    ///< 163 old/variant Jouyou Kanji that aren't in LinkedJinmei
  Frequency,    ///< 124 from 'frequency.txt' that aren't one of the above types
  Extra,        ///< loaded from 'extra.txt' (file doesn't contain above types)
  Kentei,       ///< loaded from 'kentei/*.txt' and not one of the above types
  Ucd,          ///< loaded from 'ucd.txt' and not one of the above types
  None          ///< not loaded by this program
};
/// enable #KanjiTypes to be used in an EnumList
template<> inline constexpr auto is_enumlist_with_none<KanjiTypes>{true};
/// create an EnumList for #KanjiTypes
inline const auto AllKanjiTypes{
    BaseEnumList<KanjiTypes>::create("Jouyou", "Jinmei", "LinkedJinmei",
        "LinkedOld", "Frequency", "Extra", "Kentei", "Ucd")};

/// represents the official school grade for all Jouyou Kanji
enum class KanjiGrades : Enum::Size {
  G1,  ///< Grade 1: 80 Kanji
  G2,  ///< Grade 2: 160 Kanji
  G3,  ///< Grade 3: 200 Kanji
  G4,  ///< Grade 4: 200 Kanji
  G5,  ///< Grade 5: 185 Kanji
  G6,  ///< Grade 6: 181 Kanji
  S,   ///< Secondary School: 1130 Kanji
  None ///< Not a Jouyou Kanji
};
/// enable #KanjiGrades to be used in an EnumList
template<> inline constexpr auto is_enumlist_with_none<KanjiGrades>{true};
/// create an EnumList for #KanjiGrades
inline const auto AllKanjiGrades{
    BaseEnumList<KanjiGrades>::create("G1", "G2", "G3", "G4", "G5", "G6", "S")};

/// JLPT (Japanese Language Proficiency Test) Levels
/// \details covers 2,222 total Kanji (including 1,971 Jouyou and 251 Jinmei)
enum class JlptLevels : Enum::Size {
  N5,  ///< Level N5: 103 Kanji (G1=57, G2=43, G3=3)
  N4,  ///< Level N4: 181 Kanji (G1=15, G2=74, G3=67, G4=20, G5=2, G6=3)
  N3,  ///< Level N3: 361 Kanji (G1=8, G2=43, G3=130, G4=180)
  N2,  ///< Level N2: 415 Kanji (G5=149, G6=105, S=161)
  N1,  ///< Level N1: 1162 Kanji (G5=34, G6=73, S=804, Jinmei=251)
  None ///< Not a JLPT Kanji (S=165, most Jinmei and all other types)
};
/// enable #JlptLevels to be used in an EnumList
template<> inline constexpr auto is_enumlist_with_none<JlptLevels>{true};
/// create an EnumList for #JlptLevels
inline const auto AllJlptLevels{
    BaseEnumList<JlptLevels>::create("N5", "N4", "N3", "N2", "N1")};

/// Kanji Kentei (漢字検定) Kyū (級), K = Kanken (漢検), J=Jun (準)
/// \sa https://en.wikipedia.org/wiki/Kanji_Kentei
enum class KenteiKyus : Enum::Size {
  K10, ///< Level 10 (１０級): 80 Kanji
  K9,  ///< Level 9 (９級): 160 Kanji
  K8,  ///< Level 8 (８級): 200 Kanji
  K7,  ///< Level 7 (７級): 202 Kanji
  K6,  ///< Level 6 (６級): 193 Kanji
  K5,  ///< Level 5 (５級): 191 Kanji
  K4,  ///< Level 4 (４級): 313 Kanji
  K3,  ///< Level 3 (３級): 284 Kanji
  KJ2, ///< Level Pre-2 (準２級): 328 Kanji
  K2,  ///< Level 2 (２級): 188 Kanji, has 3 non-Jouyou
  KJ1, ///< Level Pre-1 (隼１級): 940 Kanji, all non-Jouyou
  K1,  ///< Level 1 (１級): 2,780 Kanji, all non-Jouyou
  None ///< Not a Kentei Kanji
};
/// enable #KenteiKyus to be used in an EnumList
template<> inline constexpr auto is_enumlist_with_none<KenteiKyus>{true};
/// create an EnumList for #KenteiKyus
inline const auto AllKenteiKyus{BaseEnumList<KenteiKyus>::create(
    "K10", "K9", "K8", "K7", "K6", "K5", "K4", "K3", "KJ2", "K2", "KJ1", "K1")};

/// reason Kanji was added to Jinmeiyō list:
enum class JinmeiReasons : Enum::Size {
  Names,   ///< 246 Kanji: for use in names
  Print,   ///< 352 Kanji: for use in publications
  Variant, ///< 2 Kanji: allowed variant form (異体字)
  Moved,   ///< 5 Kanji: moved out of Jouyou into Jinmei
  Simple,  ///< 2 Kanji: simplified (表外漢字字体表の簡易慣用字体)
  Other,   ///< 26 Kanji: reason listed as その他
  None     ///< Not a Jinmei type Kanji
};
/// enable #JinmeiReasons to be used in an EnumList
template<> inline constexpr auto is_enumlist_with_none<JinmeiReasons>{true};
/// create an EnumList for #JinmeiReasons
inline const auto AllJinmeiReasons{BaseEnumList<JinmeiReasons>::create(
    "Names", "Print", "Variant", "Moved", "Simple", "Other")};

/// \end_group
} // namespace kanji_tools
