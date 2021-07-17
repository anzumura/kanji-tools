#include <gtest/gtest.h>

// InitGoogleTest potentially modified both argc and argv so can't declare
// argv as 'const char**'.
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
