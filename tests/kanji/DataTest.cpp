#include <tests/kanji_tools/TestData.h>
#include <tests/kanji_tools/TestKanji.h>
#include <tests/kanji_tools/TestUcd.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

constexpr auto Arg0{"test"};

const auto TestOne{std::make_shared<TestKanji>("一")},
    TestVariant{std::make_shared<TestKanji>("侮︀")};

const std::string InUcd{" in _ucd\n"};

class DataTest : public TestData {
protected:
  DataTest() : _currentDir(fs::current_path()) {
    TestOne->type(KanjiTypes::None);
  }

  void TearDown() override { fs::current_path(_currentDir); }

  Path _currentDir;
};

} // namespace

TEST_F(DataTest, Usage) {
  const std::string msg{"error msg"};
  EXPECT_THROW(call([&msg] { Data::usage(msg); }, msg), std::domain_error);
}

// command line args tests

TEST_F(DataTest, NextArgWithNoArgs) {
  // passing no args returns 0
  EXPECT_EQ(nextArg({}), 0);
}

TEST_F(DataTest, NextArgWithBadCurrentArg) {
  const auto f{[] { return nextArg({1, &Arg0}, 2); }};
  EXPECT_THROW(call(f, "current arg '2' is greater than args size '1'"),
      std::domain_error);
}

TEST_F(DataTest, NextArgWithJustArg0) {
  // call without final 'current' parameter increments to 1
  EXPECT_EQ(nextArg({1, &Arg0}), 1);
}

TEST_F(DataTest, NextArgWithCurrentArg) {
  auto arg1{"arg1"}, arg2{"arg2"};
  const char* args[]{Arg0, arg1, arg2};
  EXPECT_EQ(nextArg(args, 1), 2);
  EXPECT_EQ(nextArg(args, 2), 3);
}

TEST_F(DataTest, NextArgWithDebugArg) {
  const char* args[]{Arg0, DebugArg.c_str()};
  // skip '-data some-dir'
  EXPECT_EQ(nextArg(args), 2);
}

TEST_F(DataTest, NextArgWithDataArg) {
  const char* args[]{Arg0, DataArg.c_str(), TestDirArg};
  // skip '-data some-dir'
  EXPECT_EQ(nextArg(args), 3);
}

TEST_F(DataTest, NextArgWithDebugAndDataArgs) {
  const char* args[]{Arg0, DebugArg.c_str(), DataArg.c_str(), TestDirArg};
  // skip '-data some-dir'
  EXPECT_EQ(nextArg(args), 4);
}

TEST_F(DataTest, NextArgWithMultipleArgs) {
  auto arg1{"arg1"}, arg3{"arg3"}, arg6{"arg6"};
  const char* argv[]{
      Arg0, arg1, DebugArg.c_str(), arg3, DataArg.c_str(), TestDirArg, arg6};
  const Args args{argv};
  std::vector<const char*> actualArgs;
  for (auto i{nextArg(args)}; i < args.size(); i = nextArg(args, i))
    actualArgs.push_back(argv[i]);
  EXPECT_EQ(actualArgs, (std::vector{arg1, arg3, arg6}));
}

TEST_F(DataTest, MissingDataDirArg) {
  const char* args[]{Arg0, DataArg.c_str()};
  EXPECT_THROW(call([&args] { return getDataDir(args); },
                   "'-data' must be followed by a directory name"),
      std::domain_error);
}

TEST_F(DataTest, BadDataDirArg) {
  const char* args[]{Arg0, DataArg.c_str(), TestDirArg};
  EXPECT_THROW(call([&args] { return getDataDir(args); },
                   "'testDir' is not a valid directory"),
      std::domain_error);
}

TEST_F(DataTest, GoodDataDirArg) {
  // let Data::getDataDir find a good 'data' directory
  const auto dir{getDataDir({})};
  const char* args[]{Arg0, DataArg.c_str(), dir.c_str()};
  EXPECT_EQ(getDataDir(args), dir);
}

TEST_F(DataTest, DataDirArgToInvalidData) {
  // get a valid directory that isn't a 'data' directory, i.e., it doesn't have
  // the expected .txt files
  const auto dir{_currentDir.root_directory()};
  const char* args[]{Arg0, DataArg.c_str(), dir.c_str()};
  const std::string msg{
      "'" + dir.string() + "' does not contain 10 expected '.txt' files"};
  EXPECT_THROW(
      call([&args] { return getDataDir(args); }, msg), std::domain_error);
}

TEST_F(DataTest, SearchBasedOnArg0ForDataDir) {
  // get 'data' directory based on 'current directory' logic, i.e., look in
  // current directory for 'data' and if not found check all parent directories
  const auto expected{getDataDir({})};
  // change to a directory that shouldn't have a 'data' directory
  fs::current_path(expected.root_directory());
  ASSERT_NE(expected, fs::current_path());
  const auto arg0{expected / "testProgramName"};
  const char* args[]{arg0.c_str()};
  EXPECT_EQ(getDataDir(args), expected);
}

TEST_F(DataTest, FailToFindDataDirNoArg0) {
  fs::current_path(_currentDir.root_directory());
  const std::string msg{
      "couldn't find 'data' directory with 10 expected '.txt' files:\n- "
      "searched up from current: " +
      fs::current_path().string() +
      "\nrun in a directory where 'data' can be found or use '-data <dir>'"};
  EXPECT_THROW(call([] { return getDataDir({}); }, msg), std::domain_error);
}

