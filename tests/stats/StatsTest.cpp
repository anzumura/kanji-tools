#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/stats/Stats.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

class StatsTest : public ::testing::Test {
protected:
  static void SetUpTestCase() {
    _data = std::make_shared<KanjiData>(Args{}, _os);
  }

  StatsTest() {}

  void SetUp() override {
    _os.str(EmptyString);
    _os.clear();
    if (fs::exists(TestDir)) TearDown();
    EXPECT_TRUE(fs::create_directory(TestDir));
  }

  void TearDown() override { fs::remove_all(TestDir); }

  void write(const std::string& s) {
    std::ofstream of(TestFile);
    of << s;
    of.close();
  }

  static void run(const std::string& file, const std::string& expected) {
    const auto f{_data->dataDir() / "../tests/stats" / file};
    const char* args[]{"", f.c_str()};
    run(args, expected);
  }

  static void run(const Args& args, const std::string& expectedIn) {
    Stats stats(args, _data);
    std::stringstream expected{expectedIn};
    for (std::string i, j; std::getline(_os, i) && std::getline(expected, j);)
      ASSERT_EQ(i, j);
    // if loop completed due to 'eof' then check remaining part of other stream
    if (_os.eof())
      EXPECT_EQ(expected.str().substr(_os.str().size()), EmptyString);
    else if (expected.eof())
      EXPECT_EQ(_os.str().substr(expected.str().size()), EmptyString);
  }

  inline static std::stringstream _os;
  inline static DataPtr _data;
  inline static const fs::path TestDir{"testDir"};
  inline static const fs::path TestFile{TestDir / "test.txt"};
};

} // namespace

TEST_F(StatsTest, HelpMessage) {
  const char* args[]{"", "-h"};
  run(args, R"(kanjiStats [-bhv] file [file ...]:
  -b: show full Kanji breakdown for 'file' (instead of just a summary)
  -h: show help message for command-line options
  -v: show 'before' and 'after' versions of lines changed by Furigana removal
)");
}

TEST_F(StatsTest, NoOptions) {
  const char* args[]{""};
  const auto f{[&args] { Stats{args, _data}; }};
  EXPECT_THROW(call(f, "please specify at least one option or '-h' for help"),
      std::domain_error);
}

TEST_F(StatsTest, IllegalOption) {
  const char* args[]{"", "-a"};
  const auto f{[&args] { Stats{args, _data}; }};
  EXPECT_THROW(
      call(f, "illegal option '-a' use -h for help"), std::domain_error);
}

TEST_F(StatsTest, EndOfOptions) {
  const char* args[]{"", "--", "-h"};
  const auto f{[&args] { Stats{args, _data}; }};
  EXPECT_THROW(call(f, "file not found: -h"), std::domain_error);
}

TEST_F(StatsTest, PrintStatsForOneFile) {
  run("sample-data/wiki-articles/02-中島みゆき.txt",
      R"(>>> Stats for: '02-中島みゆき.txt' - showing top 5 Kanji per type
>>> Furigana Removed: 6, Combining Marks Replaced: 0, Variation Selectors: 0
>>>         Hiragana:   7990, unique:   71
>>>         Katakana:   7118, unique:   80
>>>     Common Kanji:   9699, unique: 1034, 100.00%
>>>        [Jouyou] :   9543, unique:  955,  98.39%  (年 688, 日 397, 中 378, 月 352, 島 338)
>>>        [Jinmei] :     98, unique:   48,   1.01%  (柏 9, 幌 8, 篇 7, 斐 7, 浩 6)
>>>  [LinkedJinmei] :     13, unique:    7,   0.13%  (龍 7, 眞 1, 兒 1, 曾 1, 槇 1)
>>>     [LinkedOld] :      3, unique:    3,   0.03%  (澤 1, 會 1, 讀 1)
>>>     [Frequency] :      6, unique:    4,   0.06%  (嘘 3, 聯 1, 噺 1, 噛 1)
>>>         [Extra] :     22, unique:    7,   0.23%  (蝕 4, 邯 4, 鄲 4, 哭 3, 嘯 3)
>>>        [Kentei] :     12, unique:    9,   0.12%  (蘋 2, 遽 2, 鶫 2, 揄 1, 揶 1)
>>>           [Ucd] :      2, unique:    1,   0.02%  (聰 2)
>>>   MB-Punctuation:   2097, unique:   13
>>>        MB-Symbol:      5, unique:    2
>>>        MB-Letter:    244, unique:   11
>>> Total Kana+Kanji: 24807 (Hiragana: 32.2%, Katakana: 28.7%, Kanji: 39.1%)
)");
}

