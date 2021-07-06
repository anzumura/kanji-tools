#include <gtest/gtest.h>

#include <kanji/KanjiData.h>
#include <kanji/Kanji.h>

#include <type_traits>

namespace kanji {

namespace fs = std::filesystem;

class KanjiDataTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "../../data";
    static const char* args[] = { arg0, arg1 };
    return args;
  }
  // Contructs KanjiData using the real data files
  KanjiDataTest() : _data(2, argv()) {}

  KanjiData _data;
};

TEST_F(KanjiDataTest, SanityChecks) {
  // basic checks
  EXPECT_EQ(_data.getLevel("院"), Levels::N4);
  EXPECT_EQ(_data.getFrequency("蝦"), 2501);
  EXPECT_EQ(_data.getStrokes("廳"), 25);
  // radical
  auto radical = _data.getRadical("鹿");
  EXPECT_EQ(radical.number(), 198);
  EXPECT_EQ(radical.name(), "鹿");
  EXPECT_EQ(radical.longName(), "鹿部（ろくぶ）");
  EXPECT_EQ(radical.reading(), "しか");
  // find
  auto result = _data.findKanji("響");
  ASSERT_TRUE(result.has_value());
  auto& k = **result;
  EXPECT_EQ(k.type(), Types::LinkedOld);
  EXPECT_EQ(k.name(), "響");
  EXPECT_EQ(k.level(), Levels::None);
  EXPECT_EQ(k.grade(), Grades::None);
  EXPECT_EQ(k.frequency(), 0);
  // totals
  EXPECT_EQ(_data.gradeTotal(Grades::G1), 80);
  EXPECT_EQ(_data.gradeTotal(Grades::G2), 160);
  EXPECT_EQ(_data.gradeTotal(Grades::G3), 200);
  EXPECT_EQ(_data.gradeTotal(Grades::G4), 200);
  EXPECT_EQ(_data.gradeTotal(Grades::G5), 185);
  EXPECT_EQ(_data.gradeTotal(Grades::G6), 181);
  EXPECT_EQ(_data.gradeTotal(Grades::S), 1130);
  EXPECT_EQ(_data.gradeTotal(Grades::None), 0);
  EXPECT_EQ(_data.levelTotal(Levels::N5), 103);
  EXPECT_EQ(_data.levelTotal(Levels::N4), 181);
  EXPECT_EQ(_data.levelTotal(Levels::N3), 361);
  EXPECT_EQ(_data.levelTotal(Levels::N2), 415);
  EXPECT_EQ(_data.levelTotal(Levels::N1), 1162);
  EXPECT_EQ(_data.levelTotal(Levels::None), 0);
  EXPECT_EQ(_data.frequencyTotal(-1), 0);
  EXPECT_EQ(_data.frequencyTotal(0), 500);
  EXPECT_EQ(_data.frequencyTotal(1), 500);
  EXPECT_EQ(_data.frequencyTotal(2), 500);
  EXPECT_EQ(_data.frequencyTotal(3), 500);
  EXPECT_EQ(_data.frequencyTotal(4), 501);
  EXPECT_EQ(_data.frequencyTotal(5), 0);
}

} // namespace kanji
