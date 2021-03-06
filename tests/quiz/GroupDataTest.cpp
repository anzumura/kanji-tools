#include <gtest/gtest.h>
#include <kt_kanji/TextKanjiData.h>
#include <kt_quiz/GroupData.h>
#include <kt_tests/WhatMismatch.h>

#include <fstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

class GroupDataTest : public ::testing::Test {
protected:
  static GroupData create() { return GroupData{_data, &TestDir}; }

  static void SetUpTestSuite() {
    _data = std::make_shared<TextKanjiData>(Args{}, _os, _es);
  }

  void SetUp() final {
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
    static const String HeaderRow{"Number\tName\tMembers"};
    write(MeaningFile, HeaderRow);
    write(PatternFile, HeaderRow);
    _es.str({});
    _es.clear();
  }

  void TearDown() final { fs::remove_all(TestDir); }

  static void write(const fs::path& f, const String& s) {
    std::ofstream of{f, std::ios_base::app};
    of << s;
    of.close();
  }

  inline static std::stringstream _os, _es;
  inline static KanjiDataPtr _data;

  inline static const String MeaningErr{" - file: meaning-groups, row: 1"};
  inline static const fs::path TestDir{"testDir"};
  inline static const fs::path MeaningFile{TestDir / "meaning-groups"},
      PatternFile{TestDir / "pattern-groups"};
};

} // namespace

TEST_F(GroupDataTest, SanityChecks) {
  // Constructs GroupData using the real '-groups.txt' data files
  GroupData groupData(_data);
  EXPECT_FALSE(groupData.meaningGroups().empty());
  EXPECT_FALSE(groupData.patternGroups().empty());
  // numbers are unique and each group member is in 'groupMap'
  const auto checkNumber{[](const GroupData::List& list, const auto& groupMap) {
    std::set<size_t> uniqueNumbers;
    for (const auto& i : list) {
      EXPECT_TRUE(uniqueNumbers.insert(i->number()).second)
          << i->name() << " has duplicate number " << i->number();
      for (const auto& j : i->members())
        EXPECT_TRUE(groupMap.contains(j->name()))
            << j->name() << "from group " << i->name() << " missing from map";
    }
  }};
  checkNumber(groupData.meaningGroups(), groupData.meaningMap());
  checkNumber(groupData.patternGroups(), groupData.patternMap());
}

TEST_F(GroupDataTest, MeaningGroup) {
  write(MeaningFile, "\n1\t???????????????\t???,???,???,???,???,???,???");
  const auto groupData{create()};
  ASSERT_TRUE(groupData.patternGroups().empty());
  ASSERT_TRUE(groupData.patternMap().empty());
  ASSERT_EQ(groupData.meaningGroups().size(), 1);
  ASSERT_EQ(groupData.meaningMap().size(), 7);
  const auto& g{**groupData.meaningGroups().begin()};
  EXPECT_EQ(g.type(), GroupType::Meaning);
  EXPECT_EQ(g.patternType(), Group::PatternType::None);
  EXPECT_EQ(g.number(), 1);
  EXPECT_EQ(g.name(), "???????????????");
  String days;
  for (auto& i : g.members()) days += i->name();
  EXPECT_EQ(days, "?????????????????????");
}

TEST_F(GroupDataTest, FamilyPatternGroup) {
  write(PatternFile, "\n1\t??????????????????????????????\t???,???");
  const auto groupData{create()};
  ASSERT_TRUE(groupData.meaningGroups().empty());
  ASSERT_TRUE(groupData.meaningMap().empty());
  ASSERT_EQ(groupData.patternGroups().size(), 1);
  ASSERT_EQ(groupData.patternMap().size(), 3);
  const auto& g{**groupData.patternGroups().begin()};
  EXPECT_EQ(g.type(), GroupType::Pattern);
  EXPECT_EQ(g.patternType(), Group::PatternType::Family);
  EXPECT_EQ(g.number(), 1);
  EXPECT_EQ(g.name(), "??????????????????????????????");
  const auto& m{g.members()};
  ASSERT_EQ(m.size(), 3);
  EXPECT_EQ(m[0]->name(), "???");
  EXPECT_EQ(m[1]->name(), "???");
  EXPECT_EQ(m[2]->name(), "???");
}

