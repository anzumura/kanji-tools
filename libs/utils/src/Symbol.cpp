#include <kanji_tools/utils/Symbol.h>

#include <numeric>
#include <stdexcept>

namespace kanji_tools {

BaseSymbol::Id BaseSymbol::check(
    size_t id, const std::string& symbolType, const std::string& symbol) {
  if (id == std::numeric_limits<Id>::max())
    throw std::domain_error{
        symbolType + ": can't add '" + symbol + "' - max capacity"};
  return static_cast<Id>(id);
}

} // namespace kanji_tools
