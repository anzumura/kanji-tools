#include <gtest/gtest.h>

#include <kanji/Table.h>

#include <sstream>

namespace kanji {

class TableTest : public ::testing::Test {
protected:
  std::stringstream _os;
};

TEST_F(TableTest, BasicTable) {
  Table t;
  t.add(Table::Row{"a", "b"});
  t.print(_os);
  const char* expected[] = {
    "+---+---+",
    "| a | b |",
    "+---+---+"
  };
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

} // namespace kanji
