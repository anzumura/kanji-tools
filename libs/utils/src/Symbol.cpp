#include <kt_utils/Exception.h>
#include <kt_utils/Symbol.h>

namespace kanji_tools {

BaseSymbol::BaseSymbol(const String& type, const String& name, Map& m, List& l)
    : _id{name.empty() ? Id{} : getId(type, name, m, l)} {}

BaseSymbol::Id BaseSymbol::getId(
    const String& type, const String& name, Map& m, List& l) {
  if (l.size() < Max) {
    // id '0' is used for 'empty' case so non-empty symbols should start at '1'
    const auto i{m.emplace(name, l.size() + 1)};
    if (i.second) l.emplace_back(&i.first->first);
    return i.first->second;
  }
  // allow finding existing symbols even if at max capacity
  if (const auto i{m.find(name)}; i != m.end()) return i->second;
  throw DomainError{type + ": can't add '" + name + "' - max capacity"};
}

} // namespace kanji_tools
