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
    static const char* arg1 = "../../data";
    static const char* args[] = {arg0, arg1};
    return args;
  }
  FileStatsTest() : _data(std::make_shared<KanjiData>(2, argv(), _os, _es)) {}

  std::stringstream _os;
  std::stringstream _es;
  const DataPtr _data;
};

TEST_F(FileStatsTest, PrintStatsForOneFile) {
  const char* testArgs[] = {"", "", "-c", "../../tests/sample-data/wiki-articles/雨月物語.txt"};
  FileStats stats(4, testArgs, _data);
  const char* expected[] = {
">>> Stats for: 雨月物語.txt",
">>>      Total Kanji:   8085, unique: 1341, 100.00%",
">>>           Jouyou:   7673, unique: 1145,  94.90%",
">>>           Jinmei:    243, unique:  101,   3.01%",
">>>     LinkedJinmei:     26, unique:   11,   0.32%",
">>>        LinkedOld:      5, unique:    4,   0.06%",
">>>            Other:     10, unique:    7,   0.12%",
">>>            Extra:     10, unique:    5,   0.12%",
">>>             None:    118, unique:   68,   1.46%",
">>>         Hiragana:   8961, unique:   70",
">>>         Katakana:    350, unique:   63",
">>>   MB-Punctuation:   1866, unique:   13",
">>>        MB-Letter:    196, unique:    5",
">>> Total Kanji+Kana: 17396 (Kanji: 46.5%, Hiragana: 51.5%, Katakana: 2.0%)"
  };
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
}

TEST_F(FileStatsTest, PrintStatsForOneDirectory) {
  const char* testArgs[] = {"", "", "-c", "../../tests/sample-data/wiki-articles"};
  FileStats stats(4, testArgs, _data);
  const char* expected[] = {
">>> Stats for: wiki-articles (3 files)",
">>>      Total Kanji:  45207, unique: 1995, 100.00%",
">>>           Jouyou:  44109, unique: 1644,  97.57%",
">>>           Jinmei:    742, unique:  189,   1.64%",
">>>     LinkedJinmei:     59, unique:   21,   0.13%",
">>>        LinkedOld:     44, unique:    8,   0.10%",
">>>            Other:     56, unique:   19,   0.12%",
">>>            Extra:     38, unique:   17,   0.08%",
">>>             None:    159, unique:   97,   0.35%",
">>>         Hiragana:  43197, unique:   79",
">>>         Katakana:  24539, unique:   83",
">>>   MB-Punctuation:  10313, unique:   33",
">>>        MB-Letter:   1240, unique:   36",
">>> Total Kanji+Kana: 112943 (Kanji: 40.0%, Hiragana: 38.2%, Katakana: 21.7%)"
  };
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
}

TEST_F(FileStatsTest, PrintStatsForMultipleDirectories) {
  const char* testArgs[] = {"", "", "-c", "../../tests/sample-data"};
  FileStats stats(4, testArgs, _data);
  const char* expected[] = {
">>> Stats for: sample-data (5 files from 3 directories)",
">>>      Total Kanji:  75601, unique: 2608, 100.00%",
">>>           Jouyou:  73121, unique: 1906,  96.72%",
">>>           Jinmei:   1486, unique:  306,   1.97%",
">>>     LinkedJinmei:     84, unique:   24,   0.11%",
">>>        LinkedOld:     46, unique:   10,   0.06%",
">>>            Other:    130, unique:   37,   0.17%",
">>>            Extra:    115, unique:   39,   0.15%",
">>>             None:    619, unique:  286,   0.82%",
">>>         Hiragana: 107960, unique:   80",
">>>         Katakana:  24721, unique:   83",
">>>   MB-Punctuation:  16497, unique:   35",
">>>        MB-Letter:   1525, unique:   39",
">>> Total Kanji+Kana: 208282 (Kanji: 36.3%, Hiragana: 51.8%, Katakana: 11.9%)"
  };
  std::string line;
  int count = 0, maxLines = std::size(expected);
  while (std::getline(_os, line)) {
    if (count == maxLines) FAIL() << "got more than " << maxLines;
    EXPECT_EQ(line, expected[count++]);
  }
}

} // namespace kanji
