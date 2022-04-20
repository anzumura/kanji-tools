#include <kanji_tools/utils/IterableEnum.h>

#include <stdexcept>

namespace kanji_tools {

void BaseEnum::BaseIterator::comparable(bool value) {
  if (!value) throw std::domain_error("not comparable");
}

void BaseEnum::BaseIterator::initialized(bool value) {
  if (!value) throw std::domain_error("not initialized");
}

void BaseEnum::BaseIterator::rangeError(const std::string& msg) {
  BaseEnum::rangeError(msg);
}

BaseEnum::BaseIterator::BaseIterator(Size index) noexcept
    : _index{index} {}

BaseEnum::Size& BaseEnum::BaseIterator::index() {
  return _index;
}

BaseEnum::Size BaseEnum::BaseIterator::index() const {
  return _index;
}

void BaseEnum::rangeError(const std::string& msg) {
  throw std::out_of_range{msg};
}

} // namespace kanji_tools
