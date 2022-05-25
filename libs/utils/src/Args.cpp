#include <kanji_tools/utils/Args.h>
#include <kanji_tools/utils/String.h>

#include <stdexcept>

namespace kanji_tools {

Args::Args(Size argc, List argv) : _argc{argc}, _argv{argv} {
  if (!argc) {
    if (argv) throw std::domain_error("argc is 0, but argv is not null");
  } else if (!argv)
    throw std::domain_error(
        "argc is " + std::to_string(argc) + ", but argv is null");
}

const char* Args::operator[](Size i) const {
  if (i >= _argc)
    throw std::range_error("index " + std::to_string(i) +
                           " must be less than argc " + std::to_string(_argc));
  return _argv[i];
}

Args::Size Args::checkInt(int argc) {
  if (argc < 0)
    throw std::range_error("argc " + std::to_string(argc) + " is less than 0");
  if (const auto limit{std::numeric_limits<Size>::max()}; argc > limit)
    throw std::range_error("argc " + std::to_string(argc) +
                           " is greater than " + std::to_string(limit));
  return static_cast<Size>(argc);
}

} // namespace kanji_tools
