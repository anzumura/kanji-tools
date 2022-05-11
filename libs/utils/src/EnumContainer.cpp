#include <kanji_tools/utils/EnumContainer.h>

#include <stdexcept>

namespace kanji_tools {

void EnumContainer::BaseIterator::comparable(bool value) {
  if (!value) throw std::domain_error("not comparable");
}

void EnumContainer::BaseIterator::initialized(bool value) {
  if (!value) throw std::domain_error("not initialized");
}

void EnumContainer::BaseIterator::rangeError(const String& msg) {
  EnumContainer::rangeError(msg);
}

EnumContainer::BaseIterator::BaseIterator(Size index) noexcept
    : _index{index} {}

EnumContainer::Size& EnumContainer::BaseIterator::index() { return _index; }

EnumContainer::Size EnumContainer::BaseIterator::index() const {
  return _index;
}

void EnumContainer::rangeError(const String& msg) {
  throw std::out_of_range{msg};
}

} // namespace kanji_tools
