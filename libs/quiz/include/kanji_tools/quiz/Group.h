#pragma once

#include <kanji_tools/kanji/KanjiData.h>

namespace kanji_tools {

enum class GroupType { Meaning, Pattern };

inline auto& operator<<(std::ostream& os, const GroupType& x) {
  return os << (x == GroupType::Meaning ? "Meaning" : "Pattern");
}

// 'Group' holds kanji groups from 'meaning-groups.txt' and 'pattern-groups.txt'
// files. Meaning groups are intended to be used for meaning categories like
// 'Animal', 'Plant', etc. whereas Pattern groups are mostly organized by
// 'non-radical' parts in order to help see related kanji that often have the
// same pronunciation.
class Group {
public:
  // For now, set max size for a group to '58' since this is the maximum number
  // of entries that the group quiz currently supports for entering answers,
  // i.e., a-z, then A-Z, then 6 more ascii characters following Z (before
  // reaching 'a' again).
  static constexpr uint16_t MaxGroupSize{58};

  // 'PatternType' is 'None' for Meaning groups, but for a Pattern Group it can
  // be one of three values:
  // - 'Family': a pattern where the first character is a parent of the other
  //   members, for example: 太 -> 太, 駄, 汰, i.e., 'parent' is a sub-component
  //   of each member. Radicals can form families, but the focus of pattern
  //   groups is on phonetic components so in general 'parent' corresponds to
  //   the 'non-radical' part of the members.
  // - 'Peer': each member has a common part contributing to pronunciation, for
  //   example 粋, 枠 and 砕 have a common 'non-radical' component.
  // - 'Reading': this type is used as a final catch-all for characters that
  //   don't logically fit into a 'Family' or 'Peer' group. These are generally
  //   simpler characters such as 凹 or 後 as well as some radicals.
  enum class PatternType { Family, Peer, Reading, None };

  // ctor throws if 'members' contains duplicates, has less than two entries or
  // has more than 'MaxGroupSize' entries
  Group(size_t number, const String& name, const KanjiData::List& members);

  Group(const Group&) = delete;
  virtual ~Group() = default;

  [[nodiscard]] virtual GroupType type() const = 0;
  [[nodiscard]] virtual PatternType patternType() const {
    return PatternType::None;
  }

  [[nodiscard]] auto number() const { return _number; }
  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto& members() const { return _members; }
  [[nodiscard]] String toString() const;
protected:
  void error(const String&) const;
private:
  const size_t _number;
  const String _name;
  const KanjiData::List _members;
};

using GroupPtr = std::shared_ptr<Group>;

class MeaningGroup : public Group {
public:
  MeaningGroup(
      size_t number, const String& name, const KanjiData::List& members)
      : Group{number, name, members} {}

  [[nodiscard]] GroupType type() const override { return GroupType::Meaning; }
};

class PatternGroup : public Group {
public:
  PatternGroup(size_t number, const String& name,
      const KanjiData::List& members, PatternType patternType);

  [[nodiscard]] GroupType type() const override { return GroupType::Pattern; }
  [[nodiscard]] PatternType patternType() const override {
    return _patternType;
  }
private:
  const PatternType _patternType;
};

std::ostream& operator<<(std::ostream&, const Group&);

} // namespace kanji_tools
