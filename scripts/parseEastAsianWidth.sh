#!/usr/bin/env bash

declare -r program='parseEastAsiaWidth.sh'

# This script parses the Unicode 'EastAsiaWidth.txt' file and outputs C++ code
# that can be used in 'utils/DisplaySize.h' for calculating 'displaySize' of
# a string, i.e., wide characters take two columns on a terminal.
# https://unicode.org/reports/tr11 - more details about widths
# https://www.unicode.org/Public/zipped/14.0.0 - EastAsiaWidth.txt (in UCD.zip)

if [ $# -lt 1 ]; then
  echo "please specify 'EastAsiaWidth.txt' file to parse"
  exit 1
fi

function msg() {
  echo "// --- $1 generated code from '$program' ---"
}

msg begin

echo "inline constexpr std::array WideBlocks{"

blocks=()

function out() {
  [ $1 = $2 ] && blocks+=("<0x$1>()") || blocks+=("<0x$1, 0x$2>()")
  prevStart=""
  prevEnd=""
}

function one_bigger() {
  printf -v gap '%d' "$((0x$1 - 0x$2))"
  [ $gap -eq 1 ]
  return $?
}

function process() {
  if [ $1 != $2 ]; then
    if [ -n "$prevStart" ]; then
      if one_bigger $1 $prevEnd; then
        # if new start is one bigger than prevEnd then extend to new end ($2)
        prevEnd=$2
      else
        # got a gap of more than one so flush prev range and start a new range
        out $prevStart $prevEnd
        prevStart=$1
        prevEnd=$2
      fi
    else
      # no previous range so simply start a new range
      prevStart=$1
      prevEnd=$2
    fi
  elif [ -n "$prevStart" ]; then
    # got a new single value: if it's one bigger than 'prevEnd' then move up
    # 'prevEnd', otherwise flush previous stored values
    if one_bigger $1 $prevEnd; then
      prevEnd=$1
    else
      out $prevStart $prevEnd
      prevStart=$1
      prevEnd=$1
    fi
  else
    # got a single value so store it
    prevStart=$1
    prevEnd=$1
  fi
}

# EastAsiaWide.txt file contains lines that have 2 values separated by ';'.
# The first value is either a single code point or a range (using ..). Examples:
#   FF71;W
#   FFE0..FFE6;F
# The second field is the width type - we want to find 'F' (full) or 'W' (wide)
while read -r i; do
  # remove comments
  line=$(echo $i | sed 's/^\([^#]*\).*/\1/')
  if [ -n "$line" ] && echo "$line" | grep -q ';[FW]'; then
    line=${line%;*}
    start=${line%%\.*}
    process $start ${line##*\.}
  fi
done <$1

[ -n "$prevStart" ] && process $prevStart $prevEnd

len=${#blocks[@]}
comma=,
for i in "${blocks[@]}"; do
  len=$((len - 1))
  [ $len -eq 0 ] && comma=""
  echo "  makeBlock$i"$comma
done
echo "};"
msg end
