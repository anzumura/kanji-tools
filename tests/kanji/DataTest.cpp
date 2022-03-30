#include <gtest/gtest.h>
#include <kanji_tools/kanji/Data.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

constexpr auto Arg0{"test"}, TestDataDir{"dir"};

namespace fs = std::filesystem;

const fs::path DataDirPath{TestDataDir};

class DataTest : public ::testing::Test, public Data {
public:
  [[nodiscard]] Kanji::OptFreq frequency(const std::string&) const override {
    return {};
  }
  [[nodiscard]] JlptLevels level(const std::string&) const override {
    return JlptLevels::None;
  }
  [[nodiscard]] KenteiKyus kyu(const std::string&) const override {
    return KenteiKyus::None;
  }
protected:
  DataTest()
      : Data{DataDirPath, DebugMode::None}, _currentDir(fs::current_path()) {}

  void TearDown() override { fs::current_path(_currentDir); }

  Path _currentDir;
};

} // namespace

TEST_F(DataTest, Usage) {
  const std::string msg{"error msg"};
  EXPECT_THROW(call([&msg] { Data::usage(msg); }, msg), std::domain_error);
}

TEST_F(DataTest, NextArgWithNoArgs) {
  // passing no args returns 0
  EXPECT_EQ(nextArg(0, nullptr), 0);
}

TEST_F(DataTest, NextArgWithCountButNoArgs) {
  EXPECT_THROW(
      call([] { return nextArg(1, nullptr); }, "argc is 1, but argv is null"),
      std::domain_error);
}

TEST_F(DataTest, NextArgWithBadCurrentArg) {
  EXPECT_THROW(call([] { return nextArg(1, &Arg0, 2); },
                   "currentArg 2 is greater than argc 1"),
      std::domain_error);
}

TEST_F(DataTest, NextArgWithJustArg0) {
  // call without final 'currentArg' parameter increments to 1
  EXPECT_EQ(nextArg(1, &Arg0), 1);
}

TEST_F(DataTest, NextArgWithCurrentArg) {
  auto arg1{"arg1"}, arg2{"arg2"};
  const char* argv[]{Arg0, arg1, arg2};
  EXPECT_EQ(nextArg(std::size(argv), argv, 1), 2);
  EXPECT_EQ(nextArg(std::size(argv), argv, 2), 3);
}

TEST_F(DataTest, NextArgWithDebugArg) {
  const char* argv[]{Arg0, DebugArg.c_str()};
  // skip '-data some-dir'
  EXPECT_EQ(nextArg(std::size(argv), argv), 2);
}

TEST_F(DataTest, NextArgWithDataArg) {
  const char* argv[]{Arg0, DataArg.c_str(), TestDataDir};
  // skip '-data some-dir'
  EXPECT_EQ(nextArg(std::size(argv), argv), 3);
}

TEST_F(DataTest, NextArgWithDebugAndDataArgs) {
  const char* argv[]{Arg0, DebugArg.c_str(), DataArg.c_str(), TestDataDir};
  // skip '-data some-dir'
  EXPECT_EQ(nextArg(std::size(argv), argv), 4);
}

TEST_F(DataTest, NextArgWithMultipleArgs) {
  auto arg1{"arg1"}, arg3{"arg3"}, arg6{"arg6"};
  const char* argv[]{
      Arg0, arg1, DebugArg.c_str(), arg3, DataArg.c_str(), TestDataDir, arg6};
  Data::ArgCount argc{std::size(argv)};
  std::vector<const char*> actualArgs;
  for (auto i{nextArg(argc, argv)}; i < argc; i = nextArg(argc, argv, i))
    actualArgs.push_back(argv[i]);
  EXPECT_EQ(actualArgs, (std::vector{arg1, arg3, arg6}));
}

TEST_F(DataTest, BadDataArgs) {
  EXPECT_THROW(call([] { return getDataDir(2, nullptr); },
                   "argc is 2, but argv is null"),
      std::domain_error);
}

TEST_F(DataTest, MissingDataDirArg) {
  const char* argv[]{Arg0, DataArg.c_str()};
  EXPECT_THROW(call([&argv] { return getDataDir(std::size(argv), argv); },
                   "'-data' must be followed by a directory name"),
      std::domain_error);
}

