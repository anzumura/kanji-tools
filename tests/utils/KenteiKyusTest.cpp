#include <gtest/gtest.h>
#include <kanji_tools/utils/KenteiKyus.h>

namespace kanji_tools {

TEST(KenteiKyusTest, CheckStrings) {
  EXPECT_EQ(toString(KenteiKyus::K10), "K10");
  EXPECT_EQ(toString(KenteiKyus::K9), "K9");
  EXPECT_EQ(toString(KenteiKyus::K8), "K8");
  EXPECT_EQ(toString(KenteiKyus::K7), "K7");
  EXPECT_EQ(toString(KenteiKyus::K6), "K6");
  EXPECT_EQ(toString(KenteiKyus::K5), "K5");
  EXPECT_EQ(toString(KenteiKyus::K4), "K4");
  EXPECT_EQ(toString(KenteiKyus::K3), "K3");
  EXPECT_EQ(toString(KenteiKyus::KJ2), "KJ2");
  EXPECT_EQ(toString(KenteiKyus::K2), "K2");
  EXPECT_EQ(toString(KenteiKyus::KJ1), "KJ1");
  EXPECT_EQ(toString(KenteiKyus::K1), "K1");
  EXPECT_EQ(toString(KenteiKyus::None), "None");
}

TEST(KenteiKyusTest, CheckValues) {
  size_t i{};
  EXPECT_EQ(AllKenteiKyus[i], KenteiKyus::K10);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K9);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K8);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K7);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K6);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K5);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K4);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K3);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::KJ2);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K2);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::KJ1);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::K1);
  EXPECT_EQ(AllKenteiKyus[++i], KenteiKyus::None);
}

} // namespace kanji_tools
