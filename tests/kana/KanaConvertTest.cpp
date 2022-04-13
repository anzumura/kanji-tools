#include <gtest/gtest.h>
#include <kanji_tools/kana/KanaConvert.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

class KanaConvertTest : public ::testing::Test {
protected:
  void run(const Args& args, const std::string& expectedIn) {
    KanaConvert(args, _os, _is);
    std::stringstream expected{expectedIn};
    for (std::string i, j; std::getline(_os, i) && std::getline(expected, j);)
      ASSERT_EQ(i, j);
    // if loop completed due to 'eof' then check remaining part of other stream
    if (_os.eof())
      EXPECT_EQ(expected.str().substr(_os.str().size()), "");
    else if (expected.eof())
      EXPECT_EQ(_os.str().substr(expected.str().size()), "");
  }

  std::stringstream _os, _is;
};

} // namespace

TEST_F(KanaConvertTest, Usage) {
  const char* args[]{"", "-?"};
  run(args,
      R"(usage: kanaConvert [-h|-k|-r] [-H|-K|-R] [-f h|n|r] [-i|-m|-n|-p] [string ...]
  -h: set conversion output to Hiragana (default)
  -k: set conversion output to Katakana
  -r: set conversion output to Rōmaji
  -H: restrict conversion input to Hiragana
  -K: restrict conversion input to Katakana
  -R: restrict conversion input to Rōmaji
  -?: prints this usage message
  -f opt: set 'opt' (can use multiple times to combine options). Options are:
      h: conform Rōmaji output more closely to 'Modern Hepburn' style
      k: conform Rōmaji output more closely to 'Kunrei Shiki' style
      n: no prolonged marks (repeat vowels instead of 'ー' for Hiragana output)
      r: remove spaces on output (only applies to Hiragana and Katakana output)
  -i: interactive mode
  -m: print Kana chart in 'Markdown' format and exit
  -n: suppress newline on output (for non-interactive mode)
  -p: print Kana chart aligned for terminal output and exit
  --: finish options, all further arguments will be treated as input files
  [string ...]: one or more strings to convert, no strings means read stdin
)");
}

TEST_F(KanaConvertTest, IllegalOption) {
  const char* args[]{"", "-a"};
  const auto f{[&args] { KanaConvert{args}; }};
  EXPECT_THROW(call(f, "illegal option: -a"), std::domain_error);
}

TEST_F(KanaConvertTest, MissingFlagOption) {
  const char* args[]{"", "-f"};
  const auto f{[&args] { KanaConvert{args}; }};
  EXPECT_THROW(
      call(f, "-f must be followed by a flag value"), std::domain_error);
}

TEST_F(KanaConvertTest, IllegalFlagOption) {
  for (const std::string i : {"a", "aa"}) {
    const char* args[]{"", "-f", i.c_str()};
    const auto f{[&args] { KanaConvert{args}; }};
    EXPECT_THROW(call(f, "illegal option for -f: " + i), std::domain_error);
  }
}

TEST_F(KanaConvertTest, MultipleProgramModes) {
  for (const auto i : {"-m", "-n", "-p"}) {
    const char* args[]{"", "-i", i};
    const auto f{[&args] { KanaConvert{args}; }};
    EXPECT_THROW(call(f, "can only specify one of -i, -m, -n, or -p"),
        std::domain_error);
  }
}

TEST_F(KanaConvertTest, InteractiveModeAndStrings) {
  const char* args[]{"", "-i", "hi"};
  const auto f{[&args] { KanaConvert{args}; }};
  EXPECT_THROW(call(f, "'-i' can't be combined with other 'string' arguments"),
      std::domain_error);
}

TEST_F(KanaConvertTest, PrintKanaChart) {
  const char* args[]{"", "-p"};
  std::stringstream os;
  KanaConvert(args, _os);
  std::string lastLine;
  size_t count{}, found{};
  // just check for a few examples
  for (std::string line; std::getline(_os, line); ++count, lastLine = line) {
    if (!count)
      EXPECT_EQ(line, ">>> Notes:");
    else {
      if (line.starts_with("| 14  | P    | ka   | か   | カ   | 304B | 30AB |"))
        ++found;
      if (line.starts_with("| 205 | N    | /    |      | ・   |      | 30FB |"))
        ++found;
      if (line.starts_with(" Monograph:  86")) ++found;
    }
  }
  EXPECT_TRUE(lastLine.starts_with("     Types: 208 (P=131, D=63, H=10, N=4)"));
  EXPECT_EQ(found, 3);
  EXPECT_EQ(count, 245);
}

TEST_F(KanaConvertTest, PrintMarkdownKanaChart) {
  const char* args[]{"", "-m"};
  std::stringstream os;
  KanaConvert(args, _os);
  std::string lastLine;
  size_t count{}, found{};
  // just check for a few examples
  for (std::string line; std::getline(_os, line); ++count, lastLine = line) {
    if (!count)
      EXPECT_EQ(line, "## **Kana Conversion Chart**");
    else {
      if (line.starts_with("| **14** | **P** | **ka** | **か** | **カ** |"))
        ++found;
      if (line.starts_with("| **205** | **N** | **/** |  | **・** |")) ++found;
      if (line.starts_with("- **Monograph:**  86")) ++found;
    }
  }
  EXPECT_TRUE(
      lastLine.starts_with("- **Types:** 208 (P=131, D=63, H=10, N=4)"));
  EXPECT_EQ(found, 3);
  // there are less lines when printing the chart with -p option since Markdown
  // table doesn't have '+---+---+' border lines
  EXPECT_EQ(count, 234);
}

} // namespace kanji_tools
