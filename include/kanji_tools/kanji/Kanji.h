#pragma once

#include <kanji_tools/kanji/KanjiGrades.h>
#include <kanji_tools/kanji/KanjiTypes.h>
#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/EnumBitmask.h>
#include <kanji_tools/utils/JlptLevels.h>
#include <kanji_tools/utils/KenteiKyus.h>
#include <kanji_tools/utils/MBUtils.h>

#include <limits>
#include <optional>

namespace kanji_tools {

// 'KanjiInfo' members can be used to select which fields are printed by
// 'Kanji::info' method. For example 'Grade | Level | Freq' will print 'grade',
// 'level' and 'frequency' fields and 'All ^ Strokes' prints all except strokes.
enum class KanjiInfo {
  Radical = 1,
  Strokes,
  Pinyin = 4,
  Grade = 8,
  Level = 16,
  Freq = 32,
  New = 64,
  Old = 128,
  Kyu = 256,
  All = 511
};

// enable bitwise operators for 'KanjiInfo'
template<> inline constexpr auto is_bitmask<KanjiInfo>{true};

class Kanji {
public:
  using Frequency = u_int16_t;
  using LinkNames = std::vector<std::string>;
  using NelsonId = u_int16_t;
  using NelsonIds = std::vector<NelsonId>;
  using OptFreq = std::optional<Frequency>;
  using OptString = std::optional<std::string>;
  using Strokes = Ucd::Strokes;

  static auto hasLink(KanjiTypes t) {
    return t == KanjiTypes::LinkedJinmei || t == KanjiTypes::LinkedOld;
  }

  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;
  // operator= is not generated since there are const members

  [[nodiscard]] virtual KanjiTypes type() const = 0;
  [[nodiscard]] virtual const std::string& meaning() const = 0;
  [[nodiscard]] virtual const std::string& reading() const = 0;

  [[nodiscard]] virtual OptFreq frequency() const { return {}; }
  [[nodiscard]] virtual KanjiGrades grade() const { return KanjiGrades::None; }
  [[nodiscard]] virtual KenteiKyus kyu() const { return KenteiKyus::None; }
  [[nodiscard]] virtual JlptLevels level() const { return JlptLevels::None; }

  // 'linkedReadings' returns true if readings were loaded from a linked kanji
  [[nodiscard]] virtual bool linkedReadings() const { return false; }

  // Some Jōyō and Jinmeiyō Kanji have 'old' (旧字体) forms:
  // - 365 Jōyō have 'oldNames': 364 have 1 'oldName' and 1 has 3 'oldNames' (弁
  //   has 辨, 瓣 and 辯)
  // - 163 'LinkedOld' type kanji end up getting created (367 - 204 that are
  //   linked jinmei)
  // - 230 Jinmeiyō are 'alternate forms' (type is 'LinkedJinmei'):
  //   - 204 are part of the 365 Jōyō oldName set
  //   - 8 are different alternate forms of Jōyō kanji (薗 駈 嶋 盃 冨 峯 埜 凉)
  //   - 18 are alternate forms of standard (633) Jinmeiyō kanji so only these
  //     will have an 'oldName'
  // - In summary, there are 383 kanji with non-empty 'oldNames' (365 + 18)
  [[nodiscard]] virtual const LinkNames& oldNames() const {
    return EmptyLinkNames;
  }

  // UcdFileKanji have an optional 'newName' field (based on Link field loaded
  // from ucd.txt). LinkedKanji also have a 'newName', i.e., the linked kanji
  // name which is the new (or more standard) version.
  [[nodiscard]] virtual OptString newName() const { return {}; }

  // Only CustomFileKanji have 'extraTypeInfo'. They have a 'number' (from
  // 'Number' column) plus:
  // - Jouyou: optionally adds the year the kanji was added to the official list
  // - Jinmei: adds the year the kanji was added as well as the 'reason' (see
  // JinmeiKanji class)
  [[nodiscard]] virtual OptString extraTypeInfo() const { return {}; }

  [[nodiscard]] auto& name() const { return _name; }

  // 'variant' is true if _name includes a Unicode 'variation selector'. In this
  // case 'nonVariantName' returns the non-variant name and 'compatibilityName'
  // returns the UCD 'compatibility' code (which is a single MB char without a
  // variation selector).
  [[nodiscard]] auto variant() const { return _nonVariantName.has_value(); }
  [[nodiscard]] auto nonVariantName() const {
    return _nonVariantName.value_or(_name);
  }
  [[nodiscard]] auto compatibilityName() const {
    return _compatibilityName.value_or(_name);
  }

  [[nodiscard]] auto frequencyOrDefault(Frequency x) const {
    return frequency().value_or(x);
  }
  [[nodiscard]] auto frequencyOrMax() const {
    return frequencyOrDefault(std::numeric_limits<Frequency>::max());
  }
  [[nodiscard]] auto& morohashiId() const { return _morohashiId; }
  [[nodiscard]] auto& nelsonIds() const { return _nelsonIds; }
  [[nodiscard]] auto& pinyin() const { return _pinyin; }
  [[nodiscard]] auto& radical() const { return _radical; }
  [[nodiscard]] auto strokes() const { return _strokes; }

