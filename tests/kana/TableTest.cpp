#include <gtest/gtest.h>
#include <kt_kana/Table.h>
#include <kt_tests/Utils.h>

#include <sstream>

namespace kanji_tools {

namespace {

class TableTest : public ::testing::Test {
protected:
  std::stringstream _os;
};

} // namespace

TEST_F(TableTest, EmptyTable) {
  Table t;
  t.print(_os);
  String line;
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
  String line;
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
  String world{"world"};
  Table t{{"hello", world}};
  t.print(_os);
  // clang-format off
  const auto expected = {
    "+-------+-------+",
    "| hello | world |",
    "+-------+-------+"};
  // clang-format on
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

TEST_F(TableTest, TableWithTitleAndEmptyRows) {
  Table t{{"hello", "world"}};
  t.add();
  t.add();
  t.add();
  t.print(_os);
  // clang-format off
  const auto expected = {
    "+-------+-------+",
    "| hello | world |",
    "|       |       |",
    "|       |       |",
    "|       |       |",
    "+-------+-------+"};
  // clang-format on
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

TEST_F(TableTest, TableWithTitleAndSectionAndEmptyRows) {
  Table t{{"hello", "world"}};
  t.add({}, true);
  t.add();
  t.add();
  t.print(_os);
  // clang-format off
  const auto expected = {
    "+-------+-------+",
    "| hello | world |",
    "+-------+-------+",
    "|       |       |",
    "|       |       |",
    "|       |       |",
    "+-------+-------+"};
  // clang-format on
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

TEST_F(TableTest, TableWithOneCell) {
  Table t;
  t.add({"a"});
  t.print(_os);
  const auto expected = {"+---+", "| a |", "+---+"};
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

TEST_F(TableTest, TableWithMultipleRowsAndColumns) {
  Table t;
  t.add({"a", "b", "c"});
  t.add({"1", "123"});
  t.print(_os);
  // clang-format off
  const auto expected = {
    "+---+-----+---+",
    "| a | b   | c |",
    "| 1 | 123 |   |",
    "+---+-----+---+"};
  const auto expectedMD = {
    "|  |  |  |",
    "| --- | --- | --- |",
    "| a | b | c |",
    "| 1 | 123 |  |"};
  // clang-format on
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
  _os.clear();
  t.printMarkdown(_os);
  EXPECT_EQ(findEqualMatches(_os, expectedMD), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

TEST_F(TableTest, TableWithTitleSectionsAndRows) {
  Table t{{"one", "two", "three"}};
  t.add({"a", "b", "c"}, true);
  t.add({"1", "123"});
  t.add({"x", "", "y", "z"}, true); // four columns
  t.print(_os);
  // clang-format off
  const auto expected = {
    "+-----+-----+-------+---+",
    "| one | two | three |   |",
    "+-----+-----+-------+---+",
    "| a   | b   | c     |   |",
    "| 1   | 123 |       |   |",
    "+-----+-----+-------+---+",
    "| x   |     | y     | z |",
    "+-----+-----+-------+---+"};
  // clang-format on
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

TEST_F(TableTest, TableWithCount) {
  Table t{{"count", "one", "two"}, true};
  t.add({"a", "b"}, true);
  t.add({"5", "789"}, true);
  t.add({"x"});
  t.print(_os);
  // clang-format off
  const auto expected = {
    "+-------+-----+-----+",
    "| count | one | two |",
    "+-------+-----+-----+",
    "| 1     | a   | b   |",
    "+-------+-----+-----+",
    "| 2     | 5   | 789 |",
    "| 3     | x   |     |",
    "+-------+-----+-----+"};
  // clang-format on
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

TEST_F(TableTest, TableWithWideCharacters) {
  Table t{{"??????", "one", "two"}, true};
  t.add({"a", "????????????"}, true);
  t.add({"5", "???"});
  t.add({"x", "y/??????"});
  t.print(_os);
  // clang-format off
  // This text aligns properly on a terminal (see comments in Table.h for more details)
  const auto expected = {
    "+------+-----+----------+",
    "| ?????? | one | two      |",
    "+------+-----+----------+",
    "| 1    | a   | ???????????? |",
    "| 2    | 5   | ???       |",
    "| 3    | x   | y/??????   |",
    "+------+-----+----------+"};
  // Markdown output doesn't try to align columns (that's done by the browser or editor)
  const auto expectedMD = {
    "| ?????? | one | two |",
    "| --- | --- | --- |",
    "| **1** | **a** | **????????????** |",
    "| 2 | 5 | ??? |",
    "| 3 | x | y/?????? |"};
  // clang-format on
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
  _os.clear();
  t.printMarkdown(_os);
  EXPECT_EQ(findEqualMatches(_os, expectedMD), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

TEST_F(TableTest, EscapePipeForMarkdown) {
  Table t{{"a", "b", "c"}};
  t.add({"1", "1|2", "3"});
  t.print(_os);
  // clang-format off
  const auto expected = {
    "+---+-----+---+",
    "| a | b   | c |",
    "| 1 | 1|2 | 3 |",
    "+---+-----+---+"};
  const auto expectedMD = {
    "| a | b | c |",
    "| --- | --- | --- |",
    "| 1 | 1\\|2 | 3 |"};
  // clang-format on
  EXPECT_EQ(findEqualMatches(_os, expected), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
  _os.clear();
  t.printMarkdown(_os);
  EXPECT_EQ(findEqualMatches(_os, expectedMD), std::nullopt);
  EXPECT_FALSE(hasMoreData(_os));
}

} // namespace kanji_tools
