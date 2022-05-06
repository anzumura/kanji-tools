**Notes:**

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
| 47 | P | shi | し | シ | 3057 | 30B7 |  | si | si |
| 48 | P | syi | しぃ | シィ |  |  |  |  |  |
| 49 | P | she | しぇ | シェ |  |  |  |  |  |
| 50 | P | sha | しゃ | シャ |  |  |  | sya | sya |
| 51 | P | shu | しゅ | シュ |  |  |  | syu | syu |
| 52 | P | sho | しょ | ショ |  |  |  | syo | syo |
| 53 | D | ji | じ | ジ | 3058 | 30B8 |  | zi | zi |
| 54 | D | jyi | じぃ | ジィ |  |  |  |  | zyi |
| 55 | D | je | じぇ | ジェ |  |  |  |  | zye, jye |
| 56 | D | ja | じゃ | ジャ |  |  |  | zya | zya, jya |
| 57 | D | ju | じゅ | ジュ |  |  |  | zyu | zyu, jyu |
| 58 | D | jo | じょ | ジョ |  |  |  | zyo | zyo, jyo |
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
| 72 | P | chi | ち | チ | 3061 | 30C1 |  | ti | ti |
| 73 | P | tyi | ちぃ | チィ |  |  |  |  |  |
| 74 | P | che | ちぇ | チェ |  |  |  |  | tye |
| 75 | P | cha | ちゃ | チャ |  |  |  | tya | tya |
| 76 | P | chu | ちゅ | チュ |  |  |  | tyu | tyu |
| 77 | P | cho | ちょ | チョ |  |  |  | tyo | tyo |
| 78 | D | di | ぢ | ヂ | 3062 | 30C2 | (ji) | (zi) |  |
| 79 | D | dyi | ぢぃ | ヂィ |  |  |  |  |  |
| 80 | D | dye | ぢぇ | ヂェ |  |  |  |  |  |
| 81 | D | dya | ぢゃ | ヂャ |  |  | (ja) | (zya) |  |
| 82 | D | dyu | ぢゅ | ヂュ |  |  | (ju) | (zyu) |  |
| 83 | D | dyo | ぢょ | ヂョ |  |  | (jo) | (zyo) |  |
| 84 | P | ltu | っ | ッ | 3063 | 30C3 |  |  | xtu |
| 85 | P | tsu | つ | ツ | 3064 | 30C4 |  | tu | tu |
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
| 146 | P | fu | ふ | フ | 3075 | 30D5 |  | hu | hu |
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

**Totals:**

- **Monographs:** 86 (Plain=48, Dakuten=21, HanDakuten=5, Small=12)
- **Digraphs:** 118 (Plain=71, Dakuten=42, HanDakuten=5)
- **All Kana:** 204 (Monographs=86, Digraphs=118), Rōmaji Variants=55)
- **Types:** 208 (P=131, D=63, H=10, N=4), N types aren't included in 'All Kana'
