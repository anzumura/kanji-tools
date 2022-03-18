#include <gtest/gtest.h>
#include <kanji_tools/tests/WhatMismatch.h>
#include <kanji_tools/utils/ColumnFile.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

TEST(ColumnFileColumnTest, DifferentNumberForDifferentName) {
  const ColumnFile::Column colA{"A"}, colB{"B"};
  EXPECT_EQ(colA.name(), "A");
  EXPECT_EQ(colB.name(), "B");
  EXPECT_NE(colA.number(), colB.number());
}

TEST(ColumnFileColumnTest, SameNumberForSameName) {
  const ColumnFile::Column colC1{"C"}, colC2{"C"};
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

  void write(const std::string& s, bool newLine = true) const {
    std::ofstream of{TestFile};
    of << s;
    if (newLine) of << '\n';
    of.close();
  }

  inline static const ColumnFile::Column Col{"Col"}, Col1{"Col1"}, Col2{"Col2"},
      Col3{"Col3"}, Col4{"Col4"};

  inline static const fs::path TestDir{"testDir"};
  inline static const fs::path TestFile{TestDir / "testFile.txt"};
};

TEST_F(ColumnFileTest, SingleColumnFile) {
  write("Col");
  const ColumnFile f(TestFile, {Col});
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
  write("Col\nVal");
  ColumnFile f{TestFile, {Col}};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.currentRow(), 1);
  EXPECT_EQ(f.get(Col), "Val");
  EXPECT_FALSE(f.nextRow());
  EXPECT_EQ(f.currentRow(), 1);
}

TEST_F(ColumnFileTest, GetEmptyValueFromOneColumn) {
  write("Col\n");
  ColumnFile f{TestFile, {Col}};
  ASSERT_TRUE(f.nextRow());
  EXPECT_TRUE(f.isEmpty(Col));
}

TEST_F(ColumnFileTest, GetValueFromMultipleColumns) {
  write("Col1\tCol2\tCol3\nVal1\tVal2\tVal3");
  ColumnFile f{TestFile, {Col1, Col2, Col3}};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(Col1), "Val1");
  EXPECT_EQ(f.get(Col2), "Val2");
  EXPECT_EQ(f.get(Col3), "Val3");
}

TEST_F(ColumnFileTest, UseNonDefaultDelimiter) {
  write("Col1|Col2|Col3\nVal1|Val2|");
  ColumnFile f{TestFile, {Col1, Col2, Col3}, '|'};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(Col1), "Val1");
  EXPECT_EQ(f.get(Col2), "Val2");
  // make sure getting a final empty value works for non-default delimiter
  EXPECT_EQ(f.get(Col3), "");
}

TEST_F(ColumnFileTest, AllowGettingEmptyValues) {
  write("Col1\tCol2\tCol3\tCol4\n\tVal2\t\t");
  ColumnFile f{TestFile, {Col1, Col2, Col3, Col4}};
  ASSERT_TRUE(f.nextRow());
  EXPECT_TRUE(f.isEmpty(Col1));
  EXPECT_FALSE(f.isEmpty(Col2));
  EXPECT_TRUE(f.isEmpty(Col3));
  EXPECT_TRUE(f.isEmpty(Col4));
  EXPECT_EQ(f.get(Col2), "Val2");
}

TEST_F(ColumnFileTest, HeaderColumnOrderDifferentThanConstructor) {
  write("Col1\tCol2\tCol3\nVal1\tVal2\tVal3");
  ColumnFile f{TestFile, {Col3, Col2, Col1}};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(Col1), "Val1");
  EXPECT_EQ(f.get(Col2), "Val2");
  EXPECT_EQ(f.get(Col3), "Val3");
}

TEST_F(ColumnFileTest, GetMultipleRows) {
  write("Col1\tCol2\tCol3\nR11\tR12\tR13\nR21\tR22\tR23");
  ColumnFile f{TestFile, {Col1, Col2, Col3}};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(Col1), "R11");
  EXPECT_EQ(f.get(Col2), "R12");
  EXPECT_EQ(f.get(Col3), "R13");
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(Col1), "R21");
  EXPECT_EQ(f.get(Col2), "R22");
  EXPECT_EQ(f.get(Col3), "R23");
  EXPECT_EQ(f.currentRow(), 2);
}

TEST_F(ColumnFileTest, NotEnoughColumns) {
  write("Col1\tCol2\tCol3\nVal1\tVal2");
  ColumnFile f{TestFile, {Col1, Col2, Col3}};
  EXPECT_THROW(call([&f] { f.nextRow(); },
                   "not enough columns - file: testFile.txt, row: 1"),
      std::domain_error);
}

