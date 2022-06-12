#pragma once

#include <kt_kanji/KanjiEnums.h>
#include <kt_kanji/Ucd.h>
#include <kt_utils/Bitmask.h>

#include <memory>
#include <optional>

namespace kanji_tools { /// \kanji_group{Kanji}
/// Kanji, LoadedKanji and OtherKanji class hierarchy

using KanjiPtr = std::shared_ptr<class Kanji>;
using KanjiDataRef = const class KanjiData&;

/// abstract base class representing a Japanese %Kanji character \kanji{Kanji}
class Kanji {
public:
  using Frequency = uint16_t;
  using Link = const KanjiPtr&;
  using LinkNames = std::vector<String>;
  using NelsonId = uint16_t;
  using NelsonIds = std::vector<NelsonId>;
  using OptString = std::optional<String>;
  using Year = uint16_t;
  // some type aliases to help make parameter lists shorter and clearer
  using Meaning = Ucd::Meaning;
  using Name = Radical::Name;
  using OldNames = const LinkNames&;
  using Reading = Ucd::Reading;

  /// members can be combined to select which fields are printed by info()
  /// \details for example `Grade | Level | Freq` prints the three listed fields
  /// and `All ^ Strokes` prints all fields except 'strokes'
  enum class Info : uint16_t {
    Radical = 1, ///< print 'radical' (部首) name and number
    Strokes,     ///< print 'strokes' (画数) count
    Pinyin = 4,  ///< print most common 'hànyǔ pīnyīn' (漢語拼音)
    Grade = 8,   ///< print 'grade' (学年)
    Level = 16,  ///< print JLPT 'level'
    Freq = 32,   ///< print 'frequency' number (from top 2,501 list)
    New = 64,    ///< print 'new' variant (generally simplified forms)
    Old = 128,   ///< print 'old' variant (generally traditional forms)
    Kyu = 256,   ///< print Kentei 'kyu'
    All = 511    ///< print all fields
  };

  virtual ~Kanji() = default;   ///< default dtor
  Kanji(const Kanji&) = delete; ///< deleted copy ctor

  /// return a unique KanjiTypes value for each leaf class type
  [[nodiscard]] virtual KanjiTypes type() const = 0;

  /// return one or more English meanings (some OtherKanji have empty meaning)
  [[nodiscard]] virtual Meaning meaning() const = 0;

  /// return a comma-separated list of Japanese readings in %Kana
  /// \details On (音) readings are in Katakana followed by Kun (訓) readings in
  /// Hiragana (OfficialLinkedKanji classes return the readings of their 'link'
  /// Kanji) \note Jouyou and Extra Kanji include a dash (-) in Kun readings
  /// before any Okurigana (送り仮名), but unfortunately this is not the case
  /// for readings loaded from 'ucd.txt'
  [[nodiscard]] virtual Reading reading() const = 0;

  /// return frequency number starting at `1` for most frequent up to `2,501`,
  /// base class returns `0` which means 'not in the top 2,501 list'
  [[nodiscard]] virtual Frequency frequency() const;

  /// return grade, base class returns `None` which means 'has no grade'
  [[nodiscard]] virtual KanjiGrades grade() const;

  /// return Kentei kyu, base class returns `None` which means 'has no kyu'
  [[nodiscard]] virtual KenteiKyus kyu() const;

  /// return JLPT level, base class returns `None` which means 'has no level'
  [[nodiscard]] virtual JlptLevels level() const;

  /// return link to official Kanji, base class returns `nullptr`
  [[nodiscard]] virtual Link link() const;

  /// return Jinmei 'reason', base class returns `None`
  [[nodiscard]] virtual JinmeiReasons reason() const;

  /// return the year Kanji was added to an official list, base class returns
  /// `0` which means 'no year was specified'
  [[nodiscard]] virtual Year year() const;

  /// return true if readings were loaded via a link
  [[nodiscard]] virtual bool linkedReadings() const;

