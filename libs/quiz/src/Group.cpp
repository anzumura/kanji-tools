#include <kanji_tools/quiz/Group.h>

namespace kanji_tools {

std::string Group::toString() const {
  return "[" + std::to_string(_number) + ' ' + name() + ']';
}

PatternGroup::PatternGroup(size_t number, const std::string& name,
    const Data::List& members, PatternType patternType)
    : Group{number, name, members}, _patternType{patternType} {
  if (patternType == PatternType::None)
    throw std::domain_error{
        "PatternGroup: '" + name + "' has invalid pattern type"};
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