TEST_F(ColumnFileTest, TooManyColumns) {
  write("Col1\tCol2\tCol3\nVal1\tVal2\tVal3\tVal4");
  ColumnFile f{TestFile, {Col1, Col2, Col3}};
  EXPECT_THROW(call([&f] { f.nextRow(); },
                   "too many columns - file: testFile.txt, row: 1"),
      std::domain_error);
}

TEST_F(ColumnFileTest, UnrecognizedHeaderError) {
  write("HeaderName");
  const ColumnFile::Column col("ColumnName");
  EXPECT_THROW(call(
                   [&] {
                     ColumnFile f{TestFile, {col}};
                   },
                   "unrecognized header 'HeaderName' - file: testFile.txt"),
      std::domain_error);
}

TEST_F(ColumnFileTest, DuplicateHeaderError) {
  write("Col\tCol");
  EXPECT_THROW(call(
                   [&] {
                     ColumnFile f{TestFile, {Col}};
                   },
                   "duplicate header 'Col' - file: testFile.txt"),
      std::domain_error);
}

TEST_F(ColumnFileTest, DuplicateColumnError) {
  write("");
  EXPECT_THROW(call(
                   [&] {
                     ColumnFile f{TestFile, {Col1, Col2, Col1}};
                   },
                   "duplicate column 'Col1' - file: testFile.txt"),
      std::domain_error);
}

TEST_F(ColumnFileTest, OneMissingColumnError) {
  write("Col1");
  EXPECT_THROW(call(
                   [&] {
                     ColumnFile f{TestFile, {Col1, Col2}};
                   },
                   "column 'Col2' not found - file: testFile.txt"),
      std::domain_error);
}

TEST_F(ColumnFileTest, MultipleMissingColumnsError) {
  write("Col1\tCol3");
  EXPECT_THROW(call(
                   [&] {
                     ColumnFile f(TestFile, {Col1, Col2, Col3, Col4});
                   },
                   "2 columns not found: 'Col2', 'Col4' - file: testFile.txt"),
      std::domain_error);
}

TEST_F(ColumnFileTest, MissingFileError) {
  EXPECT_THROW(call(
                   [&] {
                     ColumnFile f{TestFile, {Col}};
                   },
                   "doesn't exist - file: testFile.txt"),
      std::domain_error);
}

TEST_F(ColumnFileTest, NotRegularFileError) {
  EXPECT_THROW(call(
                   [&] {
                     ColumnFile f{TestDir, {Col}};
                   },
                   "not regular file - file: testDir"),
      std::domain_error);
}

