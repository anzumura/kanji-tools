#include <gtest/gtest.h>
#include <kanji_tools/quiz/Group.h>
#include <kanji_tools/utils/Utils.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

const Radical TestRadical{1, EmptyString, {}, EmptyString, EmptyString};

class TestKanji : public Kanji {
public:
  TestKanji(const std::string& name)
      : Kanji{name, name, TestRadical, 0, {}, {}, EmptyString} {}

  KanjiTypes type() const override { return KanjiTypes::None; }
  const std::string& meaning() const override { return EmptyString; }
  const std::string& reading() const override { return EmptyString; }
};

const Data::List TestMembers{
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
    const std::string name{i == Peer ? ":z" : "x:y"};
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
  const auto f{[] { MeaningGroup{3, "empty", {}}; }};
  EXPECT_THROW(call(f, "group [3 empty] has no members"), std::domain_error);
}

TEST(GroupTest, GroupWithOneMember) {
  const auto f{[] { MeaningGroup{4, "one", {TestMembers[0]}}; }};
  EXPECT_THROW(call(f, "group [4 one] has only one member"), std::domain_error);
}

TEST(GroupTest, GroupWithTooManyMembers) {
  Data::List l;
  for (size_t i{}; i <= Group::MaxGroupSize; ++i) l.push_back(TestMembers[0]);
  const auto f{[&l] { MeaningGroup{5, "big", l}; }};
  EXPECT_THROW(
      call(f, "group [5 big] has more than 58 members"), std::domain_error);
}

TEST(GroupTest, GroupWithOneDuplicateMember) {
  const auto f{[] { MeaningGroup{6, "d", {TestMembers[0], TestMembers[0]}}; }};
  EXPECT_THROW(
      call(f, "group [6 d] has 1 duplicate member: 甲"), std::domain_error);
}

TEST(GroupTest, GroupWithMultipleDuplicateMembers) {
  Data::List l{TestMembers};
  l.insert(l.end(), TestMembers.begin(), TestMembers.end());
  const auto f{[&l] { MeaningGroup{7, "m", l}; }};
  EXPECT_THROW(
      call(f, "group [7 m] has 2 duplicate members: 甲 乙"), std::domain_error);
}

TEST(GroupTest, InvalidPatternGroup) {
  const auto f{[] {
    PatternGroup{1, "bad", TestMembers, Group::PatternType::None};
  }};
  EXPECT_THROW(
      call(f, "group [1 bad] has invalid pattern type"), std::domain_error);
}

} // namespace kanji_tools
