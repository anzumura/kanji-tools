#ifndef KANJI_GROUP_H
#define KANJI_GROUP_H

#include <kanji/Kanji.h>

namespace kanji {

enum class GroupType { Meaning, Pattern };
inline std::ostream& operator<<(std::ostream& os, const GroupType& x) {
  return os << (x == GroupType::Meaning ? "Meaning" : "Pattern");
}

// 'Group' is meant to hold a group of 'related' kanji from 'meaning-groups.txt' or
// 'pattern-groups.txt' for study purposes. Meaning groups are intended to be used
// for meaning categories such as 'Animal', 'Plant', etc. whereas Pattern groups are
// intended to be organized by 'non-radical' parts in order to help see related kanji
// that only differ by radical. Right now a kanji can only belong to at most one
// group per type (Meaning or Pattern) which makes the 'group' concept arbitrary at
// best since more complex kanji could have more than one pattern (and many kanjis
// have more than one meaning).
class Group {
public:
  // 'peers' should be false for Meaning groups, but could be true for a Pattern
  // group where 'name' is just one of the 'members' (instead of being a 'parent').
  // For example, a 'peers=false' group could have name='太' and members: '太, 駄, 汰'
  // whereas a 'peers=true' group could have name='粋' and members: '粋, 枠, 砕'. Note,
  // 'name' is just an arbitrary label for a Meaning group, whereas it is the 'name'
  // of the first member in a Pattern group, i.e., the basis of the pattern (plus the
  // main common pronunciations after a colon).
  Group(int number, const std::string& name, const Data::List& members)
    : _number(number), _name(name), _members(members) {}
  virtual ~Group() = default;
  Group(const Group&) = delete;

  virtual GroupType type() const = 0;
  virtual bool peers() const { return false; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }
  const Data::List& members() const { return _members; }
  std::string toString() const { return "[" + std::to_string(_number) + ' ' + name() + (peers() ? "*]" : "]"); }
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
  PatternGroup(int number, const std::string& name, const Data::List& members, bool peers)
    : Group(number, name, members), _peers(peers) {}

  GroupType type() const override { return GroupType::Pattern; }
  bool peers() const override { return _peers; }
private:
  const bool _peers;
};

inline std::ostream& operator<<(std::ostream& os, const Group& x) {
  os << '[';
  if (x.peers()) {
    auto i = x.members().begin();
    os << "Peers ";
    if (i != x.members().end()) os << (**i).name();
  }
  return os << x.name() << ']';
}

} // namespace kanji

#endif // KANJI_GROUP_H
