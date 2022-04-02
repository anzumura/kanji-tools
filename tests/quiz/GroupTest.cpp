#include <gtest/gtest.h>
#include <kanji_tools/quiz/Group.h>
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

TEST(GroupTest, InvalidPatternGroup) {
  const auto f{[] {
    PatternGroup{1, "bad", TestMembers, Group::PatternType::None};
  }};
  EXPECT_THROW(call(f, "PatternGroup: 'bad' has invalid pattern type"),
      std::domain_error);
}

} // namespace kanji_tools
