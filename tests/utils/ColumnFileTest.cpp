#include <gtest/gtest.h>

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
  void SetUp() override {
    if (fs::exists(_testDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(_testDir));
  }
  void TearDown() override { fs::remove_all(_testDir); }
  fs::path _testDir = "testDir";
  fs::path _testFile = _testDir / "testFile.txt";
};

TEST_F(ColumnFileTest, SingleColumnFile) {
  std::ofstream of(_testFile);
  of << "Col\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(_testFile, {col});
  EXPECT_EQ(f.name(), "testFile.txt");
  EXPECT_EQ(f.columns(), 1);
  EXPECT_EQ(f.currentRow(), 0);
}

TEST_F(ColumnFileTest, GetValueFromOneColumn) {
  std::ofstream of(_testFile);
  of << "Col\nVal\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(_testFile, {col});
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.currentRow(), 1);
  EXPECT_EQ(f.get(col), "Val");
  EXPECT_FALSE(f.nextRow());
  EXPECT_EQ(f.currentRow(), 1);
}

TEST_F(ColumnFileTest, GetValueFromMultipleColumns) {
  std::ofstream of(_testFile);
  of << "Col1\tCol2\tCol3\nVal1\tVal2\tVal3\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(_testFile, {col1, col2, col3});
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(col1), "Val1");
  EXPECT_EQ(f.get(col2), "Val2");
  EXPECT_EQ(f.get(col3), "Val3");
}

TEST_F(ColumnFileTest, HeaderColumnOrderDifferentThanConstructor) {
  std::ofstream of(_testFile);
  of << "Col1\tCol2\tCol3\nVal1\tVal2\tVal3\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(_testFile, {col3, col2, col1});
  ASSERT_TRUE(f.nextRow());
  EXPECT_EQ(f.get(col1), "Val1");
  EXPECT_EQ(f.get(col2), "Val2");
  EXPECT_EQ(f.get(col3), "Val3");
}

TEST_F(ColumnFileTest, GetMultipleRows) {
  std::ofstream of(_testFile);
  of << "Col1\tCol2\tCol3\nR11\tR12\tR13\nR21\tR22\tR23\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(_testFile, {col1, col2, col3});
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
  std::ofstream of(_testFile);
  of << "Col1\tCol2\tCol3\nVal1\tVal2\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(_testFile, {col1, col2, col3});
  try {
    f.nextRow();
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("not enough columns - file: testFile.txt, row: 1"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, TooManyColumns) {
  std::ofstream of(_testFile);
  of << "Col1\tCol2\tCol3\nVal1\tVal2\tVal3\tVal4\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3");
  ColumnFile f(_testFile, {col1, col2, col3});
  try {
    f.nextRow();
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("too many columns - file: testFile.txt, row: 1"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, UnrecognizedHeaderError) {
  std::ofstream of(_testFile);
  of << "HeaderName\n";
  of.close();
  ColumnFile::Column col("ColumnName");
  try {
    ColumnFile f(_testFile, {col});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("unrecognized header 'HeaderName' - file: testFile.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, DuplicateColumnError) {
  std::ofstream of(_testFile);
  of << "HeaderName\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2");
  try {
    ColumnFile f(_testFile, {col1, col2, col1});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("duplicate column 'Col1' - file: testFile.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, OneMissingColumnError) {
  std::ofstream of(_testFile);
  of << "Col1\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2");
  try {
    ColumnFile f(_testFile, {col1, col2});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("column 'Col2' not found - file: testFile.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, MultipleMissingColumnsError) {
  std::ofstream of(_testFile);
  of << "Col1\tCol3\n";
  of.close();
  ColumnFile::Column col1("Col1"), col2("Col2"), col3("Col3"), col4("Col4");
  try {
    ColumnFile f(_testFile, {col1, col2, col3, col4});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("2 columns not found: 'Col2', 'Col4' - file: testFile.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, MissingFileError) {
  ColumnFile::Column col("Col");
  try {
    ColumnFile f(_testFile, {col});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("doesn't exist - file: testFile.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, NotRegularFileError) {
  ColumnFile::Column col("Col");
  try {
    ColumnFile f(_testDir, {col});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("not regular file - file: testDir"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, MissingHeaderRowError) {
  std::ofstream of(_testFile);
  of << "";
  of.close();
  ColumnFile::Column col("Col");
  try {
    ColumnFile f(_testFile, {col});
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("missing header row - file: testFile.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, GetBeforeNextRowError) {
  std::ofstream of(_testFile);
  of << "Col\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(_testFile, {col});
  try {
    f.get(col);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("'nextRow' must be called before calling 'get' - file: testFile.txt"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, GetUnrecognizedColumError) {
  std::ofstream of(_testFile);
  of << "Col\nVal\n";
  of.close();
  ColumnFile::Column col("Col");
  ColumnFile f(_testFile, {col});
  f.nextRow();
  ColumnFile::Column columnCreatedAfterConstruction("Created After");
  try {
    f.get(columnCreatedAfterConstruction);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("unrecognized column 'Created After' - file: testFile.txt, row: 1"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

TEST_F(ColumnFileTest, GetInvalidColumError) {
  std::ofstream of(_testFile);
  of << "Col\nVal\n";
  of.close();
  ColumnFile::Column col("Col"), columnNotIncludedInFile("Not Included");
  ColumnFile f(_testFile, {col});
  f.nextRow();
  try {
    f.get(columnNotIncludedInFile);
    FAIL() << "Expected std::domain_error";
  } catch (std::domain_error& err) {
    EXPECT_EQ(err.what(), std::string("invalid column 'Not Included' - file: testFile.txt, row: 1"));
  } catch (...) {
    FAIL() << "Expected std::domain_error";
  }
}

} // namespace kanji_tools
