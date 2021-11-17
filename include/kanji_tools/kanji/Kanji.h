#ifndef KANJI_TOOLS_KANJI_KANJI_H
#define KANJI_TOOLS_KANJI_KANJI_H

#include <kanji_tools/utils/FileList.h>
#include <kanji_tools/utils/Radical.h>

namespace kanji_tools {

// 'Grades' represents the official school grade for all Jouyou kanji
enum class Grades { G1, G2, G3, G4, G5, G6, S, None }; // S=secondary school, None=not Jouyou
constexpr std::array AllGrades{Grades::G1, Grades::G2, Grades::G3, Grades::G4,
                               Grades::G5, Grades::G6, Grades::S,  Grades::None};
const char* toString(Grades);
inline std::ostream& operator<<(std::ostream& os, const Grades& x) { return os << toString(x); }

// 'Types' is used to identify which official group (Jouyou or Jinmei) a kanji belongs to (or has a link to)
// as well as a few more groups for less common kanji:
// - Jouyou: 2136 official Jouyou kanji
// - Jinmei: 633 official Jinmei kanji
// - LinkedJinmei: 230 more Jinmei kanji that are old/variant forms of Jouyou (212) or Jinmei (18)
// - LinkedOld: old/variant Jouyou kanji that aren't in 'LinkedJinmei'
// - Other: kanji that are in the top 2501 frequency list, but not one of the first 4 types
// - Extra: kanji loaded from 'extra.txt' - shouldn't be any of the above types
// - Kentei: kanji loaded from 'kentei/k*.txt' files that aren't in any of the above types
// - None: used as a type for a kanji that hasn't been loaded
enum class Types { Jouyou, Jinmei, LinkedJinmei, LinkedOld, Other, Extra, Kentei, None };
constexpr std::array AllTypes{Types::Jouyou, Types::Jinmei, Types::LinkedJinmei, Types::LinkedOld,
                              Types::Other,  Types::Extra,  Types::Kentei,       Types::None};
const char* toString(Types);
inline std::ostream& operator<<(std::ostream& os, const Types& x) { return os << toString(x); }

// 'KanjiLegend' is meant to be used in output to briefly describe the suffix added to a kanji when
// using the 'qualifiedName' method. See comments for Kanji::qualifiedName for more details.
inline constexpr auto KanjiLegend = "'=JLPT \"=Freq ^=Jinmei ~=LinkedJinmei %=LinkedOld +=Extra #=<K1-Kentei, *=Kentei";

class Kanji {
public:
  using OptString = std::optional<std::string>;
  using OldNames = std::vector<std::string>;
  static bool hasLink(Types t) { return t == Types::LinkedJinmei || t == Types::LinkedOld; }

  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;

  virtual Types type() const = 0;
  virtual const std::string& meaning() const = 0;
  virtual const std::string& reading() const = 0;
  virtual Grades grade() const { return Grades::None; }

  // Some Jōyō and Jinmeiyō Kanji have 'old' (旧字体) forms:
  // - 365 Jōyō have 'oldNames': 364 have 1 'oldName' and 1 has 3 'oldNames' (弁 has 辨, 瓣 and 辯)
  //   - 163 'LinkedOld' type kanji end up getting created (367 - 204 that are linked jinmei)
  // - 230 Jinmeiyō are 'alternate forms' (type is 'LinkedJinmei'):
  //   - 204 are part of the 365 Jōyō oldName set
  //   - 8 are different alternate forms of Jōyō kanji (薗 駈 嶋 盃 冨 峯 埜 凉)
  //   - 18 are alternate forms of standard (633) Jinmeiyō kanji so only these will have an 'oldName'
  // - In summary, there are 383 kanji with non-empty 'oldNames' (365 + 18)
  virtual const OldNames& oldNames() const { return EmptyOldNames; }
  // Only LinkedKanji have a 'newName', i.e., the name of the linked kanji which is the new (or more standard) version
  virtual OptString newName() const { return std::nullopt; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }

