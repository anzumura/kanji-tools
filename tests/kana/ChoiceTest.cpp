#include <gtest/gtest.h>
#include <kt_kana/Choice.h>
#include <kt_tests/WhatMismatch.h>
#include <kt_utils/Exception.h>

#include <sstream>

namespace kanji_tools {

namespace {

class ChoiceTest : public ::testing::Test {
protected:
  ChoiceTest() : _choice{_os, &_is} {}

  using Choices = Choice::Choices;
  using Range = Choice::Range;

  // helper functions to call _choice.get with '\n' as input
  template <typename... Args>
  auto getMsgDef(const String& msg, const Choices& c, Args... args) {
    _is << '\n';
    return _choice.get(msg, c, args...);
  }
  template <typename... Args>
  auto rangeMsgDef(Range r, const String& msg, Args... args) {
    _is << '\n';
    return _choice.get(r, msg, args...);
  }

  // write 'x' as input (followed by '\n') before calling 'get'
  template <typename... Args>
  auto getMsg(char x, const String& msg, const Choices& c, Args... args) {
    _is << x;
    return getMsgDef(msg, c, args...);
  }
  template <typename... Args>
  auto rangeMsg(Range r, char x, const String& msg, Args... args) {
    _is << x;
    return rangeMsgDef(r, msg, args...);
  }

  // helper functions that call 'get' with an empty message
  template <typename... Args> auto getDef(const Choices& c, Args... args) {
    return getMsgDef("", c, args...);
  }
  template <typename... Args> auto rangeDef(Range r, Args... args) {
    return rangeMsgDef(r, "", args...);
  }
  template <typename... Args> auto get(char x, const Choices& c, Args... args) {
    return getMsg(x, "", c, args...);
  }
  template <typename... Args> auto range(char x, const Range& r, Args... args) {
    return rangeMsg(x, "", r, args...);
  }

  [[nodiscard]] auto getOutput() {
    String result;
    std::getline(_os, result);
    return result;
  }

