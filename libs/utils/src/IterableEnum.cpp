#include <kanji_tools/utils/IterableEnum.h>

#include <stdexcept>

namespace kanji_tools {

void BaseIterableEnum::BaseIterator::comparable(bool value) {
  if (!value) throw std::domain_error("not comparable");
}

void BaseIterableEnum::BaseIterator::initialized(bool value) {
  if (!value) throw std::domain_error("not initialized");
}

void BaseIterableEnum::BaseIterator::rangeError(const std::string& msg) {
  BaseIterableEnum::rangeError(msg);
}

BaseIterableEnum::BaseIterator::BaseIterator(Index index) noexcept
    : _index{index} {}

BaseIterableEnum::Index& BaseIterableEnum::BaseIterator::index() {
  return _index;
}

BaseIterableEnum::Index BaseIterableEnum::BaseIterator::index() const {
  return _index;
}

void BaseIterableEnum::rangeError(const std::string& msg) {
  throw std::out_of_range{msg};
}

} // namespace kanji_tools
