#include <gtest/gtest.h>
#include <kanji_tools/kana/KanaConvert.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace {

const std::string OptionsMsg{">>> current options: source="},
    EnterMsg{"\n>>> enter string (c=clear flags, f=set flag, q=quit, h=help, "
             "-k|-h|-r|-K|-H|-R):\n"};
const std::string DefOptionsMsg{OptionsMsg + "any, target=Hiragana, flags="};

constexpr size_t SkipFirstTwoLines{2}, SkipFirstFourLines{4};

class KanaConvertTest : public ::testing::Test {
protected:
  void run(
      const Args& args, const std::string& expectedIn, size_t skipLines = 0) {
    _is << "q\n"; // send 'quit' option to make sure the program exits
    KanaConvert{args, _os, &_is};
    std::stringstream expected{expectedIn};
    if (skipLines) {
      size_t skipped{0};
      for (std::string i; skipped < skipLines && std::getline(_os, i);)
        ++skipped;
      ASSERT_EQ(skipped, skipLines);
    }
    for (std::string i, j; std::getline(_os, i) && std::getline(expected, j);)
      ASSERT_EQ(i, j);
    // if loop completed due to 'eof' then check remaining part of other
    // stream
    if (_os.eof() && expectedIn.size() > _os.str().size())
      EXPECT_EQ(expectedIn.substr(_os.str().size()), "");
    else if (expected.eof() && _os.str().size() > expectedIn.size())
      EXPECT_EQ(_os.str().substr(expectedIn.size()), "");
  }

  [[nodiscard]] auto& os() { return _os; }
  [[nodiscard]] auto& is() { return _is; }
private:
  std::stringstream _os, _is;
};

} // namespace

