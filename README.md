# C++ kanji lists

Program that loads information about Kanji and supports various types of tests. The program classifies Kanjis into the following categories:
- **Jouyou**: 2136 official Jōyō (常用) kanji
- **Jinmei**: 633 official Jinmeiyō (人名用) kanji
- **Linked Jinmei**: 230 more Jinmei kanji that are old/variant forms of Jōyō (212) or Jinmei (18)
- **Linked Old**: 213 old/variant Jōyō kanji that aren't in 'Linked Jinmei'
- **Other**: kanji that are in the top 2501 frequency list, but not one of the first 4 types
- **Extra**: kanji loaded from 'extra.txt' - shouldn't be any of the above types
- **None**: used for kanji that haven't been loaded from any files

The program also loads the 214 official kanji radicals (部首).

The data directory contains the following files:
- **jouyou.txt**: loaded from [here](https://en.wikipedia.org/wiki/List_of_jōyō_kanji)
- **jinmei.txt**: loaded from [here](https://ja.wikipedia.org/wiki/人名用漢字一覧)
- **linked.jinmei.txt**: loaded from [here](https://en.wikipedia.org/wiki/Jinmeiyō_kanji)
- **strokes.txt**: loaded from [here](https://kanji.jitenon.jp/cat/jimmei.html) - covers Jinmeiyō kanji and some old forms.
- **wiki-strokes.txt**: loaded from [here](https://en.wikipedia.org/wiki/List_of_kanji_by_stroke_count) - mainly Jōyō, but also includes a few 'Other' type Kanji.
- **radicals.txt**: loaded from [here](http://etc.dounokouno.com/bushu-search/bushu-list.html)
- **frequency.txt**: top 2501 frequency kanji loaded from [KanjiCards](https://kanjicards.org/kanji-list-by-freq.html)
- **n[1-5].txt**: loaded from various sites such as [FreeTag](http://freetag.jp/index_jlpt_kanji_list.html) and [JLPT Study](https://jlptstudy.net/N2/).
- **extra.txt**: meant to hold any extra kanji of interest not in other files
- **meaning-groups.txt**: meant to hold groups of kanji with related meanings (see Data.h for more details)
- **pattern-groups.txt**: meant to hold groups of kanji with related patterns (see Data.h for more details)

Note that JLPT level lists are no longer *official* since 2010. Also, each level file only contains uniquely new kanji for the level (instead some N2 and N1 lists on the web that repeat some kanji from earlier levels). Currently the levels have the following number of kanji (with the count per type shown in brackets):
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
```
awk -F'[\t]' -v OFS="\t" '{print $1,$2,$4,$5,$3,$6,$7,$8,$9}' file
awk -F'[\t]' -v OFS="\t" 'NR==1{print}NR>1{for(i=1;i<=NF;i++) printf "%s",(i>1 ? OFS $i : NR-1);print ""}' file
cat file|tr '１２３４５６７８９０' '1234567890'|tr -d '画'
```
