#include <kt_utils/BlockRange.h>
#include <kt_utils/Exception.h>

namespace kanji_tools {

size_t BaseBlockRange::checkIndex(size_t i, size_t size) {
  if (i > size)
    throw RangeError("index '" + std::to_string(i) +
                     "' is out of range for BlockRange with size '" +
                     std::to_string(size) + "'");
  return i;
}

} // namespace kanji_tools
