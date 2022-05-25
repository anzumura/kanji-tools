#include <kanji_tools/utils/BlockRange.h>
#include <kanji_tools/utils/Exception.h>

namespace kanji_tools {

void BaseBlockRange::fill(wchar_t* i, const UnicodeBlock& block) noexcept {
  *i++ = block.wStart();
  *i++ = L'-';
  *i = block.wEnd();
}

size_t BaseBlockRange::checkIndex(size_t i, size_t size) {
  if (i > size)
    throw RangeError("index '" + std::to_string(i) +
                     "' is out of range for BlockRange with size '" +
                     std::to_string(size) + "'");
  return i;
}

} // namespace kanji_tools
