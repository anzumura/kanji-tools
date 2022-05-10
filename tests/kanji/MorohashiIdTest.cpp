#include <gtest/gtest.h>
#include <kanji_tools/kanji/MorohashiId.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

namespace {

[[nodiscard]] String error(const String& id, const String& msg) {
  return "Morohashi ID '" + id + "' " + msg;
}

} // namespace

TEST(MorohashiIdTest, EmptyId) {
  const MorohashiId id{};
  EXPECT_EQ(id.id(), 0);
  EXPECT_EQ(id.idType(), MorohashiId::IdType::Regular);
  EXPECT_FALSE(id);
  EXPECT_EQ(id.toString(), "");
}

TEST(MorohashiIdTest, IdFromEmptyString) {
  const MorohashiId id{""};
  EXPECT_EQ(id.id(), 0);
  EXPECT_EQ(id.idType(), MorohashiId::IdType::Regular);
  EXPECT_FALSE(id);
  EXPECT_EQ(id.toString(), "");
}

TEST(MorohashiIdTest, StripLeadingZeroes) {
  const MorohashiId id{"00000"};
  EXPECT_EQ(id.id(), 0);
  EXPECT_EQ(id.idType(), MorohashiId::IdType::Regular);
  EXPECT_FALSE(id);
  EXPECT_EQ(id.toString(), "");
  const MorohashiId id1{"0001"};
  EXPECT_EQ(id1.id(), 1);
  EXPECT_EQ(id1.idType(), MorohashiId::IdType::Regular);
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
  for (auto i : {"PP", "''", "P", "'", "H"})
    EXPECT_THROW(call([i] { MorohashiId{i}; }, error(i, "is invalid")),
        std::domain_error);
}

TEST(MorohashiIdTest, BadTypedZeroIds) {
  for (auto i : {"0PP", "00''", "00P", "0'", "H0"})
    EXPECT_THROW(call([i] { MorohashiId{i}; }, error(i, "can't be zero")),
        std::domain_error);
}

TEST(MorohashiIdTest, NumericString) {
  for (MorohashiId::Id i{1}; i < MorohashiId::MaxId; ++i)
    ASSERT_EQ(MorohashiId{std::to_string(i)}.id(), i);
}

TEST(MorohashiIdTest, NonDigit) {
  for (auto i : {"x", "a7", "22D4", "123f"})
    EXPECT_THROW(call([i] { MorohashiId{i}; }, error(i, "is non-numeric")),
        std::domain_error);
}

TEST(MorohashiIdTest, MaxIds) {
  const String max{std::to_string(MorohashiId::MaxId)};
  const MorohashiId id{max}, idPrime{max + "P"}, idDPrime{max + "PP"},
      idSupplemental{"H" + max};
  EXPECT_EQ(id.id(), MorohashiId::MaxId);
  EXPECT_EQ(idPrime.id(), MorohashiId::MaxId);
  EXPECT_EQ(idDPrime.id(), MorohashiId::MaxId);
  EXPECT_EQ(idSupplemental.id(), MorohashiId::MaxId);
  EXPECT_EQ(id.idType(), MorohashiId::IdType::Regular);
  EXPECT_EQ(idPrime.idType(), MorohashiId::IdType::Prime);
  EXPECT_EQ(idDPrime.idType(), MorohashiId::IdType::DoublePrime);
  EXPECT_EQ(idSupplemental.idType(), MorohashiId::IdType::Supplemental);
}

TEST(MorohashiIdTest, TooBig) {
  const size_t big{MorohashiId::MaxId + 1};
  for (auto i : {big, big + 10, big + 100, big * 2}) {
    const String id{std::to_string(i)};
    for (const auto& j : {"H" + id, id + "P", id + "PP", id + "'", id + "''"})
      EXPECT_THROW(call([j] { MorohashiId{j}; }, error(j, "exceeds max")),
          std::domain_error);
  }
}

TEST(MorohashiIdTest, StreamOperator) {
  const MorohashiId id{"123"}, idP{"045'"}, idH{"H067"}, idPP{"089PP"};
  std::stringstream s;
  s << id << ' ' << idP << ' ' << idH << ' ' << idPP;
  EXPECT_EQ(s.str(), "123 45P H67 89PP");
}

TEST(MorohashiIdTest, Equals) {
  const MorohashiId id{"123"}, diff1{"124"}, diff2{"123P"}, same{"123"};
  EXPECT_NE(id, diff1);
  EXPECT_NE(id, diff2);
  EXPECT_EQ(id, same);
}

TEST(MorohashiIdTest, Compare) {
  // sort by 'id' number, then 'idType'
  const MorohashiId id1{"1"}, id1P{"1P"}, id1PP{"1PP"}, id1H{"H1"}, id2{"2"};
  EXPECT_LT(id1, id1P);
  EXPECT_LE(id1, id1P);
  EXPECT_GE(id1PP, id1P);
  EXPECT_GT(id1PP, id1P);
  EXPECT_LT(id1PP, id1H);
  EXPECT_LT(id1H, id2);
}

} // namespace kanji_tools
