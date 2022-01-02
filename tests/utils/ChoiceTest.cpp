#include <gtest/gtest.h>

#include <kanji_tools/utils/Choice.h>

#include <sstream>

namespace kanji_tools {

class ChoiceTest : public ::testing::Test {
protected:
  ChoiceTest() : _choice(_os, &_is) {}

  std::stringstream _os;
  std::stringstream _is;
  Choice _choice;
};

TEST_F(ChoiceTest, SingleChoice) {
  _is << "a\n";
  EXPECT_EQ(_choice.get("", {{'a', ""}}), 'a');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(a): ");
}

TEST_F(ChoiceTest, NoChoicesError) {
  try {
    _choice.get("", {});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("must specify at least one choice"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ChoiceTest, NonPrintableError) {
  try {
    char escape = 27;
    _choice.get("", {{escape, ""}});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("option is non-printable: 0x1b"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ChoiceTest, TwoChoices) {
  _is << "a\n";
  EXPECT_EQ(_choice.get("", {{'a', ""}, {'b', ""}}), 'a');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(a-b): ");
}

TEST_F(ChoiceTest, TwoNonConsecutiveChoices) {
  _is << "a\n";
  EXPECT_EQ(_choice.get("", {{'a', ""}, {'c', ""}}), 'a');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(a, c): ");
}

TEST_F(ChoiceTest, MultipleConsecutiveChoices) {
  _is << "e\n";
  EXPECT_EQ(_choice.get("", {{'a', ""}, {'b', ""}, {'c', ""}, {'e', ""}, {'f', ""}}), 'e');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(a-c, e-f): ");
}

TEST_F(ChoiceTest, ConsecutiveAndNonConsecutiveChoices) {
  _is << "c\n";
  EXPECT_EQ(_choice.get("", {{'a', ""}, {'b', ""}, {'c', ""}, {'e', ""}, {'1', ""}, {'2', ""}}), 'c');
  std::string line;
  std::getline(_os, line);
  // Note: choices map is in ascii order so numbers are shown before letters.
  EXPECT_EQ(line, "(1-2, a-c, e): ");
}

TEST_F(ChoiceTest, ChoicesWithMessageAndDescriptions) {
  _is << "b\n";
  EXPECT_EQ(_choice.get("hello", {{'a', "world"}, {'b', "!"}, {'e', ""}}), 'b');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "hello (a=world, b=!, e): ");
}

TEST_F(ChoiceTest, DescriptionsAndRanges) {
  _is << "a\n";
  EXPECT_EQ(_choice.get("hello", {{'1', ""}, {'2', ""}, {'a', "world"}, {'b', "!"}, {'c', ""}, {'d', ""}}), 'a');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "hello (1-2, a=world, b=!, c-d): ");
}

TEST_F(ChoiceTest, ChoiceWithDefault) {
  _is << "\n"; // don't need to specify the choice when there's a default (just new line)
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}, '1'), '1');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2) def '1': ");
}

TEST_F(ChoiceTest, ChooseNonDefault) {
  _is << "2\n";
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}, '1'), '2');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2) def '1': ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, RangeWithDefault) {
  _is << "\n"; // don't need to specify the choice when there's a default (just new line)
  EXPECT_EQ(_choice.get("", '1', '4', '1'), '1');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-4) def '1': ");
}