  // 'variant' is true if _name includes a Unicode 'variation selector'. In this case 'nonVariantName'
  // returns the non-variant name and 'compatibilityName' returns the UCD 'compatibility' code (which
  // is a single MB char without a variation selector).
  bool variant() const { return _variant; }
  const std::string& nonVariantName() const { return _nonVariantName; }
  const std::string& compatibilityName() const { return _compatibilityName; }

  const Radical& radical() const { return _radical; }
  int strokes() const { return _strokes; } // may be zero for kanjis only loaded from frequency.txt
  const OptString& pinyin() const { return _pinyin; }
  Levels level() const { return _level; }
  Kyus kyu() const { return _kyu; }
  int frequency() const { return _frequency; }
  int frequencyOrDefault(int x) const { return _frequency ? _frequency : x; }

  bool is(Types t) const { return type() == t; }
  bool hasLevel() const { return _level != Levels::None; }
  bool hasKyu() const { return _kyu != Kyus::None; }
  bool hasGrade() const { return grade() != Grades::None; }
  bool hasMeaning() const { return !meaning().empty(); }
  bool hasReading() const { return !reading().empty(); }

  // 'InfoFields' members can be used to select which fields are printed by 'info'
  // method. For example 'GradeField | LevelField | FreqField' will print grade and
  // level fields and 'AllFields ^ StrokesField' will print all except for strokes.
  enum InfoFields {
    RadicalField = 1,
    StrokesField,
    PinyinField = 4,
    GradeField = 8,
    LevelField = 16,
    FreqField = 32,
    NewField = 64,
    OldField = 128,
    KyuField = 256,
    AllFields = 511
  };

  // 'info' returns a comma separated string with extra info (if present) including:
  //   Radical, Strokes, Grade, Level, Freq, New, Old
  // 'infoFields' can be used to control inclusion of fields (include all by default).
  // Note: some Jouyou and Jinmei kanji have multiple old/variant forms, but at most
  // one will be displayed. 'New' is for 'Linked' type kanji and will show the official
  // 'standard' form in the Jouyou or Jinmei list.
  std::string info(int infoFields = AllFields) const;

  // 'qualifiedName' returns 'name' plus an extra marker to show additional information:
  // space = Jouyou         : all 2136 Jouyou (use space since this is the most common type)
  //     ' = JLPT           : 251 Jinmei in JLPT (out of 2222 total - the other 1971 are Jouyou)
  //     " = Top Frequency  : 296 top frequency not in Jouyou or JLPT
  //     ^ = Jinmei         : 224 Jinmei not already covered by the above types
  //     ~ = Linked Jinmei  : 218 Linked Jinmei (with no frequency)
  //     % = Linked Old     : 211 Linked Old (with no frequency)
  //     + = Extra          : all kanji loaded from 'extra.txt' file
  //     # = <K1 Kentei     : 268 non-K1 Kentei Kanji that aren't in the above categories
  //     * = K1 Kentei      : 2554 K1 Kentei Kanji that aren't in the above categories
  std::string qualifiedName() const {
    auto t = type();
    return _name +
      (t == Types::Jouyou           ? ' '
         : hasLevel()               ? '\''
         : _frequency               ? '"'
         : t == Types::Jinmei       ? '^'
         : t == Types::LinkedJinmei ? '~'
         : t == Types::LinkedOld    ? '%'
         : t == Types::Extra        ? '+'
         : kyu() != Kyus::K1        ? '#'
                                    : '*');
  }
protected:
  Kanji(int number, const std::string& name, const std::string& compatibilityName, const Radical& radical, int strokes,
        const OptString& pinyin, Levels level, Kyus kyu, int frequency);
private:
  inline static const OldNames EmptyOldNames{};
  const int _number;
  const std::string _name;
  const bool _variant;
  const std::string _nonVariantName;    // same as _name if _variant is false
  const std::string _compatibilityName; // same as _name if _variant is false
  const Radical _radical;
  const int _strokes;
  const OptString _pinyin;
  const Levels _level;
  const Kyus _kyu;
  const int _frequency;
};

inline std::ostream& operator<<(std::ostream& os, const Kanji& k) { return os << k.name(); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_KANJI_H
