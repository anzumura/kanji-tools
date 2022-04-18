#include <kanji_tools/utils/Symbol.h>

#include <stdexcept>

namespace kanji_tools {

BaseSymbol::Id BaseSymbol::getId(
    const std::string& type, const std::string& name, Map& m, List& l) {
  if (name.empty()) return 0;
  if (l.size() == Max)
    throw std::domain_error{type + ": can't add '" + name + "' - max capacity"};
  // id '0' is used for 'empty' case so non-empty symbols should start at '1'
  const auto i{m.emplace(name, l.size() + 1)};
  if (i.second) l.emplace_back(&i.first->first);
  return i.first->second;
}

} // namespace kanji_tools
