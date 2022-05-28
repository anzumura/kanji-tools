#pragma once

#include <kanji_tools/utils/String.h>

#include <stdexcept>

namespace kanji_tools { /// \utils_group{Exception}

/// class for domain error exceptions (prevents copying) \utils{Exception}
class DomainError : public std::domain_error {
public:
  /// pull in base ctors
  using std::domain_error::domain_error;

  /// ctor taking #String
  explicit DomainError(const String&);

  DomainError(const DomainError&) = delete;    ///< deleted copy ctor
  auto operator=(const DomainError&) = delete; ///< deleted operator=
};

/// class for range error exceptions (prevents copying) \utils{Exception}
class RangeError : public std::range_error {
public:
  /// pull in base ctors
  using std::range_error::range_error;

  /// ctor taking #String
  explicit RangeError(const String&);

  RangeError(const RangeError&) = delete;     ///< deleted copy ctor
  auto operator=(const RangeError&) = delete; ///< deleted operator=
};

/// \end_group
} // namespace kanji_tools