TEST_F(DataTest, FailToFindDataDirWithArg0) {
  fs::current_path(_currentDir.root_directory());
  const std::string arg0{fs::current_path() / "testProgramName"};
  const std::string msg{
      "couldn't find 'data' directory with 10 expected '.txt' files:\n- "
      "searched up from current: " +
      fs::current_path().string() + "\n- searched up from arg0: " + arg0 +
      "\nrun in a directory where 'data' can be found or use '-data <dir>'"};
  const char* args[]{arg0.c_str()};
  EXPECT_THROW(
      call([&args] { return getDataDir(args); }, msg), std::domain_error);
}

TEST_F(DataTest, NoDebugArgs) {
  EXPECT_EQ(getDebugMode({}), DebugMode::None);
  const char* args[]{Arg0, "some arg", "some other arg"};
  EXPECT_EQ(getDebugMode(args), DebugMode::None);
}

TEST_F(DataTest, DebugArg) {
  const char* args[]{Arg0, "some arg", DebugArg.c_str(), "some other arg"};
  EXPECT_EQ(getDebugMode(args), DebugMode::Full);
}

TEST_F(DataTest, InfoArg) {
  const char* args[]{Arg0, "some arg", InfoArg.c_str(), "some other arg"};
  EXPECT_EQ(getDebugMode(args), DebugMode::Info);
}

TEST_F(DataTest, BothDebugAndInfoArgs) {
  const char* args[]{Arg0, DebugArg.c_str(), InfoArg.c_str()};
  EXPECT_THROW(call([&args] { return getDebugMode(args); },
                   "can only specify one '-debug' or '-info' option"),
      std::domain_error);
}

// creation sanity checks

TEST_F(DataTest, DuplicateEntry) {
  const Ucd ucd{TestUcd{}};
  EXPECT_TRUE(checkInsert(TestOne, &ucd));
  EXPECT_FALSE(checkInsert(TestOne));
  EXPECT_TRUE(_es.str().ends_with("failed to insert '一' into map\n"));
}

TEST_F(DataTest, UcdNotFound) {
  // successful insert returns true
  EXPECT_TRUE(checkInsert(TestOne));
  // sanity check failure printed to stderr
  EXPECT_TRUE(_es.str().ends_with("一 [4E00] not found" + InUcd));
}

TEST_F(DataTest, UcdNotFoundForVariant) {
  // successful insert returns true
  EXPECT_TRUE(checkInsert(TestVariant));
  // sanity check failure printed to stderr
  EXPECT_TRUE(_es.str().ends_with(
      "侮︀ [4FAE FE00] (non-variant: 侮) not found" + InUcd));
}

TEST_F(DataTest, UcdNotJoyo) {
  const Ucd ucd{TestUcd{}};
  TestOne->type(KanjiTypes::Jouyou);
  EXPECT_TRUE(checkInsert(TestOne, &ucd));
  EXPECT_TRUE(_es.str().ends_with("一 [4E00] not marked as 'Joyo'" + InUcd));
}

TEST_F(DataTest, UcdNotJinmei) {
  const Ucd ucd{TestUcd{}};
  TestOne->type(KanjiTypes::Jinmei);
  EXPECT_TRUE(checkInsert(TestOne, &ucd));
  EXPECT_TRUE(_es.str().ends_with("一 [4E00] not marked as 'Jinmei'" + InUcd));
}

TEST_F(DataTest, UcdNotLinkedJinmei) {
  const Ucd ucd{TestUcd{}};
  TestOne->type(KanjiTypes::LinkedJinmei);
  EXPECT_TRUE(checkInsert(TestOne, &ucd));
  EXPECT_TRUE(_es.str().ends_with(
      "一 [4E00] with link not marked as 'Jinmei'" + InUcd));
}

TEST_F(DataTest, UcdMissingJinmeiLinks) {
  const Ucd ucd{TestUcd{"二"}.code(U'\x4ebc').jinmei(true)};
  TestOne->type(KanjiTypes::LinkedJinmei);
  EXPECT_TRUE(checkInsert(TestOne, &ucd));
  EXPECT_TRUE(_es.str().ends_with(
      "一 [4E00] missing 'JinmeiLink' for [4EBC] 二" + InUcd));
}

TEST_F(DataTest, DuplicateCompatibilityName) {
  const Ucd ucd{TestUcd{}};
  // real Kanji with variation selectors, but with fake 'compatibility names'
  const auto k1{std::make_shared<TestKanji>("嘆︀", "十")};
  const auto k2{std::make_shared<TestKanji>("器︀", "十")};
  EXPECT_TRUE(checkInsert(k1, &ucd));
  EXPECT_TRUE(checkInsert(k2, &ucd));
  EXPECT_TRUE(
      _es.str().ends_with("failed to insert variant '器︀' into map\n"));
}

// test loading from files

TEST_F(DataTest, FrequencyReadingDuplicate) {
  write("Name\tReading\n呑\tトン、ドン、の-む\n呑\tトン、ドン、の-む");
  EXPECT_THROW(call([this] { loadFrequencyReadings(TestFile); },
                   "duplicate name - file: testFile.txt, row: 2"),
      std::domain_error);
}

TEST_F(DataTest, LinkedJinmeiEntryNotFound) {
  write("亜\t亞");
  // no Jouyou Kanji are loaded at this point so any entry will cause an error
  EXPECT_THROW(call([this] { populateLinkedKanji(TestFile); },
                   "'亜' not found - file: testFile.txt"),
      std::domain_error);
}

TEST_F(DataTest, LinkedJinmeiBadLine) {
  write("亜亞");
  EXPECT_THROW(call([this] { populateLinkedKanji(TestFile); },
                   "bad line '亜亞' - file: testFile.txt"),
      std::domain_error);
}

} // namespace kanji_tools
