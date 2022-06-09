#pragma once

#include <kanji_tools/kanji/KanjiData.h>

namespace kanji_tools { /// \quiz_group{Group}
/// Group class hierarchy for holding groups of related Kanji

/// two different types of groups are supported
enum class GroupType {
  Meaning, ///< Kanji 'meanings' like 'Animal', 'Plant', etc., see MeaningGroup
  Pattern  ///< Kanji 'patterns' (mostly 'non-radical' parts), see PatternGroup
};

/// stream operator for GroupType
std::ostream& operator<<(std::ostream&, const GroupType&);

/// base class for MeaningGroup and PatternGroup \quiz{Group}
class Group {
public:
  using Number = uint16_t;
  using Members = KanjiData::List;

  /// Set max size for a group to '58' since this is the max number of entries
  /// that a 'group quiz' currently supports for entering answers, i.e., a-z,
  /// then A-Z, then 6 characters following Z (before reaching 'a' again).
  static constexpr uint16_t MaxGroupSize{58};

  /// type of pattern for a PatternGroup object or 'None' for MeaningGroup
  enum class PatternType {
    Family, ///< The main entry is a 'sub-component' of the other members, e.g.:
            ///< 太 is the parent of 駄 and 汰. Radicals can form a family, but
            ///< the focus of PatternGroup is phonetic components, so in general
            ///< 'parent' corresponds to the 'non-radical' part of the members.
    Peer,    ///< Exch member has a 'common part' contributing to pronunciation,
             ///< e.g.: 粋, 枠 and 砕 have a common 'non-radical' component.
    Reading, ///< Used as a catch-all for Kanji that don't logically fit into
             ///< Family or Peer groups. These are generally simple characters
             ///< such as 凹 or 後 as well as some radicals.
    None     ///< Used for MeaningGroup, i.e., no pattern type
  };

  Group(const Group&) = delete;
  virtual ~Group() = default;

  [[nodiscard]] virtual GroupType type() const = 0;
  [[nodiscard]] virtual PatternType patternType() const;

  [[nodiscard]] auto number() const { return _number; }
  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto& members() const { return _members; }
  [[nodiscard]] String toString() const;
protected:
  /// ctor called by derived classes
  /// \param number unique number per derived group type starting at `1`
  /// \param name group name in Japanese (format depends on derived type)
  /// \param members list of unique group members
  /// \throw DomainError if `members` has duplicates, has less than two entries
  ///     or has more than #MaxGroupSize entries
  Group(Number number, const String& name, const Members& members);

  /// used by ctor for reporting errors
  /// \param msg to be added to the exception message
  /// \throw DomainError containing the group name and `msg`
  void error(const String& msg) const;
private:
  const Number _number;
  const String _name;
  const Members _members;
};

using GroupPtr = std::shared_ptr<Group>;

/// groups based on Kanji 'meanings' \quiz{Group}
///
/// A Kanji can be in more than one MeaningGroup since choosing only one group
/// for some (even very common) Kanji would make other groups seem incomplete,
/// e.g., if '金' was only in the '色' group then the '時間：曜日' group would
/// be missing a day. These groups are defined in 'meaning-groups.txt' and are
/// currently fairly ad-hoc (containing about 1,500 Kanji) - some groups could
/// probably be split up more and many more Kanji should be added to existing or
/// new groups.
class MeaningGroup : public Group {
public:
  MeaningGroup(Number, const String& name, const Members&);

  [[nodiscard]] GroupType type() const final;
};

/// groups based on Kanji 'patterns' \quiz{Group}
///
/// These groups are mostly organized by 'non-radical' parts of Kanji in order
/// to help see related Kanji that often have the same pronunciation. GroupData
/// class enforces that one Kanji can't be in more than one PatternGroup. This
/// restriction can lead to ambiguities for some more complex Kanji so in these
/// cases, the pattern that best fits related pronunciation was chosen (as well
/// as preferring to group by 'non-radical') for data in 'pattern-groups.txt'.
///
/// The groups in 'pattern-groups.txt' are fairly comprehensive, containing
/// close to 6,000 unique Kanji (including all Jouyou, Jinmei, Frequency and
/// Extra types). The convention used for the group 'name' field depends on the
/// pattern type:
/// \li Family: `"P：R"` where `P` is parent Kanji and `R` is 1 to 3 of the most
///   common readings across the group, e.g., "亜：ア、アク". Note, `P` isn't
///   repeated in the member list, but does get added as a member by GroupData
/// \li Peer: `"：R"` where `R` is the same as above
/// \li Reading: `"R"` where `R` is the same as above
class PatternGroup : public Group {
public:
  PatternGroup(Number, const String& name, const Members&, PatternType);

  [[nodiscard]] GroupType type() const final;
  [[nodiscard]] PatternType patternType() const final;
private:
  const PatternType _patternType;
};

std::ostream& operator<<(std::ostream&, const Group&);

/// \end_group
} // namespace kanji_tools
