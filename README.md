# C++ kanji Tools

## Table of Contents

**[Introduction](#Introduction)**\
**[- Project Structure](#Project-Structure)**\
**[- VS Code Setup](#VS-Code-Setup)**\
**[- Compiler Diagnostic Flags](#Compiler-Diagnostic-Flags)**\
**[Kana Convert](#Kana-Convert)**\
**[- Kana Conversion Chart](#Kana-Conversion-Chart)**\
**[Kanji Data](#Kanji-Data)**\
**[- Kanji Class Hierarchy](#Kanji-Class-Hierarchy)**\
**[- JLPT Kanji](#JLPT-Kanji)**\
**[- Jōyō Kanji](#Jōyō-Kanji)**\
**[- Data Directory](#Data-Directory)**\
**[Kanji Quiz](#Kanji-Quiz)**\
**[- Kanji Quiz Runtime Memory](#Kanji-Quiz-Runtime-Memory)**\
**[- Kanji Quiz Binary File Size](#Kanji-Quiz-Binary-File-Size)**\
**[Kanji Stats](#Kanji-Stats)**\
**[- Aozora](#Aozora)**\
**[Helpful Commands](#Helpful-Commands)**

## Introduction

This repository contains code for four 'main' programs:

- **kanaConvert**: program that converts between Hiragana, Katakana and Rōmaji
- **kanjiFormat**: program used to format test/sample-data/books files (from 青空文庫 - see below)
- **kanjiQuiz**: interactive program that allows a user to choose from various types of quizzes
- **kanjiStats**: classifies and counts multi-byte characters in a file or directory tree

The initial goal for this project was to create a program that could parse multi-byte (UTF-8) input and classify Japanese **kanji** (漢字) characters into *official* categories in order to determine how many kanji fall into each category in real-world examples. The *quiz* program was added later once the initial work was done for loading and classifying kanji. The *format* program was created to help with a specific use-case that came up while gathering sample text from Aozora - it's a small program that relies on some of the generic code already created for the *stats* program.

### Project Structure

The project is build using *cmake* (installed via Homebrew) so there is a *CMakeLists.txt* file in the top directory that builds five *libs* (C++ static libraries for now), the four main programs (mentioned in the Introduction) plus all the test code. The tests are written using **[GoogleTest](https://github.com/google/googletest.git)** test framework. The code is split out across the following directories:

- **apps**: *CMakeLists.txt* and a *.cpp* file for each main programs
- **build**: generated directory for build targets and *cmake* dependencies
- **data**: data files described in **[Kanji Data](#Kanji-Data)** section
- **scripts**: *.sh* bash scripts for working with *Unicode* data files
- **libs**: has a directory per lib, each containing:
  - **include**: *.h* files for the lib
  - **src**: *CMakeLists.txt* and *.cpp* files for the lib
- **tests**: has *testMain.cpp*, an *include* directory and a directory per lib:
  - each sub-directory has *CMakeLists.txt* and *.cpp* files

The five libraries are:

- **utils**: utility classes used by all 4 main programs
- **kana**: code used by *kanaConvert* program (depends on **utils** lib)
- **kanji**: code for loading Kanji and Ucd data (depends on **kana** lib)
- **stats**: code used by *kanjiStats* program (depends on **kanji** lib)
- **quiz**: code used by *kanjiQuiz* program (depends on **kanji** lib)

### VS Code Setup

The code was written using **[VS Code](https://code.visualstudio.com)** IDE on an *M1 Mac* and compiles with either **clang++** (version 13.1.6) installed via *Xcode* command-line tools (`xcode-select --install`) or **g++-11** (version 11.2.0) installed via **[Homebrew](https://brew.sh)** (`brew install gcc`). Some other useful brew formulas for this project are: `bash`, `clang-format`, `cmake` and `gcovr`). It should also build on other *Unix*/*Linux* systems, but there are assumptions related to *wchar_t* and multi-byte handling that won't currently compile on *Windows 10*. 

Here are some links that might help with setup:

- **[VS Code - Clang on macOS](https://code.visualstudio.com/docs/cpp/config-clang-mac)**
- **[VS Code - Build with CMake](https://code.visualstudio.com/docs/cpp/cmake-linux)**

I am using the following VS Code extensions:

- **[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)**
- **[C++ TestMate](https://marketplace.visualstudio.com/items?itemName=matepek.vscode-catch2-test-adapter)**
- **[Clock](https://marketplace.visualstudio.com/items?itemName=angelo-breuer.clock)**
- **[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)**
- **[Code Spell Checker](https://marketplace.visualstudio.com/items?itemName=streetsidesoftware.code-spell-checker)**
- **[CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)**
- **[shell-format](https://marketplace.visualstudio.com/items?itemName=foxundermoon.shell-format)**

**Notes**:

- **Clock** and **shell-format** are of course not required for building code
- **Code Spell Checker** is also optional, but there are lots of entries for it in *.vscode/settings.json* (partly caused by all the Japanese words in test code)
- **CodeLLDB** is only used for debugging (see comments in *.vscode/launch.json* for more details)

### Compiler Diagnostic Flags

The code builds without warnings using a large set of diagnostic flags such as **-Wall**, **-Wextra** (equivalent to **-W**), **-Wconversion**, etc.. **-Werror** is also included to ensure the code remains warning-free. Finally, only one type of warning has been disabled (requiring parentheses for some expressions that seemed excessive).

The following table shows flags used per compiler (**Common** shows flags used for both). Diagnostics enabled by default or enabled via another flag such as **-Wall** are not included (at least that's the intention):

| Compiler | Standard | Diagnostic Flags | Disabled |
| --- | --- | --- | --- |
| Common | | -Wall -Wconversion -Wdeprecated -Werror -Wextra -Wextra-semi -Wignored-qualifiers -Wold-style-cast -Wpedantic -Wsuggest-override -Wswitch-enum -Wzero-as-null-pointer-constant | |
| Clang | c++2a | -Wcovered-switch-default -Wduplicate-enum -Wheader-hygiene -Wloop-analysis -Wshadow-all -Wsuggest-destructor-override -Wunreachable-code-aggressive | -Wno-logical-op-parentheses |
| GCC | c++20 | -Wnon-virtual-dtor -Woverloaded-virtual -Wshadow -Wuseless-cast | -Wno-parentheses |

**Notes**:

- **-Wpedantic** means 'Issue all the warnings demanded by strict ISO C++'
- **-Wswitch-enum** happens even when a 'default' label exists (so it's very strict) whereas **-Wswitch** (enabled by default) only warns about a missing enum value when there is no 'default'.
- Here are links for more info:
  - **Clang**: https://clang.llvm.org/docs/DiagnosticsReference.html
  - **GCC**: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html

## Kana Convert

The *kanaConvert* program was created to parse the UniHan XML files (from Unicode Consortium) which have 'On' (音) and 'Kun' (訓) readings, but only in Rōmaji. The program can read stdin and parse command line args:

```;
$ kanaConvert atatakai
あたたかい
$ kanaConvert kippu
きっぷ
$ echo kippu | kanaConvert -k  # can be used in pipes
キップ
$ echo ジョン・スミス | kanaConvert -r
jon/sumisu
$ echo かんよう　かんじ | kanaConvert -r
kan'you kanji
$ kanaConvert -r ラーメン  # uses macrons when converting from 'prolong mark'
rāmen
$ kanaConvert -h rāmen
らーめん
$ kanaConvert -r こゝろ  # supports repeat marks
kokoro
$ kanaConvert -r スヾメ
suzume
$ kanaConvert -k qarutetto  # supports multiple romaji variants:
クァルテット
$ kanaConvert -k kwarutetto
クァルテット
```

The program also supports various flags for controlling conversion (like Hepburn or Kunrei) and it has an interactive mode as well. Passing '-p' to *kanaConvert* causes it to print out a Kana Chart that shows the Rōmaji letter combinations that are supported along with some notes and totals. The output is aligned properly in a terminal using a fixed font (or an IDE depending on the font - see Table.h for more details). However, the output isn't aligned properly in a Markdown code block (wide to narrow character ratio isn't exactly 2:1) so there's also a '-m' option to print using markdown formatting.

- Note: the terminal output (-p) puts a border line between sections (sections for the kana chart table are groups of related kana symbols, i.e., 'a', 'ka', 'sa', etc.), but for markdown (-m) the rows are in bold instead:

### Kana Conversion Chart

**Notes**:

- Abbreviations used below: Roma=Rōmaji, Hira=Hiragana, Kata=Katakana,
                            Uni=Unicode, Hepb=Hepburn, Kunr=Kunrei
- Roma is mainly 'Modern Hepburn', but can be 'Nihon Shiki' or 'Wāpuro'
- Hepb and Kunr are only populated when they would produce different output
  - Values in () means 'output-only' since inputting leads to a different kana
- 'Roma Variants' are alternative key combinations that lead to the same kana
- When populated, Roma, Hira and Kata columns are unique (no duplicates)
- Unicode values are only shown for 'monograph' entries
- Some 'digraphs' may not be in any real words, but include for completeness
- Chart output is sorted by Hiragana, so 'a, ka, sa, ta, na, ...' ordering
- Katakana 'dakuten w' (ヷ, ヸ, ヹ, ヺ) aren't supported (no conversion exist)
- Type values: P=Plain Kana, D=Dakuten, H=HanDakuten, N=None
- Type 'N' includes:
  - Middle Dot/Interpunct (・): maps to Rōmaji '/' to match IME keyboard entry
  - Prolong Mark (ー): convert to/from macrons (ā, ī, ū, ē, ō)
  - Repeat symbols (ゝ, ゞ, ヽ, ヾ): only supported when 'target' is Rōmaji

| No. | Type | Roma | Hira | Kata | HUni | KUni | Hepb | Kunr | Roma Variants |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| **1** | **P** | **la** | **ぁ** | **ァ** | **3041** | **30A1** |  |  | **xa** |
| 2 | P | a | あ | ア | 3042 | 30A2 |  |  |  |
| 3 | P | li | ぃ | ィ | 3043 | 30A3 |  |  | xi |
| 4 | P | i | い | イ | 3044 | 30A4 |  |  |  |
| 5 | P | ye | いぇ | イェ |  |  |  |  |  |
| 6 | P | lu | ぅ | ゥ | 3045 | 30A5 |  |  | xu |
| 7 | P | u | う | ウ | 3046 | 30A6 |  |  | wu |
| 8 | P | wi | うぃ | ウィ |  |  |  |  |  |
| 9 | P | we | うぇ | ウェ |  |  |  |  |  |
| 10 | P | le | ぇ | ェ | 3047 | 30A7 |  |  | xe, lye, xye |
| 11 | P | e | え | エ | 3048 | 30A8 |  |  |  |
| 12 | P | lo | ぉ | ォ | 3049 | 30A9 |  |  | xo |
| 13 | P | o | お | オ | 304A | 30AA |  |  |  |
| **14** | **P** | **ka** | **か** | **カ** | **304B** | **30AB** |  |  |  |
| 15 | D | ga | が | ガ | 304C | 30AC |  |  |  |
| 16 | P | ki | き | キ | 304D | 30AD |  |  |  |
| 17 | P | kyi | きぃ | キィ |  |  |  |  |  |
| 18 | P | kye | きぇ | キェ |  |  |  |  |  |
| 19 | P | kya | きゃ | キャ |  |  |  |  |  |
| 20 | P | kyu | きゅ | キュ |  |  |  |  |  |
| 21 | P | kyo | きょ | キョ |  |  |  |  |  |
| 22 | D | gi | ぎ | ギ | 304E | 30AE |  |  |  |
| 23 | D | gyi | ぎぃ | ギィ |  |  |  |  |  |
| 24 | D | gye | ぎぇ | ギェ |  |  |  |  |  |
| 25 | D | gya | ぎゃ | ギャ |  |  |  |  |  |
| 26 | D | gyu | ぎゅ | ギュ |  |  |  |  |  |
| 27 | D | gyo | ぎょ | ギョ |  |  |  |  |  |
| 28 | P | ku | く | ク | 304F | 30AF |  |  |  |
| 29 | P | qa | くぁ | クァ |  |  |  |  | kwa |
| 30 | P | qi | くぃ | クィ |  |  |  |  | kwi, qwi |
| 31 | P | qu | くぅ | クゥ |  |  |  |  | kwu, qwu |
| 32 | P | qe | くぇ | クェ |  |  |  |  | kwe, qwe |
| 33 | P | qo | くぉ | クォ |  |  |  |  | kwo, qwo |
| 34 | P | qwa | くゎ | クヮ |  |  |  |  |  |
| 35 | D | gu | ぐ | グ | 3050 | 30B0 |  |  |  |
| 36 | D | gwa | ぐぁ | グァ |  |  |  |  |  |
| 37 | D | gwi | ぐぃ | グィ |  |  |  |  |  |
| 38 | D | gwu | ぐぅ | グゥ |  |  |  |  |  |
| 39 | D | gwe | ぐぇ | グェ |  |  |  |  |  |
| 40 | D | gwo | ぐぉ | グォ |  |  |  |  |  |
| 41 | P | ke | け | ケ | 3051 | 30B1 |  |  |  |
| 42 | D | ge | げ | ゲ | 3052 | 30B2 |  |  |  |
| 43 | P | ko | こ | コ | 3053 | 30B3 |  |  |  |
| 44 | D | go | ご | ゴ | 3054 | 30B4 |  |  |  |
| **45** | **P** | **sa** | **さ** | **サ** | **3055** | **30B5** |  |  |  |
| 46 | D | za | ざ | ザ | 3056 | 30B6 |  |  |  |
| 47 | P | shi | し | シ | 3057 | 30B7 |  | si |  |
| 48 | P | syi | しぃ | シィ |  |  |  |  |  |
| 49 | P | she | しぇ | シェ |  |  |  |  |  |
| 50 | P | sha | しゃ | シャ |  |  |  | sya |  |
| 51 | P | shu | しゅ | シュ |  |  |  | syu |  |
| 52 | P | sho | しょ | ショ |  |  |  | syo |  |
| 53 | D | ji | じ | ジ | 3058 | 30B8 |  | zi |  |
| 54 | D | jyi | じぃ | ジィ |  |  |  |  | zyi |
| 55 | D | je | じぇ | ジェ |  |  |  |  | zye, jye |
| 56 | D | ja | じゃ | ジャ |  |  |  | zya | jya |
| 57 | D | ju | じゅ | ジュ |  |  |  | zyu | jyu |
| 58 | D | jo | じょ | ジョ |  |  |  | zyo | jyo |
| 59 | P | su | す | ス | 3059 | 30B9 |  |  |  |
| 60 | P | swa | すぁ | スァ |  |  |  |  |  |
| 61 | P | swi | すぃ | スィ |  |  |  |  |  |
| 62 | P | swu | すぅ | スゥ |  |  |  |  |  |
| 63 | P | swe | すぇ | スェ |  |  |  |  |  |
| 64 | P | swo | すぉ | スォ |  |  |  |  |  |
| 65 | D | zu | ず | ズ | 305A | 30BA |  |  |  |
| 66 | P | se | せ | セ | 305B | 30BB |  |  |  |
| 67 | D | ze | ぜ | ゼ | 305C | 30BC |  |  |  |
| 68 | P | so | そ | ソ | 305D | 30BD |  |  |  |
| 69 | D | zo | ぞ | ゾ | 305E | 30BE |  |  |  |
| **70** | **P** | **ta** | **た** | **タ** | **305F** | **30BF** |  |  |  |
| 71 | D | da | だ | ダ | 3060 | 30C0 |  |  |  |
| 72 | P | chi | ち | チ | 3061 | 30C1 |  | ti |  |
| 73 | P | tyi | ちぃ | チィ |  |  |  |  |  |
| 74 | P | che | ちぇ | チェ |  |  |  |  | tye |
| 75 | P | cha | ちゃ | チャ |  |  |  | tya |  |
| 76 | P | chu | ちゅ | チュ |  |  |  | tyu |  |
| 77 | P | cho | ちょ | チョ |  |  |  | tyo |  |
| 78 | D | di | ぢ | ヂ | 3062 | 30C2 | (ji) | (zi) |  |
| 79 | D | dyi | ぢぃ | ヂィ |  |  |  |  |  |
| 80 | D | dye | ぢぇ | ヂェ |  |  |  |  |  |
| 81 | D | dya | ぢゃ | ヂャ |  |  | (ja) | (zya) |  |
| 82 | D | dyu | ぢゅ | ヂュ |  |  | (ju) | (zyu) |  |
| 83 | D | dyo | ぢょ | ヂョ |  |  | (jo) | (zyo) |  |
| 84 | P | ltu | っ | ッ | 3063 | 30C3 |  |  | xtu |
| 85 | P | tsu | つ | ツ | 3064 | 30C4 |  | tu |  |
| 86 | P | tsa | つぁ | ツァ |  |  |  |  |  |
| 87 | P | tsi | つぃ | ツィ |  |  |  |  |  |
| 88 | P | tse | つぇ | ツェ |  |  |  |  |  |
| 89 | P | tso | つぉ | ツォ |  |  |  |  |  |
| 90 | D | du | づ | ヅ | 3065 | 30C5 | (zu) | (zu) |  |
| 91 | P | te | て | テ | 3066 | 30C6 |  |  |  |
| 92 | P | thi | てぃ | ティ |  |  |  |  |  |
| 93 | P | the | てぇ | テェ |  |  |  |  |  |
| 94 | P | tha | てゃ | テャ |  |  |  |  |  |
| 95 | P | thu | てゅ | テュ |  |  |  |  |  |
| 96 | P | tho | てょ | テョ |  |  |  |  |  |
| 97 | D | de | で | デ | 3067 | 30C7 |  |  |  |
| 98 | D | dhi | でぃ | ディ |  |  |  |  |  |
| 99 | D | dhe | でぇ | デェ |  |  |  |  |  |
| 100 | D | dha | でゃ | デャ |  |  |  |  |  |
| 101 | D | dhu | でゅ | デュ |  |  |  |  |  |
| 102 | D | dho | でょ | デョ |  |  |  |  |  |
| 103 | P | to | と | ト | 3068 | 30C8 |  |  |  |
| 104 | P | twa | とぁ | トァ |  |  |  |  |  |
| 105 | P | twi | とぃ | トィ |  |  |  |  |  |
| 106 | P | twu | とぅ | トゥ |  |  |  |  |  |
| 107 | P | twe | とぇ | トェ |  |  |  |  |  |
| 108 | P | two | とぉ | トォ |  |  |  |  |  |
| 109 | D | do | ど | ド | 3069 | 30C9 |  |  |  |
| 110 | D | dwa | どぁ | ドァ |  |  |  |  |  |
| 111 | D | dwi | どぃ | ドィ |  |  |  |  |  |
| 112 | D | dwu | どぅ | ドゥ |  |  |  |  |  |
| 113 | D | dwe | どぇ | ドェ |  |  |  |  |  |
| 114 | D | dwo | どぉ | ドォ |  |  |  |  |  |
| **115** | **P** | **na** | **な** | **ナ** | **306A** | **30CA** |  |  |  |
| 116 | P | ni | に | ニ | 306B | 30CB |  |  |  |
| 117 | P | nyi | にぃ | ニィ |  |  |  |  |  |
| 118 | P | nye | にぇ | ニェ |  |  |  |  |  |
| 119 | P | nya | にゃ | ニャ |  |  |  |  |  |
| 120 | P | nyu | にゅ | ニュ |  |  |  |  |  |
| 121 | P | nyo | にょ | ニョ |  |  |  |  |  |
| 122 | P | nu | ぬ | ヌ | 306C | 30CC |  |  |  |
| 123 | P | ne | ね | ネ | 306D | 30CD |  |  |  |
| 124 | P | no | の | ノ | 306E | 30CE |  |  |  |
| **125** | **P** | **ha** | **は** | **ハ** | **306F** | **30CF** |  |  |  |
| 126 | D | ba | ば | バ | 3070 | 30D0 |  |  |  |
| 127 | H | pa | ぱ | パ | 3071 | 30D1 |  |  |  |
| 128 | P | hi | ひ | ヒ | 3072 | 30D2 |  |  |  |
| 129 | P | hyi | ひぃ | ヒィ |  |  |  |  |  |
| 130 | P | hye | ひぇ | ヒェ |  |  |  |  |  |
| 131 | P | hya | ひゃ | ヒャ |  |  |  |  |  |
| 132 | P | hyu | ひゅ | ヒュ |  |  |  |  |  |
| 133 | P | hyo | ひょ | ヒョ |  |  |  |  |  |
| 134 | D | bi | び | ビ | 3073 | 30D3 |  |  |  |
| 135 | D | byi | びぃ | ビィ |  |  |  |  |  |
| 136 | D | bye | びぇ | ビェ |  |  |  |  |  |
| 137 | D | bya | びゃ | ビャ |  |  |  |  |  |
| 138 | D | byu | びゅ | ビュ |  |  |  |  |  |
| 139 | D | byo | びょ | ビョ |  |  |  |  |  |
| 140 | H | pi | ぴ | ピ | 3074 | 30D4 |  |  |  |
| 141 | H | pyi | ぴぃ | ピィ |  |  |  |  |  |
| 142 | H | pye | ぴぇ | ピェ |  |  |  |  |  |
| 143 | H | pya | ぴゃ | ピャ |  |  |  |  |  |
| 144 | H | pyu | ぴゅ | ピュ |  |  |  |  |  |
| 145 | H | pyo | ぴょ | ピョ |  |  |  |  |  |
| 146 | P | fu | ふ | フ | 3075 | 30D5 |  | hu |  |
| 147 | P | fa | ふぁ | ファ |  |  |  |  | fwa, hwa |
| 148 | P | fi | ふぃ | フィ |  |  |  |  | fyi, fwi, hwi |
| 149 | P | fwu | ふぅ | フゥ |  |  |  |  |  |
| 150 | P | fe | ふぇ | フェ |  |  |  |  | fye, fwe, hwe |
| 151 | P | fo | ふぉ | フォ |  |  |  |  | fwo, hwo |
| 152 | P | fya | ふゃ | フャ |  |  |  |  |  |
| 153 | P | fyu | ふゅ | フュ |  |  |  |  |  |
| 154 | P | fyo | ふょ | フョ |  |  |  |  |  |
| 155 | D | bu | ぶ | ブ | 3076 | 30D6 |  |  |  |
| 156 | H | pu | ぷ | プ | 3077 | 30D7 |  |  |  |
| 157 | P | he | へ | ヘ | 3078 | 30D8 |  |  |  |
| 158 | D | be | べ | ベ | 3079 | 30D9 |  |  |  |
| 159 | H | pe | ぺ | ペ | 307A | 30DA |  |  |  |
| 160 | P | ho | ほ | ホ | 307B | 30DB |  |  |  |
| 161 | D | bo | ぼ | ボ | 307C | 30DC |  |  |  |
| 162 | H | po | ぽ | ポ | 307D | 30DD |  |  |  |
| **163** | **P** | **ma** | **ま** | **マ** | **307E** | **30DE** |  |  |  |
| 164 | P | mi | み | ミ | 307F | 30DF |  |  |  |
| 165 | P | myi | みぃ | ミィ |  |  |  |  |  |
| 166 | P | mye | みぇ | ミェ |  |  |  |  |  |
| 167 | P | mya | みゃ | ミャ |  |  |  |  |  |
| 168 | P | myu | みゅ | ミュ |  |  |  |  |  |
| 169 | P | myo | みょ | ミョ |  |  |  |  |  |
| 170 | P | mu | む | ム | 3080 | 30E0 |  |  |  |
| 171 | P | me | め | メ | 3081 | 30E1 |  |  |  |
| 172 | P | mo | も | モ | 3082 | 30E2 |  |  |  |
| **173** | **P** | **lya** | **ゃ** | **ャ** | **3083** | **30E3** |  |  | **xya** |
| 174 | P | ya | や | ヤ | 3084 | 30E4 |  |  |  |
| 175 | P | lyu | ゅ | ュ | 3085 | 30E5 |  |  | xyu |
| 176 | P | yu | ゆ | ユ | 3086 | 30E6 |  |  |  |
| 177 | P | lyo | ょ | ョ | 3087 | 30E7 |  |  | xyo |
| 178 | P | yo | よ | ヨ | 3088 | 30E8 |  |  |  |
| **179** | **P** | **ra** | **ら** | **ラ** | **3089** | **30E9** |  |  |  |
| 180 | P | ri | り | リ | 308A | 30EA |  |  |  |
| 181 | P | ryi | りぃ | リィ |  |  |  |  |  |
| 182 | P | rye | りぇ | リェ |  |  |  |  |  |
| 183 | P | rya | りゃ | リャ |  |  |  |  |  |
| 184 | P | ryu | りゅ | リュ |  |  |  |  |  |
| 185 | P | ryo | りょ | リョ |  |  |  |  |  |
| 186 | P | ru | る | ル | 308B | 30EB |  |  |  |
| 187 | P | re | れ | レ | 308C | 30EC |  |  |  |
| 188 | P | ro | ろ | ロ | 308D | 30ED |  |  |  |
| **189** | **P** | **lwa** | **ゎ** | **ヮ** | **308E** | **30EE** |  |  | **xwa** |
| 190 | P | wa | わ | ワ | 308F | 30EF |  |  |  |
| 191 | P | wyi | ゐ | ヰ | 3090 | 30F0 | (i) | (i) |  |
| 192 | P | wye | ゑ | ヱ | 3091 | 30F1 | (e) | (e) |  |
| 193 | P | wo | を | ヲ | 3092 | 30F2 | (o) | (o) |  |
| 194 | P | n | ん | ン | 3093 | 30F3 |  |  |  |
| 195 | D | vu | ゔ | ヴ | 3094 | 30F4 |  |  |  |
| 196 | D | va | ゔぁ | ヴァ |  |  |  |  |  |
| 197 | D | vi | ゔぃ | ヴィ |  |  |  |  |  |
| 198 | D | ve | ゔぇ | ヴェ |  |  |  |  |  |
| 199 | D | vo | ゔぉ | ヴォ |  |  |  |  |  |
| 200 | D | vya | ゔゃ | ヴャ |  |  |  |  |  |
| 201 | D | vyu | ゔゅ | ヴュ |  |  |  |  |  |
| 202 | D | vyo | ゔょ | ヴョ |  |  |  |  |  |
| 203 | P | lka | ゕ | ヵ | 3095 | 30F5 |  |  | xka |
| 204 | P | lke | ゖ | ヶ | 3096 | 30F6 |  |  | xke |
| **205** | **N** | **/** |  | **・** |  | **30FB** |  |  |  |
| 206 | N |  |  | ー |  | 30FC |  |  |  |
| 207 | N |  | ゝ | ヽ | 309D | 30FD |  |  |  |
| 208 | N |  | ゞ | ヾ | 309E | 30FE |  |  |  |

**Totals**:

- **Monograph:**  86 (Plain=48, Dakuten=21, HanDakuten=5, Small=12)
- **Digraphs:** 118 (Plain=71, Dakuten=42, HanDakuten=5)
- **All Kana:** 204 (Monographs=86, Digraphs=118), Rōmaji Variants=55
- **Types:** 208 (P=131, D=63, H=10, N=4), N types are not included in 'All Kana'

## Kanji Data

To support **kanjiStats** and **kanjiQuiz** programs, *KanjiData* class loads and breaks down kanji into the following categories:

- **Jouyou**: 2136 official Jōyō (常用) kanji
- **Jinmei**: 633 official Jinmeiyō (人名用) kanji
- **LinkedJinmei**: 230 more Jinmei kanji that are old/variant forms of Jōyō (212) or Jinmei (18)
- **LinkedOld**: 213 old/variant Jōyō kanji that aren't in 'Linked Jinmei'
- **Frequency**: kanji that are in the top 2501 frequency list, but not one of the first 4 types
- **Extra**: kanji loaded from 'extra.txt' - shouldn't be in any of the above types
- **Kentei**: kanji loaded from 'kentei/*' - Kanji Kentei (漢字検定) that aren't any of the above types
- **Ucd**: kanji that are in 'ucd.txt', but not already one of the above types
- **None**: kanji that haven't been loaded from any files

### Kanji Class Hierarchy

The following diagram shows the class hierarchy for the **Kanji** class (* = abstract class). There are currently 8 concrete types:

```;
     Kanji*
       |
       +----------------------------------------+
       |                                        |
 NonLinkedKanji*                           LinkedKanji*
       |                                        |
       +------------------------+               +---------------+
       |                        |               |               |
CustomFileKanji*          UcdFileKanji* LinkedJinmeiKanji LinkedOldKanji
       |                        |
       +------------+           +------------+
       |            |           |            |
 OfficialKanji* ExtraKanji StandardKanji* UcdKanji
       |                        |
       +-----------+            +------------+
       |           |            |            |
  JouyouKanji JinmeiKanji FrequencyKanji KenteiKanji
```

The classes derived from **Kanji** add the following fields (* = optional):

- **NonLinkedKanji**: *meaning*, *reading*
- **LinkedKanji**: *frequency*\*, *kyu*, *link* (points to new/standard kanji)
- **CustomFileKanji**: *kyu*, *number*, *oldNames*
- **UcdFileKanji**: *hasOldLinks*, *linkNames*, *linkedReadings*
- **OfficialKanji**: *frequency*\*, *level*, *year*\*
- **ExtraKanji**: *newName*\*
- **StandardKanji**: *kyu*
- **JouyouKanji**: *grade* (school grade when the kanji is introduced)
- **JinmeiKanji**: *reason* (official reason for inclusion in Jinmeiyoō list)
- **FrequencyKanji**: *frequency*

### JLPT Kanji

Note that JLPT level lists are no longer *official* since 2010. Also, each level file only contains uniquely new kanji for the level (as opposed to some N2 and N1 lists on the web that repeat some kanji from earlier levels). Currently the levels have the following number of kanji:

- N5: 103 -- all Jōyō
- N4: 181 -- all Jōyō
- N3: 361 -- all Jōyō
- N2: 415 -- all Jōyō
- N1: 1162 -- 911 Jōyō, 251 Jinmei

All kanji in levels N5 to N2 are in the Top 2501 frequency list, but N1 contains 25 Jōyō and 83 Jinmei kanji that are not in the Top 2501 frequency list.

### Jōyō Kanji

Kyōiku (教育) kanji grades are included in the Jōyō list. Here is a breakdown of the count per grade as well as how many per JLPT level per grade (*None* means not included in any of the JLPT levels)
Grade | Total | N5  | N4  | N3  | N2  | N1  | None
----- | ----- | --- | --- | --- | --- | --- | ----
**1** | 80    | 57  | 15  | 8   |     |     |
**2** | 160   | 43  | 74  | 43  |     |     |
**3** | 200   | 3   | 67  | 130 |     |     |
**4** | 200   |     | 20  | 180 |     |     |
**5** | 185   |     | 2   |     | 149 | 34  |
**6** | 181   |     | 3   |     | 105 | 73  |
**S** | 1130  |     |     |     | 161 | 804 | 165
Total | 2136  | 103 | 181 | 361 | 415 | 911 | 165

Total for all grades is the same as the total Jōyō (2136) and all are in the Top 2501 frequency list except for 99 *S* (Secondary School) kanjis.

The program also loads the 214 official kanji radicals (部首).

### Data Directory

The **data** directory contains the following files:

- **jouyou.txt**: loaded from [here](https://en.wikipedia.org/wiki/List_of_jōyō_kanji) - note, the radicals in this list reflect the original radicals from **Kāngxī Zìdiǎn / 康煕字典（こうきじてん）** so a few characters have the radicals of their old form, i.e., 円 has radical 口 (from the old form 圓).
- **jinmei.txt**: loaded from [here](https://ja.wikipedia.org/wiki/人名用漢字一覧) and most of the readings from [here](https://ca.wikipedia.org/w/index.php?title=Jinmeiyō_kanji)
- **linked-jinmei.txt**: loaded from [here](https://en.wikipedia.org/wiki/Jinmeiyō_kanji)
- **frequency.txt**: top 2501 frequency kanji loaded from [KanjiCards](https://kanjicards.org/kanji-list-by-freq.html)
- **extra.txt**: holds details for 'extra kanji of interest' not already in the above four files
- **ucd.txt**: data extracted from Unicode 'UCD' (see *scripts/parseUcdAllFlat.sh* for details and links)
- **frequency-readings.txt**: holds readings of some Top Frequency kanji that aren't in Jouyou or Jinmei lists
- **radicals.txt**: loaded from [here](http://etc.dounokouno.com/bushu-search/bushu-list.html)
- **strokes.txt**: loaded from [here](https://kanji.jitenon.jp/cat/jimmei.html) - covers Jinmeiyō kanji and some old forms.
- **wiki-strokes.txt**: loaded from [here](https://en.wikipedia.org/wiki/List_of_kanji_by_stroke_count) - mainly Jōyō, but also includes a few 'Frequency' type Kanji.
- **jlpt/n[1-5].txt**: loaded from various sites such as [FreeTag](http://freetag.jp/index_jlpt_kanji_list.html) and [JLPT Study](https://jlptstudy.net/N2/).
- **kentei/k\*.txt**: loaded from [here](https://kanjijoho.com/cat/kyu.html)
- **jukugo/*.txt**: loaded from [here](https://sites.google.com/a/h7a.org/kanjicompounds/)
- **meaning-groups.txt**: meant to hold groups of kanji with related meanings (see *Group.h* for more details) - some ideas came from [here](https://en.wikipedia.org/wiki/List_of_kanji_by_concept)
- **pattern-groups.txt**: meant to hold groups of kanji with related patterns (see *Group.h* for more details)

No external databases are used so far, but while writing some of the code (like in *UnicodeBlock.h* for example), the following links were very useful: [Unicode Office Site - Charts](https://www.unicode.org/charts/) and [Compat](https://www.compart.com/en/unicode/).

## Kanji Quiz

The **kanjiQuiz** program supports running various types of quizzes (in review or test mode) as well as looking up details of a kanji from the command-line. If no options are provided then the user is prompted for mode, quiz type, etc. or command-line options can be used to jump directly to the desired type of quiz or kanji lookup. The following is the output from the `-h` (help) option:

```;
kanjiQuiz [-hs] [-f[1-5] | -g[1-6s] | -k[1-9a-c] | -l[1-5] -m[1-4] | -p[1-4]]
          [-r[num] | -t[num]] [kanji]
    -h   show this help message for command-line options
    -s   show English meanings by default (can be toggled on/off later)

  The following options allow choosing the quiz/review type optionally followed
  by question list type (grade, level, etc.) instead of being prompted:
    -f   'frequency' (optional frequency group '1-5')
    -g   'grade' (optional grade '1-6', 's' = Secondary School)
    -k   'kyu' (optional Kentei Kyu '1-9', 'a' = 10, 'b' = 準１級, 'c' = 準２級)
    -l   'level' (optional JLPT level number '1-5')
    -m   'meaning' (optional kanji type '1-4')
    -p   'pattern' (optional kanji type '1-4')

  The following options can be followed by a 'num' to specify where to start in
  the question list (use negative to start from the end or 0 for random order).
    -r   review mode
    -t   test mode

  kanji  show details for a kanji instead of starting a review or test

Examples:
  kanjiQuiz -f        # start 'frequency' quiz (prompts for 'bucket' number)
  kanjiQuiz -r40 -l1  # start 'JLPT N1' review beginning at the 40th entry

Note: 'kanji' can be UTF-8, frequency (between 1 and 2501), 'm' followed by
Morohashi ID (index in Dai Kan-Wa Jiten), 'n' followed by Classic Nelson ID
or 'u' followed by Unicode. For example, theses all produce the same output:
  kanjiQuiz 奉
  kanjiQuiz 1624
  kanjiQuiz m5894
  kanjiQuiz n212
  kanjiQuiz u5949
```

When using the quiz program to lookup a kanji, the output includes a brief legend followed by some details such as **Radical**, **Strokes**, **Pinyin**, **Frequency**, **Old** or **New** variants, **Meaning**, **Reading**, etc.. The **Similar** list comes from the *pattern-groups.txt* file and (the very ad-hoc) **Category** comes from the *meaning-groups.txt* file. **Morohashi** and **Nelson** IDs are shown if they exist as well as any **Jukugo** examples loaded from *data/jukugo* files (there are only about 18K *Jukugo* entries so these lists are pretty limited).

```;
~/cdev/kanji-tools $ ./build/apps/kanjiQuiz 龍
>>> Legend:
Fields: N[1-5]=JLPT Level, K[1-10]=Kentei Kyu, G[1-6]=Grade (S=Secondary School)
Suffix: .=常用 '=JLPT "=Freq ^=人名用 ~=LinkJ %=LinkO +=Extra @=検定 #=1級 *=Ucd

Showing details for 龍 [9F8D], Block CJK, Version 1.1, LinkedJinmei
Rad 龍(212), Strokes 16, lóng, Frq 1734, New 竜*
    Meaning: dragon
    Reading: リュウ、たつ
    Similar: 襲. 籠. 寵^ 瀧~ 朧+ 聾@ 壟# 蘢# 隴# 瓏#
  Morohashi: 48818
 Nelson IDs: 3351 5440
   Category: [動物：爬虫類]
     Jukugo: 龍頭蛇尾（りゅうとうだび） 烏龍茶（うーろんちゃ） 画龍点睛（がりょうてんせい）
```

Here are some runtime memory and (statically linked) file size stats for **kanjiQuiz**. These stats are more relevant for this program compared to the others since it loads more Kanji related data including all *groups* and *jukugo*. *Sanitize* stats are only available for *Clang* - they cause a lot more runtime memory to be used, but slightly smaller debug binary file size (presumably because of less inlining).

### Kanji Quiz Runtime Memory

| Compiler | Debug Sanitize | Debug | Release |
| --- | --- | --- | --- |
| Clang | 126.8 MB | 30.0 MB | 30.7 MB |
| GCC | | 41.9 MB | 41.8 MB |

### Kanji Quiz Binary File Size

| Compiler | Debug Sanitize | Debug | Release |
| --- | --- | --- | --- |
| Clang | 8.0 MB | 8.3 MB | 744 KB |
| GCC | | 4.3 MB | 983 KB |

## Kanji Stats

The **kanjiStats** program takes a list of one or more files (or directories) and outputs a summary of counts of various types of multi-byte characters. More detailed information can also be shown depending on command line options. In order to get more accurate stats about percentages of *Kanji*, *Hiragana* and *Katakana*, the program attempts to strip away all *Furigana* before counting.

Here is the output from processing a set of files containing lyrics for *中島みゆき (Miyuki Nakajima)* songs:

```;
~/cdev/kanji-tools $ ./build/apps/kanjiStats ~/songs
>>> Stats for: 'songs' (634 files from 62 directories) - showing top 5 Kanji per type
>>> Furigana Removed: 436, Combining Marks Replaced: 253, Variation Selectors: 0
>>>         Hiragana: 146379, unique:   77
>>>         Katakana:   9315, unique:   79
>>>     Common Kanji:  52406, unique: 1642, 100.00%
>>>        [Jouyou] :  50804, unique: 1398,  96.94%  (人 1440, 私 836, 日 785, 見 750, 何 626)
>>>        [Jinmei] :    986, unique:  114,   1.88%  (逢 95, 叶 68, 淋 56, 此 44, 遥 42)
>>>  [LinkedJinmei] :     36, unique:    7,   0.07%  (駈 13, 龍 10, 遙 5, 凛 3, 國 2)
>>>     [Frequency] :    203, unique:   15,   0.39%  (嘘 112, 叩 15, 呑 15, 頬 12, 叱 11)
>>>         [Extra] :    377, unique:  108,   0.72%  (怯 29, 騙 21, 囁 19, 繋 19, 禿 16)
>>>   MB-Punctuation:    946, unique:   12
>>>        MB-Symbol:     13, unique:    2
>>>        MB-Letter:   1429, unique:   54
>>> Total Kana+Kanji: 208100 (Hiragana: 70.3%, Katakana: 4.5%, Kanji: 25.2%)
```

### Aozora

There is also a **tests/stats/sample-data** directory that contains files used for testing. The **wiki-articles** directory contains text from several wiki pages and **books** contains text from books found on [青空文庫 (Aozora Bunko)](https://www.aozora.gr.jp/) (with *furigana* preserved in wide brackets).

The books pulled from Aozora were in Shift JIS format so the following steps were used on *macOS* to convert them to UTF-8:

- Load the HTML version of the book in **Safari**
- Select All, then Copy-Paste to **Notes** - this keeps the *furigana*, but puts it on a separate line
- Open *file1* in **Terminal** using *vi* and paste in the text from **Notes**, then save and exit.
  - Copying straight from the browser to *vi* puts the *furigana* immediately after the kanji (with no space, brackets, newline, etc.) which makes it pretty much impossible to 'regex' it out when producing stats (and difficult to read as well).
  - Extremely rare kanji that are just embedded images in the HTML (instead of real Shift JIS values) do show up in **Notes**, but of course they don't end up getting pasted into the plain text file in *vi*. These need to be entered by hand (since they do exist in Unicode).
  - **MS Word** also captures the *furigana* from the HTML, but it ends up being above unrelated text. When pasting to *vi* the *furigana* is put in standard brackets, but in incorrect locations which makes it useless (but at least it can be easily removed which is better than the straight to *vi* option). However, a more serious problem is that **MS Word** (macOS version 2019) also seemed to randomly drop parts of the text (maybe an encoding conversion issue?) which was a showstopper.
- Run the **kanjiFormat** program (from *build/apps*) on *file1* and redirect the output to *file2*
- *file2* should now have properly formatted *furigana* in wide brackets following the *kanji sequence* on the same line.
- run 'fold *file2*>*file1*' to split up the really long lines to 80 columns.

## Helpful Commands

Helpful commands for re-ordering columns, re-numbering (assuming header and first column should be numbered starting at 1) and converting double byte to single byte:

```bash
awk -F'[\t]' -v OFS="\t" '{print $1,$2,$4,$5,$3,$6,$7,$8,$9}' file
awk -F'[\t]' -v OFS="\t" 'NR==1{print}NR>1{for(i=1;i<=NF;i++) printf "%s",(i>1 ? OFS $i : NR-1);print ""}' file
cat file|tr '１２３４５６７８９０' '1234567890'|tr -d '画'
```
