#include <gtest/gtest.h>
#include <kanji_tools/utils/Args.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

TEST(ArgsTest, SizeWithNoArgs) {
  const auto f{[] { Args{1, {}}; }};
  EXPECT_THROW(call(f, "size is 1, but args is null"), std::domain_error);
}

TEST(ArgsTest, NoSizeWithArgs) {
  const char* args{"test"};
  const auto f{[&args] { Args{0, &args}; }};
  EXPECT_THROW(call(f, "size is 0, but args is not null"), std::domain_error);
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
  EXPECT_THROW(call(small, "size -1 is less than 0"), std::domain_error);
  const auto big{[&argv] {
    Args{std::numeric_limits<Args::Size>::max() + 1, argv};
  }};
  EXPECT_THROW(call(big, "size 65536 is greater than 65535"), std::domain_error);
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
      call([&args] { return args[3]; }, "index 3 must be less than size 3"),
      std::domain_error);
}

TEST(ArgsTest, OperatorBool) {
  const char* args[]{"a"};
  const Args empty{}, nonEmpty{args};
  EXPECT_FALSE(empty);
  EXPECT_TRUE(nonEmpty);
}

} // namespace kanji_tools
