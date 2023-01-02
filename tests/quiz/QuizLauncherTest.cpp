#include <gtest/gtest.h>
#include <kt_kanji/TextKanjiData.h>
#include <kt_quiz/QuizLauncher.h>
#include <kt_tests/Utils.h>
#include <kt_tests/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

class QuizLauncherTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    _data = std::make_shared<TextKanjiData>(Args{}, _os, _es);
    _groupData = std::make_shared<GroupData>(_data);
    _jukugoData = std::make_shared<JukugoData>(_data);
  }

  static void reset() {
    _os.str({});
    _os.clear();
    _es.str({});
    _es.clear();
  }

  QuizLauncherTest() { reset(); }

  inline static const String Help{", use -h for help"};

  inline static std::stringstream _os, _es;
  inline static KanjiDataPtr _data;
  inline static GroupDataPtr _groupData;
  inline static JukugoDataPtr _jukugoData;

  template <size_t N>
  static void run(const char* (&args)[N], std::istream* in = {}) {
    QuizLauncher{args, _data, _groupData, _jukugoData, in};
  }

  auto& is() { return _is; }

private:
  std::stringstream _is;
};

TEST_F(QuizLauncherTest, HelpMessage) {
  const char* args[]{"", "-h"};
  run(args);
  // look for a few strings instead of comparing the whole output
  const auto expected = {
      "-s   show English meanings by default (can be toggled on/off later)",
      "-r   review mode", "-t   test mode"};
  EXPECT_EQ(findEndMatches(_os, expected), std::nullopt);
}

TEST_F(QuizLauncherTest, ValidOptions) {
  // loop over all the different quiz types (plus a valid question list)
  for (const auto i : {"-g5", "-f2", "-kc", "-l3", "-m1", "-p4"})
    // loop over different question orders: 1=beginning, -1=end, 0=random
    for (const auto j : {"-r1", "-r-1", "-r0"}) {
      const char* args[]{"", i, j};
      if (i[1] == 'p') is() << "1\n"; // select pattern group bucket
      is() << "/\n";                  // send 'quit' option
      run(args, &is());
      EXPECT_TRUE(
          _os.str().ends_with("Select (-=show meanings, .=next, /=quit): "));
      EXPECT_EQ(_es.str(), "");
      reset();
    }
}

TEST_F(QuizLauncherTest, QuestionOrderQuit) {
  const char* args[]{"", "-p1", "-r"};
  is() << "/\n"; // quit instead of choosing a question order
  run(args, &is());
  EXPECT_TRUE(_os.str().ends_with(
      "List order (/=quit, b=from beginning, e=from end, r=random) def 'r': "));
}

TEST_F(QuizLauncherTest, IllegalOption) {
  const char* args[]{"", "-s", "-j"};
  const auto f{[&args] { QuizLauncher{args, _data, _groupData, _jukugoData}; }};
  EXPECT_THROW(call(f, "illegal option '-j'" + Help), DomainError);
}

TEST_F(QuizLauncherTest, MultipleModes) {
  const char* args[]{"", "-r", "-t"};
  const auto f{[&args] { QuizLauncher{args, _data, _groupData, _jukugoData}; }};
  EXPECT_THROW(
      call(f, "only one mode (-r or -t) can be specified" + Help), DomainError);
}

TEST_F(QuizLauncherTest, MultipleQuizTypes) {
  const char* args[]{"", "-g", "-l"};
  const auto f{[&args] { QuizLauncher{args, _data, _groupData, _jukugoData}; }};
  EXPECT_THROW(
      call(f, "only one quiz type can be specified" + Help), DomainError);
}

TEST_F(QuizLauncherTest, InvalidFormat) {
  for (const auto i :
      {"-g7", "-fa", "-kd", "-l6", "-m0", "-p5", "-tx", "-ry"}) {
    const char* args[]{"", i};
    const auto f{[&args] {
      QuizLauncher{args, _data, _groupData, _jukugoData};
    }};
    EXPECT_THROW(call(f, "invalid format for '-" + String{i[1], '\''} + Help),
        DomainError);
  }
}

