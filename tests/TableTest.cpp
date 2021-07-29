#include <gtest/gtest.h>

#include <kanji/Table.h>

#include <sstream>

namespace kanji {

class TableTest : public ::testing::Test {
protected:
  std::stringstream _os;
};

TEST_F(TableTest, EmptyTable) {
  Table t;
  t.print(_os);
  std::string line;
  // shouldn't print anything
  EXPECT_FALSE(std::getline(_os, line));
  _os.clear();
  t.printMarkdown(_os);
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(TableTest, TableWithOnlyEmptyRows) {
  Table t;
  t.add();
  t.print(_os);
  std::string line;
  // one empty row prints nothing
  EXPECT_FALSE(std::getline(_os, line));
  _os.clear();
  t.printMarkdown(_os);
  EXPECT_FALSE(std::getline(_os, line));
  t.add();
  t.add();
  // multiple empty rows still prints nothing
  EXPECT_FALSE(std::getline(_os, line));
  _os.clear();
  t.printMarkdown(_os);
  EXPECT_FALSE(std::getline(_os, line));
}

TEST_F(TableTest, TableWithJustTitles) {
  std::string world("world");
  Table t({"hello", world});
  t.print(_os);
  // clang-format off
  const char* expected[] = {
    "+-------+-------+",
    "| hello | world |",
    "+-------+-------+"};
  // clang-format on
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(TableTest, TableWithTitleAndEmptyRows) {
  Table t({"hello", "world"});
  t.add();
  t.add();
  t.add();
  t.print(_os);
  // clang-format off
  const char* expected[] = {
    "+-------+-------+",
    "| hello | world |",
    "|       |       |",
    "|       |       |",
    "|       |       |",
    "+-------+-------+"};
  // clang-format on
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(TableTest, TableWithTitleAndSectionAndEmptyRows) {
  Table t({"hello", "world"});
  t.add({}, true);
  t.add();
  t.add();
  t.print(_os);
  // clang-format off
  const char* expected[] = {
    "+-------+-------+",
    "| hello | world |",
    "+-------+-------+",
    "|       |       |",
    "|       |       |",
    "|       |       |",
    "+-------+-------+"};
  // clang-format on
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(TableTest, TableWithOneCell) {
  Table t;
  t.add({"a"});
  t.print(_os);
  const char* expected[] = {"+---+", "| a |", "+---+"};
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(TableTest, TableWithMultipleRowsAndColumns) {
  Table t;
  t.add({"a", "b", "c"});
  t.add({"1", "123"});
  t.print(_os);
  // clang-format off
  const char* expected[] = {
    "+---+-----+---+",
    "| a | b   | c |",
    "| 1 | 123 |   |",
    "+---+-----+---+"};
  const char* expectedMD[] = {
    "|  |  |  |",
    "| --- | --- | --- |",
    "| a | b | c |",
    "| 1 | 123 |  |"};
  // clang-format on
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
  _os.clear();
  t.printMarkdown(_os);
  count = 0;
  maxLines = std::size(expectedMD);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expectedMD[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(TableTest, TableWithTitleAndSectionsAndRows) {
  Table t({"one", "two", "three"});
  t.add({"a", "b", "c"}, true);
  t.add({"1", "123"});
  t.add({"x", "", "y", "z"}, true); // four columns
  t.print(_os);
  // clang-format off
  const char* expected[] = {
    "+-----+-----+-------+---+",
    "| one | two | three |   |",
    "+-----+-----+-------+---+",
    "| a   | b   | c     |   |",
    "| 1   | 123 |       |   |",
    "+-----+-----+-------+---+",
    "| x   |     | y     | z |",
    "+-----+-----+-------+---+"};
  // clang-format on
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(TableTest, TableWithCount) {
  Table t({"count", "one", "two"}, true);
  t.add({"a", "b"}, true);
  t.add({"5", "789"}, true);
  t.add({"x"});
  t.print(_os);
  // clang-format off
  const char* expected[] = {
    "+-------+-----+-----+",
    "| count | one | two |",
    "+-------+-----+-----+",
    "| 1     | a   | b   |",
    "+-------+-----+-----+",
    "| 2     | 5   | 789 |",
    "| 3     | x   |     |",
    "+-------+-----+-----+"};
  // clang-format on
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(TableTest, TableWithWideCharacters) {
  Table t({"数字", "one", "two"}, true);
  t.add({"a", "カタカナ"}, true);
  t.add({"5", "中"});
  t.add({"x", "y/はい"});
  t.print(_os);
  // clang-format off
  // This text aligns properly on a terminal (see comments in Table.h for more details)
  const char* expected[] = {
    "+------+-----+----------+",
    "| 数字 | one | two      |",
    "+------+-----+----------+",
    "| 1    | a   | カタカナ |",
    "| 2    | 5   | 中       |",
    "| 3    | x   | y/はい   |",
    "+------+-----+----------+"};
  // Markdown output doesn't try to align columns (that's done by the browser or editor)
  const char* expectedMD[] = {
    "| 数字 | one | two |",
    "| --- | --- | --- |",
    "| **1** | **a** | **カタカナ** |",
    "| 2 | 5 | 中 |",
    "| 3 | x | y/はい |"};
  // clang-format on
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
  _os.clear();
  t.printMarkdown(_os);
  count = 0;
  maxLines = std::size(expectedMD);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expectedMD[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

} // namespace kanji
