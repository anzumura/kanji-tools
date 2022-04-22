#include <gtest/gtest.h>
#include <kanji_tools/kanji/MorohashiId.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

TEST(MorohashiIdTest, EmptyId) {
  const MorohashiId id{};
  EXPECT_EQ(id.id(), 0);
  EXPECT_EQ(id.idType(), MorohashiId::IdType::None);
  EXPECT_FALSE(id);
  EXPECT_EQ(id.toString(), "");
}

TEST(MorohashiIdTest, IdFromEmptyString) {
  const MorohashiId id{""};
  EXPECT_EQ(id.id(), 0);
  EXPECT_EQ(id.idType(), MorohashiId::IdType::None);
  EXPECT_FALSE(id);
  EXPECT_EQ(id.toString(), "");
}

TEST(MorohashiIdTest, StripLeadingZeroes) {
  const MorohashiId id{"00000"};
  EXPECT_EQ(id.id(), 0);
  EXPECT_EQ(id.idType(), MorohashiId::IdType::None);
  EXPECT_FALSE(id);
  EXPECT_EQ(id.toString(), "");
  const MorohashiId id1{"0001"};
  EXPECT_EQ(id1.id(), 1);
  EXPECT_EQ(id1.idType(), MorohashiId::IdType::None);
  EXPECT_TRUE(id1);
  EXPECT_EQ(id1.toString(), "1");
}

TEST(MorohashiIdTest, PrimeId) {
  for (auto i : {"3P", "3'", "003P", "003'"}) {
    const MorohashiId id(i);
    EXPECT_EQ(id.id(), 3);
    EXPECT_EQ(id.idType(), MorohashiId::IdType::Prime);
    EXPECT_EQ(id.toString(), "3P");
  }
}

TEST(MorohashiIdTest, DoublePrimeId) {
  for (auto i : {"7PP", "7''", "007PP", "007''"}) {
    const MorohashiId id(i);
    EXPECT_EQ(id.id(), 7);
    EXPECT_EQ(id.idType(), MorohashiId::IdType::DoublePrime);
    EXPECT_EQ(id.toString(), "7PP");
  }
}

TEST(MorohashiIdTest, SupplementalId) {
  for (auto i : {"H10", "H0010"}) {
    const MorohashiId id(i);
    EXPECT_EQ(id.id(), 10);
    EXPECT_EQ(id.idType(), MorohashiId::IdType::Supplemental);
    EXPECT_EQ(id.toString(), "H10");
  }
}

TEST(MorohashiIdTest, BadEmptyTypedIds) {
  const std::string msg{"invalid Id: "};
  for (auto i : {"PP", "''", "P", "'", "H"})
    EXPECT_THROW(call([i] { MorohashiId{i}; }, msg + i), std::domain_error);
}

TEST(MorohashiIdTest, BadTypedZeroIds) {
  const std::string msg{"typed Id can't be zero: "};
  for (auto i : {"0PP", "00''", "00P", "0'", "H0"})
    EXPECT_THROW(call([i] { MorohashiId{i}; }, msg + i), std::domain_error);
}

TEST(MorohashiIdTest, NumericString) {
  for (MorohashiId::Id i{1}; i < MorohashiId::MaxId; ++i) {
    const std::string s{std::to_string(i)};
    const MorohashiId id{s};
    ASSERT_EQ(id.id(), i);
    ASSERT_EQ(id.idType(), MorohashiId::IdType::None);
    ASSERT_TRUE(id);
    ASSERT_EQ(id.toString(), s);
  }
}

TEST(MorohashiIdTest, NonDigit) {
  const std::string msg{"non-numeric Id: "};
  for (auto i : {"x", "a7", "22D4", "123f"})
    EXPECT_THROW(call([i] { MorohashiId{i}; }, msg + i), std::domain_error);
}

TEST(MorohashiIdTest, MaxIds) {
  const std::string max{std::to_string(MorohashiId::MaxId)};
  const MorohashiId id{max}, idPrime{max + "P"}, idDPrime{max + "PP"},
      idSupplemental{"H" + max};
  EXPECT_EQ(id.id(), MorohashiId::MaxId);
  EXPECT_EQ(idPrime.id(), MorohashiId::MaxId);
  EXPECT_EQ(idDPrime.id(), MorohashiId::MaxId);
  EXPECT_EQ(idSupplemental.id(), MorohashiId::MaxId);
  EXPECT_EQ(id.idType(), MorohashiId::IdType::None);
  EXPECT_EQ(idPrime.idType(), MorohashiId::IdType::Prime);
  EXPECT_EQ(idDPrime.idType(), MorohashiId::IdType::DoublePrime);
  EXPECT_EQ(idSupplemental.idType(), MorohashiId::IdType::Supplemental);
}

TEST(MorohashiIdTest, TooBig) {
  const size_t big{MorohashiId::MaxId + 1};
  const std::string msg{"Id exceeds max: "};
  for (auto i : {big, big + 10, big + 100, big * 2}) {
    const std::string id{std::to_string(i)};
    for (auto j : {"H" + id, id + "P", id + "PP", id + "'", id + "''"})
      EXPECT_THROW(call([j] { MorohashiId{j}; }, msg + j), std::domain_error);
  }
}

} // namespace kanji_tools