TEST_F(StatsTest, PrintStatsForOneDirectory) {
  run("sample-data/wiki-articles",
      R"(>>> Stats for: 'wiki-articles' (3 files) - showing top 5 Kanji per type
>>> Furigana Removed: 39, Combining Marks Replaced: 0, Variation Selectors: 0
>>>         Hiragana:  43197, unique:   79
>>>         Katakana:  24442, unique:   83
>>>     Common Kanji:  45207, unique: 1995, 100.00%
>>>        [Jouyou] :  44109, unique: 1644,  97.57%  (年 1737, 日 1042, 郎 949, 月 895, 拓 847)
>>>        [Jinmei] :    742, unique:  189,   1.64%  (之 60, 彦 52, 篇 27, 祐 20, 伊 18)
>>>  [LinkedJinmei] :     59, unique:   21,   0.13%  (峯 11, 龍 7, 藝 5, 瀧 5, 眞 4)
>>>     [LinkedOld] :     44, unique:    8,   0.10%  (澤 36, 齋 2, 會 1, 濱 1, 畫 1)
>>>     [Frequency] :     56, unique:   19,   0.12%  (渕 24, 倶 5, 嘘 4, 娼 3, 諌 3)
>>>         [Extra] :     61, unique:   23,   0.13%  (婬 18, 妾 4, 蝕 4, 邯 4, 鄲 4)
>>>        [Kentei] :    124, unique:   81,   0.27%  (剪 10, 畸 9, 滸 4, 薛 3, 闍 3)
>>>           [Ucd] :     12, unique:   10,   0.03%  (畀 2, 聰 2, 侔 1, 偪 1, 揜 1)
>>>   MB-Punctuation:  10247, unique:   23
>>>        MB-Symbol:     42, unique:    8
>>>        MB-Letter:   1204, unique:   36
>>> Total Kana+Kanji: 112846 (Hiragana: 38.3%, Katakana: 21.7%, Kanji: 40.1%)
)");
}

TEST_F(StatsTest, PrintParentDirectoryIfLastComponentIsSlash) {
  const auto file{
      _data->dataDir() / "../tests/stats/sample-data/wiki-articles/"};
  const char* args[]{"", file.c_str()};
  Stats stats(args, _data);
  auto found{false};
  for (std::string line; !found && std::getline(_os, line);)
    found = line.starts_with(">>> Stats for: 'wiki-articles' (3 files)");
  EXPECT_TRUE(found);
}

TEST_F(StatsTest, PrintStatsForMultipleDirectories) {
  run("sample-data",
      R"(>>> Stats for: 'sample-data' (5 files from 3 directories) - showing top 5 Kanji per type
>>> Furigana Removed: 3397, Combining Marks Replaced: 0, Variation Selectors: 0
>>>         Hiragana: 162560, unique:   80
>>>         Katakana:  24689, unique:   83
>>>     Common Kanji:  96137, unique: 2636, 100.00%
>>>        [Jouyou] :  93398, unique: 1918,  97.15%  (私 2747, 年 1838, 日 1299, 人 1168, 郎 999)
>>>        [Jinmei] :   1663, unique:  306,   1.73%  (坐 62, 之 60, 厨 55, 彦 52, 廻 51)
>>>  [LinkedJinmei] :     87, unique:   24,   0.09%  (燈 20, 峯 12, 龍 7, 藝 5, 瀧 5)
>>>     [LinkedOld] :     47, unique:   11,   0.05%  (澤 36, 齋 2, 嶽 1, 挾 1, 插 1)
>>>     [Frequency] :    148, unique:   37,   0.15%  (渕 24, 苅 24, 呑 17, 嘘 14, 叱 10)
>>>         [Extra] :    233, unique:   56,   0.24%  (厭 36, 婬 18, 椒 14, 掻 13, 婢 12)
>>>        [Kentei] :    520, unique:  257,   0.54%  (掟 11, 剪 10, 烟 9, 畸 9, 竟 8)
>>>           [Ucd] :     41, unique:   27,   0.04%  (樏 5, 筯 5, 譃 3, 欝 2, 畀 2)
>>>   MB-Punctuation:  22102, unique:   23
>>>        MB-Symbol:     45, unique:    9
>>>        MB-Letter:   1704, unique:   39
>>> Total Kana+Kanji: 283386 (Hiragana: 57.4%, Katakana: 8.7%, Kanji: 33.9%)
)");
}

