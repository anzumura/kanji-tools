#include <gtest/gtest.h>
#include <kt_tests/WhatMismatch.h>
#include <kt_utils/ColumnFile.h>
#include <kt_utils/Exception.h>

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

namespace {

class ColumnFileTest : public ::testing::Test {
protected:
  inline static const ColumnFile::Column Col{"Col"}, Col1{"Col1"}, Col2{"Col2"},
      Col3{"Col3"}, Col4{"Col4"};
  inline static const String FileMsg{" - file: testFile.txt"},
      ConvertError{"failed to convert to "};
  inline static const fs::path TestDir{"testDir"};
  inline static const fs::path TestFile{TestDir / "testFile.txt"};

  inline static auto create(const ColumnFile::Columns& c, char delim = '\t') {
    return ColumnFile{TestFile, c, delim};
  }

  void SetUp() final {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
  }

  void TearDown() final { fs::remove_all(TestDir); }

  static void write(const String& s, bool newLine = true) {
    std::ofstream of{TestFile};
    of << s;
    if (newLine) of << '\n';
    of.close();
  }

  static ColumnFile write(
      const ColumnFile::Columns& c, const String& s, bool newLine = true) {
    write(s, newLine);
    return create(c);
  }
};

} // namespace

TEST_F(ColumnFileTest, CreateColumns) {
  const ColumnFile::Column test1{"test1"};
  EXPECT_EQ(test1.name(), "test1");
  const ColumnFile::Column diff{"diff"}, same{"test1"};
  // 'test1' and 'diff' have different names and numbers and are 'not equal'
  EXPECT_NE(diff.name(), test1.name());
  EXPECT_NE(diff.number(), test1.number());
  EXPECT_NE(diff, test1);
  // 'test1' and 'same' have the same name and number and are 'equal'
  EXPECT_EQ(same.name(), test1.name());
  EXPECT_EQ(same.number(), test1.number());
  EXPECT_EQ(same, test1);
}

TEST_F(ColumnFileTest, SingleColumnFile) {
  const auto f{write({Col}, "Col")};
  EXPECT_EQ(f.fileName(), "testFile.txt");
  EXPECT_EQ(f.columns(), 1);
  EXPECT_EQ(f.currentRow(), 0);
}

TEST_F(ColumnFileTest, NoColumnsError) {
  EXPECT_THROW(
      call([] { create({}); }, "must specify at least one column" + FileMsg),
      DomainError);
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
      DomainError);
}

TEST_F(ColumnFileTest, TooManyColumns) {
  auto f{write({Col1, Col2, Col3}, "Col1\tCol2\tCol3\nVal1\tVal2\tVal3\tVal4")};
  EXPECT_THROW(
      call([&f] { f.nextRow(); }, "too many columns" + FileMsg + ", row: 1"),
      DomainError);
}

TEST_F(ColumnFileTest, UnrecognizedHeaderError) {
  const ColumnFile::Columns c{ColumnFile::Column{"B"}};
  EXPECT_THROW(
      call([&c] { write(c, "A"); }, "unrecognized header 'A'" + FileMsg),
      DomainError);
}

TEST_F(ColumnFileTest, DuplicateHeaderError) {
  write("Col\tCol");
  EXPECT_THROW(call([] { create({Col}); }, "duplicate header 'Col'" + FileMsg),
      DomainError);
}

TEST_F(ColumnFileTest, DuplicateColumnError) {
  write("");
  const auto f{[] { create({Col1, Col2, Col1}); }};
  EXPECT_THROW(call(f, "duplicate column 'Col1'" + FileMsg), DomainError);
}

TEST_F(ColumnFileTest, OneMissingColumnError) {
  write("Col1");
  const auto f{[] { create({Col1, Col2}); }};
  EXPECT_THROW(call(f, "column 'Col2' not found" + FileMsg), DomainError);
}

TEST_F(ColumnFileTest, MultipleMissingColumnsError) {
  write("Col1\tCol3");
  const auto f{[] { create({Col1, Col2, Col3, Col4}); }};
  EXPECT_THROW(
      call(f, "2 columns not found: 'Col2', 'Col4'" + FileMsg), DomainError);
}

TEST_F(ColumnFileTest, MissingFileError) {
  EXPECT_THROW(
      call([] { create({Col}); }, "doesn't exist" + FileMsg), DomainError);
}

TEST_F(ColumnFileTest, NotRegularFileError) {
  const auto f{[] { ColumnFile{TestDir, {Col}}; }};
  EXPECT_THROW(call(f, "not regular file - file: testDir"), DomainError);
}

TEST_F(ColumnFileTest, MissingHeaderRowError) {
  EXPECT_THROW(
      call([] { write({Col}, "", false); }, "missing header row" + FileMsg),
      DomainError);
}

TEST_F(ColumnFileTest, GetBeforeNextRowError) {
  const auto f{write({Col}, "Col")};
  EXPECT_THROW(call([&f] { f.get(Col); },
                   "'nextRow' must be called before calling 'get'" + FileMsg),
      DomainError);
}

