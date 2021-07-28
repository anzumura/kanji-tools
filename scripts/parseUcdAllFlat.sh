#!/usr/bin/env bash

program="parseUcdAllFlat.sh"

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
  # It would be nice to use grep -Po '(?<=tag=")[^"]*'', but macOS grep doesn't
  # -P (perl regex) flag. Homebrew 'ggrep' does support it though ...
  value=$(echo "$1"|grep -o $2=\"[^\"]*|cut -d'"' -f2)
  eval $2=\"$value\"
}

echo -e "Name\tRadical\tStrokes\tJoyo\tMeaning\tOn\tKun"

# There are 13,371 characters with On or Kun in BMP. For now, only include ones
# that have a value for 'kNelson' (Classic Nelson Japanese Dictionary). Nelson
# has 5,398 entries and 5,331 of these have at least one Japanese reading.
grep 'kNelson="[^"]' $1|grep -E '(kJapaneseKun="[^"]|kJapaneseOn="[^"])'|
while read -r i; do
  get "$i" cp
  # For now only support values from BMP (so codes with 4 hex). It looks like
  # there are actually only 21 Kanji beyond BMP with Japanese readings.
  if [ ${#cp} -eq 4 ]; then
    get "$i" kRSUnicode
    get "$i" kTotalStrokes
    get "$i" kJoyoKanji
    get "$i" kDefinition
    get "$i" kJapaneseOn
    get "$i" kJapaneseKun
    echo -e "\u$cp\t${kRSUnicode%%[.\']*}\t$kTotalStrokes\t${kJoyoKanji:+Y}\t\
$kDefinition\t$kJapaneseOn\t$kJapaneseKun"
  fi
done
