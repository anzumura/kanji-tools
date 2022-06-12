#include <gtest/gtest.h>
#include <kt_utils/Exception.h>

namespace kanji_tools {

TEST(ExceptionTest, DomainErrorCtors) {
  const auto charMsg{"testMsg"};
  const String msg{charMsg};
  EXPECT_STREQ(charMsg, DomainError{charMsg}.what());
  EXPECT_EQ(msg, DomainError{msg}.what());
}

TEST(ExceptionTest, RangeErrorCtors) {
  const auto charMsg{"testMsg"};
  const String msg{charMsg};
  EXPECT_STREQ(charMsg, RangeError{charMsg}.what());
  EXPECT_EQ(msg, RangeError{msg}.what());
}

} // namespace kanji_tools
