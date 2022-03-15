#include <gtest/gtest.h>
#include <kanji_tools/tests/WhatMismatch.h>
#include <kanji_tools/utils/ColumnFile.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

TEST(ColumnFileColumnTest, DifferentNumberForDifferentName) {
  ColumnFile::Column colA("A"), colB("B");
  EXPECT_EQ(colA.name(), "A");
  EXPECT_EQ(colB.name(), "B");
  EXPECT_NE(colA.number(), colB.number());
}

TEST(ColumnFileColumnTest, SameNumberForSameName) {
  ColumnFile::Column colC1("C"), colC2("C");
  EXPECT_EQ(colC1.name(), colC2.name());
  EXPECT_EQ(colC1.number(), colC2.number());
}

class ColumnFileTest : public ::testing::Test {
protected:
  inline static const std::string convertError{"failed to convert to "};
  void SetUp() override {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
  }
  void TearDown() override { fs::remove_all(TestDir); }
  inline static const fs::path TestDir{"testDir"};
  inline static const fs::path TestFile{TestDir / "testFile.txt"};
};

TEST_F(ColumnFileTest, SingleColumnFile) {
  std::ofstream of(TestFile);
  of << "Col\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  EXPECT_EQ(f.name(), "testFile.txt");
  EXPECT_EQ(f.columns(), 1);
  EXPECT_EQ(f.currentRow(), 0);
}

TEST_F(ColumnFileTest, NoColumnsError) {
  EXPECT_THROW(call([] { ColumnFile f(TestFile, {}); },
                    "must specify at least one column - file: testFile.txt"),
               std::domain_error);
}

TEST_F(ColumnFileTest, GetValueFromOneColumn) {
  std::ofstream of(TestFile);
  of << "Col\nVal\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.currentRow(), 1);
  EXPECT_EQ(f.get(col), "Val");
  EXPECT_FALSE(f.nextRow());
  EXPECT_EQ(f.currentRow(), 1);
}

TEST_F(ColumnFileTest, GetEmptyValueFromOneColumn) {
  std::ofstream of(TestFile);
  of << "Col\n\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  ASSERT_TRUE(f.nextRow());
  EXPECT_TRUE(f.isEmpty(col));
}

TEST_F(ColumnFileTest, GetValueFromMultipleColumns) {
  std::ofstream of(TestFile);
  of << "Col1\tCol2\tCol3\nVal1\tVal2\tVal3\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(TestFile, {col1, col2, col3});
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(col1), "Val1");
  EXPECT_EQ(f.get(col2), "Val2");
  EXPECT_EQ(f.get(col3), "Val3");
}

TEST_F(ColumnFileTest, UseNonDefaultDelimiter) {
  std::ofstream of(TestFile);
  of << "Col1|Col2|Col3\nVal1|Val2|\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(TestFile, {col1, col2, col3}, '|');
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(col1), "Val1");
  EXPECT_EQ(f.get(col2), "Val2");
  // make sure getting a final empty value works for non-default delimiter
  EXPECT_EQ(f.get(col3), "");
}

TEST_F(ColumnFileTest, AllowGettingEmptyValues) {
  std::ofstream of(TestFile);
  of << "Col1\tCol2\tCol3\tCol4\n\tVal2\t\t\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3"), col4("Col4");
  ColumnFile f(TestFile, {col1, col2, col3, col4});
  ASSERT_TRUE(f.nextRow());
  EXPECT_TRUE(f.isEmpty(col1));
  EXPECT_FALSE(f.isEmpty(col2));
  EXPECT_TRUE(f.isEmpty(col3));
  EXPECT_TRUE(f.isEmpty(col4));
  EXPECT_EQ(f.get(col2), "Val2");
}

TEST_F(ColumnFileTest, HeaderColumnOrderDifferentThanConstructor) {
  std::ofstream of(TestFile);
  of << "Col1\tCol2\tCol3\nVal1\tVal2\tVal3\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(TestFile, {col3, col2, col1});
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(col1), "Val1");
  EXPECT_EQ(f.get(col2), "Val2");
  EXPECT_EQ(f.get(col3), "Val3");
}

TEST_F(ColumnFileTest, GetMultipleRows) {
  std::ofstream of(TestFile);
  of << "Col1\tCol2\tCol3\nR11\tR12\tR13\nR21\tR22\tR23\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(TestFile, {col1, col2, col3});
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(col1), "R11");
  EXPECT_EQ(f.get(col2), "R12");
  EXPECT_EQ(f.get(col3), "R13");
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(col1), "R21");
  EXPECT_EQ(f.get(col2), "R22");
  EXPECT_EQ(f.get(col3), "R23");
  EXPECT_EQ(f.currentRow(), 2);
}