TEST_F(ColumnFileTest, GetUnrecognizedColumError) {
  auto f{write({Col}, "Col\nVal")};
  EXPECT_TRUE(f.nextRow());
  const ColumnFile::Column columnCreatedAfterConstruction{"Created After"};
  EXPECT_THROW(
      call([&] { f.get(columnCreatedAfterConstruction); },
          "unrecognized column 'Created After'" + FileMsg + ", row: 1"),
      DomainError);
}

TEST_F(ColumnFileTest, GetInvalidColumError) {
  const ColumnFile::Column columnNotIncludedInFile{"Not Included"};
  auto f{write({Col}, "Col\nVal")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.get(columnNotIncludedInFile); },
                   "invalid column 'Not Included'" + FileMsg + ", row: 1"),
      DomainError);
}

TEST_F(ColumnFileTest, GetU64) {
  auto f{write({Col}, "Col\n123")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getU64(Col), 123);
}

TEST_F(ColumnFileTest, GetU64Error) {
  auto f{write({Col}, "Col\nblah")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.getU64(Col); },
                   ConvertError + "unsigned number" + FileMsg +
                       ", row: 1, column: 'Col', value: 'blah'"),
      DomainError);
}

TEST_F(ColumnFileTest, GetU64MaxValueError) {
  const auto maxValue{123U};
  std::ofstream of{TestFile};
  of << "Col\n" << maxValue << '\n' << maxValue << '\n' << maxValue + 1 << '\n';
  of.close();
  auto f{create({Col})};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getU64(Col, maxValue), maxValue);
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getU64(Col, 0), maxValue); // 0 implies no max value
  EXPECT_TRUE(f.nextRow());
  const String msg{"exceeded max value of "};
  EXPECT_THROW(call([&] { f.getU64(Col, maxValue); },
                   msg + std::to_string(maxValue) + FileMsg +
                       ", row: 3, column: 'Col', value: '" +
                       std::to_string(maxValue + 1) + "'"),
      DomainError);
}

TEST_F(ColumnFileTest, GetUInt) {
  constexpr uint8_t expected{123};
  auto f{write({Col}, "Col\n123")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getUInt<uint8_t>(Col), expected);
  EXPECT_EQ(f.getU8(Col), expected); // convenience function
}

TEST_F(ColumnFileTest, GetUIntError) {
  auto f{write({Col}, "Col\n1234")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(
      call([&] { f.getU8(Col); }, "exceeded max value of 255" + FileMsg +
                                      ", row: 1, column: 'Col', value: '1234'"),
      DomainError);
}

TEST_F(ColumnFileTest, GetOptU64) {
  auto f{write({Col}, "Col\n123\n")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getOptU64(Col), 123);
  EXPECT_TRUE(f.nextRow());
  EXPECT_FALSE(f.getOptU64(Col));
}

TEST_F(ColumnFileTest, GetOptU64Error) {
  auto f{write({Col}, "Col\nblah")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.getOptU64(Col); },
                   ConvertError + "unsigned number" + FileMsg +
                       ", row: 1, column: 'Col', value: 'blah'"),
      DomainError);
}

TEST_F(ColumnFileTest, GetOptUInt) {
  auto f{write({Col}, "Col\n123\n")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getOptUInt<uint16_t>(Col), 123);
  EXPECT_TRUE(f.nextRow());
  EXPECT_FALSE(f.getOptU16(Col));
}

TEST_F(ColumnFileTest, GetOptUIntError) {
  auto f{write({Col}, "Col\n256")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_THROW(call([&] { f.getOptU8(Col); },
                   "exceeded max value of 255" + FileMsg +
                       ", row: 1, column: 'Col', value: '256'"),
      DomainError);
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
      DomainError);
}

TEST_F(ColumnFileTest, GetWChar) {
  const ColumnFile::Column c1{"1"}, c2{"2"};
  auto f{write({c1, c2}, "1\t2\n898B\t20B9F")};
  EXPECT_TRUE(f.nextRow());
  EXPECT_EQ(f.getChar32(c1), 35211);
  EXPECT_EQ(f.getChar32(c2), 134047);
}

TEST_F(ColumnFileTest, GetWCharError) {
  auto f{write({Col}, "Col\nAAA\n123456\nABCd\nDEFG")};
  const auto _ = {
      "size must be 4 or 5" + FileMsg + ", row: 1, column: 'Col', value: 'AAA'",
      "size must be 4 or 5" + FileMsg +
          ", row: 2, column: 'Col', value: '123456'",
      "invalid hex" + FileMsg + ", row: 3, column: 'Col', value: 'ABCd'",
      "invalid hex" + FileMsg + ", row: 4, column: 'Col', value: 'DEFG'"};
  for (auto& i : _) {
    f.nextRow();
    EXPECT_THROW(call([&] { f.getChar32(Col); }, ConvertError + "Code, " += i),
        DomainError);
  }
}

} // namespace kanji_tools
