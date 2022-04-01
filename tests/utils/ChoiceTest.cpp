#include <gtest/gtest.h>
#include <kanji_tools/utils/Choice.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

class ChoiceTest : public ::testing::Test {
protected:
  ChoiceTest() : _choice{_os, &_is} {}

  using Choices = Choice::Choices;
  using Range = Choice::Range;

  // helper functions to call _choice.get with '\n' as input
  template<typename... Args>
  auto getMsgDef(const std::string& msg, const Choices& c, Args... args) {
    _is << '\n';
    return _choice.get(msg, c, args...);
  }
  template<typename... Args>
  auto rangeMsgDef(const std::string& msg, const Range& r, Args... args) {
    _is << '\n';
    return _choice.get(msg, r, args...);
  }

  // write 'x' as input (followed by '\n') before calling 'get'
  template<typename... Args>
  auto getMsg(char x, const std::string& msg, const Choices& c, Args... args) {
    _is << x;
    return getMsgDef(msg, c, args...);
  }
  template<typename... Args>
  auto rangeMsg(char x, const std::string& msg, const Range& r, Args... args) {
    _is << x;
    return rangeMsgDef(msg, r, args...);
  }

  // helper functions that call 'get' with an empty message
  template<typename... Args> auto getDef(const Choices& c, Args... args) {
    return getMsgDef("", c, args...);
  }
  template<typename... Args> auto rangeDef(const Range& r, Args... args) {
    return rangeMsgDef("", r, args...);
  }
  template<typename... Args> auto get(char x, const Choices& c, Args... args) {
    return getMsg(x, "", c, args...);
  }
  template<typename... Args> auto range(char x, const Range& r, Args... args) {
    return rangeMsg(x, "", r, args...);
  }

  [[nodiscard]] auto getOutput() {
    std::string result;
    std::getline(_os, result);
    return result;
  }

  std::stringstream _os;
  std::stringstream _is;
  Choice _choice;
};

} // namespace

TEST_F(ChoiceTest, SingleChoice) {
  EXPECT_EQ(get('a', {{'a', ""}}), 'a');
  EXPECT_EQ(getOutput(), "(a): ");
}

TEST_F(ChoiceTest, NoChoicesError) {
  EXPECT_THROW(call([this] { getDef({}); }, "must specify at least one choice"),
      std::domain_error);
}

TEST_F(ChoiceTest, NonPrintableError) {
  const char esc{27};
  const auto f{[esc, this] { getDef({{esc, ""}}); }};
  EXPECT_THROW(call(f, "option is non-printable: 0x1b"), std::domain_error);
}

TEST_F(ChoiceTest, TwoChoices) {
  EXPECT_EQ(get('a', {{'a', ""}, {'b', ""}}), 'a');
  EXPECT_EQ(getOutput(), "(a-b): ");
}

TEST_F(ChoiceTest, TwoNonConsecutiveChoices) {
  EXPECT_EQ(get('a', {{'a', ""}, {'c', ""}}), 'a');
  EXPECT_EQ(getOutput(), "(a, c): ");
}

TEST_F(ChoiceTest, MultipleConsecutiveChoices) {
  EXPECT_EQ(
      get('e', {{'a', ""}, {'b', ""}, {'c', ""}, {'e', ""}, {'f', ""}}), 'e');
  EXPECT_EQ(getOutput(), "(a-c, e-f): ");
}

TEST_F(ChoiceTest, ConsecutiveAndNonConsecutiveChoices) {
  EXPECT_EQ(get('c', {{'a', ""}, {'b', ""}, {'c', ""}, {'e', ""}, {'1', ""},
                         {'2', ""}}),
      'c');
  // Note: choices map is in ascii order so numbers are shown before letters.
  EXPECT_EQ(getOutput(), "(1-2, a-c, e): ");
}

TEST_F(ChoiceTest, ChoicesWithMessageAndDescriptions) {
  EXPECT_EQ(getMsg('b', "hello", {{'a', "world"}, {'b', "!"}, {'e', ""}}), 'b');
  EXPECT_EQ(getOutput(), "hello (a=world, b=!, e): ");
}

