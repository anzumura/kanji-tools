#ifndef KANJI_TOOLS_KANJI_KANJI_H
#define KANJI_TOOLS_KANJI_KANJI_H

#include <kanji_tools/kanji/Radical.h>
#include <kanji_tools/utils/JlptLevels.h>
#include <kanji_tools/utils/KanjiGrades.h>
#include <kanji_tools/utils/KanjiTypes.h>
#include <kanji_tools/utils/KenteiKyus.h>

#include <optional>

namespace kanji_tools {

// 'KanjiLegend' is meant to be used in output to briefly describe the suffix added to a kanji when
// using the 'qualifiedName' method. See comments for Kanji::qualifiedName for more details.
inline constexpr auto KanjiLegend = "'=JLPT \"=Freq ^=Jinmei ~=LinkedJinmei %=LinkedOld +=Extra #=<K1-Kentei, *=Kentei";

class Kanji {
public:
  using OptString = std::optional<std::string>;
  using OldNames = std::vector<std::string>;
  static bool hasLink(KanjiTypes t) { return t == KanjiTypes::LinkedJinmei || t == KanjiTypes::LinkedOld; }

  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;

  virtual KanjiTypes type() const = 0;
  virtual const std::string& meaning() const = 0;
  virtual const std::string& reading() const = 0;
  virtual KanjiGrades grade() const { return KanjiGrades::None; }

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
  JlptLevels level() const { return _level; }
  KenteiKyus kyu() const { return _kyu; }
  int frequency() const { return _frequency; }
  int frequencyOrDefault(int x) const { return _frequency ? _frequency : x; }

  bool is(KanjiTypes t) const { return type() == t; }
  bool hasLevel() const { return _level != JlptLevels::None; }
  bool hasKyu() const { return _kyu != KenteiKyus::None; }
  bool hasGrade() const { return grade() != KanjiGrades::None; }
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
      (t == KanjiTypes::Jouyou           ? ' '
         : hasLevel()                    ? '\''
         : _frequency                    ? '"'
         : t == KanjiTypes::Jinmei       ? '^'
         : t == KanjiTypes::LinkedJinmei ? '~'
         : t == KanjiTypes::LinkedOld    ? '%'
         : t == KanjiTypes::Extra        ? '+'
         : kyu() != KenteiKyus::K1       ? '#'
                                         : '*');
  }
protected:
  Kanji(int number, const std::string& name, const std::string& compatibilityName, const Radical& radical, int strokes,
        const OptString& pinyin, JlptLevels level, KenteiKyus kyu, int frequency);
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
  const JlptLevels _level;
  const KenteiKyus _kyu;
  const int _frequency;
};

inline std::ostream& operator<<(std::ostream& os, const Kanji& k) { return os << k.name(); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_KANJI_H