TEST_F(ColumnFileTest, NotEnoughColumns) {
  std::ofstream of(TestFile);
  of << "Col1\tCol2\tCol3\nVal1\tVal2\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(TestFile, {col1, col2, col3});
  EXPECT_THROW(call([&f] { f.nextRow(); },
                    "not enough columns - file: testFile.txt, row: 1"),
               std::domain_error);
}

TEST_F(ColumnFileTest, TooManyColumns) {
  std::ofstream of(TestFile);
  of << "Col1\tCol2\tCol3\nVal1\tVal2\tVal3\tVal4\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(TestFile, {col1, col2, col3});
  EXPECT_THROW(call([&f] { f.nextRow(); },
                    "too many columns - file: testFile.txt, row: 1"),
               std::domain_error);
}

TEST_F(ColumnFileTest, UnrecognizedHeaderError) {
  std::ofstream of(TestFile);
  of << "HeaderName\n";
  of.close();
  ColumnFile::Column col("ColumnName");
  EXPECT_THROW(call([&] { ColumnFile f(TestFile, {col}); },
                    "unrecognized header 'HeaderName' - file: testFile.txt"),
               std::domain_error);
}

TEST_F(ColumnFileTest, DuplicateHeaderError) {
  std::ofstream of(TestFile);
  of << "Col\tCol\n";
  of.close();
  ColumnFile::Column col("Col");
  EXPECT_THROW(call([&] { ColumnFile f(TestFile, {col}); },
                    "duplicate header 'Col' - file: testFile.txt"),
               std::domain_error);
}

TEST_F(ColumnFileTest, DuplicateColumnError) {
  std::ofstream of(TestFile);
  of << "HeaderName\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2");
  EXPECT_THROW(call(
                 [&] {
                   ColumnFile f(TestFile, {col1, col2, col1});
                 },
                 "duplicate column 'Col1' - file: testFile.txt"),
               std::domain_error);
}

TEST_F(ColumnFileTest, OneMissingColumnError) {
  std::ofstream of(TestFile);
  of << "Col1\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2");
  EXPECT_THROW(call(
                 [&] {
                   ColumnFile f(TestFile, {col1, col2});
                 },
                 "column 'Col2' not found - file: testFile.txt"),
               std::domain_error);
}

TEST_F(ColumnFileTest, MultipleMissingColumnsError) {
  std::ofstream of(TestFile);
  of << "Col1\tCol3\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3"), col4("Col4");
  EXPECT_THROW(call(
                 [&] {
                   ColumnFile f(TestFile, {col1, col2, col3, col4});
                 },
                 "2 columns not found: 'Col2', 'Col4' - file: testFile.txt"),
               std::domain_error);
}

TEST_F(ColumnFileTest, MissingFileError) {
  ColumnFile::Column col("Col");
  EXPECT_THROW(call([&] { ColumnFile f(TestFile, {col}); },
                    "doesn't exist - file: testFile.txt"),
               std::domain_error);
}

TEST_F(ColumnFileTest, NotRegularFileError) {
  ColumnFile::Column col("Col");
  EXPECT_THROW(call([&] { ColumnFile f(TestDir, {col}); },
                    "not regular file - file: testDir"),
               std::domain_error);
}

TEST_F(ColumnFileTest, MissingHeaderRowError) {
  std::ofstream of(TestFile);
  of << "";
  of.close();
  ColumnFile::Column col("Col");
  EXPECT_THROW(call([&] { ColumnFile f(TestFile, {col}); },
                    "missing header row - file: testFile.txt"),
               std::domain_error);
}

TEST_F(ColumnFileTest, GetBeforeNextRowError) {
  std::ofstream of(TestFile);
  of << "Col\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  EXPECT_THROW(
    call([&] { f.get(col); },
         "'nextRow' must be called before calling 'get' - file: testFile.txt"),
    std::domain_error);
}

TEST_F(ColumnFileTest, GetUnrecognizedColumError) {
  std::ofstream of(TestFile);
  of << "Col\nVal\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  ColumnFile::Column columnCreatedAfterConstruction("Created After");
  EXPECT_THROW(
    call([&] { f.get(columnCreatedAfterConstruction); },
         "unrecognized column 'Created After' - file: testFile.txt, row: 1"),
    std::domain_error);
}

TEST_F(ColumnFileTest, GetInvalidColumError) {
  std::ofstream of(TestFile);
  of << "Col\nVal\n";
  of.close();
  ColumnFile::Column col("Col"), columnNotIncludedInFile("Not Included");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  EXPECT_THROW(
    call([&] { f.get(columnNotIncludedInFile); },
         "invalid column 'Not Included' - file: testFile.txt, row: 1"),
    std::domain_error);
}

TEST_F(ColumnFileTest, GetULong) {
  std::ofstream of(TestFile);
  of << "Col\n123\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  EXPECT_EQ(f.getULong(col), 123);
}