TEST_F(StatsTest, NonUcdKanji) {
  // 'UCD' here refers to Kanji in 'data/ucd.txt' which is a filtered set of
  // kanji from the original complete set (see scripts/parseUcdAllFlat.sh for
  // details).
  write("丆㐁"); // include examples from 'common' and 'rare' unicode blocks
  const char* args[]{"", "testDir"};
  run(args, R"(>>> Stats for: 'testDir' - showing top 5 Kanji per type
>>>    Non-UCD Kanji:      2, unique:    2           (㐁 1, 丆 1)
>>> Total Kana+Kanji: 2 (Kanji: 100.0%)
)");
}

TEST_F(StatsTest, ShowBreakdown) {
  write("ああア西西東南南南巽𫞉㐁");
  const char* args[]{"", "testDir", "-b"};
  run(args, R"(>>> Stats for: 'testDir' - showing top 5 Kanji per type
>>>         Hiragana:      2, unique:    1
>>>         Katakana:      1, unique:    1
>>>     Common Kanji:      7, unique:    4, 100.00%
>>>        [Jouyou] :      6, unique:    3,  85.71%  (南 3, 西 2, 東 1)
>>>        [Jinmei] :      1, unique:    1,  14.29%  (巽 1)
>>> Showing Breakdown for 'Common Kanji':
  Rank  [Val Num] Freq, LV, Type
  1     [南    3]  341, N5, Jouyou
  2     [西    2]  259, N5, Jouyou
  3     [東    1]   37, N5, Jouyou
  4     [巽    1] 2061, N1, Jinmei
>>>       Rare Kanji:      1, unique:    1           (𫞉 1)
>>> Showing Breakdown for 'Rare Kanji':
  Rank  [Val Num] Freq, LV, Type
  1     [𫞉    1]    0, --, Ucd
>>>    Non-UCD Kanji:      1, unique:    1           (㐁 1)
>>> Showing Breakdown for 'Non-UCD Kanji':
  Rank  [Val Num], Unicode, Highest Count File
  1     [㐁    1],  U+3401, test.txt
>>> Total Kana+Kanji: 12 (Hiragana: 16.7%, Katakana: 8.3%, Kanji: 75.0%)
)");
}

TEST_F(StatsTest, ShowVerbose) {
  write(R"(何時（いつ）までと区切りましょう　突然で驚かぬように
めでたさも　かなしさも　手に負えぬ　天任せ
行（ゆ）く方（かた）も　来（こ）し方（かた）も　齢寿（よわいことぶき）天任せ

１足す１が２と限らない世界
１引く１が０（ゼロ）にならない世界
あてにしてた梯子（はしご）が外（はず）されても
まだまだ人は昇るつもりの世界
)");
  // using '-v' causes the program to show all 'Furigana' substitutions
  const char* args[]{"", "testDir", "-v"};
  run(args, R"(>>> Showing all Furigana replacements:
Tag 'test.txt'
  1 : 何時（いつ）までと区切りましょう　突然で驚かぬように
    : 何時までと区切りましょう　突然で驚かぬように
  2 : 行（ゆ）く方（かた）も　来（こ）し方（かた）も　齢寿（よわいことぶき）天任せ
    : 行く方も　来し方も　齢寿天任せ
  3 : １引く１が０（ゼロ）にならない世界
    : １引く１が０にならない世界
  4 : あてにしてた梯子（はしご）が外（はず）されても
    : あてにしてた梯子が外されても
>>> Stats for: 'testDir' - showing top 5 Kanji per type
>>> Furigana Removed: 4, Combining Marks Replaced: 0, Variation Selectors: 0
>>>         Hiragana:     67, unique:   31
>>>     Common Kanji:     33, unique:   26, 100.00%
>>>        [Jouyou] :     32, unique:   25,  96.97%  (世 3, 界 3, 方 2, 任 2, 天 2)
>>>        [Jinmei] :      1, unique:    1,   3.03%  (梯 1)
>>>        MB-Letter:      6, unique:    3
>>> Total Kana+Kanji: 100 (Hiragana: 67.0%, Kanji: 33.0%)
)");
}

} // namespace kanji_tools