  [[nodiscard]] auto& os() { return _os; }
  [[nodiscard]] auto& is() { return _is; }
  [[nodiscard]] auto& choice() { return _choice; }

private:
  std::stringstream _os, _is;
  Choice _choice;
};

} // namespace

TEST_F(ChoiceTest, SingleChoice) {
  EXPECT_EQ(get('a', {{'a', emptyString()}}), 'a');
  EXPECT_EQ(getOutput(), "(a): ");
}

TEST_F(ChoiceTest, NoChoicesError) {
  EXPECT_THROW(call([this] { getDef({}); }, "must specify at least one choice"),
      DomainError);
}

TEST_F(ChoiceTest, NonPrintableError) {
  const char esc{27};
  const auto f{[esc, this] { getDef({{esc, emptyString()}}); }};
  EXPECT_THROW(call(f, "option is non-printable: 0x1b"), DomainError);
}

TEST_F(ChoiceTest, TwoChoices) {
  EXPECT_EQ(get('a', {{'a', emptyString()}, {'b', emptyString()}}), 'a');
  EXPECT_EQ(getOutput(), "(a-b): ");
}

TEST_F(ChoiceTest, TwoNonConsecutiveChoices) {
  EXPECT_EQ(get('a', {{'a', emptyString()}, {'c', emptyString()}}), 'a');
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
  EXPECT_EQ(getDef({{'1', emptyString()}, {'2', emptyString()}}, '1'), '1');
  EXPECT_EQ(getOutput(), "(1-2) def '1': ");
}

TEST_F(ChoiceTest, ChooseNonDefault) {
  EXPECT_EQ(get('2', {{'1', emptyString()}, {'2', emptyString()}}, '1'), '2');
  EXPECT_EQ(getOutput(), "(1-2) def '1': ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, RangeWithDefault) {
  // don't need to specify the choice when there's a default
  EXPECT_EQ(rangeDef({'1', '4'}, '1'), '1');
  EXPECT_EQ(getOutput(), "(1-4) def '1': ");
}

TEST_F(ChoiceTest, InvalidRange) {
  const auto f{[this] { rangeDef({'2', '1'}); }};
  EXPECT_THROW(
      call(f, "first range option '2' is greater than last '1'"), DomainError);
}

TEST_F(ChoiceTest, NonPrintableFirstRange) {
  const auto f{[this] { rangeDef({'\0', 'a'}); }};
  EXPECT_THROW(
      call(f, "first range option is non-printable: 0x00"), DomainError);
}

TEST_F(ChoiceTest, NonPrintableLastRange) {
  constexpr char ten{10};
  const auto f{[this, ten] { rangeDef({'a', ten}); }};
  EXPECT_THROW(
      call(f, "last range option is non-printable: 0x0a"), DomainError);
}

TEST_F(ChoiceTest, RangeWithNoDefault) {
  EXPECT_EQ(rangeMsg({'a', 'z'}, 'b', "pick"), 'b');
  EXPECT_EQ(getOutput(), "pick (a-z): ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, RangeAndChoices) {
  EXPECT_EQ(
      rangeMsg({'a', 'f'}, 'g', "pick", Choices{{'g', "good"}, {'y', "yes"}}),
      'g');
  EXPECT_EQ(getOutput(), "pick (a-f, g=good, y=yes): ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, RangeChoicesAndDefault) {
  EXPECT_EQ(rangeMsgDef(
                {'a', 'f'}, "pick", Choices{{'g', "good"}, {'y', "yes"}}, 'y'),
      'y');
  EXPECT_EQ(getOutput(), "pick (a-f, g=good, y=yes) def 'y': ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, NewLineWithoutDefault) {
  is() << "\n";
  EXPECT_EQ(get('2', {{'1', emptyString()}, {'2', emptyString()}}), '2');
  // Note: new line is not sent to console when prompting for an option since
  // the user should be entering their choice on the same line as the 'prompt'
  // message. If they choose an invalid option and press enter then the 'prompt'
  // message is sent again to output.
  EXPECT_EQ(getOutput(), "(1-2): (1-2): ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, ChooseBadOption) {
  is() << "3\n";
  EXPECT_EQ(get('2', {{'1', emptyString()}, {'2', emptyString()}}), '2');
  EXPECT_EQ(getOutput(), "(1-2): (1-2): ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, ChooseBadOptionWithDefault) {
  is() << "3\n";
  EXPECT_EQ(get('2', {{'1', emptyString()}, {'2', emptyString()}}, '1'), '2');
  EXPECT_EQ(getOutput(), "(1-2) def '1': (1-2) def '1': ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, QuitOption) {
  EXPECT_FALSE(choice().quit());
  EXPECT_FALSE(choice().isQuit('q'));
  choice().setQuit('q');
  EXPECT_TRUE(choice().isQuit('q'));
  EXPECT_EQ(choice().quit(), 'q');
  EXPECT_EQ(choice().quitDescription(), "quit");
  EXPECT_EQ(get('q', {{'1', emptyString()}, {'2', emptyString()}}), 'q');
  EXPECT_EQ(getOutput(), "(1-2, q=quit): ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, QuitDescription) {
  EXPECT_FALSE(choice().quit());
  EXPECT_FALSE(choice().isQuit('s'));
  choice().setQuit('s', "??????");
  EXPECT_TRUE(choice().isQuit('s'));
  EXPECT_EQ(choice().quit(), 's');
  EXPECT_EQ(choice().quitDescription(), "??????");
  EXPECT_EQ(get('s', {{'1', emptyString()}, {'2', emptyString()}}), 's');
  EXPECT_EQ(getOutput(), "(1-2, s=??????): ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, SetQuitFromConstructor) {
  Choice choice{os(), 'e'};
  EXPECT_EQ(choice.quit(), 'e');
  EXPECT_EQ(choice.quitDescription(), "quit"); // default quit description
  Choice choiceWithQuitDescription{os(), 'e', "end"};
  EXPECT_EQ(choiceWithQuitDescription.quitDescription(), "end");
}

TEST_F(ChoiceTest, NonPrintableQuitError) {
  EXPECT_THROW(call([this] { choice().setQuit(22); },
                   "quit option is non-printable: 0x16"),
      DomainError);
}

TEST_F(ChoiceTest, NonPrintableQuitFromConstructorError) {
  constexpr char bad{23};
  const auto f{[this, bad] { Choice{os(), bad}; }};
  EXPECT_THROW(call(f, "quit option is non-printable: 0x17"), DomainError);
}

TEST_F(ChoiceTest, UseQuitOption) {
  is() << "q\n";
  choice().setQuit('q');
  EXPECT_EQ(choice().get(emptyString(), true, {{'1', ""}, {'2', ""}}), 'q');
  EXPECT_EQ(getOutput(), "(1-2, q=quit): ");
  EXPECT_TRUE(os().eof());
  os().clear(); // need to clear 'eof' state on _os before calling 'get' again
  ASSERT_FALSE(os().eof());
  is() << "2\n";
  // specify false for 'useSkip' parameter to skip using quit option
  EXPECT_EQ(choice().get(emptyString(), false, {{'1', ""}, {'2', ""}}), '2');
  EXPECT_EQ(getOutput(), "(1-2): ");
}

TEST_F(ChoiceTest, ClearQuitOption) {
  is() << "q\n1\n";
  choice().setQuit('q');
  choice().clearQuit();
  EXPECT_FALSE(choice().quit());
  EXPECT_EQ(choice().get(emptyString(), {{'1', ""}, {'2', ""}}), '1');
  EXPECT_EQ(getOutput(), "(1-2): (1-2): ");
  EXPECT_TRUE(os().eof());
}

TEST_F(ChoiceTest, MissingDefaultOption) {
  const auto f{[this] {
    return choice().get(emptyString(), {{'a', "abc"}, {'b', "123"}}, 'e');
  }};
  EXPECT_THROW(call(f, "default option 'e' not in choices"), DomainError);
}

TEST_F(ChoiceTest, DuplicateQuitOption) {
  choice().setQuit('q');
  for (bool useQuit : {false, true}) {
    const auto f{[useQuit, this] {
      return choice().get(emptyString(), useQuit, {{'q', "abc"}});
    }};
    EXPECT_THROW(call(f, "quit option 'q' already in choices"), DomainError);
  }
}

TEST_F(ChoiceTest, DuplicateRangeOption) {
  const Choices choices{{'a', "12"}, {'c', "34"}};
  const String start{"range option '"}, end{"' already in choices"};
  for (char rangeStart : {'a', 'b'}) {
    const auto f{[rangeStart, &choices, this] {
      return choice().get({rangeStart, 'c'}, emptyString(), choices); // NOLINT
    }};
    EXPECT_THROW(
        call(f, start + (rangeStart == 'a' ? 'a' : 'c') += end), DomainError);
  }
}

} // namespace kanji_tools
