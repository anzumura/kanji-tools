#include <gtest/gtest.h>
#include <tests/kanji_tools/TestUcd.h>
#include <tests/kanji_tools/WhatMismatch.h>

namespace kanji_tools {

namespace {

// 'ptrCast' is used in Layout test
template<typename T>
[[nodiscard]] constexpr size_t ptrCast(const T& x) noexcept {
  static_assert(sizeof(size_t) == sizeof(&x));
  return reinterpret_cast<size_t>(&x);
}

} // namespace

TEST(UcdTest, Size) {
  EXPECT_EQ(sizeof(bool), 1);
  EXPECT_EQ(sizeof(UcdLinkTypes), 1);
  EXPECT_EQ(sizeof(MorohashiId), 4);
  EXPECT_EQ(sizeof(size_t), 8);
  EXPECT_EQ(sizeof(std::string*), 8);
  EXPECT_EQ(sizeof(Ucd::Links), 24);
#ifdef __clang__
  EXPECT_EQ(sizeof(Ucd), 184);
  EXPECT_EQ(sizeof(UcdEntry), 24);
  EXPECT_EQ(sizeof(std::string), 24);
#else
  EXPECT_EQ(sizeof(Ucd), 232);
  EXPECT_EQ(sizeof(UcdEntry), 32);
  EXPECT_EQ(sizeof(std::string), 32);
#endif
}

TEST(UcdTest, Layout) {
#ifdef __clang__
  const size_t stringDiff{};
#else
  const size_t stringDiff{8}; // gcc string is 8 bytes bigger than clang
#endif
  const Ucd u{TestUcd{}};
  const size_t start{ptrCast(u)};
  EXPECT_EQ(ptrCast(u.entry()), start);
  EXPECT_EQ(ptrCast(u.block()) - start, 24 + stringDiff);
  EXPECT_EQ(ptrCast(u.version()) - start, 26 + stringDiff);
  EXPECT_EQ(ptrCast(u.pinyin()) - start, 28 + stringDiff);
  // sources=30, linkType=31, radical=32, strokes=34
  EXPECT_EQ(ptrCast(u.morohashiId()) - start, 36 + stringDiff);
  EXPECT_EQ(ptrCast(u.links()) - start, 40 + stringDiff);
  EXPECT_EQ(ptrCast(u.nelsonIds()) - start, 64 + stringDiff);
  size_t i{2};
  EXPECT_EQ(ptrCast(u.jSource()) - start, 88 + stringDiff * i);
  EXPECT_EQ(ptrCast(u.meaning()) - start, 112 + stringDiff * ++i);
  EXPECT_EQ(ptrCast(u.onReading()) - start, 136 + stringDiff * ++i);
  EXPECT_EQ(ptrCast(u.kunReading()) - start, 160 + stringDiff * ++i);
}

TEST(UcdEntry, GoodCodeAndName) {
  const UcdEntry e{0x96f7, "雷"};
  EXPECT_EQ(e.code(), 0x96f7);
  EXPECT_EQ(e.name(), "雷");
}

TEST(UcdEntry, BadName) {
  const auto msg{[](const std::string& i) {
    return "name '" + i + "' isn't a recognized Kanji";
  }};
  for (auto i : {"", "a", "こ", "。", "雷鳴", "轟く"})
    EXPECT_THROW(call([i] { UcdEntry{{}, i}; }, msg(i)), std::domain_error);
}

TEST(UcdEntry, BadCode) {
  constexpr Code ThunderCompat{0xf949}; // normal 'thunder' is 96F7
  const auto f{[] { UcdEntry{ThunderCompat, "雷"}; }};
  EXPECT_THROW(call(f, "code 'F949' doesn't match '96F7'"), std::domain_error);
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
