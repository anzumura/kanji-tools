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
  inline static const ColumnFile::Column Col{"Col"}, Col1{"Col1"}, Col2{"Col2"},
      Col3{"Col3"}, Col4{"Col4"};
  inline static const std::string FileMsg{" - file: testFile.txt"},
      ConvertError{"failed to convert to "};
  inline static const fs::path TestDir{"testDir"};
  inline static const fs::path TestFile{TestDir / "testFile.txt"};

  inline static auto create(const ColumnFile::Columns& c, char delim = '\t') {
    return ColumnFile{TestFile, c, delim};
  }

  void SetUp() override {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
  }

  void TearDown() override { fs::remove_all(TestDir); }

  static void write(const std::string& s, bool newLine = true) {
    std::ofstream of{TestFile};
    of << s;
    if (newLine) of << '\n';
    of.close();
  }

  static ColumnFile write(
      const ColumnFile::Columns& c, const std::string& s, bool newLine = true) {
    write(s, newLine);
    return create(c);
  }
};

TEST_F(ColumnFileTest, SingleColumnFile) {
  const auto f{write({Col}, "Col")};
  EXPECT_EQ(f.name(), "testFile.txt");
  EXPECT_EQ(f.columns(), 1);
  EXPECT_EQ(f.currentRow(), 0);
}

TEST_F(ColumnFileTest, NoColumnsError) {
  EXPECT_THROW(
      call([] { create({}); }, "must specify at least one column" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetValueFromOneColumn) {
  auto f{write({Col}, "Col\nVal")};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.currentRow(), 1);
  EXPECT_EQ(f.get(Col), "Val");
  EXPECT_FALSE(f.nextRow());
  EXPECT_EQ(f.currentRow(), 1);
}

TEST_F(ColumnFileTest, GetEmptyValueFromOneColumn) {
  auto f{write({Col}, "Col\n")};
  ASSERT_TRUE(f.nextRow());
  EXPECT_TRUE(f.isEmpty(Col));
}

TEST_F(ColumnFileTest, GetValueFromMultipleColumns) {
  auto f{write({Col1, Col2, Col3}, "Col1\tCol2\tCol3\nVal1\tVal2\tVal3")};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(Col1), "Val1");
  EXPECT_EQ(f.get(Col2), "Val2");
  EXPECT_EQ(f.get(Col3), "Val3");
}

TEST_F(ColumnFileTest, UseNonDefaultDelimiter) {
  write("Col1|Col2|Col3\nVal1|Val2|");
  auto f{create({Col1, Col2, Col3}, '|')};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(Col1), "Val1");
  EXPECT_EQ(f.get(Col2), "Val2");
  // make sure getting a final empty value works for non-default delimiter
  EXPECT_EQ(f.get(Col3), "");
}

TEST_F(ColumnFileTest, AllowGettingEmptyValues) {
  auto f{write({Col1, Col2, Col3, Col4}, "Col1\tCol2\tCol3\tCol4\n\tVal2\t\t")};
  ASSERT_TRUE(f.nextRow());
  EXPECT_TRUE(f.isEmpty(Col1));
  EXPECT_FALSE(f.isEmpty(Col2));
  EXPECT_TRUE(f.isEmpty(Col3));
  EXPECT_TRUE(f.isEmpty(Col4));
  EXPECT_EQ(f.get(Col2), "Val2");
}

TEST_F(ColumnFileTest, HeaderColumnOrderDifferentThanConstructor) {
  auto f{write({Col3, Col2, Col1}, "Col1\tCol2\tCol3\nVal1\tVal2\tVal3")};
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(Col1), "Val1");
  EXPECT_EQ(f.get(Col2), "Val2");
  EXPECT_EQ(f.get(Col3), "Val3");
}

TEST_F(ColumnFileTest, GetMultipleRows) {
  auto f{write(
      {Col1, Col2, Col3}, "Col1\tCol2\tCol3\nR11\tR12\tR13\nR21\tR22\tR23")};
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
  auto f{write({Col1, Col2, Col3}, "Col1\tCol2\tCol3\nVal1\tVal2")};
  EXPECT_THROW(
      call([&f] { f.nextRow(); }, "not enough columns" + FileMsg + ", row: 1"),
      std::domain_error);
}

TEST_F(ColumnFileTest, TooManyColumns) {
  auto f{write({Col1, Col2, Col3}, "Col1\tCol2\tCol3\nVal1\tVal2\tVal3\tVal4")};
  EXPECT_THROW(
      call([&f] { f.nextRow(); }, "too many columns" + FileMsg + ", row: 1"),
      std::domain_error);
}

