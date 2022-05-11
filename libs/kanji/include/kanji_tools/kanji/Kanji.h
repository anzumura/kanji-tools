#pragma once

#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/Bitmask.h>
#include <kanji_tools/utils/JlptLevels.h>
#include <kanji_tools/utils/KenteiKyus.h>

#include <memory>
#include <optional>

namespace kanji_tools {

// 'KanjiTypes' is used to identify which official group (Jouyou or Jinmei) a
// kanji belongs to (or has a link to) as well as a few more groups for less
// common kanji:
// - Jouyou: 2136 official Jouyou kanji
// - Jinmei: 633 official Jinmei kanji
// - LinkedJinmei: 230 more Jinmei kanji that are old/variant forms of Jouyou
//   (212) or Jinmei (18)
// - LinkedOld: old/variant Jouyou kanji that aren't in 'LinkedJinmei'
// - Frequency: kanji in top 2501 frequency list and in the above types
// - Extra: kanji loaded from 'extra.txt' - shouldn't be any of the above types
// - Kentei: kanji loaded from 'kentei/k*.txt' files that aren't included above
// - Ucd: kanji loaded from 'ucd.txt' file that aren't included above
// - None: used as a type for a kanji that haven't been loaded
enum class KanjiTypes : EnumContainer::Size {
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

enum class KanjiGrades : EnumContainer::Size;
enum class JinmeiReasons : EnumContainer::Size;

// 'KanjiInfo' members can be used to select which fields are printed by
// 'Kanji::info' method. For example 'Grade | Level | Freq' will print 'grade',
// 'level' and 'frequency' fields and 'All ^ Strokes' prints all except strokes.
enum class KanjiInfo : uint16_t {
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

using KanjiPtr = std::shared_ptr<class Kanji>;

class Kanji {
public:
  using Frequency = uint16_t;
  using LinkNames = std::vector<String>;
  using NelsonId = uint16_t;
  using NelsonIds = std::vector<NelsonId>;
  using OptString = std::optional<String>;
  using Year = uint16_t;
  // some type aliases to help make parameter lists shorter and clearer
  using Meaning = Ucd::Meaning;
  using Name = UcdEntry::Name;
  using OldNames = const LinkNames&;
  using Reading = Ucd::Reading;

  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;

  [[nodiscard]] virtual KanjiTypes type() const = 0;
  [[nodiscard]] virtual Meaning meaning() const = 0;
  [[nodiscard]] virtual Reading reading() const = 0;

  // Base Kanji class provided default implementations for the following virtual
  // functions. For enums like KenteiKyus and KanjiGrades, 'None' is returned
  // and for 'frequency' and 'year' zero is returned (means no value).

  [[nodiscard]] virtual Frequency frequency() const;
  [[nodiscard]] virtual KanjiGrades grade() const;
  [[nodiscard]] virtual KenteiKyus kyu() const;
  [[nodiscard]] virtual JlptLevels level() const;
  [[nodiscard]] virtual KanjiPtr link() const;
  [[nodiscard]] virtual JinmeiReasons reason() const;
  [[nodiscard]] virtual Year year() const;

  // 'linkedReadings' returns true if readings were loaded from a linked kanji
  [[nodiscard]] virtual bool linkedReadings() const;

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
  [[nodiscard]] virtual OldNames oldNames() const { return EmptyLinkNames; }

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

  [[nodiscard]] Name name() const { return _name.name(); }
  [[nodiscard]] auto variant() const { return _name.isVariant(); }
  [[nodiscard]] auto nonVariantName() const { return _name.nonVariant(); }

  // 'compatibilityName' returns 'compatibilityName' if it exists, otherwise it
  // returns the string value of '_name'.
  // (which is should be a single MB char without a variation selector)
  [[nodiscard]] String compatibilityName() const;