TEST_F(ChoiceTest, InvalidRange) {
  try {
    _choice.get("", '2', '1');
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("first range option '2' is greater than last '1'"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ChoiceTest, NonPrintableFirstRange) {
  try {
    _choice.get("", {}, 'a');
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("first range option is non-printable: 0x00"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ChoiceTest, NonPrintableLastRange) {
  try {
    _choice.get("", 'a', 10);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("last range option is non-printable: 0x0a"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ChoiceTest, RangeWithNoDefault) {
  _is << "b\n";
  EXPECT_EQ(_choice.get("pick", 'a', 'z'), 'b');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "pick (a-z): ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, RangeAndChoices) {
  _is << "g\n";
  EXPECT_EQ(_choice.get("pick", 'a', 'f', {{'g', "good"}, {'y', "yes"}}), 'g');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "pick (a-f, g=good, y=yes): ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, RangeChoicesAndDefault) {
  _is << "\n";
  EXPECT_EQ(_choice.get("pick", 'a', 'f', {{'g', "good"}, {'y', "yes"}}, 'y'), 'y');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "pick (a-f, g=good, y=yes) def 'y': ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, NewLineWithoutDefault) {
  _is << "\n2\n";
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}), '2');
  std::string line;
  std::getline(_os, line);
  // Note: new line is not sent to console when prompting for an option since the user should
  // be entering their choice on the same line as the 'prompt' message. If they choose an
  // invalid option and presss enter then the 'prompt' message is sent again to output.
  EXPECT_EQ(line, "(1-2): (1-2): ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, ChooseBadOption) {
  _is << "3\n2\n";
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}), '2');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2): (1-2): ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, ChooseBadOptionWithDefault) {
  _is << "3\n2\n";
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}, '1'), '2');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2) def '1': (1-2) def '1': ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, QuitOption) {
  _is << "q\n";
  EXPECT_FALSE(_choice.quit());
  EXPECT_FALSE(_choice.isQuit('q'));
  _choice.setQuit('q');
  EXPECT_TRUE(_choice.isQuit('q'));
  EXPECT_EQ(_choice.quit(), 'q');
  EXPECT_EQ(_choice.quitDescription(), "quit");
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}), 'q');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2, q=quit): ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, QuitDescription) {
  _is << "s\n";
  EXPECT_FALSE(_choice.quit());
  EXPECT_FALSE(_choice.isQuit('s'));
  _choice.setQuit('s', "終了");
  EXPECT_TRUE(_choice.isQuit('s'));
  EXPECT_EQ(_choice.quit(), 's');
  EXPECT_EQ(_choice.quitDescription(), "終了");
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}), 's');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2, s=終了): ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, SetQuitFromConstructor) {
  Choice choice(_os, 'e');
  EXPECT_EQ(choice.quit(), 'e');
  EXPECT_EQ(choice.quitDescription(), "quit"); // default quit description
  Choice choiceWithQuitDescription(_os, 'e', "end");
  EXPECT_EQ(choiceWithQuitDescription.quitDescription(), "end");
}

TEST_F(ChoiceTest, NonPrintableQuitError) {
  try {
    _choice.setQuit(22);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("quit option is non-printable: 0x16"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ChoiceTest, NonPrintableQuitFromConstructorError) {
  try {
    Choice choice(_os, 23);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("quit option is non-printable: 0x17"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ChoiceTest, UseQuitOption) {
  _is << "q\n";
  _choice.setQuit('q');
  EXPECT_EQ(_choice.get("", true, {{'1', ""}, {'2', ""}}), 'q');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2, q=quit): ");
  EXPECT_TRUE(_os.eof());
  _os.clear(); // need to clear 'eof' state on _os before calling 'get' again
  ASSERT_FALSE(_os.eof());
  _is << "2\n";
  // specify false for 'useSkip' parameter to skip using quit option
  EXPECT_EQ(_choice.get("", false, {{'1', ""}, {'2', ""}}), '2');
  EXPECT_TRUE(std::getline(_os, line));
  EXPECT_EQ(line, "(1-2): ");
}

TEST_F(ChoiceTest, ClearQuitOption) {
  _is << "q\n1\n";
  _choice.setQuit('q');
  _choice.clearQuit();
  EXPECT_FALSE(_choice.quit());
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}), '1');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2): (1-2): ");
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(ChoiceTest, MissingDefaultOption) {
  try {
    _choice.get("", {{'a', "abc"}, {'b', "123"}}, 'e');
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("default option 'e' not in choices"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ChoiceTest, DuplicateQuitOption) {
  _choice.setQuit('q');
  for (bool useQuit : {false, true})
    try {
      _choice.get("", useQuit, {{'q', "abc"}});
      FAIL() << "Expected std::domain_error";
    } catch (std::domain_error& err) {
      EXPECT_EQ(err.what(), std::string("quit option 'q' already in choices"));
    } catch (...) {
      FAIL() << "Expected std::domain_error";
    }
}

TEST_F(ChoiceTest, DuplicateRangeOption) {
  Choice::Choices choices = {{'a', "12"}, {'c', "34"}};
  const std::string start("range option '"), end("' already in choices");
  for (char rangeStart : {'a', 'b'})
    try {
      _choice.get("", rangeStart, 'c', choices);
      FAIL() << "Expected std::domain_error";
    } catch (std::domain_error& err) {
      EXPECT_EQ(err.what(), start + (rangeStart == 'a' ? 'a' : 'c') + end);
    } catch (...) {
      FAIL() << "Expected std::domain_error";
    }
}

} // namespace kanji_tools
