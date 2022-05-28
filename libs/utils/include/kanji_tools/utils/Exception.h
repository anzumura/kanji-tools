#pragma once

#include <kanji_tools/utils/String.h>

#include <stdexcept>

namespace kanji_tools { /// \utils_group{Exception}

/// class for domain error exceptions (prevents copying) \utils{Exception}
class DomainError : public std::domain_error {
public:
  explicit DomainError(const String&);
  explicit DomainError(const char*);
  DomainError(const DomainError&) = delete;
  DomainError& operator=(const DomainError&) = delete;
};

/// class for range error exceptions (prevents copying) \utils{Exception}
class RangeError : public std::range_error {
public:
  explicit RangeError(const String&);
  explicit RangeError(const char*);
  RangeError(const RangeError&) = delete;
  RangeError& operator=(const RangeError&) = delete;
};

/// \end_group
} // namespace kanji_tools
