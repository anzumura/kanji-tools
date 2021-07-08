#include <gtest/gtest.h>

#include <kanji/Choice.h>

#include <sstream>

namespace kanji {

class ChoiceTest : public ::testing::Test {
protected:
  ChoiceTest() : _choice(_os, _is) {}

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
  EXPECT_EQ(line, "(1-2) default '1': ");
}

TEST_F(ChoiceTest, ChooseNonDefault) {
  _is << "2\n";
  EXPECT_EQ(_choice.get("", {{'1', ""}, {'2', ""}}, '1'), '2');
  std::string line;
  std::getline(_os, line);
  EXPECT_EQ(line, "(1-2) default '1': ");
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
  EXPECT_EQ(line, "(1-2) default '1': (1-2) default '1': ");
  EXPECT_FALSE(std::getline(_os, line));
}

} // namespace kanji
