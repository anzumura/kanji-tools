#include <kanji_tools/utils/Args.h>

#include <stdexcept>
#include <string>

namespace kanji_tools {

Args::Args(Size size, List args) : _size{size}, _args{args} {
  if (!size) {
    if (args) throw std::domain_error("size is 0, but args is not null");
  } else if (!args)
    throw std::domain_error(
        "size is " + std::to_string(size) + ", but args is null");
}

const char* Args::operator[](Size i) const {
  if (i >= _size)
    throw std::domain_error("index " + std::to_string(i) +
                            " must be less than size " + std::to_string(_size));
  return _args[i];
}

Args::Size Args::checkInt(int size) {
  if (size < 0)
    throw std::domain_error("size " + std::to_string(size) + " is less than 0");
  if (const auto limit{std::numeric_limits<Size>::max()}; size > limit)
    throw std::domain_error("size " + std::to_string(size) +
                            " is greater than " + std::to_string(limit));
  return static_cast<Size>(size);
}

} // namespace kanji_tools
