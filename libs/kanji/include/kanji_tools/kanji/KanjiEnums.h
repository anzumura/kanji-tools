#pragma once

#include <kanji_tools/utils/EnumList.h>

namespace kanji_tools { /// \kanji_group{KanjiEnums}
/// Kanji related enums

/// used to identify which official group (Jouyou or Jinmei) a Kanji belongs to
/// (or has a link to) as well as a few more groups for less common Kanji:
enum class KanjiTypes : Enum::Size {
  Jouyou,       ///< 2136 official Jōyō (常用) Kanji
  Jinmei,       ///< 633 official Jinmeiyō (人名用) Kanji
  LinkedJinmei, ///< 230 more Jinmei Kanji (includes old/variant forms of 212
                ///< Jouyou and and 18 Jinmei)
  LinkedOld,    ///< old/variant Jouyou Kanji that aren't in 'LinkedJinmei'
  Frequency,    ///< Kanji in top 2501 frequency list and in the above types
  Extra,  ///< Kanji loaded from 'extra.txt' (and not any of the above types)
  Kentei, ///< Kanji loaded from 'kentei/k*.txt' files (not included above)
  Ucd,    ///< Kanji loaded from 'ucd.txt' file that aren't included above
  None    ///< used for Kanji that haven't been loaded from 'data/*.txt' files
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
  K10, ///< Level 10 (１０級): 80 Kanji (Jouyou=80)
  K9,  ///< Level 9 (９級): 160 Kanji (Jouyou=160)
  K8,  ///< Level 8 (８級): 200 Kanji (Jouyou=200)
  K7,  ///< Level 7 (７級): 202 Kanji (Jouyou=202)
  K6,  ///< Level 6 (６級): 193 Kanji (Jouyou=193)
  K5,  ///< Level 5 (５級): 191 Kanji (Jouyou=191)
  K4,  ///< Level 4 (４級): 313 Kanji (Jouyou=313)
  K3,  ///< Level 3 (３級): 284 Kanji (Jouyou=284)
  KJ2, ///< Level Pre-2 (準２級): 328 Kanji (Jouyou=328)
  K2,  ///< Level 2 (２級): 188 Kanji (Jouyou=184, Frequency=3, Kentei=1)
  KJ1, ///< Level Pre-1 (隼１級): 940 Kanji (Jinmei=569, LinkedJinmei=11,
       ///< Frequency=60, Extra=33, Kentei=267)
  K1, ///< Level 1 (１級): 2,780 Kanji (Jinmei=63, LinkedJinmei=2, LinkedOld=2,
      ///< Frequency=58, Extra=101, Kentei=2,554)
  None ///< Not a Kentei Kanji
};
/// enable #KenteiKyus to be used in an EnumList
template<> inline constexpr auto is_enumlist_with_none<KenteiKyus>{true};
/// create an EnumList for #KenteiKyus
inline const auto AllKenteiKyus{BaseEnumList<KenteiKyus>::create(
    "K10", "K9", "K8", "K7", "K6", "K5", "K4", "K3", "KJ2", "K2", "KJ1", "K1")};

/// represents reason Kanji was added to Jinmeiyō list:
enum class JinmeiReasons : Enum::Size {
  Names,   ///< for use in names
  Print,   ///< for use in publications
  Variant, ///< allowed variant form (異体字)
  Moved,   ///< moved out of Jouyou into Jinmei
  Simple,  ///< simplified form (表外漢字字体表の簡易慣用字体)
  Other,   ///< reason listed as その他
  None ///< all JinmeiKanji have one of the above reasons, None is used for base
       ///< virtual function return value (similar to other Kanji enums)
};
/// enable #JinmeiReasons to be used in an EnumList
template<> inline constexpr auto is_enumlist_with_none<JinmeiReasons>{true};
/// create an EnumList for #JinmeiReasons
inline const auto AllJinmeiReasons{BaseEnumList<JinmeiReasons>::create(
    "Names", "Print", "Variant", "Moved", "Simple", "Other")};

/// \end_group
} // namespace kanji_tools
