#!/usr/bin/env bash

declare -r program="ucdDiff.sh"

# Location of 'ucd.all.flat.zip': https://unicode.org/Public/UCD/latest/ucdxml
# More info on UCD file: https://unicode.org/reports/tr38/

# default value for UCD file
declare -r defaultFile=ucd.all.flat.xml

oldFile=~/ucd/13/$defaultFile
newFile=~/ucd/14/$defaultFile

function usage() {
  echo "$program: $1"
  echo ""
  echo "usage: $program [-f 'input file'] 'code point' ['code point']"
  echo "       $program -d [-o 'old file'] [-n 'new file'] 'code point'"
  echo "  -f 'input file': default: '$newFile'"
  echo "  -d: compare the same code point from two different UCD files (useful"
  echo "      to see diffs across Unicode versions)."
  echo "  -o 'old file': default '$oldFile'"
  echo "  -n 'new file': default '$newFile'"
  echo ""
  echo "if only one 'code point' is specified then print all fields, otherwise"
  echo "just print the fields that are different. Note, 'code point' should be"
  echo "valid hex unicode code points like '4db5' or '20B9F'."
  exit 1
}

function checkFile() {
  [[ $# -lt 2 ]] && usage "'$1' must be followed by a UCD file"
  [[ -f $2 ]] || usage "'$2' ia not a valid file"
}

if [[ $1 = -d ]]; then
  shift
  while true; do
    if [[ $1 = -o ]]; then
      checkFile "$@"
      oldFile=$2
      shift 2
    elif [[ $1 = -n ]]; then
      checkFile "$@"
      newFile=$2
      shift 2
    else
      break
    fi
  done
  [[ $oldFile = $newFile ]] && usage "'old file' is the same as 'new file'"
  [[ $# -gt 1 ]] && usage "too many arguments for '-d'"
else
  if [[ $1 = -f ]]; then
    checkFile "$@"
    oldFile=$2
    newFile=$2
    shift 2
  else
    oldFile=$newFile
  fi
  [[ $# -gt 2 ]] && usage "too many arguments"
fi

[[ $# -lt 1 ]] && usage "must specify at least one code point"

function getXml() {
  echo $2 | grep -qE '^[1-9A-E]?[0-9A-F]{4}$' || usage "invalid code point '$2'"
  xml=$(grep "cp=\"$2\"" "$1") || usage "'$2' not found in: $1"
  xml="$(echo "${xml%/>}" | sed 's/" /"\n/g' | grep -v '<char cp=')"
}

declare -r firstCode="${1^^}"
getXml $oldFile $firstCode

if [[ $oldFile = $newFile ]]; then
  if [[ $# -lt 2 ]]; then
    echo -e "Code $firstCode (\U$firstCode) from '$newFile'\n$xml"
    exit 0
  fi
  secondCode=${2^^}
  [[ $firstCode = $secondCode ]] && usage "code points are the same"
else
  secondCode=$firstCode
fi

declare -r firstXml=$xml
getXml $newFile $secondCode

declare -r diffOut="$(diff <(echo "$firstXml") <(echo "$xml"))"

function diffs() {
  local -r s="$(echo "$diffOut" | grep "^$1" | sed "s/$1/ /")"
  [[ -n $s ]] && echo -e "Code $2 (\U$2) from '$3'\n$s"
}

diffs '<' $firstCode $oldFile
diffs '>' $secondCode $newFile
