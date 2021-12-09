# C++ kanji Tools

This repository contains code for four 'main' programs:

- **kanaConvert**: program that converts between Hiragana, Katakana and Rōmaji
- **kanjiFormat**: program used to format test/sample-data/books files (from 青空文庫 - see below)
- **kanjiQuiz**: interactive program that allows a user to choose from various types of quizzes
- **kanjiStats**: classifies and counts multi-byte characters in a file or directory tree

The initial goal for this project was to create a program that could parse multi-byte (UTF-8) input and classify Japanese **kanji** (漢字) characters into *official* categories in order to determine how many kanji fall into each category in real-world examples. The *quiz* program was added later once the initial work was done for loading and classifying kanji. The *format* program was created to help with a specific use-case that came up while gathering sample text from Aozora - it's a small program that relies on some of the generic code already created for the *stats* program.

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

## **Kana Conversion Chart**

### **Notes:**

- Roma=Rōmaji, Hira=Hiragana, Kata=Katakana, Uni=Unicode, Hepb=Hepburn, Kunr=Kunrei
- Roma is mainly 'Modern Hepburn', but can be 'Nihon Shiki' or 'Wāpuro' in some cases
- Hepb and Kunr are only populated when they would produce different output
  - Values in () means 'output-only' since inputting leads to a different kana
- 'Roma Variants' are alternative keyboard combinations that lead to the same kana
- When populated, Roma, Hira and Kata columns are unique (no duplicates)
- Unicode values are only shown for 'monograph' entries
- Some 'digraphs' may not be in any real words, but they are typeable and thus included
- Chart output is sorted by Hiragana, so 'a, ka, sa, ta, na, ...' ordering
- Katakana 'dakuten w' (ヷ, ヸ, ヹ, ヺ) aren't supported (no standard Hiragana or Romaji)
- Type values: P=Plain Kana, D=Dakuten, H=HanDakuten, N=None
- Type 'N' includes:
  - Middle Dot/Interpunct (・): maps to Rōmaji '/' to match usual IME keyboard entry
  - Prolong Mark (ー): conversion via macrons (ā, ī, ū, ē, ō) so no single Rōmaji value
  - Repeat symbols (ゝ, ゞ, ヽ, ヾ): conversion only supported when 'target' is Rōmaji

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

### **Totals:**

- **Monograph:**  86 (Plain=48, Dakuten=21, HanDakuten=5, Small=12)
- **Digraphs:** 118 (Plain=71, Dakuten=42, HanDakuten=5)
- **All Kana:** 204 (Monographs=86, Digraphs=118), Rōmaji Variants=55
- **Types:** 208 (P=131, D=63, H=10, N=4), N types are not included in 'All Kana'

To support these programs, *KanjiData* class loads and breaks down kanji into the following categories:

- **Jouyou**: 2136 official Jōyō (常用) kanji
- **Jinmei**: 633 official Jinmeiyō (人名用) kanji
- **LinkedJinmei**: 230 more Jinmei kanji that are old/variant forms of Jōyō (212) or Jinmei (18)
- **LinkedOld**: 213 old/variant Jōyō kanji that aren't in 'Linked Jinmei'
- **Frequency**: kanji that are in the top 2501 frequency list, but not one of the first 4 types
- **Extra**: kanji loaded from 'extra.txt' - shouldn't be in any of the above types
- **Kentei**: kanji loaded from 'kentei/*' - Kanji Kentei (漢字検定) that aren't any of the above types
- **Ucd**: kanji that are in 'ucd.txt', but not already one of the above types
- **None**: kanji that haven't been loaded from any files

Class Hierarchy for **Kanji** class (* = abstract class):

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

The program also loads the 214 official kanji radicals (部首).

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
- **meaning-groups.txt**: meant to hold groups of kanji with related meanings (see *Group.h* for more details)
- **pattern-groups.txt**: meant to hold groups of kanji with related patterns (see *Group.h* for more details)

No external databases are used so far, but while writing some of the code (like in *UnicodeBlock.h* for example), the following links were very useful: [Unicode Office Site - Charts](https://www.unicode.org/charts/) and [Compat](https://www.compart.com/en/unicode/).

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

Note that JLPT level lists are no longer *official* since 2010. Also, each level file only contains uniquely new kanji for the level (as opposed to some N2 and N1 lists on the web that repeat some kanji from earlier levels). Currently the levels have the following number of kanji:

- N5: 103 -- all Jōyō
- N4: 181 -- all Jōyō
- N3: 361 -- all Jōyō
- N2: 415 -- all Jōyō
- N1: 1162 -- 911 Jōyō, 251 Jinmei

All kanji in levels N5 to N2 are in the Top 2501 frequency list, but N1 contains 25 Jōyō and 83 Jinmei kanji that are not in the Top 2501 frequency list.

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

Helpful commands for re-ordering columns, re-numbering (assuming header and first column should be numbered starting at 1) and converting double byte to single byte:

```bash
awk -F'[\t]' -v OFS="\t" '{print $1,$2,$4,$5,$3,$6,$7,$8,$9}' file
awk -F'[\t]' -v OFS="\t" 'NR==1{print}NR>1{for(i=1;i<=NF;i++) printf "%s",(i>1 ? OFS $i : NR-1);print ""}' file
cat file|tr '１２３４５６７８９０' '1234567890'|tr -d '画'
```