TEST_F(ColumnFileTest, MissingHeaderRowError) {
  write("", false);
  EXPECT_THROW(call(
                   [&] {
                     ColumnFile f{TestFile, {Col}};
                   },
                   "missing header row - file: testFile.txt"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetBeforeNextRowError) {
  write("Col");
  ColumnFile f{TestFile, {Col}};
  EXPECT_THROW(
      call([&] { f.get(Col); },
          "'nextRow' must be called before calling 'get' - file: testFile.txt"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetUnrecognizedColumError) {
  write("Col\nVal");
  ColumnFile f{TestFile, {Col}};
  EXPECT_TRUE(f.nextRow());
  const ColumnFile::Column columnCreatedAfterConstruction{"Created After"};
  EXPECT_THROW(
      call([&] { f.get(columnCreatedAfterConstruction); },
          "unrecognized column 'Created After' - file: testFile.txt, row: 1"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetInvalidColumError) {
  write("Col\nVal");
  const ColumnFile::Column columnNotIncludedInFile{"Not Included"};
  ColumnFile f{TestFile, {Col}};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(
      call([&] { f.get(columnNotIncludedInFile); },
          "invalid column 'Not Included' - file: testFile.txt, row: 1"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetULong) {
  write("Col\n123");
  ColumnFile f{TestFile, {Col}};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getULong(Col), 123);
}

TEST_F(ColumnFileTest, GetULongError) {
  write("Col\nblah");
  ColumnFile f{TestFile, {Col}};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.getULong(Col); },
                   convertError + "unsigned long - file: testFile.txt, row: "
                                  "1, column: 'Col', value: 'blah'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetULongMaxValueError) {
  const auto maxValue{123U};
  std::ofstream of{TestFile};
  of << "Col\n" << maxValue << '\n' << maxValue << '\n' << maxValue + 1 << '\n';
  of.close();
  ColumnFile f{TestFile, {Col}};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getULong(Col, maxValue), maxValue);
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getULong(Col, 0), maxValue); // 0 implies no max value
  EXPECT_TRUE(f.nextRow());
  std::string msg{"exceeded max value of "};
  EXPECT_THROW(
      call([&] { f.getULong(Col, maxValue); },
          msg + std::to_string(maxValue) +
              " - file: testFile.txt, row: 3, column: 'Col', value: '" +
              std::to_string(maxValue + 1) + "'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetUInt) {
  write("Col\n123");
  ColumnFile f{TestFile, {Col}};
  EXPECT_TRUE(f.nextRow());
  u_int8_t expected{123};
  EXPECT_EQ(f.getUInt<u_int8_t>(Col), expected);
  EXPECT_EQ(f.getU8(Col), expected); // convenience function
}

TEST_F(ColumnFileTest, GetUIntError) {
  write("Col\n1234");
  ColumnFile f{TestFile, {Col}};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.getU8(Col); },
                   "exceeded max value of 255 - file: testFile.txt, row: 1, "
                   "column: 'Col', value: '1234'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetOptULong) {
  write("Col\n123\n");
  ColumnFile f{TestFile, {Col}};
  f.nextRow();
  EXPECT_EQ(f.getOptULong(Col), 123);
  EXPECT_TRUE(f.nextRow());
  EXPECT_FALSE(f.getOptULong(Col));
}

TEST_F(ColumnFileTest, GetOptULongError) {
  write("Col\nblah");
  ColumnFile f{TestFile, {Col}};
  f.nextRow();
  EXPECT_THROW(call([&] { f.getOptULong(Col); },
                   convertError + "unsigned long - file: testFile.txt, row: "
                                  "1, column: 'Col', value: 'blah'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetOptUInt) {
  write("Col\n123\n");
  ColumnFile f(TestFile, {Col});
  f.nextRow();
  EXPECT_EQ(f.getOptUInt<u_int16_t>(Col), 123);
  EXPECT_TRUE(f.nextRow());
  EXPECT_FALSE(f.getOptU16(Col));
}

TEST_F(ColumnFileTest, GetOptUIntError) {
  write("Col\n256");
  ColumnFile f(TestFile, {Col});
  f.nextRow();
  EXPECT_THROW(call([&] { f.getOptU8(Col); },
                   "exceeded max value of 255 - file: testFile.txt, row: 1, "
                   "column: 'Col', value: '256'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetBool) {
  write("1\t2\t3\t4\t5\nY\tT\tN\tF\t");
  const ColumnFile::Column c1("1"), c2("2"), c3("3"), c4("4"), c5("5");
  ColumnFile f{TestFile, {c1, c2, c3, c4, c5}};
  f.nextRow();
  EXPECT_TRUE(f.getBool(c1));
  EXPECT_TRUE(f.getBool(c2));
  EXPECT_FALSE(f.getBool(c3));
  EXPECT_FALSE(f.getBool(c4));
  EXPECT_FALSE(f.getBool(c5));
}

TEST_F(ColumnFileTest, GetBoolError) {
  write("Col\nx");
  ColumnFile f(TestFile, {Col});
  f.nextRow();
  EXPECT_THROW(
      call([&] { f.getBool(Col); },
          convertError +
              "bool - file: testFile.txt, row: 1, column: 'Col', value: 'x'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetWChar) {
  write("1\t2\n898B\t20B9F");
  const ColumnFile::Column c1("1"), c2("2");
  ColumnFile f{TestFile, {c1, c2}};
  f.nextRow();
  EXPECT_EQ(f.getWChar(c1), 35211);
  EXPECT_EQ(f.getWChar(c2), 134047);
}

TEST_F(ColumnFileTest, GetWCharError) {
  write("Col\nAAA\n123456\nABCd\nDEFG");
  ColumnFile f(TestFile, {Col});
  const auto _ = {"size must be 4 or 5 - file: testFile.txt, row: "
                  "1, column: 'Col', value: 'AAA'",
      "size must be 4 or 5 - file: testFile.txt, row: "
      "2, column: 'Col', value: '123456'",
      "invalid hex - file: testFile.txt, row: 3, "
      "column: 'Col', value: 'ABCd'",
      "invalid hex - file: testFile.txt, row: 4, "
      "column: 'Col', value: 'DEFG'"};
  for (auto i : _) {
    f.nextRow();
    EXPECT_THROW(
        call([&] { f.getWChar(Col); }, convertError + "char32_t, " + i),
        std::domain_error);
  }
}

} // namespace kanji_tools