TEST_F(ColumnFileTest, GetULongError) {
  std::ofstream of(TestFile);
  of << "Col\nblah\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  EXPECT_THROW(call([&] { f.getULong(col); },
                    convertError + "unsigned long - file: testFile.txt, row: "
                                   "1, column: 'Col', value: 'blah'"),
               std::domain_error);
}

TEST_F(ColumnFileTest, GetSizeMaxValueError) {
  std::ofstream of(TestFile);
  const auto maxValue{123U};
  of << "Col\n" << maxValue << '\n' << maxValue << '\n' << maxValue + 1 << '\n';
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  EXPECT_EQ(f.getULong(col, maxValue), maxValue);
  f.nextRow();
  EXPECT_EQ(f.getULong(col, 0), maxValue); // 0 implies no max value
  f.nextRow();
  std::string msg{"exceeded max value of "};
  EXPECT_THROW(call([&] { f.getULong(col, maxValue); },
                    msg + std::to_string(maxValue) +
                      " - file: testFile.txt, row: 3, column: 'Col', value: '" +
                      std::to_string(maxValue + 1) + "'"),
               std::domain_error);
}

TEST_F(ColumnFileTest, GetUInt) {
  std::ofstream of(TestFile);
  of << "Col\n123\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  u_int8_t expected{123};
  EXPECT_EQ(f.getUInt<u_int8_t>(col), expected);
}

TEST_F(ColumnFileTest, GetUIntError) {
  std::ofstream of(TestFile);
  of << "Col\n1234\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  EXPECT_THROW(call([&] { f.getUInt<u_int8_t>(col); },
                    "exceeded max value of 255 - file: testFile.txt, row: 1, "
                    "column: 'Col', value: '1234'"),
               std::domain_error);
}

TEST_F(ColumnFileTest, GetOptULong) {
  std::ofstream of(TestFile);
  of << "Col\n123\n\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  EXPECT_EQ(f.getOptULong(col), 123);
  EXPECT_TRUE(f.nextRow());
  EXPECT_FALSE(f.getOptULong(col));
}

TEST_F(ColumnFileTest, GetOptULongError) {
  std::ofstream of(TestFile);
  of << "Col\nblah\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  EXPECT_THROW(call([&] { f.getOptULong(col); },
                    convertError + "unsigned long - file: testFile.txt, row: "
                                   "1, column: 'Col', value: 'blah'"),
               std::domain_error);
}

TEST_F(ColumnFileTest, GetBool) {
  std::ofstream of(TestFile);
  of << "1\t2\t3\t4\t5\nY\tT\tN\tF\t\n";
  of.close();
  ColumnFile::Column c1("1"), c2("2"), c3("3"), c4("4"), c5("5");
  ColumnFile f(TestFile, {c1, c2, c3, c4, c5});
  f.nextRow();
  EXPECT_TRUE(f.getBool(c1));
  EXPECT_TRUE(f.getBool(c2));
  EXPECT_FALSE(f.getBool(c3));
  EXPECT_FALSE(f.getBool(c4));
  EXPECT_FALSE(f.getBool(c5));
}

TEST_F(ColumnFileTest, GetBoolError) {
  std::ofstream of(TestFile);
  of << "Col\nx\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  f.nextRow();
  EXPECT_THROW(
    call([&] { f.getBool(col); },
         convertError +
           "bool - file: testFile.txt, row: 1, column: 'Col', value: 'x'"),
    std::domain_error);
}

TEST_F(ColumnFileTest, GetWChar) {
  std::ofstream of(TestFile);
  of << "1\t2\n898B\t20B9F\n";
  of.close();
  ColumnFile::Column c1("1"), c2("2");
  ColumnFile f(TestFile, {c1, c2});
  f.nextRow();
  EXPECT_EQ(f.getWChar(c1), 35211);
  EXPECT_EQ(f.getWChar(c2), 134047);
}

TEST_F(ColumnFileTest, GetWCharError) {
  std::ofstream of(TestFile);
  of << "Col\nAAA\n123456\nABCd\nDEFG\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(TestFile, {col});
  for (auto i : {std::string("size must be 4 or 5 - file: testFile.txt, row: "
                             "1, column: 'Col', value: 'AAA'"),
                 std::string("size must be 4 or 5 - file: testFile.txt, row: "
                             "2, column: 'Col', value: '123456'"),
                 std::string("invalid hex - file: testFile.txt, row: 3, "
                             "column: 'Col', value: 'ABCd'"),
                 std::string("invalid hex - file: testFile.txt, row: 4, "
                             "column: 'Col', value: 'DEFG'")}) {
    f.nextRow();
    EXPECT_THROW(
      call([&] { f.getWChar(col); }, convertError + "char32_t, " + i),
      std::domain_error);
  }
}

} // namespace kanji_tools
