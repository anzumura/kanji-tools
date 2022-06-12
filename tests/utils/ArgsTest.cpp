#include <gtest/gtest.h>
#include <kt_tests/WhatMismatch.h>
#include <kt_utils/Args.h>
#include <kt_utils/Exception.h>

namespace kanji_tools {

TEST(ArgsTest, SizeWithNoArgs) {
  const auto f{[] { Args{1, {}}; }};
  EXPECT_THROW(call(f, "argc is 1, but argv is null"), DomainError);
}

TEST(ArgsTest, NoSizeWithArgs) {
  const char* args{"test"};
  const auto f{[&args] { Args{0, &args}; }};
  EXPECT_THROW(call(f, "argc is 0, but argv is not null"), DomainError);
}

TEST(ArgsTest, IntArgs) {
  const int x{3};
  const char* argv[]{"a", "bb", "ccc"};
  const Args args{x, argv};
  EXPECT_EQ(args.size(), 3);
}

TEST(ArgsTest, IntArgsOutOfRange) {
  const char* argv[]{"a", "bb", "ccc"};
  const auto small{[&] { Args{-1, argv}; }};
  EXPECT_THROW(call(small, "argc -1 is less than 0"), RangeError);
  const auto big{[&argv] {
    Args{std::numeric_limits<Args::Size>::max() + 1, argv};
  }};
  EXPECT_THROW(call(big, "argc 65536 is greater than 65535"), RangeError);
}

TEST(ArgsTest, Index) {
  const char* argv[]{"a", "bb", "ccc"};
  const Args args{argv};
  EXPECT_EQ(args.size(), 3);
  EXPECT_STREQ(args[0], "a");
  EXPECT_STREQ(args[1], "bb");
  EXPECT_STREQ(args[2], "ccc");
}

TEST(ArgsTest, IndexOutOfRange) {
  const char* argv[]{"a", "bb", "ccc"};
  const Args args{argv};
  EXPECT_THROW(
      call([&args] { return args[3]; }, "index 3 must be less than argc 3"),
      RangeError);
}

TEST(ArgsTest, OperatorBool) {
  const char* args[]{"a"};
  const Args empty{}, nonEmpty{args};
  EXPECT_FALSE(empty);
  EXPECT_TRUE(nonEmpty);
}

} // namespace kanji_tools