  /// return list of old names (usually empty)
  /// \details some JouyouKanji and JinmeiKanji have 'old' (旧字体) forms:
  /// \li 365 JouyouKanji: 364 have 'oldNames' with one entry and ono has
  ///   'oldNames' with three entries (弁 has 辨, 瓣 and 辯)
  /// \li 18 JinmeiKanji: alternate forms of standard JinmeiKanji
  /// \li several hundred Kanji of other types also have non-empty 'oldNames'
  ///   populated from 'ucd.txt' 'Traditional Links'
  /// LinkedOldKanji end up getting created (367 - 204 linkedJinmeiKanji)
  [[nodiscard]] virtual OldNames oldNames() const { return EmptyLinkNames; }

  /// return the new name (usually not defined)
  /// \details OtherKanji can populate this field based on 'Simplified Links'
  /// loaded from 'ucd.txt'). OfficialLinkedKanji also set 'newName' to the name
  /// of the `link` Kanji (which is the 'new/standard' version)
  [[nodiscard]] virtual OptString newName() const { return {}; }

  /// return an optional String with extra information depending on type
  /// \details currently this method is overridden by:
  /// \li NumberedKanji: returns `number`
  /// \li OfficialKanji: optionally adds `year` if it's non-zero
  /// \li JinmeiKanji: adds `reason`
  [[nodiscard]] virtual OptString extraTypeInfo() const { return {}; }

  /// return the name (usually a single UTF-8 character, but can include a
  /// variation selector)
  [[nodiscard]] Name name() const { return _name.name(); }

  /// return true if name() has a variation selector
  [[nodiscard]] auto variant() const { return _name.isVariant(); }

  /// return name() without any variation selector
  [[nodiscard]] auto nonVariantName() const { return _name.nonVariant(); }

  /// return a 'compatibility name' if defined, otherwise return same as name()
  /// \details a compatibility name is a single UTF-8 that can be used instead
  /// of a base character plus variation selector
  [[nodiscard]] String compatibilityName() const;

  /// return frequency() if it's non-zero, otherwise return `x`
  [[nodiscard]] Frequency frequencyOrDefault(Frequency x) const;

  /// return frequency() if it's non-zero, otherwise return max Frequency value
  [[nodiscard]] Frequency frequencyOrMax() const;

  /// return 'Morohashi ID' ('Dai Kan-Wa Jiten' index number)
  [[nodiscard]] auto& morohashiId() const { return _morohashiId; }

  /// return list of 'Classic Nelson IDs' (usually a single value, but can be
  /// empty or have more than one value in a few cases)
  [[nodiscard]] auto& nelsonIds() const { return _nelsonIds; }

  /// return most common 'hànyǔ pīnyīn' (from UCD 'kMandarin' property)
  [[nodiscard]] auto& pinyin() const { return _pinyin; }

  /// return reference to Radical object
  [[nodiscard]] auto& radical() const { return _radical; }

  /// return copy of Strokes object
  [[nodiscard]] auto strokes() const { return _strokes; }

  /// return true if type() is `t`
  [[nodiscard]] bool is(KanjiTypes t) const;

  /// return true if grade() isn't `None` (so true for all JouyouKanji)
  [[nodiscard]] bool hasGrade() const;

  /// return true if (Kentei) kyu() isn't `None`
  [[nodiscard]] bool hasKyu() const;

  /// return true if (JLPT) level() isn't `None`
  [[nodiscard]] bool hasLevel() const;

  /// return true if meaning() isn't empty
  [[nodiscard]] bool hasMeaning() const;

  /// return true if nelsonIds() isn't empty
  [[nodiscard]] bool hasNelsonIds() const;

  /// return true if reading() isn't empty
  [[nodiscard]] bool hasReading() const;

  /// return a comma separated string containing values for the given `fields`
  /// \details if a field is requested, but it's empty or doesn't have any data
  /// (like a `None` value for an enum) then it won't be included
  /// \note multiple 'Old' links are separated by '／' (wide slash) and a link
  /// is followed by '*' if it was used to pull in readings.
  [[nodiscard]] String info(Info fields = Info::All) const;

