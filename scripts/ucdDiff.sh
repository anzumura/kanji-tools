#!/usr/bin/env bash

declare -r program="ucdDiff.sh"

# Location of 'ucd.all.flat.zip': https://unicode.org/Public/UCD/latest/ucdxml
# More info on UCD file: https://unicode.org/reports/tr38/

# default value for UCD file
inputFile=~/ucd/14/ucd.all.flat.xml

function usage() {
  echo "$1"
  echo ""
  echo "usage: $program [-f 'input file'] 'code point' ['code point']"
  echo "  code point: valid hex unicode code point like '4db5' or '20B9F'"
  echo "  input file: ucd.all.flat.xml file (see script comments for details)"
  echo ""
  echo "if only one code point is specified then print all fields"
  echo "if two code points are specified then only print differences"
  echo "if 'input file' is not specified then it defaults to: $inputFile"
  exit 1
}

if [[ $1 = -f ]]; then
  [[ $# -lt 2 ]] && usage "'-f' must be followed by a UCD file"
  [[ -f $2 ]] || usage "'$2' ia not a valid file"
  inputFile="$2"
  shift 2
fi

[[ $# -lt 1 ]] && usage "must specify at least one code point"
[[ $# -gt 2 ]] && usage "too many arguments"

function getXml() {
  echo $1 | grep -qE '^[1-9A-E]?[0-9A-F]{4}$' || usage "invalid code point '$1'"
  xml=$(grep "cp=\"$1\"" "$inputFile") || usage "'$1' not found in UCD file"
  xml="$(echo "${xml%/>}" | sed 's/" /"\n/g' | grep -v '<char cp=')"
}

declare -r firstCode="${1^^}"
getXml "$firstCode"

if [[ $# -lt 2 ]]; then
  echo "$xml"
  exit 0
fi

declare -r firstXml="$xml" secondCode="${2^^}"
getXml "$secondCode"

declare -r diffOut="$(diff <(echo "$firstXml") <(echo "$xml"))"

function diffs() {
  local -r s="$(echo "$diffOut" | grep "^$1" | sed "s/$1/ /")"
  [[ -n $s ]] && echo -e "$2\n$s"
}

diffs '<' $firstCode
diffs '>' $secondCode