  [[nodiscard]] Frequency frequencyOrDefault(Frequency x) const;
  [[nodiscard]] Frequency frequencyOrMax() const;
  [[nodiscard]] auto& morohashiId() const { return _morohashiId; }
  [[nodiscard]] auto& nelsonIds() const { return _nelsonIds; }
  [[nodiscard]] auto& pinyin() const { return _pinyin; }
  [[nodiscard]] auto& radical() const { return _radical; }
  [[nodiscard]] auto strokes() const { return _strokes; }

  [[nodiscard]] bool is(KanjiTypes t) const;
  [[nodiscard]] bool hasGrade() const;
  [[nodiscard]] bool hasKyu() const;
  [[nodiscard]] bool hasLevel() const;
  [[nodiscard]] bool hasMeaning() const;
  [[nodiscard]] bool hasNelsonIds() const;
  [[nodiscard]] bool hasReading() const;

  // 'info' returns a comma separated string with extra info (if present)
  // including: Radical, Strokes, Pinyin, Grade, Level, Freq, New, Old and Kyu.
  // 'fields' can be used to control inclusion of fields (include all by
  // default). Note: multiple 'Old' links are separated by '／' (wide slash) and
  // a link is followed by '*' if it was used to pull in readings.
  [[nodiscard]] String info(KanjiInfo fields = KanjiInfo::All) const;

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
  [[nodiscard]] String qualifiedName() const;

  // Used to sort 'Kanji' in a way that corresponds to 'qualifiedName' output,
  // i.e., Jouyou followed by JLPT followed by Frequency, etc.. If within the
  // same 'qualifiedNameRank' then sort by strokes, frequency (if exists) and
  // finally andcompatibilityName (in unicode).
  [[nodiscard]] bool orderByQualifiedName(const Kanji&) const;

  [[nodiscard]] bool operator==(const Kanji&) const;

  // 'Legend' is meant to be used in output to briefly describe the suffix added
  // to a kanji when using the 'qualifiedName' method. See comments for
  // Kanji::qualifiedName for more details.
  static constexpr auto Legend{".=常用 '=JLPT \"=Freq ^=人名用 ~=LinkJ %=LinkO "
                               "+=Extra @=検定 #=1級 *=Ucd"};
protected:
  // ctor used by 'LinkedKanji' and 'NonLinkedKanji' classes
  Kanji(const class Data&, Name, RadicalRef, Strokes, UcdPtr);

  // ctor used by above ctor as well as 'TestKanji' class
  Kanji(Name name, const OptString& compatibilityName, RadicalRef radical,
      Strokes strokes, const Pinyin& pinyin, const MorohashiId& morohashiId,
      NelsonIds nelsonIds)
      : _name{name}, _compatibilityName{compatibilityName}, _radical{radical},
        _strokes{strokes}, _pinyin{pinyin}, _morohashiId{morohashiId},
        _nelsonIds{std::move(nelsonIds)} {}

  inline static const LinkNames EmptyLinkNames;
private:
  // 'KanjiName' is a helper class that provides additional checks and methods
  // related to the string 'name' of a Kanji (possibly extend this more later)
  class KanjiName {
  public:
    explicit KanjiName(Name name);

    [[nodiscard]] Name name() const { return _name; }

    // 'isVariant' is true if _name includes a Unicode 'variation selector'. In
    // this case 'nonVariant' returns _name without the selector.
    [[nodiscard]] bool isVariant() const;
    [[nodiscard]] String nonVariant() const;
  private:
    const String _name;
  };

  // 'QualifiedNames' stores the suffixes for qualified names in order of most
  // common to least common (see comments for 'qualifiedName' method and
  // 'Legend' string above for more details).
  static constexpr std::array QualifiedNames{
      '.', '\'', '"', '^', '~', '%', '+', '@', '#', '*'};

  [[nodiscard]] uint16_t qualifiedNameRank() const;

  // name related fields
  const KanjiName _name;
  const OptString _compatibilityName;

  // all kanji have an official radical and non-zero strokes
  RadicalRef _radical; // reference to an entry in RadicalData::_radicals
  const Strokes _strokes;

  // optional fields
  const Pinyin _pinyin;
  const MorohashiId _morohashiId;
  const NelsonIds _nelsonIds;
};

} // namespace kanji_tools