  [[nodiscard]] auto is(KanjiTypes t) const { return type() == t; }
  [[nodiscard]] auto hasGrade() const { return hasValue(grade()); }
  [[nodiscard]] auto hasKyu() const { return hasValue(kyu()); }
  [[nodiscard]] auto hasLevel() const { return hasValue(level()); }
  [[nodiscard]] auto hasMeaning() const { return !meaning().empty(); }
  [[nodiscard]] auto hasNelsonIds() const { return !_nelsonIds.empty(); }
  [[nodiscard]] auto hasReading() const { return !reading().empty(); }

  // 'info' returns a comma separated string with extra info (if present)
  // including: Radical, Strokes, Pinyin, Grade, Level, Freq, New, Old and Kyu.
  // 'fields' can be used to control inclusion of fields (include all by
  // default). Note: multiple 'Old' links are separated by '／' (wide slash) and
  // a link is followed by '*' if it was used to pull in readings.
  [[nodiscard]] std::string info(KanjiInfo fields = KanjiInfo::All) const;

  // Return 'name' plus an extra 'suffix' to show more info. Suffixes are:
  //   . = Jouyou        : 2136 Jouyou
  //   ' = JLPT          : 251 Jinmei in JLPT - the other 1971 JLPT are Jouyou
  //   " = Top Frequency : 296 top frequency not in Jouyou or JLPT
  //   ^ = Jinmei        : 224 Jinmei not already covered by the above types
  //   ~ = Linked Jinmei : 218 Linked Jinmei (with no frequency)
  //   % = Linked Old    : 211 Linked Old (with no frequency)
  //   + = Extra         : all kanji loaded from 'extra.txt' file
  //   @ = <K1 Kentei    : 268 non-K1 Kentei Kanji that aren't included above
  //   # = K1 Kentei     : 2554 K1 Kentei Kanji that aren't included above
  //   * = Ucd           : kanji loaded from 'ucd.txt' not included above
  [[nodiscard]] auto qualifiedName() const {
    return _name + QualifiedNames[qualifiedNameRank()];
  }

  // Used to sort 'Kanji' in a way that corresponds to 'qualifiedName' output,
  // i.e., Jouyou followed by JLPT followed by Frequency, etc.. If within the
  // same 'qualifiedNameRank' then sort by strokes, frequency (if exists) and
  // finally andcompatibilityName (in unicode).
  [[nodiscard]] bool orderByQualifiedName(const Kanji&) const;

  // 'Legend' is meant to be used in output to briefly describe the suffix added
  // to a kanji when using the 'qualifiedName' method. See comments for
  // Kanji::qualifiedName for more details.
  static constexpr auto Legend{".=常用 '=JLPT \"=Freq ^=人名用 ~=LinkJ %=LinkO "
                               "+=Extra @=検定 #=1級 *=Ucd"};
protected:
  Kanji(const std::string& name, const OptString& compatibilityName,
      const Radical& radical, Strokes strokes, const OptString& morohashiId,
      const NelsonIds& nelsonIds, const OptString& pinyin);
  inline static const LinkNames EmptyLinkNames;
private:
  // 'QualifiedNames' stores the suffixes for qualified names in order of most
  // common to least common (see comments for 'qualifiedName' method and
  // 'Legend' string above for more details).
  static constexpr std::array QualifiedNames{
      '.', '\'', '"', '^', '~', '%', '+', '@', '#', '*'};

  [[nodiscard]] u_int8_t qualifiedNameRank() const {
    const auto t{type()};
    // Note: '7' is for non-K1 Kentei, '8' is for K1 Kentei and '9' is for Ucd
    // (so the least common)
    return t == KanjiTypes::Jouyou         ? 0
           : hasLevel()                    ? 1
           : frequency()                   ? 2
           : t == KanjiTypes::Jinmei       ? 3
           : t == KanjiTypes::LinkedJinmei ? 4
           : t == KanjiTypes::LinkedOld    ? 5
           : t == KanjiTypes::Extra        ? 6
           : t == KanjiTypes::Ucd          ? 9
           : kyu() != KenteiKyus::K1       ? 7
                                           : 8;
  }

  // name related fields
  const std::string _name;
  const OptString _nonVariantName;
  const OptString _compatibilityName;

  // all kanji have an official radical and non-zero strokes
  const Radical& _radical; // reference to an entry in RadicalData::_radicals
  const Strokes _strokes;

  // optional fields
  const OptString _morohashiId;
  const NelsonIds _nelsonIds;
  const OptString _pinyin;
};

inline auto& operator<<(std::ostream& os, const Kanji& k) {
  return os << k.name();
}

} // namespace kanji_tools
