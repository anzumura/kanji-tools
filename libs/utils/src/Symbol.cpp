#include <kanji_tools/utils/Symbol.h>

#include <numeric>
#include <stdexcept>

namespace kanji_tools {

BaseSymbol::Id BaseSymbol::getId(
    const std::string& type, const std::string& name, Map& m, List& l) {
  if (l.size() == std::numeric_limits<Id>::max())
    throw std::domain_error{type + ": can't add '" + name + "' - max capacity"};
  const auto i{m.emplace(name, l.size())};
  if (i.second) l.emplace_back(&i.first->first);
  return i.first->second;
}

} // namespace kanji_tools