TEST_F(DataTest, BadDataDirArg) {
  const char* argv[]{Arg0, DataArg.c_str(), TestDataDir};
  EXPECT_THROW(call([&argv] { return getDataDir(std::size(argv), argv); },
                   "'dir' is not a valid directory"),
      std::domain_error);
}

TEST_F(DataTest, GoodDataDirArg) {
  // let Data::getDataDir find a good 'data' directory
  const auto dir{getDataDir(0, {})};
  const char* argv[]{Arg0, DataArg.c_str(), dir.c_str()};
  EXPECT_EQ(getDataDir(std::size(argv), argv), dir);
}

TEST_F(DataTest, DataDirArgToInvalidData) {
  // get a valid directory that isn't a 'data' directory, i.e., it doesn't have
  // the expected .txt files
  const auto dir{_currentDir.root_directory()};
  const char* argv[]{Arg0, DataArg.c_str(), dir.c_str()};
  const std::string msg{
      "'" + dir.string() + "' does not contain 12 expected '.txt' files"};
  EXPECT_THROW(call([&argv] { return getDataDir(std::size(argv), argv); }, msg),
      std::domain_error);
}

TEST_F(DataTest, SearchBasedOnArg0ForDataDir) {
  // get 'data' directory based on 'current directory' logic, i.e., look in
  // current directory for 'data' and if not found check all parent directories
  const auto expected{getDataDir(0, nullptr)};
  // change to a directory that shouldn't have a 'data' directory
  fs::current_path(expected.root_directory());
  ASSERT_NE(expected, fs::current_path());
  const auto arg0{expected / "testProgramName"};
  const char* argv[]{arg0.c_str()};
  EXPECT_EQ(getDataDir(1, argv), expected);
}

TEST_F(DataTest, FailToFindDataDirNoArg0) {
  fs::current_path(_currentDir.root_directory());
  const std::string msg{
      "couldn't find 'data' directory with 12 expected '.txt' files:\n- "
      "searched up from current: " +
      fs::current_path().string() +
      "\nrun in a directory where 'data' can be found or use '-data <dir>'"};
  EXPECT_THROW(
      call([] { return getDataDir(0, nullptr); }, msg), std::domain_error);
}

TEST_F(DataTest, FailToFindDataDirWithArg0) {
  fs::current_path(_currentDir.root_directory());
  const std::string arg0{fs::current_path() / "testProgramName"};
  const std::string msg{
      "couldn't find 'data' directory with 12 expected '.txt' files:\n- "
      "searched up from current: " +
      fs::current_path().string() + "\n- searched up from arg0: " + arg0 +
      "\nrun in a directory where 'data' can be found or use '-data <dir>'"};
  const char* argv[]{arg0.c_str()};
  EXPECT_THROW(
      call([&argv] { return getDataDir(1, argv); }, msg), std::domain_error);
}

TEST_F(DataTest, NoDebugArgs) {
  EXPECT_EQ(getDebugMode(0, nullptr), DebugMode::None);
  const char* argv[]{Arg0, "some arg", "some other arg"};
  EXPECT_EQ(getDebugMode(std::size(argv), argv), DebugMode::None);
}

TEST_F(DataTest, BadDebugArgs) {
  EXPECT_THROW(call([] { return getDebugMode(3, nullptr); },
                   "argc is 3, but argv is null"),
      std::domain_error);
}

TEST_F(DataTest, DebugArg) {
  const char* argv[]{Arg0, "some arg", DebugArg.c_str(), "some other arg"};
  EXPECT_EQ(getDebugMode(std::size(argv), argv), DebugMode::Full);
}

TEST_F(DataTest, InfoArg) {
  const char* argv[]{Arg0, "some arg", InfoArg.c_str(), "some other arg"};
  EXPECT_EQ(getDebugMode(std::size(argv), argv), DebugMode::Info);
}

TEST_F(DataTest, BothDebugAndInfoArgs) {
  const char* argv[]{Arg0, DebugArg.c_str(), InfoArg.c_str()};
  EXPECT_THROW(call([&argv] { return getDebugMode(std::size(argv), argv); },
                   "can only specify one '-debug' or '-info' option"),
      std::domain_error);
}

} // namespace kanji_tools