TEST_F(KanaConvertTest, Usage) {
  const char* args[]{"", "-?"};
  run(args,
      R"(usage: kanaConvert -i
       kanaConvert [-n] string ...
       kanaConvert -m|-p|-?
  -i: interactive mode
  -n: suppress newline on output (for non-interactive mode)
  -m: print Kana chart in 'Markdown' format and exit
  -p: print Kana chart aligned for terminal output and exit
  -?: prints this usage message
  --: finish options, subsequent args are treated as strings to convert
  string ...: one or more strings to convert, no strings means read stdin
  
options for setting conversion source and target types as well as conversion
related flags can also be specified:
  -f opt: set 'opt' (can use multiple times to combine options). Options are:
    h: conform Rōmaji output more closely to 'Modern Hepburn' style
    k: conform Rōmaji output more closely to 'Kunrei Shiki' style
    n: no prolonged marks (repeat vowels instead of 'ー' for Hiragana output)
    r: remove spaces on output (only applies to Hiragana and Katakana output)
  -h: set conversion output to Hiragana (default)
  -k: set conversion output to Katakana
  -r: set conversion output to Rōmaji
  -H: restrict conversion input to Hiragana
  -K: restrict conversion input to Katakana
  -R: restrict conversion input to Rōmaji
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
  for (const auto i : {"-i", "-m", "-n", "-p"}) {
    const char* args[]{"", "-i", i};
    const auto f{[&args] { KanaConvert{args}; }};
    EXPECT_THROW(call(f, "can only specify one of -i, -m, -n, or -p"),
        std::domain_error);
  }
}

TEST_F(KanaConvertTest, InteractiveOrPrintOptionsAndStrings) {
  for (const auto i : {"-i", "-m", "-p"}) {
    const char* args[]{"", i, "hi"};
    const auto f{[&args] { KanaConvert{args}; }};
    EXPECT_THROW(
        call(f, "'string' args can't be combined with '-i', '-m' or '-p'"),
        std::domain_error);
  }
}

TEST_F(KanaConvertTest, NoStringsAndNoInteractiveMode) {
  const char* args[]{""};
  const auto f{[&args, this] { KanaConvert{args, os(), &is()}; }};
  EXPECT_THROW(call(f, "provide one or more 'strings' to convert or specify "
                       "'-i' for interactive mode"),
      std::domain_error);
}

TEST_F(KanaConvertTest, PrintKanaChart) {
  const char* args[]{"", "-p"};
  KanaConvert(args, os());
  std::string lastLine;
  size_t count{}, found{};
  // just check for a few examples
  for (std::string line; std::getline(os(), line); ++count, lastLine = line) {
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
  KanaConvert(args, os());
  std::string lastLine;
  size_t count{}, found{};
  // just check for a few examples
  for (std::string line; std::getline(os(), line); ++count, lastLine = line) {
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
  // there are less lines when printing the chart with -m option (compared to -p
  // option) since the Markdown table doesn't have '+---+---+' type border lines
  EXPECT_EQ(count, 234);
}

TEST_F(KanaConvertTest, InteractiveMode) {
  const char* args[]{"", "-i"};
  run(args, OptionsMsg + "any, target=Hiragana, flags=None" + EnterMsg);
}

TEST_F(KanaConvertTest, HiraganaTarget) {
  const char* args[]{"", "-i", "-h"};
  run(args, OptionsMsg + "any, target=Hiragana, flags=None" + EnterMsg);
}

TEST_F(KanaConvertTest, KatakanaTarget) {
  const char* args[]{"", "-i", "-k"};
  run(args, OptionsMsg + "any, target=Katakana, flags=None" + EnterMsg);
}

TEST_F(KanaConvertTest, RomajiTarget) {
  const char* args[]{"", "-i", "-r"};
  run(args, OptionsMsg + "any, target=Romaji, flags=None" + EnterMsg);
}

TEST_F(KanaConvertTest, HiraganaSource) {
  // no conversion will happen since source and target are the same, but a user
  // could interactively changes the source or target
  const char* args[]{"", "-i", "-H"};
  run(args, OptionsMsg + "Hiragana, target=Hiragana, flags=None" + EnterMsg);
}

TEST_F(KanaConvertTest, KatakanaSource) {
  const char* args[]{"", "-i", "-K"};
  run(args, OptionsMsg + "Katakana, target=Hiragana, flags=None" + EnterMsg);
}

TEST_F(KanaConvertTest, RomajiSource) {
  const char* args[]{"", "-i", "-R"};
  run(args, OptionsMsg + "Romaji, target=Hiragana, flags=None" + EnterMsg);
}

TEST_F(KanaConvertTest, SetHepburnFlag) {
  const char* args[]{"", "-i", "-f", "h"};
  run(args, DefOptionsMsg + "Hepburn" + EnterMsg);
}

TEST_F(KanaConvertTest, SetKunreiFlag) {
  const char* args[]{"", "-i", "-f", "k"};
  run(args, DefOptionsMsg + "Kunrei" + EnterMsg);
}

TEST_F(KanaConvertTest, SetNoProlongFlag) {
  const char* args[]{"", "-i", "-f", "n"};
  run(args, DefOptionsMsg + "NoProlongMark" + EnterMsg);
}

TEST_F(KanaConvertTest, SetRemoveSpacesFlag) {
  const char* args[]{"", "-i", "-f", "r"};
  run(args, DefOptionsMsg + "RemoveSpaces" + EnterMsg);
}

TEST_F(KanaConvertTest, SetMultipleFlags) {
  const char* args[]{"", "-i", "-f", "n", "-f", "r"};
  run(args, DefOptionsMsg + "NoProlongMark|RemoveSpaces" + EnterMsg);
}

TEST_F(KanaConvertTest, ConvertOneString) {
  const char* args[]{"", "hi"};
  run(args, "ひ\n");
}

TEST_F(KanaConvertTest, EndOfOptions) {
  const char* args[]{"", "--", "hi"};
  run(args, "ひ\n");
}

TEST_F(KanaConvertTest, ConvertMultipleStrings) {
  const char* args[]{"", "ze", "hi"};
  run(args, "ぜ　ひ\n");
}

TEST_F(KanaConvertTest, ConvertMultipleStringsNoSpace) {
  const char* args[]{"", "-f", "r", "ze", "hi"};
  run(args, "ぜひ\n");
}

// Interactive Mode tests

TEST_F(KanaConvertTest, InteractiveConvert) {
  const char* args[]{"", "-i"};
  is() << "kippu\n";
  run(args, "きっぷ\n", SkipFirstTwoLines);
}

TEST_F(KanaConvertTest, InteractiveHelp) {
  const char* args[]{"", "-i"};
  is() << "h\n";
  run(args, R"(  -h: set conversion output to Hiragana
  -k: set conversion output to Katakana
  -r: set conversion output to Rōmaji
  -H: restrict conversion input to Hiragana
  -K: restrict conversion input to Katakana
  -R: restrict conversion input to Rōmaji
>>> current options: source=any, target=Hiragana, flags=None
>>> enter string (c=clear flags, f=set flag, q=quit, h=help, -k|-h|-r|-K|-H|-R):
)",
      SkipFirstTwoLines);
}

TEST_F(KanaConvertTest, InteractiveSetFlag) {
  const char* args[]{"", "-i", "-r"}; // Rōmaji target
  // set flag to Kunrei and convert 'し' (which is 'si' in Kunrei style Rōmaji)
  is() << "f\nk\nし\n";
  run(args,
      ">>> enter flag option (h=Hepburn, k=Kunrei, n=NoProlongMark, "
      "r=RemoveSpaces): " +
          OptionsMsg + "any, target=Romaji, flags=Kunrei" + EnterMsg + "si\n",
      SkipFirstTwoLines);
}

TEST_F(KanaConvertTest, InteractiveSetAndClearFlag) {
  const char* args[]{"", "-i", "-r"};
  // 'c' clears any flags so 'し' should convert to 'shi' (the default)
  is() << "f\nk\nc\nし\n";
  run(args, OptionsMsg + "any, target=Romaji, flags=None" + EnterMsg + "shi\n",
      SkipFirstFourLines);
}

TEST_F(KanaConvertTest, InteractiveChangeTarget) {
  const char* args[]{"", "-i"};
  is() << "-k\nrāmen\n";
  run(args, "ラーメン\n", SkipFirstFourLines);
}

TEST_F(KanaConvertTest, InteractiveIllegalOption) {
  const char* args[]{"", "-i"};
  is() << "-o\n";
  run(args, "  illegal option: -o\n" + DefOptionsMsg + "None" + EnterMsg,
      SkipFirstTwoLines);
}

} // namespace kanji_tools
