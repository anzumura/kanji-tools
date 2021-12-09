#ifndef KANJI_TOOLS_KANJI_KANJI_H
#define KANJI_TOOLS_KANJI_KANJI_H

#include <kanji_tools/kanji/KanjiGrades.h>
#include <kanji_tools/kanji/KanjiTypes.h>
#include <kanji_tools/kanji/Radical.h>
#include <kanji_tools/utils/JlptLevels.h>
#include <kanji_tools/utils/KenteiKyus.h>
#include <kanji_tools/utils/MBUtils.h>

#include <optional>

namespace kanji_tools {

class Kanji {
public:
  using OptString = std::optional<std::string>;
  using LinkNames = std::vector<std::string>;
  using NelsonIds = std::vector<int>;
  static bool hasLink(KanjiTypes t) { return t == KanjiTypes::LinkedJinmei || t == KanjiTypes::LinkedOld; }

  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;

  virtual KanjiTypes type() const = 0;
  virtual const std::string& meaning() const = 0;
  virtual const std::string& reading() const = 0;
  virtual KanjiGrades grade() const { return KanjiGrades::None; }
  // 'linkedReadings' returns true if readings were loaded from a linked kanji
  virtual bool linkedReadings() const { return false; }

  // Some Jōyō and Jinmeiyō Kanji have 'old' (旧字体) forms:
  // - 365 Jōyō have 'oldNames': 364 have 1 'oldName' and 1 has 3 'oldNames' (弁 has 辨, 瓣 and 辯)
  //   - 163 'LinkedOld' type kanji end up getting created (367 - 204 that are linked jinmei)
  // - 230 Jinmeiyō are 'alternate forms' (type is 'LinkedJinmei'):
  //   - 204 are part of the 365 Jōyō oldName set
  //   - 8 are different alternate forms of Jōyō kanji (薗 駈 嶋 盃 冨 峯 埜 凉)
  //   - 18 are alternate forms of standard (633) Jinmeiyō kanji so only these will have an 'oldName'
  // - In summary, there are 383 kanji with non-empty 'oldNames' (365 + 18)
  virtual const LinkNames& oldNames() const { return EmptyLinkNames; }
  // UcdFileKanji have an optional 'newName' field (based on Link field loaded from ucd.txt). LinkedKanji
  // also have a 'newName', i.e., the linked kanji name which is the new (or more standard) version.
  virtual OptString newName() const { return std::nullopt; }
  // Only CustomFileKanji have 'extraTypeInfo'. They have a 'number' (from 'Number' column) plus:
  // - Jouyou: optionally adds the year the kanji was added to the official list
  // - Jinmei: adds the year the kanji was added as well as the 'reason' (see JinmeiKanji class)
  virtual OptString extraTypeInfo() const { return std::nullopt; }

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
  const OptString& morohashiId() const { return _morohashiId; }
  const NelsonIds& nelsonIds() const { return _nelsonIds; }
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
  bool hasMorohashId() const { return _morohashiId.has_value(); }
  bool hasNelsonIds() const { return !_nelsonIds.empty(); }

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
  //   Radical, Strokes, Pinyin, Grade, Level, Freq, New, Old and Kyu
  // 'infoFields' can be used to control inclusion of fields (include all by default).
  // Note: multiple 'Old' links are separated by '／' (wide slash) and a link is followed
  // by '*' if it was used to pull in readings.
  std::string info(int infoFields = AllFields) const;

  // 'qualifiedName' returns 'name' plus an extra marker to show additional information:
  //     . = Jouyou         : 2136 Jouyou
  //     ' = JLPT           : 251 Jinmei in JLPT (out of 2222 total - the other 1971 are Jouyou)
  //     " = Top Frequency  : 296 top frequency not in Jouyou or JLPT
  //     ^ = Jinmei         : 224 Jinmei not already covered by the above types
  //     ~ = Linked Jinmei  : 218 Linked Jinmei (with no frequency)
  //     % = Linked Old     : 211 Linked Old (with no frequency)
  //     + = Extra          : all kanji loaded from 'extra.txt' file
  //     @ = <K1 Kentei     : 268 non-K1 Kentei Kanji that aren't in the above categories
  //     # = K1 Kentei      : 2554 K1 Kentei Kanji that aren't in the above categories
  //     * = Ucd            : all kanji loaded from 'ucd.txt' file that aren't in the above categories
  std::string qualifiedName() const { return _name + QualifiedNames[qualifiedNameRank()]; }

  // 'orderByQualifiedName' can be used to sort 'Kanji' in a way that corresponds to 'qualifiedName' output,
  // i.e., Jouyou followed by JLPT followed by Frequency, etc.. If within the same 'qualifiedNameRank' then
  // sort by strokes, frequency, variant and (unicode) compatibilityName.
  bool orderByQualifiedName(const Kanji& x) const {
    return qualifiedNameRank() < x.qualifiedNameRank() ||
      qualifiedNameRank() == x.qualifiedNameRank() &&
      (strokes() < x.strokes() ||
       strokes() == x.strokes() &&
         (frequency() < x.frequency() ||
          frequency() == x.frequency() &&
            (variant() < x.variant() ||
             variant() == x.variant() && toUnicode(compatibilityName()) < toUnicode(x.compatibilityName()))));
  }

  // 'Legend' is meant to be used in output to briefly describe the suffix added to a kanji when
  // using the 'qualifiedName' method. See comments for Kanji::qualifiedName for more details.
  static constexpr auto Legend = ".=常用 '=JLPT \"=Freq ^=人名用 ~=LinkJ %=LinkO +=Extra @=検定 #=1級 *=Ucd";
protected:
  Kanji(const std::string& name, const std::string& compatibilityName, const Radical& radical, int strokes,
        const OptString& pinyin, const OptString& morohashiId, const NelsonIds& nelsonIds, JlptLevels level,
        KenteiKyus kyu, int frequency);
  inline static const LinkNames EmptyLinkNames{};
private:
  // 'QualifiedNames' stores the suffixes for qualified names in order of most common to least common (see
  // comments for 'qualifiedName' method and 'Legend' string above for more details).
  static constexpr std::array QualifiedNames = {'.', '\'', '"', '^', '~', '%', '+', '@', '#', '*'};

  int qualifiedNameRank() const {
    auto t = type();
    // Note: '7' is for non-K1 Kentei, '8' is for K1 Kentei and '9' is for Ucd (so the least common)
    return t == KanjiTypes::Jouyou    ? 0
      : hasLevel()                    ? 1
      : _frequency                    ? 2
      : t == KanjiTypes::Jinmei       ? 3
      : t == KanjiTypes::LinkedJinmei ? 4
      : t == KanjiTypes::LinkedOld    ? 5
      : t == KanjiTypes::Extra        ? 6
      : t == KanjiTypes::Ucd          ? 9
      : kyu() != KenteiKyus::K1       ? 7
                                      : 8;
  }

  const std::string _name;
  const bool _variant;
  const std::string _nonVariantName;    // same as _name if _variant is false
  const std::string _compatibilityName; // same as _name if _variant is false
  const Radical _radical;
  const int _strokes;
  const OptString _pinyin;
  const OptString _morohashiId;
  const NelsonIds _nelsonIds;
  const JlptLevels _level;
  const KenteiKyus _kyu;
  const int _frequency;
};

inline std::ostream& operator<<(std::ostream& os, const Kanji& k) { return os << k.name(); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_KANJI_H
