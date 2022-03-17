#pragma once

#include <string>

namespace kanji_tools {

// 'WhatMismatch' is an exception class that is thrown by the 'call' function
// below when an exception has a 'what' that doesn't match the expected value.
class WhatMismatch : public std::runtime_error {
public:
  WhatMismatch(const std::string& expectedWhat, const std::exception& e)
      : std::runtime_error{
            "expected: '" + expectedWhat + "', actual: '" + e.what() + '\''} {}
};

// 'call' is a helper function for tests that expect exceptions to be thrown. It
// calls the given function 'f' and then checks if the exception raised matches
// 'expectedWhat'. If it doesn't match then 'WhatMismatch' is thrown, otherwise
// the original exception is re-thrown to be handled by the test macro
// (EXPECT_THROW, ASSERT_THROW, etc.) which is only able to test the type of
// exception (not the 'what' string).
template<typename F> auto call(const F& f, const std::string& expectedWhat) {
  try {
    return f();
  } catch (const std::exception& e) {
    if (expectedWhat != e.what()) throw WhatMismatch(expectedWhat, e);
    throw;
  }
}

} // namespace kanji_tools
