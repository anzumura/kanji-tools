#include <kt_utils/EnumContainer.h>
#include <kt_utils/Exception.h>

namespace kanji_tools {

void Enum::BaseIterator::comparable(bool value) {
  if (!value) throw DomainError("not comparable");
}

void Enum::BaseIterator::initialized(bool value) {
  if (!value) throw DomainError("not initialized");
}

void Enum::BaseIterator::rangeError(const String& msg) {
  Enum::rangeError(msg);
}

Enum::BaseIterator::BaseIterator(Size index) noexcept : _index{index} {}

Enum::Size& Enum::BaseIterator::index() { return _index; }

Enum::Size Enum::BaseIterator::index() const { return _index; }

void Enum::rangeError(const String& msg) { throw RangeError{msg}; }

} // namespace kanji_tools
