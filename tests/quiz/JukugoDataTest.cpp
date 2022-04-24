#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/JukugoData.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

class JukugoDataTest : public ::testing::Test {
protected:
  static JukugoData create() { return JukugoData{_data, &TestDir}; }

  static void SetUpTestCase() { _data = std::make_shared<KanjiData>(); }

  void SetUp() override {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
    static const std::string HeaderRow{"Number\tName\tMembers"};
    // make files for each grade ('g1' to 'g6')
    for (auto i{1}; i < AllKanjiGrades.size() - 1; ++i)
      write("g" + std::to_string(i) + ".txt", {});
    write(OtherFile, {});
  }

  void TearDown() override { fs::remove_all(TestDir); }

  void write(const std::string& f, const std::string& s) {
    std::ofstream of{TestDir / f, std::ios_base::app};
    of << s;
    of.close();
  }

  inline static std::stringstream _os, _es;
  inline static DataPtr _data;

  inline static const std::string Grade1File{"g1.txt"}, OtherFile{"other.txt"},
      Err{" - line: 1, file: "};
  inline static const fs::path TestDir{"testDir"};
};

} // namespace

TEST_F(JukugoDataTest, CreateJukugoFromGrade1File) {
  write(Grade1File, "青空 (あおぞら)");
  const auto jukugoData{create()};
  const auto& result{jukugoData.find("青")};
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0]->name(), "青空");
  EXPECT_EQ(result[0]->reading(), "あおぞら");
  EXPECT_EQ(result[0]->grade(), KanjiGrades::G1);
  EXPECT_EQ(result, jukugoData.find("空"));
}

TEST_F(JukugoDataTest, CreateJukugoFromGrade2File) {
  write("g2.txt", "合図 (あいず)");
  const auto jukugoData{create()};
  const auto& result{jukugoData.find("図")};
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0]->name(), "合図");
  EXPECT_EQ(result[0]->grade(), KanjiGrades::G2);
}

TEST_F(JukugoDataTest, CreateJukugoFromOtherFile) {
  write(OtherFile, "鶴 ... 千羽鶴(せんばづる) 丹頂鶴(たんちょうづる)");
  const auto jukugoData{create()};
  const auto& result{jukugoData.find("鶴")};
  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0]->name(), "千羽鶴");
  EXPECT_EQ(result[0]->reading(), "せんばづる");
  EXPECT_EQ(result[0]->grade(), KanjiGrades::S);
  EXPECT_EQ(result[1]->name(), "丹頂鶴");
  EXPECT_EQ(result[1]->reading(), "たんちょうづる");
  EXPECT_EQ(result[1]->grade(), KanjiGrades::S);
}

TEST_F(JukugoDataTest, GradeFileMissingOpenBracket) {
  write(Grade1File, "青空 あおぞら)");
  EXPECT_THROW(
      call([] { create(); }, "failed to find open bracket" + Err + Grade1File),
      std::domain_error);
}

TEST_F(JukugoDataTest, GradeFileMissingSpace) {
  write(Grade1File, "青空(あおぞら)");
  EXPECT_THROW(call([] { create(); },
                   "open bracket should follow a space" + Err + Grade1File),
      std::domain_error);
}

TEST_F(JukugoDataTest, GradeFileMissingCloseBracket) {
  write(Grade1File, "青空 (あおぞら");
  EXPECT_THROW(
      call([] { create(); }, "failed to find close bracket" + Err + Grade1File),
      std::domain_error);
}

TEST_F(JukugoDataTest, GradeFileCloseBracketNotLastCharacter) {
  write(Grade1File, "青空 (あおぞら) ");
  EXPECT_THROW(
      call([] { create(); },
          "close bracket should be the last character" + Err + Grade1File),
      std::domain_error);
}

TEST_F(JukugoDataTest, OtherFileMissingOpenBracket) {
  write(OtherFile, "鶴 ... 千羽鶴(せんばづる) 丹頂鶴 たんちょうづる)");
  EXPECT_THROW(
      call([] { create(); }, "failed to find open bracket" + Err + OtherFile),
      std::domain_error);
}

TEST_F(JukugoDataTest, OtherFileMissingCloseBracket) {
  write(OtherFile, "鶴 ... 千羽鶴(せんばづる) 丹頂鶴(たんちょうづる");
  EXPECT_THROW(
      call([] { create(); }, "failed to find close bracket" + Err + OtherFile),
      std::domain_error);
}

TEST_F(JukugoDataTest, OtherFileMissingDots) {
  write(OtherFile, "鶴 .. 千羽鶴(せんばづる) 丹頂鶴(たんちょうづる");
  EXPECT_THROW(
      call([] { create(); }, "line is missing '...'" + Err + OtherFile),
      std::domain_error);
}

TEST_F(JukugoDataTest, IgnoreDuplicateInSameFile) {
  write(Grade1File, "青白 (あおじろ)\n青空 (あおぞら)\n青白 (あおじろ)");
  const auto jukugoData{create()};
  const auto& result{jukugoData.find("青")};
  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0]->name(), "青白");
  EXPECT_EQ(result[1]->name(), "青空");
}

TEST_F(JukugoDataTest, FailForDuplicateInDifferentFile) {
  write(Grade1File, "青白 (あおじろ)\n青空 (あおぞら)");
  write("g2.txt", "青白 (あおじろ)");
  EXPECT_THROW(
      call([] { create(); },
          "jukugo '青白' found in more than one file" + Err + "g2.txt"),
      std::domain_error);
}

} // namespace kanji_tools
