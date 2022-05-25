#pragma once

#include <kanji_tools/utils/String.h>

#include <stdexcept>

namespace kanji_tools {

//! \lib_utils{Exception} class for domain error exceptions (prevents copying)
class DomainError : public std::domain_error {
public:
  explicit DomainError(const String&);
  explicit DomainError(const char*);
  DomainError(const DomainError&) = delete;
  DomainError& operator=(const DomainError&) = delete;
};

//! \lib_utils{Exception} class for range error exceptions (prevents copying)
class RangeError : public std::range_error {
public:
  explicit RangeError(const String&);
  explicit RangeError(const char*);
  RangeError(const RangeError&) = delete;
  RangeError& operator=(const RangeError&) = delete;
};

} // namespace kanji_tools
