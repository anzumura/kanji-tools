#include <gtest/gtest.h>
#include <tests/kanji_tools/TestUcd.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

namespace {

// 'ptrCast' is used in Layout test
template<typename T>
[[nodiscard]] constexpr size_t ptrCast(const T& x) noexcept {
  return reinterpret_cast<size_t>(&x);
}

} // namespace

TEST(UcdTest, Size) {
  EXPECT_EQ(sizeof(bool), 1);
  EXPECT_EQ(sizeof(UcdLinkTypes), 1);
  EXPECT_EQ(sizeof(size_t), 8);
  EXPECT_EQ(sizeof(std::string*), 8);
  EXPECT_EQ(sizeof(Ucd::Links), 24);
#ifdef __clang__
  EXPECT_EQ(sizeof(Ucd), 224);
  EXPECT_EQ(sizeof(UcdEntry), 32);
  EXPECT_EQ(sizeof(std::string), 24);
#else
  EXPECT_EQ(sizeof(Ucd), 280);
  EXPECT_EQ(sizeof(UcdEntry), 40);
  EXPECT_EQ(sizeof(std::string), 32);
#endif
}

TEST(UcdTest, Layout) {
  const Ucd u{TestUcd{}};
  const auto start{ptrCast(u)};
  const auto entry{ptrCast(u.entry())};
  const auto block{ptrCast(u.block())};
  const auto version{ptrCast(u.version())};
  const auto pinyin{ptrCast(u.pinyin())};
  const auto links{ptrCast(u.links())};
  const auto morohashiId{ptrCast(u.morohashiId())};
  const auto nelsonIds{ptrCast(u.nelsonIds())};
  const auto jSource{ptrCast(u.jSource())};
  const auto meaning{ptrCast(u.meaning())};
  const auto onReading{ptrCast(u.onReading())};
  const auto kunReading{ptrCast(u.kunReading())};
#ifdef __clang__
  const size_t stringDiff{};
#else
  const size_t stringDiff{8}; // gcc string is 8 bytes bigger than clang
#endif
  EXPECT_EQ(start, entry);
  EXPECT_EQ(block - start, 32 + stringDiff);
  EXPECT_EQ(version - start, 34 + stringDiff);
  EXPECT_EQ(pinyin - start, 36 + stringDiff);
  // sources=38, linkType=39, linkedReadings=40, radical=44
  // strokes=48, variantStrokes=52
  EXPECT_EQ(links - start, 56 + stringDiff);
  EXPECT_EQ(morohashiId - start, 80 + stringDiff);
  EXPECT_EQ(nelsonIds - start, 104 + stringDiff * 2);
  EXPECT_EQ(jSource - start, 128 + stringDiff * 3);
  EXPECT_EQ(meaning - start, 152 + stringDiff * 4);
  EXPECT_EQ(onReading - start, 176 + stringDiff * 5);
  EXPECT_EQ(kunReading - start, 200 + stringDiff * 6);
}

TEST(UcdTest, SourcesTooLong) {
  const std::string s{"GHJKHJK"};
  EXPECT_THROW(call([&s] { return Ucd{TestUcd{}.sources(s)}; },
                   "sources '" + s + "' exceeds max size"),
      std::domain_error);
}

TEST(UcdTest, SourcesHasDuplicate) {
  const std::string s{"GHH"};
  EXPECT_THROW(call([&s] { return Ucd{TestUcd{}.sources(s)}; },
                   "sources '" + s + "' has duplicate value: H"),
      std::domain_error);
}

TEST(UcdTest, SourcesUnrecognized) {
  const std::string s{"JKL"};
  EXPECT_THROW(call([&s] { return Ucd{TestUcd{}.sources(s)}; },
                   "sources '" + s + "' has unrecognized value: L"),
      std::domain_error);
}

TEST(UcdTest, SetSources) {
  const Ucd noSources{TestUcd{}};
  EXPECT_EQ(noSources.sources(), "");
  EXPECT_FALSE(noSources.joyo());
  EXPECT_FALSE(noSources.jinmei());
  for (auto i : {std::pair{false, false}, std::pair{false, true},
           std::pair{true, false}, std::pair{true, true}}) {
    const Ucd ucd{TestUcd{}.sources("VTKJHG").joyo(i.first).jinmei(i.second)};
    EXPECT_EQ(ucd.sources(), "GHJKTV"); // sources are returned in alpha order
    EXPECT_EQ(ucd.joyo(), i.first);
    EXPECT_EQ(ucd.jinmei(), i.second);
  }
}

TEST(UcdTest, CodeAndName) {
  const Ucd ucd{TestUcd{"学"}.code(u'\x5b66').block("CJK").version("1.1")};
  EXPECT_EQ(ucd.code(), u'\x5b66');
  EXPECT_EQ(ucd.block().name(), "CJK");
  EXPECT_EQ(ucd.version().name(), "1.1");
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::None);
  EXPECT_EQ(ucd.codeAndName(), "[5B66] 学");
}

TEST(UcdTest, LinkCodeAndNames) {
  const Ucd ucd{
      TestUcd{"學"}.links({{u'\x5b66', "学"}}, UcdLinkTypes::Simplified)};
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::Simplified);
  EXPECT_EQ(ucd.linkCodeAndNames(), "[5B66] 学");
}

TEST(UcdTest, MultipleLinkCodeAndNames) {
  const Ucd ucd{TestUcd("并").links(
      {{u'\x4e26', "並"}, {u'\x4f75', "併"}}, UcdLinkTypes::Traditional)};
  EXPECT_EQ(ucd.linkType(), UcdLinkTypes::Traditional);
  EXPECT_EQ(ucd.linkCodeAndNames(), "[4E26] 並, [4F75] 併");
}

} // namespace kanji_tools
