#include <kanji_tools/quiz/Group.h>

namespace kanji_tools {

Group::Group(size_t number, const std::string& name, const Data::List& members)
    : _number{number}, _name{name}, _members{members} {
  if (members.empty()) error("no members");
  if (members.size() == 1) error("only one member");
  if (members.size() > MaxGroupSize)
    error("more than " + std::to_string(MaxGroupSize) + " members");
  std::set<std::string> names, dups;
  for (auto& i : members)
    if (!names.emplace(i->name()).second) dups.emplace(i->name());
  if (!dups.empty()) {
    std::string msg{std::to_string(dups.size()) + " duplicate member" +
                    (dups.size() > 1 ? "s:" : ":")};
    // output the list of dups in the same order as they were in 'members'
    for (auto& i : members)
      if (dups.erase(i->name())) msg += " " + i->name();
    error(msg);
  }
}

std::string Group::toString() const {
  return "[" + std::to_string(_number) + ' ' + name() + ']';
}

void Group::error(const std::string& msg) const {
  throw std::domain_error{"group " + toString() + " has " + msg};
}

PatternGroup::PatternGroup(size_t number, const std::string& name,
    const Data::List& members, PatternType patternType)
    : Group{number, name, members}, _patternType{patternType} {
  if (patternType == PatternType::None) error("invalid pattern type");
}

std::ostream& operator<<(std::ostream& os, const Group& x) {
  os << '[';
  if (x.patternType() == Group::PatternType::Peer) {
    os << "Peers ";
    if (const auto i{x.members().begin()}; i != x.members().end())
      os << (**i).name();
  }
  return os << x.name() << ']';
}

} // namespace kanji_tools
