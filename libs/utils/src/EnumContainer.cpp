#include <kanji_tools/utils/EnumContainer.h>

#include <stdexcept>

namespace kanji_tools {

void Enum::BaseIterator::comparable(bool value) {
  if (!value) throw std::domain_error("not comparable");
}

void Enum::BaseIterator::initialized(bool value) {
  if (!value) throw std::domain_error("not initialized");
}

void Enum::BaseIterator::rangeError(const String& msg) {
  Enum::rangeError(msg);
}

Enum::BaseIterator::BaseIterator(Size index) noexcept : _index{index} {}

Enum::Size& Enum::BaseIterator::index() { return _index; }

Enum::Size Enum::BaseIterator::index() const { return _index; }

void Enum::rangeError(const String& msg) { throw std::out_of_range{msg}; }

} // namespace kanji_tools