TEST_F(GroupDataTest, PeerPatternGroup) {
  write(PatternFile, "\n1\t??????????????????\t???,???,???");
  const auto groupData{create()};
  ASSERT_TRUE(groupData.meaningGroups().empty());
  ASSERT_TRUE(groupData.meaningMap().empty());
  ASSERT_EQ(groupData.patternGroups().size(), 1);
  ASSERT_EQ(groupData.patternMap().size(), 3);
  const auto& g{**groupData.patternGroups().begin()};
  EXPECT_EQ(g.type(), GroupType::Pattern);
  EXPECT_EQ(g.patternType(), Group::PatternType::Peer);
  EXPECT_EQ(g.number(), 1);
  EXPECT_EQ(g.name(), "??????????????????");
  const auto& m{g.members()};
  ASSERT_EQ(m.size(), 3);
  EXPECT_EQ(m[0]->name(), "???");
  EXPECT_EQ(m[1]->name(), "???");
  EXPECT_EQ(m[2]->name(), "???");
}

TEST_F(GroupDataTest, ReadingPatternGroup) {
  write(PatternFile, "\n1\t?????????\t???,???");
  const auto groupData{create()};
  ASSERT_TRUE(groupData.meaningGroups().empty());
  ASSERT_TRUE(groupData.meaningMap().empty());
  ASSERT_EQ(groupData.patternGroups().size(), 1);
  ASSERT_EQ(groupData.patternMap().size(), 2);
  const auto& g{**groupData.patternGroups().begin()};
  EXPECT_EQ(g.type(), GroupType::Pattern);
  EXPECT_EQ(g.patternType(), Group::PatternType::Reading);
  EXPECT_EQ(g.number(), 1);
  EXPECT_EQ(g.name(), "?????????");
  const auto& m{g.members()};
  ASSERT_EQ(m.size(), 2);
  EXPECT_EQ(m[0]->name(), "???");
  EXPECT_EQ(m[1]->name(), "???");
}

TEST_F(GroupDataTest, ReadMultipleRows) {
  write(PatternFile, "\n1\t??????????????????????????????\t???,???\n2\t????????????\t???");
  const auto groupData{create()};
  ASSERT_EQ(groupData.patternGroups().size(), 2);
  ASSERT_EQ(groupData.patternMap().size(), 5);
  auto i{groupData.patternGroups().begin()};
  EXPECT_EQ((*i)->members().size(), 3);
  EXPECT_EQ((*++i)->members().size(), 2);
}

TEST_F(GroupDataTest, SameKanjiInMultipleMeaningGroups) {
  write(MeaningFile, "\n1\t???\t???,???,???\n2\t???????????????\t???,???,???,???,???,???,???");
  const auto groupData{create()};
  EXPECT_EQ(groupData.meaningGroups().size(), 2);
  EXPECT_EQ(groupData.meaningMap().size(), 10);
  EXPECT_EQ(_es.str(), ""); // no errors
}

TEST_F(GroupDataTest, SameKanjiInMultiplePatternGroups) {
  // put ??? in both groups (which isn't correct - it should only be in group 2)
  write(PatternFile, "\n1\t??????????????????????????????\t???,???,???\n2\t????????????\t???");
  const auto groupData{create()};
  EXPECT_EQ(groupData.patternGroups().size(), 2);
  EXPECT_EQ(groupData.patternMap().size(), 5);
  EXPECT_EQ(groupData.patternMap().at("???")->number(), 1);
  // data is loaded an no exceptions are thrown, but patternMap entry for the
  // duplicate points at the first group loaded and an error message is written
  // to stderr to help clean (this was helpful when creating the file to allow
  // fixing multiple problems at a time instead of failing on the first error)
  EXPECT_TRUE(_es.str().ends_with(
      "??? from [2 ????????????] already in [1 ??????????????????????????????]\n"));
}

TEST_F(GroupDataTest, UnknownKanji) {
  write(MeaningFile, "\n1\t??????\t???,???,???");
  // a Kanji that hasn't been loaded from any data files (so ultimately not in
  // 'data/ucd.txt') causes both an exception as well as output to stderr
  EXPECT_THROW(
      call([] { create(); }, "group failed to load all members" + MeaningErr),
      DomainError);
  EXPECT_TRUE(_es.str().ends_with(
      "failed to find member ??? in group: '??????', number: 1\n"));
}

TEST_F(GroupDataTest, CreateGroupError) {
  write(MeaningFile, "\n1\t??????\t???,???,???");
  EXPECT_THROW(call([] { create(); },
                   "group [1 ??????] has 1 duplicate member: ???" + MeaningErr),
      DomainError);
  EXPECT_EQ(_es.str(), "");
}

} // namespace kanji_tools
