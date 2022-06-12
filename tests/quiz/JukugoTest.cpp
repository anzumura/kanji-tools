#include <gtest/gtest.h>
#include <kt_quiz/Jukugo.h>
#include <kt_tests/WhatMismatch.h>

namespace kanji_tools {

TEST(JukugoTest, CreateJukugo) {
  const Jukugo j{"漢字", "かんじ", KanjiGrades::G3};
  EXPECT_EQ(j.name(), "漢字");
  EXPECT_EQ(j.reading(), "かんじ");
  EXPECT_EQ(j.grade(), KanjiGrades::G3);
}

TEST(JukugoTest, AllowProlongMarkInReading) {
  const Jukugo j{"珈琲", "こーひー", KanjiGrades::S};
  EXPECT_EQ(j.nameAndReading(), "珈琲（こーひー）");
}

TEST(JukugoTest, NoKanji) {
  const auto f{[] { Jukugo{"ゆき", "ゆき", KanjiGrades::G2}; }};
  EXPECT_THROW(call(f, "jukugo 'ゆき' contains no Kanji"), DomainError);
}

TEST(JukugoTest, SingleKanji) {
  const auto f{[] { Jukugo{"ね雪", "ゆき", KanjiGrades::G2}; }};
  EXPECT_THROW(
      call(f, "jukugo 'ね雪' must contain two or more Kanji"), DomainError);
}

TEST(JukugoTest, BadReading) {
  const auto f{[] { Jukugo{"根雪", "ネユキ", KanjiGrades::G2}; }};
  EXPECT_THROW(
      call(f, "jukugo '根雪' reading must be all Hiragana"), DomainError);
}

} // namespace kanji_tools
