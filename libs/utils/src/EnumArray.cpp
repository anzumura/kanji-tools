#include <kanji_tools/utils/EnumArray.h>

namespace kanji_tools {

BaseEnumArray::~BaseEnumArray() = default;

void BaseEnumArray::domainError(const String& msg) {
  throw std::domain_error{msg};
}

} // namespace kanji_tools
