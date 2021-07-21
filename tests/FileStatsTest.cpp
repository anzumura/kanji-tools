#include <gtest/gtest.h>

#include <kanji/FileStats.h>
#include <kanji/Kanji.h>
#include <kanji/KanjiData.h>

#include <sstream>

namespace kanji {

class FileStatsTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "-data";
    static const char* arg2 = "../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }
  FileStatsTest() : _data(std::make_shared<KanjiData>(3, argv(), _os, _es)) {}

  std::stringstream _os;
  std::stringstream _es;
  const DataPtr _data;
};

TEST_F(FileStatsTest, PrintStatsForOneFile) {
  const char* testArgs[] = {"", "../../tests/sample-data/wiki-articles/02-中島みゆき.txt"};
  FileStats stats(std::size(testArgs), testArgs, _data);
  const char* expected[] = {
    ">>> Stats for: 02-中島みゆき.txt - showing 5 most frequent kanji per type",
    ">>>     Common Kanji:   9699, unique: 1034, 100.00%",
    ">>>        [Jouyou] :   9543, unique:  955,  98.39%  (年 688, 日 397, 中 378, 月 352, 島 338)",
    ">>>        [Jinmei] :     98, unique:   48,   1.01%  (柏 9, 幌 8, 篇 7, 斐 7, 浩 6)",
    ">>>  [LinkedJinmei] :     13, unique:    7,   0.13%  (龍 7, 眞 1, 兒 1, 曾 1, 槇 1)",
    ">>>     [LinkedOld] :      3, unique:    3,   0.03%  (澤 1, 會 1, 讀 1)",
    ">>>         [Other] :      6, unique:    4,   0.06%  (嘘 3, 聯 1, 噺 1, 噛 1)",
    ">>>         [Extra] :     22, unique:    7,   0.23%  (蝕 4, 邯 4, 鄲 4, 哭 3, 嘯 3)",
    ">>>          [None] :     14, unique:   10,   0.14%  (聰 2, 蘋 2, 遽 2, 鶫 2, 揄 1)",
    ">>>         Hiragana:   7990, unique:   71",
    ">>>         Katakana:   7118, unique:   80",
    ">>>   MB-Punctuation:   2097, unique:   13",
    ">>>        MB-Symbol:      5, unique:    2",
    ">>>        MB-Letter:    244, unique:   11",
    ">>> Total Kanji+Kana: 24807 (Common Kanji: 39.1%, Hiragana: 32.2%, Katakana: 28.7%)"};
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(FileStatsTest, PrintStatsForOneDirectory) {
  const char* testArgs[] = {"", "../../tests/sample-data/wiki-articles"};
  FileStats stats(std::size(testArgs), testArgs, _data);
  const char* expected[] = {
    ">>> Stats for: wiki-articles (3 files) - showing 5 most frequent kanji per type",
    ">>>     Common Kanji:  45207, unique: 1995, 100.00%",
    ">>>        [Jouyou] :  44109, unique: 1644,  97.57%  (年 1737, 日 1042, 郎 949, 月 895, 拓 847)",
    ">>>        [Jinmei] :    742, unique:  189,   1.64%  (之 60, 彦 52, 篇 27, 祐 20, 伊 18)",
    ">>>  [LinkedJinmei] :     59, unique:   21,   0.13%  (峯 11, 龍 7, 藝 5, 瀧 5, 眞 4)",
    ">>>     [LinkedOld] :     44, unique:    8,   0.10%  (澤 36, 齋 2, 會 1, 濱 1, 畫 1)",
    ">>>         [Other] :     56, unique:   19,   0.12%  (渕 24, 倶 5, 嘘 4, 娼 3, 諌 3)",
    ">>>         [Extra] :     42, unique:   21,   0.09%  (妾 4, 蝕 4, 邯 4, 鄲 4, 哭 3)",
    ">>>          [None] :    155, unique:   93,   0.34%  (婬 18, 剪 10, 畸 9, 滸 4, 薛 3)",
    ">>>         Hiragana:  43197, unique:   79",
    ">>>         Katakana:  24539, unique:   83",
    ">>>   MB-Punctuation:  10247, unique:   23",
    ">>>        MB-Symbol:     42, unique:    8",
    ">>>        MB-Letter:   1240, unique:   36",
    ">>> Total Kanji+Kana: 112943 (Common Kanji: 40.0%, Hiragana: 38.2%, Katakana: 21.7%)"};
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

TEST_F(FileStatsTest, PrintParentDirectoryIfLastComponentIsSlash) {
  const char* testArgs[] = {"", "../../tests/sample-data/wiki-articles/"};
  FileStats stats(std::size(testArgs), testArgs, _data);
  std::string line;
  bool found = false;
  while (!found && std::getline(_os, line))
    found = line.starts_with(">>> Stats for: wiki-articles (3 files)");
  EXPECT_TRUE(found);
}

TEST_F(FileStatsTest, PrintStatsForMultipleDirectories) {
  const char* testArgs[] = {"", "../../tests/sample-data"};
  FileStats stats(std::size(testArgs), testArgs, _data);
  const char* expected[] = {
    ">>> Stats for: sample-data (5 files from 3 directories) - showing 5 most frequent kanji per type",
    ">>>     Common Kanji:  96112, unique: 2622, 100.00%",
    ">>>        [Jouyou] :  93398, unique: 1918,  97.18%  (私 2747, 年 1838, 日 1299, 人 1168, 郎 999)",
    ">>>        [Jinmei] :   1663, unique:  306,   1.73%  (坐 62, 之 60, 厨 55, 彦 52, 廻 51)",
    ">>>  [LinkedJinmei] :     87, unique:   24,   0.09%  (燈 20, 峯 12, 龍 7, 藝 5, 瀧 5)",
    ">>>     [LinkedOld] :     46, unique:   10,   0.05%  (澤 36, 齋 2, 嶽 1, 挾 1, 會 1)",
    ">>>         [Other] :    148, unique:   37,   0.15%  (渕 24, 苅 24, 呑 17, 嘘 14, 叱 10)",
    ">>>         [Extra] :    177, unique:   52,   0.18%  (厭 36, 掻 13, 妾 7, 怯 5, 攫 5)",
    ">>>          [None] :    593, unique:  275,   0.62%  (婬 18, 椒 14, 婢 12, 拵 12, 掟 11)",
    ">>>         Hiragana: 162592, unique:   80",
    ">>>         Katakana:  24939, unique:   83",
    ">>>   MB-Punctuation:  22127, unique:   24",
    ">>>        MB-Symbol:     45, unique:    9",
    ">>>        MB-Letter:   1730, unique:   39",
    ">>> Total Kanji+Kana: 283643 (Common Kanji: 33.9%, Hiragana: 57.3%, Katakana: 8.8%)"};
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
  EXPECT_EQ(count, maxLines);
}

} // namespace kanji