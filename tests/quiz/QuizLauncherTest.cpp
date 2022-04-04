#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/QuizLauncher.h>
#include <tests/kanji_tools/WhatMismatch.h>

#include <sstream>

namespace kanji_tools {

class QuizLauncherTest : public ::testing::Test {
protected:
  static void SetUpTestCase() {
    _data = std::make_shared<KanjiData>(Args{}, _os, _es);
    _groupData = std::make_shared<GroupData>(_data);
    _jukugoData = std::make_shared<JukugoData>(_data);
  }

  QuizLauncherTest() { reset(); }

  void reset() const {
    _os.str(EmptyString);
    _os.clear();
    _es.str(EmptyString);
    _es.clear();
  }

  std::stringstream _is;

  inline static std::stringstream _os, _es;
  inline static DataPtr _data;
  inline static GroupDataPtr _groupData;
  inline static JukugoDataPtr _jukugoData;

  inline static const std::string Help{", use -h for help"};
};

TEST_F(QuizLauncherTest, HelpMessage) {
  const char* args[]{"", "-h"};
  QuizLauncher{args, _data, _groupData, _jukugoData};
  // look for a few strings instead of comparing the whole output
  const auto expected = {"-r   review mode", "-t   test mode",
      "-s   show English meanings by default (can be toggled on/off later)"};
  size_t found{};
  for (std::string line; std::getline(_os, line);)
    for (const auto& i : expected)
      if (line.ends_with(i)) {
        ++found;
        break;
      }
  EXPECT_EQ(found, std::size(expected));
}

TEST_F(QuizLauncherTest, ValidOptions) {
  for (const auto i : {"-g5", "-f2", "-kc", "-l3", "-m1", "-p4"}) {
    const char* args[]{"", i, "-r1"};
    if (i[1] == 'p') _is << "1\n"; // select pattern group bucket
    _is << "/\n";                  // send 'quit' option
    QuizLauncher{args, _data, _groupData, _jukugoData, &_is};
    EXPECT_TRUE(
        _os.str().ends_with("Select (-=show meanings, .=next, /=quit): "));
    EXPECT_EQ(_es.str(), EmptyString);
  }
}

TEST_F(QuizLauncherTest, IllegalOption) {
  const char* args[]{"", "-s", "-j"};
  const auto f{[&args] { QuizLauncher{args, _data, _groupData, _jukugoData}; }};
  EXPECT_THROW(call(f, "illegal option '-j'" + Help), std::domain_error);
}

TEST_F(QuizLauncherTest, MultipleModes) {
  const char* args[]{"", "-r", "-t"};
  const auto f{[&args] { QuizLauncher{args, _data, _groupData, _jukugoData}; }};
  EXPECT_THROW(call(f, "only one mode (-r or -t) can be specified" + Help),
      std::domain_error);
}

TEST_F(QuizLauncherTest, MultipleQuizTypes) {
  const char* args[]{"", "-g", "-l"};
  const auto f{[&args] { QuizLauncher{args, _data, _groupData, _jukugoData}; }};
  EXPECT_THROW(
      call(f, "only one quiz type can be specified" + Help), std::domain_error);
}

TEST_F(QuizLauncherTest, InvalidFormat) {
  for (const auto i :
      {"-g7", "-f6", "-kd", "-l6", "-m0", "-p5", "-tx", "-ry"}) {
    const char* args[]{"", i};
    const auto f{[&args] {
      QuizLauncher{args, _data, _groupData, _jukugoData};
    }};
    EXPECT_THROW(
        call(f, "invalid format for '-" + std::string{i[1], '\''} + Help),
        std::domain_error);
  }
}

TEST_F(QuizLauncherTest, InvalidQuestionNumber) {
  const char* args[]{"", "-r81", "-g1"};
  const auto f{[&args] { QuizLauncher{args, _data, _groupData, _jukugoData}; }};
  EXPECT_THROW(call(f, "entry num '81' is larger than total questions: 80"),
      std::domain_error);
}

TEST_F(QuizLauncherTest, QuestionExceedsLimit) {
  for (const auto i : {"-r66000", "-t67000"}) {
    const char* args[]{"", i};
    const auto f{[&args] {
      QuizLauncher{args, _data, _groupData, _jukugoData};
    }};
    EXPECT_THROW(call(f, "value for '" + std::string(i, 2) + "' exceeds limit"),
        std::domain_error);
  }
}

TEST_F(QuizLauncherTest, SetProgramMode) {
  for (auto& i :
      {std::pair{"--", false}, std::pair{"-r", false}, std::pair{"-t", true}}) {
    const char* args[]{"", i.first};
    // specifying '&_is' causes launcher to not start automatically
    QuizLauncher quiz{args, _data, _groupData, _jukugoData, &_is};
    EXPECT_EQ(quiz.isTestMode(), i.second);
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
    QuizLauncher quiz{args, _data, _groupData, _jukugoData};
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
    QuizLauncher quiz{args, _data, _groupData, _jukugoData};
    EXPECT_EQ(_os.str(), expected);
    reset();
  }
}

TEST_F(QuizLauncherTest, ShowUnicodeNotInUcd) {
  const auto expected{R"(>>> Legend:
Fields: N[1-5]=JLPT Level, K[1-10]=Kentei Kyu, G[1-6]=Grade (S=Secondary School)
Suffix: .=常用 '=JLPT "=Freq ^=人名用 ~=LinkJ %=LinkO +=Extra @=検定 #=1級 *=Ucd
Sources: G=China / Singapore, H=Hong Kong, J=Japan, K=Korea, T=Taiwan, V=Vietnam

㐁 [3401] --- Not found in 'ucd.txt'
)"};
  const char* args[]{"", "u3401"};
  QuizLauncher quiz{args, _data, _groupData, _jukugoData};
  EXPECT_EQ(_os.str(), expected);
}

TEST_F(QuizLauncherTest, ShowByMorohashiNotFound) {
  const char* args[]{"", "m99P"};
  QuizLauncher quiz{args, _data, _groupData, _jukugoData};
  EXPECT_EQ(_os.str(), "Found 0 matches for Morohashi ID 99P\n");
}

TEST_F(QuizLauncherTest, ShowByNelsonNotFound) {
  const char* args[]{"", "n6000"};
  QuizLauncher quiz{args, _data, _groupData, _jukugoData};
  EXPECT_EQ(_os.str(), "Found 0 matches for Nelson ID 6000\n");
}

TEST_F(QuizLauncherTest, ShowByFrequencyNotFound) {
  const char* args[]{"", "2502"};
  const auto f{[&args] {
    QuizLauncher quiz{args, _data, _groupData, _jukugoData};
  }};
  EXPECT_THROW(
      call(f, "Kanji not found for frequency '2502'"), std::domain_error);
}

TEST_F(QuizLauncherTest, InvalidMorohashiId) {
  const char* args[]{"", "m123Q"};
  const auto f{[&args] {
    QuizLauncher quiz{args, _data, _groupData, _jukugoData};
  }};
  EXPECT_THROW(call(f, "invalid Morohashi ID '123Q'"), std::domain_error);
}

TEST_F(QuizLauncherTest, InvalidNelsonId) {
  const char* args[]{"", "n123B"};
  const auto f{[&args] {
    QuizLauncher quiz{args, _data, _groupData, _jukugoData};
  }};
  EXPECT_THROW(call(f, "invalid Nelson ID '123B'"), std::domain_error);
}

TEST_F(QuizLauncherTest, InvalidUnicode) {
  const char* args[]{"", "uABC"};
  const auto f{[&args] {
    QuizLauncher quiz{args, _data, _groupData, _jukugoData};
  }};
  EXPECT_THROW(call(f, "invalid Unicode value 'ABC'"), std::domain_error);
}

TEST_F(QuizLauncherTest, UnrecognizedKanji) {
  const char* args[]{"", "a"};
  const auto f{[&args] {
    QuizLauncher quiz{args, _data, _groupData, _jukugoData};
  }};
  EXPECT_THROW(
      call(f, "unrecognized 'kanji' value 'a'" + Help), std::domain_error);
}

} // namespace kanji_tools
