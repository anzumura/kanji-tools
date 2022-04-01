#include <gtest/gtest.h>
#include <kanji_tools/utils/Args.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

TEST(ArgsTest, SizeWithNoArgs) {
  EXPECT_THROW(call(
                   [] {
                     Args{1, {}};
                   },
                   "size is 1, but args is null"),
      std::domain_error);
}

TEST(ArgsTest, NoSizeWithArgs) {
  const char* args{"test"};
  EXPECT_THROW(call(
                   [&args] {
                     Args{0, &args};
                   },
                   "size is 0, but args is not null"),
      std::domain_error);
}

TEST(ArgsTest, IntArgs) {
  const int x{3};
  const char* argv[]{"a", "bb", "ccc"};
  const Args args{x, argv};
  EXPECT_EQ(args.size(), 3);
}

TEST(ArgsTest, IntArgsOutOfRange) {
  const int tooSmall{-1}, tooBig{256};
  const char* argv[]{"a", "bb", "ccc"};
  EXPECT_THROW(call(
                   [&] {
                     Args{tooSmall, argv};
                   },
                   "size -1 is less than 0"),
      std::domain_error);
  EXPECT_THROW(call(
                   [&] {
                     Args{tooBig, argv};
                   },
                   "size 256 is greater than 255"),
      std::domain_error);
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
