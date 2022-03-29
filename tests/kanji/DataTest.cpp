#include <gtest/gtest.h>
#include <kanji_tools/kanji/Data.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

constexpr auto Arg0{"test"}, DebugArg{"-debug"}, DataArg{"-data"},
    DataDir{"dir"};

} // namespace

TEST(DataTest, Usage) {
  const std::string msg{"error msg"};
  EXPECT_THROW(call([&msg] { Data::usage(msg); }, msg), std::domain_error);
}

TEST(DataTest, NextArgWithJustArg0) {
  // call without final 'currentArg' parameter increments to 1
  EXPECT_EQ(Data::nextArg(1, &Arg0), 1);
}

TEST(DataTest, NextArgWithCurrentArg) {
  auto arg1{"arg1"}, arg2{"arg2"};
  const char* argv[]{Arg0, arg1, arg2};
  EXPECT_EQ(Data::nextArg(std::size(argv), argv, 1), 2);
  EXPECT_EQ(Data::nextArg(std::size(argv), argv, 2), 3);
}

TEST(DataTest, NextArgWithDebugArg) {
  const char* argv[]{Arg0, DebugArg};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 2);
}

TEST(DataTest, NextArgWithDataArg) {
  const char* argv[]{Arg0, DataArg, DataDir};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 3);
}

TEST(DataTest, NextArgWithDebugAndDataArgs) {
  const char* argv[]{Arg0, DebugArg, DataArg, DataDir};
  // skip '-data some-dir'
  EXPECT_EQ(Data::nextArg(std::size(argv), argv), 4);
}

TEST(DataTest, NextArgWithMultipleArgs) {
  auto arg1{"arg1"}, arg3{"arg3"}, arg6{"arg6"};
  const char* argv[]{Arg0, arg1, DebugArg, arg3, DataArg, DataDir, arg6};
  Data::ArgCount argc{std::size(argv)};
  std::vector<const char*> actualArgs;
  for (auto i{Data::nextArg(argc, argv)}; i < argc;
       i = Data::nextArg(argc, argv, i))
    actualArgs.push_back(argv[i]);
  EXPECT_EQ(actualArgs, (std::vector{arg1, arg3, arg6}));
}

} // namespace kanji_tools