TEST_F(ChoiceTest, DescriptionsAndRanges) {
  EXPECT_EQ(getMsg('a', "hello",
                {{'1', ""}, {'2', ""}, {'a', "world"}, {'b', "!"}, {'c', ""},
                    {'d', ""}}),
      'a');
  EXPECT_EQ(getOutput(), "hello (1-2, a=world, b=!, c-d): ");
}

TEST_F(ChoiceTest, ChoiceWithDefault) {
  // don't need to specify the choice when there's a default
  EXPECT_EQ(getDef({{'1', ""}, {'2', ""}}, '1'), '1');
  EXPECT_EQ(getOutput(), "(1-2) def '1': ");
}

TEST_F(ChoiceTest, ChooseNonDefault) {
  EXPECT_EQ(get('2', {{'1', ""}, {'2', ""}}, '1'), '2');
  EXPECT_EQ(getOutput(), "(1-2) def '1': ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, RangeWithDefault) {
  // don't need to specify the choice when there's a default
  EXPECT_EQ(rangeDef({'1', '4'}, '1'), '1');
  EXPECT_EQ(getOutput(), "(1-4) def '1': ");
}

TEST_F(ChoiceTest, InvalidRange) {
  const auto f{[this] { rangeDef({'2', '1'}); }};
  EXPECT_THROW(call(f, "first range option '2' is greater than last '1'"),
      std::domain_error);
}

TEST_F(ChoiceTest, NonPrintableFirstRange) {
  const auto f{[this] { rangeDef({'\0', 'a'}); }};
  EXPECT_THROW(
      call(f, "first range option is non-printable: 0x00"), std::domain_error);
}

TEST_F(ChoiceTest, NonPrintableLastRange) {
  const auto f{[this] { rangeDef({'a', 10}); }};
  EXPECT_THROW(
      call(f, "last range option is non-printable: 0x0a"), std::domain_error);
}

TEST_F(ChoiceTest, RangeWithNoDefault) {
  EXPECT_EQ(rangeMsg('b', "pick", {'a', 'z'}), 'b');
  EXPECT_EQ(getOutput(), "pick (a-z): ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, RangeAndChoices) {
  EXPECT_EQ(
      rangeMsg('g', "pick", {'a', 'f'}, Choices{{'g', "good"}, {'y', "yes"}}),
      'g');
  EXPECT_EQ(getOutput(), "pick (a-f, g=good, y=yes): ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, RangeChoicesAndDefault) {
  EXPECT_EQ(rangeMsgDef(
                "pick", {'a', 'f'}, Choices{{'g', "good"}, {'y', "yes"}}, 'y'),
      'y');
  EXPECT_EQ(getOutput(), "pick (a-f, g=good, y=yes) def 'y': ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, NewLineWithoutDefault) {
  _is << "\n";
  EXPECT_EQ(get('2', {{'1', ""}, {'2', ""}}), '2');
  // Note: new line is not sent to console when prompting for an option since
  // the user should be entering their choice on the same line as the 'prompt'
  // message. If they choose an invalid option and presss enter then the
  // 'prompt' message is sent again to output.
  EXPECT_EQ(getOutput(), "(1-2): (1-2): ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, ChooseBadOption) {
  _is << "3\n";
  EXPECT_EQ(get('2', {{'1', ""}, {'2', ""}}), '2');
  EXPECT_EQ(getOutput(), "(1-2): (1-2): ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, ChooseBadOptionWithDefault) {
  _is << "3\n";
  EXPECT_EQ(get('2', {{'1', ""}, {'2', ""}}, '1'), '2');
  EXPECT_EQ(getOutput(), "(1-2) def '1': (1-2) def '1': ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, QuitOption) {
  EXPECT_FALSE(_choice.quit());
  EXPECT_FALSE(_choice.isQuit('q'));
  _choice.setQuit('q');
  EXPECT_TRUE(_choice.isQuit('q'));
  EXPECT_EQ(_choice.quit(), 'q');
  EXPECT_EQ(_choice.quitDescription(), "quit");
  EXPECT_EQ(get('q', {{'1', ""}, {'2', ""}}), 'q');
  EXPECT_EQ(getOutput(), "(1-2, q=quit): ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, QuitDescription) {
  EXPECT_FALSE(_choice.quit());
  EXPECT_FALSE(_choice.isQuit('s'));
  _choice.setQuit('s', "終了");
  EXPECT_TRUE(_choice.isQuit('s'));
  EXPECT_EQ(_choice.quit(), 's');
  EXPECT_EQ(_choice.quitDescription(), "終了");
  EXPECT_EQ(get('s', {{'1', ""}, {'2', ""}}), 's');
  EXPECT_EQ(getOutput(), "(1-2, s=終了): ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, SetQuitFromConstructor) {
  Choice choice{_os, 'e'};
  EXPECT_EQ(choice.quit(), 'e');
  EXPECT_EQ(choice.quitDescription(), "quit"); // default quit description
  Choice choiceWithQuitDescription{_os, 'e', "end"};
  EXPECT_EQ(choiceWithQuitDescription.quitDescription(), "end");
}

TEST_F(ChoiceTest, NonPrintableQuitError) {
  EXPECT_THROW(call([this] { _choice.setQuit(22); },
                   "quit option is non-printable: 0x16"),
      std::domain_error);
}

TEST_F(ChoiceTest, NonPrintableQuitFromConstructorError) {
  const auto f{[this] { Choice{_os, 23}; }};
  EXPECT_THROW(
      call(f, "quit option is non-printable: 0x17"), std::domain_error);
}

TEST_F(ChoiceTest, UseQuitOption) {
  _is << "q\n";
  _choice.setQuit('q');
  EXPECT_EQ(_choice.get("", true, {{'1', ""}, {'2', ""}}), 'q');
  EXPECT_EQ(getOutput(), "(1-2, q=quit): ");
  EXPECT_TRUE(_os.eof());
  _os.clear(); // need to clear 'eof' state on _os before calling 'get' again
  ASSERT_FALSE(_os.eof());
  _is << "2\n";
  // specify false for 'useSkip' parameter to skip using quit option
  EXPECT_EQ(_choice.get("", false, {{'1', ""}, {'2', ""}}), '2');
  EXPECT_EQ(getOutput(), "(1-2): ");
}

TEST_F(ChoiceTest, ClearQuitOption) {
  _is << "q\n1\n";
  _choice.setQuit('q');
  _choice.clearQuit();
  EXPECT_FALSE(_choice.quit());
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}), '1');
  EXPECT_EQ(getOutput(), "(1-2): (1-2): ");
  EXPECT_TRUE(_os.eof());
}

TEST_F(ChoiceTest, MissingDefaultOption) {
  const auto f{[this] { _choice.get("", {{'a', "abc"}, {'b', "123"}}, 'e'); }};
  EXPECT_THROW(call(f, "default option 'e' not in choices"), std::domain_error);
}

TEST_F(ChoiceTest, DuplicateQuitOption) {
  _choice.setQuit('q');
  for (bool useQuit : {false, true}) {
    const auto f{[useQuit, this] { _choice.get("", useQuit, {{'q', "abc"}}); }};
    EXPECT_THROW(
        call(f, "quit option 'q' already in choices"), std::domain_error);
  }
}

TEST_F(ChoiceTest, DuplicateRangeOption) {
  const Choices choices{{'a', "12"}, {'c', "34"}};
  const std::string start{"range option '"}, end{"' already in choices"};
  for (char rangeStart : {'a', 'b'}) {
    const auto f{[&, this] { _choice.get("", {rangeStart, 'c'}, choices); }};
    EXPECT_THROW(call(f, start + (rangeStart == 'a' ? 'a' : 'c') + end),
        std::domain_error);
  }
}

} // namespace kanji_tools
