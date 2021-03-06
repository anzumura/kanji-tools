#include <gtest/gtest.h>
#include <kt_quiz/Group.h>
#include <kt_tests/TestKanji.h>
#include <kt_tests/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

const KanjiData::List TestMembers{
    std::make_shared<TestKanji>("甲"), std::make_shared<TestKanji>("乙")};

} // namespace

TEST(GroupTest, CreateMeaningGroup) {
  const MeaningGroup g{1, "mg", TestMembers};
  EXPECT_EQ(g.type(), GroupType::Meaning);
  EXPECT_EQ(g.patternType(), Group::PatternType::None);
  EXPECT_EQ(g.number(), 1);
  EXPECT_EQ(g.name(), "mg");
  EXPECT_EQ(g.members(), TestMembers);
  EXPECT_EQ(g.toString(), "[1 mg]");
  std::stringstream s;
  s << g;
  EXPECT_EQ(s.str(), "[mg]");
}

TEST(GroupTest, CreatePatternGroup) {
  using enum Group::PatternType;
  for (const auto i : {Family, Peer, Reading}) {
    const String name{i == Peer ? ":z" : "x:y"};
    const PatternGroup g{2, name, TestMembers, i};
    EXPECT_EQ(g.type(), GroupType::Pattern);
    EXPECT_EQ(g.patternType(), i);
    EXPECT_EQ(g.number(), 2);
    EXPECT_EQ(g.name(), name);
    EXPECT_EQ(g.members(), TestMembers);
    EXPECT_EQ(g.toString(), "[2 " + name + "]");
    std::stringstream s;
    s << g;
    EXPECT_EQ(s.str(), i == Peer ? "[Peers 甲:z]" : "[x:y]");
  }
}

TEST(GroupTest, GroupWithNoMembers) {
  const auto f{[] { MeaningGroup{{}, "empty", {}}; }};
  EXPECT_THROW(call(f, "group [0 empty] has no members"), DomainError);
}

TEST(GroupTest, GroupWithOneMember) {
  const auto f{[] { MeaningGroup{{}, "one", {TestMembers[0]}}; }};
  EXPECT_THROW(call(f, "group [0 one] has only one member"), DomainError);
}

TEST(GroupTest, GroupWithTooManyMembers) {
  KanjiData::List l;
  for (size_t i{}; i <= Group::MaxGroupSize; ++i) l.push_back(TestMembers[0]);
  const auto f{[&l] { MeaningGroup{{}, "big", l}; }};
  EXPECT_THROW(call(f, "group [0 big] has more than 58 members"), DomainError);
}

TEST(GroupTest, GroupWithOneDuplicateMember) {
  const auto f{[] { MeaningGroup{{}, "d", {TestMembers[0], TestMembers[0]}}; }};
  EXPECT_THROW(call(f, "group [0 d] has 1 duplicate member: 甲"), DomainError);
}

TEST(GroupTest, GroupWithMultipleDuplicateMembers) {
  KanjiData::List l{TestMembers};
  l.insert(l.end(), TestMembers.begin(), TestMembers.end());
  const auto f{[&l] { MeaningGroup{{}, "m", l}; }};
  EXPECT_THROW(
      call(f, "group [0 m] has 2 duplicate members: 甲 乙"), DomainError);
}

TEST(GroupTest, InvalidPatternGroup) {
  const auto f{[] {
    PatternGroup{{}, "bad", TestMembers, Group::PatternType::None};
  }};
  EXPECT_THROW(call(f, "group [0 bad] has invalid pattern type"), DomainError);
}

} // namespace kanji_tools
