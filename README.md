# C++ kanji

This repository contains code for two 'main' programs:
- **kanjiQuiz**: interactive program that allows a user to choose from various types of quizzes
- **kanjiFormat**: program used to format test/sample-data/books files (from 青空文庫 - see below)
- **kanjiStats**: classifies and counts multi-byte characters in a file or directory tree

To support these programs, *KanjiData* class loads and breaks down **kanji** (漢字) into the following categories:
- **Jouyou**: 2136 official Jōyō (常用) kanji
- **Jinmei**: 633 official Jinmeiyō (人名用) kanji
- **Linked Jinmei**: 230 more Jinmei kanji that are old/variant forms of Jōyō (212) or Jinmei (18)
- **Linked Old**: 213 old/variant Jōyō kanji that aren't in 'Linked Jinmei'
- **Other**: kanji that are in the top 2501 frequency list, but not one of the first 4 types
- **Extra**: kanji loaded from 'extra.txt' - shouldn't be in any of the above types
- **None**: used to classify kanji that haven't been loaded from any files

The program also loads the 214 official kanji radicals (部首).

The **data** directory contains the following files:
- **jouyou.txt**: loaded from [here](https://en.wikipedia.org/wiki/List_of_jōyō_kanji) - note, the radicals in this list reflect the original radicals from **Kāngxī Zìdiǎn / 康煕字典（こうきじてん）** so a few characters have the radicals of their old form, i.e., 円 has radical 口 (from the old form 圓).
- **jinmei.txt**: loaded from [here](https://ja.wikipedia.org/wiki/人名用漢字一覧) and most of the readings from [here](https://ca.wikipedia.org/w/index.php?title=Jinmeiyō_kanji)
- **linked-jinmei.txt**: loaded from [here](https://en.wikipedia.org/wiki/Jinmeiyō_kanji)
- **strokes.txt**: loaded from [here](https://kanji.jitenon.jp/cat/jimmei.html) - covers Jinmeiyō kanji and some old forms.
- **wiki-strokes.txt**: loaded from [here](https://en.wikipedia.org/wiki/List_of_kanji_by_stroke_count) - mainly Jōyō, but also includes a few 'Other' type Kanji.
- **radicals.txt**: loaded from [here](http://etc.dounokouno.com/bushu-search/bushu-list.html)
- **frequency.txt**: top 2501 frequency kanji loaded from [KanjiCards](https://kanjicards.org/kanji-list-by-freq.html)
- **n[1-5].txt**: loaded from various sites such as [FreeTag](http://freetag.jp/index_jlpt_kanji_list.html) and [JLPT Study](https://jlptstudy.net/N2/).
- **extra.txt**: meant to hold any extra kanji of interest not in other files
- **other-readings.txt**: holds readings of some Top Frequency kanji that aren't in Jouyou or Jinmei lists
- **meaning-groups.txt**: meant to hold groups of kanji with related meanings (see *Group.h* for more details)
- **pattern-groups.txt**: meant to hold groups of kanji with related patterns (see *Group.h* for more details)

There is also a **tests/sample-data** directory that contains files used for testing. The **wiki-articles** directory contains text from several wiki pages and **books** contains text from books found on [青空文庫 (Aozora Bunko)](https://www.aozora.gr.jp/) (with *furigana* preserved in wide brackets).

The books pulled from Aozora were in Shift JIS format so the following steps were used on *macOS* to convert them to UTF-8:
- Load the HTML version of the book in **Safari**
- Select All, then Copy-Paste to **Notes** - this keeps the *furigana*, but puts it on a separate line
- Open *file1* in **Terminal** using *vi* and paste in the text from **Notes**, then save and exit.
- Run the **kanjiFormat** program (from *build/apps*) on *file1* and redirect the output to *file2*
- *file2* should now have properly formatted *furigana* in wide brackets following the *kanji sequence* on the same line.

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
```
awk -F'[\t]' -v OFS="\t" '{print $1,$2,$4,$5,$3,$6,$7,$8,$9}' file
awk -F'[\t]' -v OFS="\t" 'NR==1{print}NR>1{for(i=1;i<=NF;i++) printf "%s",(i>1 ? OFS $i : NR-1);print ""}' file
cat file|tr '１２３４５６７８９０' '1234567890'|tr -d '画'
```
