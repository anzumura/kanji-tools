#!/usr/bin/env bash

declare -r program="parseUcdAllFlat.sh"

# This script searches the Unicode 'ucd.all.flat.xml' file for characters that
# have a Japanese reading (On or Kun) and prints out a tab-separated line with
# the following 7 values:
# - Name: character in utf8
# - Radical: radical number (1 to 214)
# - Strokes: total strokes (including the radical)
# - Joyo: 'Y' (if part of 2010 Jōyō list) or blank
# - Meaning: semicolin separated English definitions
# - On: space-separated Japanese On readings (in all-caps Rōmaji)
# - Kun: space-separated Japanese Kun readings (in all-caps Rōmaji)
# Location of 'ucd.all.flat.zip': https://unicode.org/Public/UCD/latest/ucdxml
# This script was used to create 'data/ucd.txt'.

if [ $# -lt 1 ]; then
  echo "please specify 'ucd.all.flat.xml' file to parse"
  exit 1
fi

function get() {
  unset -v $2
  eval $(echo "$1" | grep -o " $2=\"[^\"]*\"")
}

echo -e "Name\tRadical\tStrokes\tJoyo\tMeaning\tOn\tKun"

# There are 13,392 characters with On or Kun readings in ucd.all.flat.xml plus
# one more, 𠮟 (U+20B9F), marked as a kJoyoKanji that currently doesn't have any
# On/Kun readings since it seems to be a variant of 叱 (U+53F1, しか-る).  Don't
# do any more filtering since other 'Japanese' source type tags either filter
# too much or don't make much of a difference. For example:
# - 'kNelson' (Classic 'Nelson Japanese-English Character Dictionary') reduced
#   to 5,331, but this also filtered out 7 Jōyō and 48 Jinmei Kanji. Nelson has
#   5,398 entries, but not all of them have a Japanese reading.
# - 'kJis0' produced 6,355, but still missed 4 Jōyo and 15 Jinmei.
# - 'kMorohashi' produced 12,966 Kanji having On or Kun (out of 18,168 total)
#   so this isn't much of a reduction compared to just using On/Kun filtering.
# - 'kIRGDaiKanwaZiten' resulted in 12,943 (not much of a reduction) and there's
#   also a proposal to remove this property (and expand 'kMorohashi') so best to
#   avoid using it: https://www.unicode.org/L2/L2021/21032-unihan-kmorohashi.pdf
grep -E '(kJoyoKanji="[^"]|kJapanese[OK].*n="[^"])' $1 |
  while read -r i; do
    get "$i" cp
    get "$i" kRSUnicode
    get "$i" kTotalStrokes
    get "$i" kJoyoKanji
    get "$i" kDefinition
    get "$i" kJapaneseOn
    get "$i" kJapaneseKun
    echo -e "\U$cp\t${kRSUnicode%%[.\']*}\t$kTotalStrokes\t${kJoyoKanji:+Y}\t\
$kDefinition\t$kJapaneseOn\t$kJapaneseKun"
  done