TEST_F(QuizLauncherTest, InvalidQuestionNumber) {
  const char* args[]{"", "-r81", "-g1"};
  EXPECT_THROW(call([&args] { run(args); },
                   "entry num '81' is larger than total questions: 80"),
      DomainError);
}

TEST_F(QuizLauncherTest, QuestionExceedsLimit) {
  for (const auto i : {"-r66000", "-t67000"}) {
    const char* args[]{"", i};
    EXPECT_THROW(call([&args] { run(args); },
                     "value for '" + String(i, 2) + "' exceeds limit"),
        DomainError);
  }
}

TEST_F(QuizLauncherTest, SetProgramMode) {
  for (auto& i :
      {std::pair{"--", false}, std::pair{"-r", false}, std::pair{"-t", true}}) {
    const char* args[]{"", i.first};
    // specifying '&is()' causes launcher to not start automatically
    const QuizLauncher quiz{args, _data, _groupData, _jukugoData, &is()};
    EXPECT_EQ(quiz.isQuizMode(), i.second);
  }
}

TEST_F(QuizLauncherTest, ShowDetails) {
  const auto expected{R"(>>> Legend:
Fields: N[1-5]=JLPT Level, K[1-10]=Kentei Kyu, G[1-6]=Grade (S=Secondary School)
Suffix: .=常用 '=JLPT "=Freq ^=人名用 ~=LinkJ %=LinkO +=Extra @=検定 #=1級 *=Ucd
Sources: G=China / Singapore, H=Hong Kong, J=Japan, K=Korea, T=Taiwan, V=Vietnam

奉 [5949], Blk CJK, Ver 1.1, Sources GHJKTV (J0-4A74), Jouyou (#1833)
Rad 大(37), Strokes 8, fèng, S, N1, Frq 1624, K3
    Meaning: observance
    Reading: ホウ、（ブ）、たてまつ-る
    Similar: 俸. 棒. 捧"
  Morohashi: 5894
  Nelson ID: 212
     Jukugo: 10
ご奉仕（ごほうし）  御奉仕（ごほうし）  信奉（しんぽう）    
奉行（ぶぎょう）    奉賀（ほうが）      奉公（ほうこう）    
奉仕（ほうし）      奉書（ほうしょ）    奉職（ほうしょく）  
奉納（ほうのう）    

)"};
  for (const auto i : {"奉", "1624", "m5894", "n212", "u5949"}) {
    const char* args[]{"", i};
    run(args);
    EXPECT_EQ(_os.str(), expected);
    reset();
  }
}

TEST_F(QuizLauncherTest, ShowDetailsForNonJouyou) {
  const auto expected{R"(>>> Legend:
Fields: N[1-5]=JLPT Level, K[1-10]=Kentei Kyu, G[1-6]=Grade (S=Secondary School)
Suffix: .=常用 '=JLPT "=Freq ^=人名用 ~=LinkJ %=LinkO +=Extra @=検定 #=1級 *=Ucd
Sources: G=China / Singapore, H=Hong Kong, J=Japan, K=Korea, T=Taiwan, V=Vietnam

仔 [4ED4], Blk CJK, Ver 1.1, Sources GHJKTV (J0-3B46), Jinmei (#14 2004 [Print])
Rad 人(9), Strokes 5, zǐ, KJ1
    Meaning: small thing, child; young animal
    Reading: シ、こ、た-える
    Similar: 子. 好. 字. 厚. 李' 孜"
  Morohashi: 367
  Nelson ID: 358
     Jukugo: 仔犬（こいぬ） 仔牛（こうし）

)"};
  for (const auto i : {"仔", "m367", "n358", "u4ed4"}) {
    const char* args[]{"", i};
    run(args);
    EXPECT_EQ(_os.str(), expected);
    reset();
  }
}

TEST_F(QuizLauncherTest, ShowDetailsForMultipleKanji) {
  const auto expected{R"(>>> Legend:
Fields: N[1-5]=JLPT Level, K[1-10]=Kentei Kyu, G[1-6]=Grade (S=Secondary School)
Suffix: .=常用 '=JLPT "=Freq ^=人名用 ~=LinkJ %=LinkO +=Extra @=検定 #=1級 *=Ucd

Found 3 matches for Nelson ID 1491:

㡡 [3861], Blk CJK_Ext_A, Ver 3.0, Sources GJ (J4-287B), Ucd
Rad 巾(50), Strokes 15, chú, New 幮
    Meaning: (a variant of 幮 U+5E6E, 𢅥 U+22165) a screen used to make a temporary kitchen
    Reading: チュ、ジウ、とばり、かや
 Nelson IDs: 1487 1491

幮 [5E6E], Blk CJK, Ver 1.1, Sources GHJKT (J14-2C21), Ucd
Rad 巾(50), Strokes 18, chú
    Meaning: a screen used to make a temporary kitchen
    Reading: チュ、ジウ、とばり、かや
  Morohashi: 9134
  Nelson ID: 1491

𢅥 [22165], Blk CJK_Ext_B, Ver 3.1, Sources G, Ucd
Rad 巾(50), Strokes 17, chú, New 㡡
    Meaning: variant of 㡡 U+3861, a screen to make a temporary kitchen; bed curtain
    Reading: ジュ、チュ
  Nelson ID: 1491

)"};
  const char* args[]{"", "n1491"};
  run(args);
  EXPECT_EQ(_os.str(), expected);
}

TEST_F(QuizLauncherTest, ShowUnicodeNotInUcd) {
  const auto expected{R"(>>> Legend:
Fields: N[1-5]=JLPT Level, K[1-10]=Kentei Kyu, G[1-6]=Grade (S=Secondary School)
Suffix: .=常用 '=JLPT "=Freq ^=人名用 ~=LinkJ %=LinkO +=Extra @=検定 #=1級 *=Ucd
Sources: G=China / Singapore, H=Hong Kong, J=Japan, K=Korea, T=Taiwan, V=Vietnam

㐁 [3401] --- Not found in 'ucd.txt'
)"};
  const char* args[]{"", "u3401"};
  run(args);
  EXPECT_EQ(_os.str(), expected);
}

TEST_F(QuizLauncherTest, ShowByMorohashiNotFound) {
  const char* args[]{"", "m99P"};
  run(args);
  EXPECT_EQ(_os.str(), "Found 0 matches for Morohashi ID 99P\n");
}

TEST_F(QuizLauncherTest, ShowByNelsonNotFound) {
  const char* args[]{"", "n6000"};
  run(args);
  EXPECT_EQ(_os.str(), "Found 0 matches for Nelson ID 6000\n");
}

TEST_F(QuizLauncherTest, ShowByFrequencyNotFound) {
  const char* args[]{"", "2502"};
  EXPECT_THROW(
      call([&args] { run(args); }, "Kanji not found for frequency '2502'"),
      DomainError);
}

TEST_F(QuizLauncherTest, InvalidMorohashiId) {
  const char* args[]{"", "m123Q"};
  EXPECT_THROW(
      call([&args] { run(args); }, "Morohashi ID '123Q' is non-numeric"),
      DomainError);
}

TEST_F(QuizLauncherTest, InvalidNelsonId) {
  const char* args[]{"", "n123B"};
  EXPECT_THROW(call([&args] { run(args); }, "Nelson ID '123B' is non-numeric"),
      DomainError);
}

TEST_F(QuizLauncherTest, InvalidUnicode) {
  const char* args[]{"", "uABC"};
  EXPECT_THROW(call([&args] { run(args); }, "Unicode value 'ABC' is invalid"),
      DomainError);
}

TEST_F(QuizLauncherTest, UnrecognizedKanji) {
  const char* args[]{"", "a"};
  EXPECT_THROW(
      call([&args] { run(args); }, "unrecognized 'kanji' value 'a'" + Help),
      DomainError);
}

} // namespace kanji_tools
