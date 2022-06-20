#pragma once

#include <kt_utils/String.h>

namespace kanji_tools {

/// exception class thrown by the call() function below when an exception has a
/// what() string that doesn't match the expected value
class WhatMismatch final : public std::runtime_error {
public:
  WhatMismatch(const String& expectedWhat, const std::exception& e)
      : std::runtime_error{
            "expected: '" + expectedWhat + "', actual: '" + e.what() + '\''} {}
};

/// checks what() of exceptions thrown by `f` against `expectedWhat` and throws
/// WhatMismatch if they don't match, otherwise re-throws the original exception
/// \details This function is helpful when using test macros like EXPECT_THROW
/// or ASSERT_THROW which can only check the type of exception thrown (not the
/// the 'what' message)
/// \tparam F type of `f` (can return any type, but can't take any arguments)
/// \param f function to call
/// \param expectedWhat expected value of what() of the exception thrown
/// \return the return value of `f`
/// \throw WhatMismatch if `f` throws an exception and `expectedWhat` doesn't
///     match what() (otherwise the original exception is re-thrown)
template <typename F> auto call(const F& f, const String& expectedWhat) {
  try {
    return f();
  } catch (const std::exception& e) {
    if (expectedWhat != e.what()) throw WhatMismatch{expectedWhat, e};
    throw;
  }
}

} // namespace kanji_tools