TEST_F(ColumnFileTest, UnrecognizedHeaderError) {
  const ColumnFile::Columns c{{"B"}};
  EXPECT_THROW(
      call([&c] { write(c, "A"); }, "unrecognized header 'A'" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, DuplicateHeaderError) {
  write("Col\tCol");
  EXPECT_THROW(call([] { create({Col}); }, "duplicate header 'Col'" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, DuplicateColumnError) {
  write("");
  EXPECT_THROW(call(
                   [] {
                     create({Col1, Col2, Col1});
                   },
                   "duplicate column 'Col1'" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, OneMissingColumnError) {
  write("Col1");
  EXPECT_THROW(call(
                   [] {
                     create({Col1, Col2});
                   },
                   "column 'Col2' not found" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, MultipleMissingColumnsError) {
  write("Col1\tCol3");
  EXPECT_THROW(call(
                   [] {
                     create({Col1, Col2, Col3, Col4});
                   },
                   "2 columns not found: 'Col2', 'Col4'" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, MissingFileError) {
  EXPECT_THROW(call([] { create({Col}); }, "doesn't exist" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, NotRegularFileError) {
  EXPECT_THROW(call(
                   [] {
                     ColumnFile{TestDir, {Col}};
                   },
                   "not regular file - file: testDir"),
      std::domain_error);
}

TEST_F(ColumnFileTest, MissingHeaderRowError) {
  EXPECT_THROW(
      call([] { write({Col}, "", false); }, "missing header row" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetBeforeNextRowError) {
  const auto f{write({Col}, "Col")};
  EXPECT_THROW(call([&f] { f.get(Col); },
                   "'nextRow' must be called before calling 'get'" + FileMsg),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetUnrecognizedColumError) {
  auto f{write({Col}, "Col\nVal")};
  EXPECT_TRUE(f.nextRow());
  const ColumnFile::Column columnCreatedAfterConstruction{"Created After"};
  EXPECT_THROW(
      call([&] { f.get(columnCreatedAfterConstruction); },
          "unrecognized column 'Created After'" + FileMsg + ", row: 1"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetInvalidColumError) {
  const ColumnFile::Column columnNotIncludedInFile{"Not Included"};
  auto f{write({Col}, "Col\nVal")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.get(columnNotIncludedInFile); },
                   "invalid column 'Not Included'" + FileMsg + ", row: 1"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetULong) {
  auto f{write({Col}, "Col\n123")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getULong(Col), 123);
}

TEST_F(ColumnFileTest, GetULongError) {
  auto f{write({Col}, "Col\nblah")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.getULong(Col); },
                   ConvertError + "unsigned long" + FileMsg +
                       ", row: 1, column: 'Col', value: 'blah'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetULongMaxValueError) {
  const auto maxValue{123U};
  std::ofstream of{TestFile};
  of << "Col\n" << maxValue << '\n' << maxValue << '\n' << maxValue + 1 << '\n';
  of.close();
  auto f{create({Col})};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getULong(Col, maxValue), maxValue);
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getULong(Col, 0), maxValue); // 0 implies no max value
  EXPECT_TRUE(f.nextRow());
  std::string msg{"exceeded max value of "};
  EXPECT_THROW(call([&] { f.getULong(Col, maxValue); },
                   msg + std::to_string(maxValue) + FileMsg +
                       ", row: 3, column: 'Col', value: '" +
                       std::to_string(maxValue + 1) + "'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetUInt) {
  auto f{write({Col}, "Col\n123")};
  EXPECT_TRUE(f.nextRow());
  u_int8_t expected{123};
  EXPECT_EQ(f.getUInt<u_int8_t>(Col), expected);
  EXPECT_EQ(f.getU8(Col), expected); // convenience function
}

TEST_F(ColumnFileTest, GetUIntError) {
  auto f{write({Col}, "Col\n1234")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(
      call([&] { f.getU8(Col); }, "exceeded max value of 255" + FileMsg +
                                      ", row: 1, column: 'Col', value: '1234'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetOptULong) {
  auto f{write({Col}, "Col\n123\n")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getOptULong(Col), 123);
  EXPECT_TRUE(f.nextRow());
  EXPECT_FALSE(f.getOptULong(Col));
}

TEST_F(ColumnFileTest, GetOptULongError) {
  auto f{write({Col}, "Col\nblah")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.getOptULong(Col); },
                   ConvertError + "unsigned long" + FileMsg +
                       ", row: 1, column: 'Col', value: 'blah'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetOptUInt) {
  auto f{write({Col}, "Col\n123\n")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getOptUInt<u_int16_t>(Col), 123);
  EXPECT_TRUE(f.nextRow());
  EXPECT_FALSE(f.getOptU16(Col));
}

TEST_F(ColumnFileTest, GetOptUIntError) {
  auto f{write({Col}, "Col\n256")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.getOptU8(Col); },
                   "exceeded max value of 255" + FileMsg +
                       ", row: 1, column: 'Col', value: '256'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetBool) {
  const ColumnFile::Column c1{"1"}, c2{"2"}, c3{"3"}, c4{"4"}, c5{"5"};
  auto f{write({c1, c2, c3, c4, c5}, "1\t2\t3\t4\t5\nY\tT\tN\tF\t")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_TRUE(f.getBool(c1));
  EXPECT_TRUE(f.getBool(c2));
  EXPECT_FALSE(f.getBool(c3));
  EXPECT_FALSE(f.getBool(c4));
  EXPECT_FALSE(f.getBool(c5));
}

TEST_F(ColumnFileTest, GetBoolError) {
  auto f{write({Col}, "Col\nx")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(
      call([&] { f.getBool(Col); }, ConvertError + "bool" + FileMsg +
                                        ", row: 1, column: 'Col', value: 'x'"),
      std::domain_error);
}

TEST_F(ColumnFileTest, GetWChar) {
  const ColumnFile::Column c1{"1"}, c2{"2"};
  auto f{write({c1, c2}, "1\t2\n898B\t20B9F")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getWChar(c1), 35211);
  EXPECT_EQ(f.getWChar(c2), 134047);
}

TEST_F(ColumnFileTest, GetWCharError) {
  auto f{write({Col}, "Col\nAAA\n123456\nABCd\nDEFG")};
  const auto _ = {
      "size must be 4 or 5" + FileMsg + ", row: 1, column: 'Col', value: 'AAA'",
      "size must be 4 or 5" + FileMsg +
          ", row: 2, column: 'Col', value: '123456'",
      "invalid hex" + FileMsg + ", row: 3, column: 'Col', value: 'ABCd'",
      "invalid hex" + FileMsg + ", row: 4, column: 'Col', value: 'DEFG'"};
  for (auto i : _) {
    f.nextRow();
    EXPECT_THROW(
        call([&] { f.getWChar(Col); }, ConvertError + "char32_t, " + i),
        std::domain_error);
  }
}

} // namespace kanji_tools
