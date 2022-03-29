#!/usr/bin/env bash

declare -r program=ucdDiff.sh defaultFile=ucd.all.flat.xml

# UCD = "Unicode Character Database": http://www.unicode.org/reports/tr42/
# Location of 'ucd.all.flat.zip': https://unicode.org/Public/UCD/latest/ucdxml
# Aliases: http://www.unicode.org/Public/UCD/latest/ucd/PropertyAliases.txt
# More info on UCD 'Unihan': https://unicode.org/reports/tr38/

# Ensure LANG is 'UTF-8' to enable things like: echo -e "\U72AC"
if [[ ! $LANG =~ UTF-8 ]]; then
  for s in jp_JP en_US; do
    locale -a | grep -q $s.UTF-8 && export LANG=$s.UTF-8 && break
  done
  [[ ! $LANG =~ UTF-8 ]] && echo "failed to find a UTF-8 locale" && exit 1
fi

oldFile=~/ucd/13/$defaultFile
newFile=~/ucd/14/$defaultFile
printOnlyCJK=false

function usage() {
  echo "$program: $1"
  echo ""
  echo "usage: $program [-f 'input file'] 'code point' ['code point']"
  echo "       $program -p [-f 'input file'] 'code point'"
  echo "       $program -d [-o 'old file'] [-n 'new file'] 'code point'"
  echo "  -f 'input file': default: '$newFile'"
  echo "  -p: print non-empty 'CJK' fields (as well as a few general fields)"
  echo "  -d: compare the same code point from two different UCD files (useful"
  echo "      to see diffs across Unicode versions)."
  echo "  -o 'old file': default '$oldFile'"
  echo "  -n 'new file': default '$newFile'"
  echo ""
  echo "if only one 'code point' is specified then print all fields, otherwise"
  echo "just print the fields that are different. Note, 'code point' should be"
  echo "unicode values like 3400, 4db5 or 20B9F or a UTF-8 value like 犬 or 猫."
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
  [[ $# -gt 1 ]] && usage "only one code point should be specified with '-d'"
else
  oldFile=$newFile
  while true; do
    if [[ $1 = -f ]]; then
      checkFile "$@"
      oldFile=$2
      newFile=$2
      shift 2
    elif [[ $1 = -p ]]; then
      printOnlyCJK=true
      shift
    else
      break
    fi
  done
  if $printOnlyCJK; then
    [[ $# -gt 1 ]] && usage "only one code point should be specified with '-p'"
  else
    [[ $# -gt 2 ]] && usage "too many arguments"
  fi
fi

[[ $# -lt 1 ]] && usage "must specify at least one code point"

function getCodePoint() {
  if echo "$1" | grep -q '[^ -~]'; then
    [[ ${#1} -gt 1 ]] && usage "code point '$1' should be a single UTF-8 value"
    # get the Unicode (4 of 5 digit hex with caps) value from UTF-8
    codePoint=$(echo -n $1 | iconv -s -f UTF-8 -t UTF-32BE | xxd -p) ||
      usage "failed to get code point from '$1'"
    printf -v codePoint '%04X' 0x$codePoint 2>/dev/null ||
      usage "failed to get code point from '$1', hex '$codePoint'"
  else
    codePoint=${1^^}
    echo $codePoint | grep -qE '^[1-9A-E]?[0-9A-F]{4}$' ||
      usage "invalid code point '$1'"
  fi
}

function getXml() {
  xml=$(grep "<char cp=\"$codePoint\"" "$1") ||
    usage "'$codePoint' not found in: $1"
  xml="$(echo "${xml%/>}" | sed 's/" /"\n/g' | grep -v '<char cp=')"
}

function printFields() {
  for i in $*; do
    echo -n ", $(echo "$xml" | grep -o "^$i=\"[^\"]*")"
  done
  echo ""
}

getCodePoint $1
getXml $oldFile

if [[ $oldFile = $newFile ]]; then
  if [[ $# -lt 2 ]]; then
    echo -en "Code $codePoint (\U$codePoint)"
    if $printOnlyCJK; then
      # age='Unicode Version', blk='Unicode Block", sc='Unicode Script', ...
      printFields age blk sc Ideo Radical
      echo "$xml" | grep '^k[A-Z]' | grep -v '""'
    else
      echo -e " from '$newFile'\n$xml"
    fi | sed 's/="/: /g' | tr -d '"'
    exit 0
  fi
  firstCode=$codePoint
  getCodePoint $2
  [[ $firstCode = $codePoint ]] && usage "code points are the same"
else
  firstCode=$codePoint
fi

declare -r firstXml=$xml
getXml $newFile

declare -r diffOut="$(diff <(echo "$firstXml") <(echo "$xml"))"

if [[ -z $diffOut ]]; then
  echo "no diffs found"
  exit 0
fi

function diffs() {
  local -r s="$(echo "$diffOut" | grep "^$1" | sed "s/$1/ /")"
  [[ -n $s ]] && echo -e "Code $2 (\U$2) from '$3'\n$s"
}

diffs '<' $firstCode $oldFile
diffs '>' $codePoint $newFile
