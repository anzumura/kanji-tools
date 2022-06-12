#include <kt_utils/Exception.h>

namespace kanji_tools {

DomainError::DomainError(const String& s) : std::domain_error{s} {}

RangeError::RangeError(const String& s) : std::range_error{s} {}

} // namespace kanji_tools
