#ifndef KANJI_TOOLS_QUIZ_GROUP_H
#define KANJI_TOOLS_QUIZ_GROUP_H

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

enum class GroupType { Meaning, Pattern };
inline auto& operator<<(std::ostream& os, const GroupType& x) {
  return os << (x == GroupType::Meaning ? "Meaning" : "Pattern");
}

// 'Group' holds kanji groups loaded from 'meaning-groups.txt' and 'pattern-groups.txt' files.
// Meaning groups are intended to be used for meaning categories like 'Animal', 'Plant', etc.
// whereas Pattern groups are mostly organized by 'non-radical' parts in order to help see
// related kanji that often have the same pronunciation.
//
// The 'GroupData' class prevents a kanji from being in multiple Pattern groups which can be
// ambiguous for some more complex kanji. In these fairly rare cases, the pattern that best
// fits related pronunciation was chosen (as well as preferring grouping by 'non-radical').
// This restriction doesn't apply to Meaning groups since choosing only one meaning for some
// (even very common) kanji would make other groups seem incomplete, e.g., if '金' was only
// in the '色' group then the '時間：曜日' group would be missing a day.
class Group {
public:
  // 'PatternType' is 'None' for Meaning groups, but for a Pattern Group it can be one of
  // three values:
  // - 'Family': a pattern where the first character is a parent of the other members, for
  //   example: 太 -> 太, 駄, 汰, i.e., 'parent' is a sub-component of each member. Radicals
  //   can form families, but the focus of pattern groups is on phonetic components so in
  //   general 'parent' should correspond to the 'non-radical' part of the members.
  // - 'Peer': each member has a common part contributing to common pronunciation if possible,
  //   for example: 粋, 枠 and 砕 all have a common 'non-radical' component.
  // - 'Reading': this type is used as a final catch-all for characters that don't logically
  //   fit into a 'Family' or 'Peer' group. There are generally simpler characters such as 凹
  //   or 後 as well as some radicals.
  enum class PatternType { Family, Peer, Reading, None };

  Group(int number, const std::string& name, const Data::List& members)
    : _number(number), _name(name), _members(members) {}
  virtual ~Group() = default;
  Group(const Group&) = delete;

  virtual GroupType type() const = 0;
  virtual PatternType patternType() const { return PatternType::None; }

  int number() const { return _number; }
  auto& name() const { return _name; }
  auto& members() const { return _members; }
  auto toString() const { return "[" + std::to_string(_number) + ' ' + name() + ']'; }
private:
  const int _number;
  const std::string _name;
  const Data::List _members;
};

class MeaningGroup : public Group {
public:
  MeaningGroup(int number, const std::string& name, const Data::List& members) : Group(number, name, members) {}

  GroupType type() const override { return GroupType::Meaning; }
};

class PatternGroup : public Group {
public:
  PatternGroup(int number, const std::string& name, const Data::List& members, PatternType patternType)
    : Group(number, name, members), _patternType(patternType) {}

  GroupType type() const override { return GroupType::Pattern; }
  PatternType patternType() const override { return _patternType; }
private:
  const PatternType _patternType;
};

inline auto& operator<<(std::ostream& os, const Group& x) {
  os << '[';
  if (x.patternType() == Group::PatternType::Peer) {
    auto i = x.members().begin();
    os << "Peers ";
    if (i != x.members().end()) os << (**i).name();
  }
  return os << x.name() << ']';
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_GROUP_H