  /// return name() plus an extra 'suffix' to show more info
  /// \details Suffixes are:
  /// \li . = Jouyou        : 2136 Jouyou
  /// \li ' = JLPT          : 251 Jinmei in JLPT - other 1971 JLPT are Jouyou
  /// \li " = Top Frequency : 296 top frequency not in Jouyou or JLPT
  /// \li ^ = Jinmei        : 224 Jinmei not already covered by the above types
  /// \li ~ = Linked Jinmei : 218 Linked Jinmei (with no frequency)
  /// \li % = Linked Old    : 211 Linked Old (with no frequency)
  /// \li + = Extra         : all kanji loaded from 'extra.txt' file
  /// \li @ = <K1 Kentei    : 268 non-K1 Kentei Kanji that aren't included above
  /// \li # = K1 Kentei     : 2554 K1 Kentei Kanji that aren't included above
  /// \li * = Ucd           : kanji loaded from 'ucd.txt' not included above
  [[nodiscard]] String qualifiedName() const;

  /// Sort in a way that corresponds to qualifiedName() output, i.e., 'Jouyou'
  /// followed by 'JLPT' followed by 'Frequency', etc.. If both Kanji have the
  /// same qualifiedNameRank() then return orderByStrokes().
  [[nodiscard]] bool orderByQualifiedName(const Kanji&) const;

  /// Sort by stokes() (smallest first) and if they are the same then sort by
  /// frequency() and finally compatibilityName() (in unicode).
  [[nodiscard]] bool orderByStrokes(const Kanji&) const;

  [[nodiscard]] bool operator==(const Kanji&) const;

  /// can be used in ouput to briefly describe the suffix added when using the
  /// qualifiedName() method
  static constexpr auto Legend{".=常用 '=JLPT \"=Freq ^=人名用 ~=LinkJ %=LinkO "
                               "+=Extra @=検定 #=1級 *=Ucd"};
protected:
  /// ctor used by OfficialLinkedKanji and LoadedKanji classes
  Kanji(KanjiDataRef, Name, RadicalRef, Strokes, UcdPtr);

  /// ctor used by above ctor as well as test code
  Kanji(Name name, const OptString& compatibilityName, RadicalRef radical,
      Strokes strokes, const Pinyin& pinyin, const MorohashiId& morohashiId,
      NelsonIds nelsonIds)
      : _name{name}, _compatibilityName{compatibilityName}, _radical{radical},
        _strokes{strokes}, _pinyin{pinyin}, _morohashiId{morohashiId},
        _nelsonIds{std::move(nelsonIds)} {}

  inline static const LinkNames EmptyLinkNames;
private:
  /// helper class that provides additional checks and methods related to String
  /// 'name' of a Kanji (possibly extend this more later)
  class KanjiName final {
  public:
    explicit KanjiName(Name name);

    [[nodiscard]] Name name() const { return _name; }

    /// true if #_name includes a Unicode 'variation selector'. In this case
    /// nonVariant() returns #_name without the selector.
    [[nodiscard]] bool isVariant() const;
    [[nodiscard]] String nonVariant() const;
  private:
    const String _name;
  };

  /// suffixes for qualified names in order of most common to least common, see
  /// comments for qualifiedName()
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

/// enable bitwise operators for Kanji::Info
template<> inline constexpr auto is_bitmask<Kanji::Info>{true};

// Kanji derived classes

/// contains 'meaning' and 'reading' fields \kanji{Kanji}
class LoadedKanji : public Kanji {
public:
  [[nodiscard]] Meaning meaning() const final { return _meaning; }
  [[nodiscard]] Reading reading() const final { return _reading; }
protected:
  /// ctor used by OtherKanji and ExtraKanji to populate links from Ucd data
  [[nodiscard]] static LinkNames linkNames(UcdPtr);

  /// ctor used by NumberedKanji
  LoadedKanji(
      KanjiDataRef, Name, RadicalRef, Strokes, Meaning, Reading, UcdPtr);

