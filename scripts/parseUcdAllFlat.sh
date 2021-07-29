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
# More info on UCD file: https://unicode.org/reports/tr38/
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

# declare arrays to help support links in kJoyoKanji
declare -A definition on kun

# First filter to only get Japan sourced Kanji (kIRG_JSource) - this reduces the
# huge set down to 16,226 entries. Then filter for characters with at least one
# Japanese reading (On/Kun) or a value in kJoyoKanji. This further reduces the
# set to 12,276. There are actually 1,117 entries with On/Kun that don't have
# JSource like 4E13 (专) and 4E20 (丠), but don't load them for now).
# Don't do any more filtering since other 'Japan' source tags either filter too
# much or don't make much of a difference. For example:
# - 'kNelson' (Classic 'Nelson Japanese-English Character Dictionary') has 5,398
#   entries (5,331 with On/Kun), but missed 7 Jōyō and 48 Jinmei Kanji.
# - 'kJis0' has 6,356 (6,354 with On/Kun), but missed 4 Jōyō and 15 Jinmei.
# - 'kMorohashi' has 18,168 (12,966 with On/Kun) so this isn't much different
#   compared to just using On/Kun filtering.
# - 'kIRGDaiKanwaZiten' has 17,864 (12,942 with On/Kun). There's also a proposal
#   to remove this property (and expand 'kMorohashi') so it's probably best not
#   to use it: https://www.unicode.org/L2/L2021/21032-unihan-kmorohashi.pdf
# - 'kRSAdobe_Japan1_6' has 13,457 (12,357 have On/Kun).
# Note: IRG = 'Ideographic Research Group', a part of ISO/IEC JTC1/SC2/WG2.
grep 'kIRG_JSource="[^"]' $1 | grep -E '(kJoyoKanji="[^"]|kJapanese[OK].*n="[^"])' |
  while read -r i; do
    get "$i" cp
    get "$i" kRSUnicode
    get "$i" kTotalStrokes
    get "$i" kJoyoKanji
    get "$i" kDefinition
    get "$i" kJapaneseOn
    get "$i" kJapaneseKun
    if [ -n "$kJoyoKanji" ]; then
      if [[ $kJoyoKanji =~ U+ ]]; then
        # There are 4 entries that have a Joyo link: 5265, 53F1, 586B and 982C
        link=${kJoyoKanji#U+}
        # Store definition and on/kun for linked joyo since since these values
        # are missing for 𠮟 (U+20B9F) which replaces 叱 (53F1) しか-る.
        definition[$link]="$kDefinition"
        on[$link]="$kJapaneseOn"
        kun[$link]="$kJapaneseKun"
        # Need to unset kJoyoKanji for this entry since the official version
        # is the 'link' entry.
        unset -v kJoyoKanji
      elif [ -n "${definition[$cp]}" ]; then
        kDefinition=${kDefinition:-${definition[$cp]}}
        kJapaneseOn=${kJapaneseOn:-${on[$cp]}}
        kJapaneseKun=${kJapaneseKun:-${kun[$cp]}}
      fi
    fi
    echo -e "\U$cp\t${kRSUnicode%%[.\']*}\t$kTotalStrokes\t${kJoyoKanji:+Y}\t\
$kDefinition\t$kJapaneseOn\t$kJapaneseKun"
  done