  /// ctor used by NumberedKanji and OtherKanji: looks up 'meaning' and
  /// 'strokes' from `UcdPtr`
  LoadedKanji(KanjiDataRef, Name, RadicalRef, Reading, UcdPtr);
private:
  const String _meaning;
  const String _reading;
};

/// base class for Kanji with fields mainly loaded from 'ucd.txt' as opposed to
/// Kanji loaded from 'jouyou.txt', 'jinmei.txt', and 'extra.txt' \kanji{Kanji}
///
/// There are 'hasOldLinks' and 'linkNames' fields for supporting 'ucd links' as
/// well as 'linkedReadings'. OtherKanji aren't part of JLPT.
class OtherKanji : public LoadedKanji {
public:
  [[nodiscard]] const LinkNames& oldNames() const final;
  [[nodiscard]] OptString newName() const final;
  [[nodiscard]] bool linkedReadings() const final { return _linkedReadings; }
protected:
  /// ctor used by 'StandardKanji': has 'reading'
  OtherKanji(KanjiDataRef, Name, Reading, UcdPtr);
  /// ctor used by 'StandardKanji' and 'UcdKanji': looks up 'reading'
  OtherKanji(KanjiDataRef, Name, UcdPtr);
private:
  const bool _hasOldLinks;

  /// Use 'LinkNames' instead of trying to hold pointers to other Kanji since
  /// 'ucd links' are more arbitrary than the standard 'official' Jinmei and
  /// Jouyou linked Kanji (ie official variants). Ucd links can potentially even
  /// be circular depending on how the source data is parsed and there are also
  /// cases of links to another ucd Kanji with a link.
  const LinkNames _linkNames;

  const bool _linkedReadings;
};

/// base class for FrequencyKanji and KenteiKanji \kanji{Kanji}
///
/// StandardKanji have a 'kyu' field
class StandardKanji : public OtherKanji {
public:
  [[nodiscard]] KenteiKyus kyu() const final { return _kyu; }
protected:
  /// ctor used by FrequencyKanji: has 'reading' and looks up 'kyu'
  StandardKanji(KanjiDataRef, Name, Reading);

  /// ctor used by FrequencyKanji: looks up 'kyu'
  StandardKanji(KanjiDataRef, Name);

  /// ctor used by KenteiKanji: has 'kyu'
  StandardKanji(KanjiDataRef, Name, KenteiKyus);
private:
  const KenteiKyus _kyu;
};

/// class for Kanji in the top 2,501 frequency list ('frequency.txt') that
/// haven't already been loaded from a 'jouyou' or 'jinmei' file \kanji{Kanji}
class FrequencyKanji final : public StandardKanji {
public:
  /// ctor used for FrequencyKanji without a reading
  FrequencyKanji(KanjiDataRef, Name, Frequency);

  /// ctor used for FrequencyKanji with a reading from 'frequency-readings.txt'
  FrequencyKanji(KanjiDataRef, Name, Reading, Frequency);

  [[nodiscard]] KanjiTypes type() const final { return KanjiTypes::Frequency; }
  [[nodiscard]] Frequency frequency() const final { return _frequency; }
private:
  const Frequency _frequency;
};

/// class for kanji in 'kentei/k*.txt' files that aren't already pulled in from
/// other files \kanji{Kanji}
class KenteiKanji final : public StandardKanji {
public:
  KenteiKanji(KanjiDataRef, Name, KenteiKyus);

  [[nodiscard]] KanjiTypes type() const final { return KanjiTypes::Kentei; }
};

/// class for Kanji in 'ucd.txt' file that aren't already included in any other
/// 'types' \kanji{Kanji}
///
/// Many of these Kanji have a Morohashi ID (ie they are in 'Dai Kan-Wa Jiten'),
/// but others are pulled in via links and may not even have a Japanese reading.
class UcdKanji final : public OtherKanji {
public:
  UcdKanji(KanjiDataRef, const Ucd&);

  [[nodiscard]] KanjiTypes type() const final { return KanjiTypes::Ucd; }
};

/// \end_group
} // namespace kanji_tools
