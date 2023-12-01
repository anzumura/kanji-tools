#!/usr/bin/env bash

declare -r program=parseUcdAllFlat.sh

# This script searches the Unicode 'ucd.all.flat.xml' file for characters that
# have a Japanese reading (On or Kun), Morohashi ID or a non-empty JSource and
# prints a tab-separated line with the following 20 values:
# - Code: Unicode code point (4 or 5 digit hex code)
# - Name: character in UTF-8
# - Block: name of the Unicode block (from the 'blk' tag)
# - Version: Unicode version this character was added (from 'age' tag)
# - Radical: radical number (1 to 214)
# - Strokes: total strokes (including the radical)
# - VStrokes: strokes for first different 'adobe' count (blank if no diffs)
# - Pinyin: optional first pīnyīn (拼音) reading from 'kMandarin'
# - MorohashiId: optional 'Dai Kan-Wa Jiten (大漢和辞典)' index number
# - NelsonIds: optional list of comma-separated 'Classic Nelson' Ids
# - Sources: list of sources (see comments at the end of this script)
# - JSource: from kIRG_JSource (see comments et the end of this script)
# - Joyo: 'Y' if part of Jōyō list or blank
# - Jinmei: 'Y' if part of Jinmeiyō list or blank
# - LinkCodes: optional list of comma-separated link values in Unicode
# - LinkNames: optional list of comma-separated link values in UTF-8
# - LinkType: optional single value for the type of all the links
# - Meaning: optional semicolon separated English definitions
# - On: optional space-separated Japanese On readings (in all-caps Rōmaji)
# - Kun: optional space-separated Japanese Kun readings (in all-caps Rōmaji)
# - Japanese: optional space-separated On/Kun readings in Kana (since ver 15.1)
#
# Further explanations are in comments at the end of this script.

# Ensure LANG is 'UTF-8' so things like ${j:0:1} get a single utf8 value instead
# of just a single byte.
if [[ ! $LANG =~ UTF-8 ]]; then
  for s in jp_JP en_US; do
    locale -a | grep -q $s.UTF-8 && export LANG=$s.UTF-8 && break
  done
  [[ ! $LANG =~ UTF-8 ]] && echo "failed to find a UTF-8 locale" && exit 1
fi

if [[ $# -lt 2 ]]; then
  echo "usage: $program 'input file' 'output file'"
  echo "  input file: ucd.all.flat.xml file (see script comments for details)"
  echo "  output file: where to write output (for example, data/ucd.txt)"
  exit 1
fi
declare -r ucdFile=$1 outFile=$2

function log() {
  echo $2 "$(date +%T): $1"
}

log "Begin parsing '$ucdFile'"

declare -i radical

# 'setVars': sets global vars for each property in the XML record passed in. It
# will also unset any vars specified after the XML string. The exit status is 0
# if the record should be processed.
function setVars() {
  local -r xml=${1:6} # strip leading '<char '
  shift               # more 'vars to unset' can be specified after $1 (XML arg)
  unset -v kJoyoKanji kJinmeiyoKanji kMorohashi $*
  eval ${xml%/>} # s	trip trailing '/>' and set vars from remaining XML string
  radical=${kRSUnicode%%[.\']*}
  # Simplified radicals, i.e., a value ending with a single quote, shouldn't be
  # processed unless the radical is 199 (麥 麦). For example, 4336 (䌶) has
  # kRSUnicode="120'.3" so it will result in a non-zero exit code. Also keep
  # radical 211'' since the double quote means 'non-Chinese simplified' - for
  # example 9F62 (齢) has kRSUnicode=211''.5 (and it's a Jouyou Kanji)
  [[ $radical =~ 199|211 || ! $kRSUnicode =~ ^[0-9]*\' ]]
}

function setOnKun() {
  resultOn=$1
  resultKun=$2
}

# global arrays to help support links for variant and compat kanji
declare -A morohashi jSource definition on kun linkBack noLink readingLink

# there are 18 Jinmeiyō Kanji that link to other Jinmei, but unfortunately UCD
# data seems to have some mistakes (where the link points from the standard to
# the variant instead). For example 4E98 (亘) has kJinmeiyoKanji="2010:U+4E99"
# and 4E99 (亙) has kJinmeiyoKanji="2010". This contradicts the description of
# the field (since 4E98 is the standard form). Here's the official description:
#   The version year is either 2010 (861 ideographs), 2015 (one ideograph), or
#   2017 (one ideograph), and 230 ideographs are variants for which the code
#   point of the standard Japanese form is specified.
# For now, store the 18 kanji in 'noLink' to block setting links.
while read -r i; do
  noLink[${i%\ *}]=${i#*\ }
done <<EOF
4E98 亘
51DC 凜
5C2D 尭
5DCC 巌
6643 晃
6867 桧
69D9 槙
6E1A 渚
732A 猪
7422 琢
7950 祐
7962 祢
7977 祷
7984 禄
798E 禎
7A63 穣
840C 萌
9065 遥
EOF

# 'onKunRegex' is used as a filter by 'populateArrays' function since we only
# want to link to an entry that has an 'On' and/or 'Kun' reading.
declare -r onKunRegex='kJapanese[OK].*n="[^"]'

# 'printResults' loop uses 'onKunRegex' as well as the following regexes:
declare -r jSourceRegex='kIRG_JSource="[^"]' # has a JSource
declare -r morohashiRegex='kMorohashi="[^"]' # has a Morohashi ID
declare -r nelsonRegex='kNelson="[^"]'       # has a Nelson ID

# nelsonRegex only pulls in a few Kanji not included in the other regexes, but
# add it to the filter to help make 'find by Nelson ID' as complete as possible.
outFilter="$onKunRegex|$jSourceRegex|$morohashiRegex|$nelsonRegex"

# Populate 'on', 'kun', 'definition', 'jSource' and 'morohashi' arrays. Arrays
# need to be populated before 'printResults' to handle links to entries later in
# the file like '5DE2 (巢)' which links to '5DE3 (巣)'.
function populateArrays() {
  log 'Populate arrays ... ' -n
  local -i count=0
  local s
  while read -r s; do
    setVars "$s" kDefinition kJapaneseOn kJapaneseKun || continue
    if [[ -n $kMorohashi ]]; then
      # --- Before version 15.1 the following logic was needed:
      # Remove leading 0's and change single quotes to 'P' (Prime) so that 04138
      # changes to 4138 (maps to 嗩) and 04138' changes to 4138P (maps to 嘆).
      kMorohashi=$(echo $kMorohashi | sed -e 's/^0*//' -e "s/'/P/g")
      # --- Unicode 15.1 fixed above problems, but made some other changes that
      # require stripping 'selectors' after : and removing duplicate zero padded
      # ids like for 342C which has '296 00296'
      # --- Keep old and new logic for backwards compat
      kMorohashi=$(echo $kMorohashi | sed -e 's/[ :].*$//')
      # There are a few kMorohashi values that are all 0's so check if non-empty
      # again before setting global array.
      [[ -n $kMorohashi ]] && morohashi[$cp]=$kMorohashi
    fi
    [[ -n $kIRG_JSource ]] && jSource[$cp]=$kIRG_JSource
    if [[ -n $kJoyoKanji ]]; then
      # There are 4 entries with a Joyo link: 5265, 53F1, 586B and 982C. Store
      # definition/on/kun since since they are missing for some linked Jinmeiyo
      # Kanji as well as Joyo 𠮟 (U+20B9F) which replaces 叱 (53F1) しか-る.
      [[ $kJoyoKanji =~ U+ ]] && cp+=" ${kJoyoKanji#U+}"
    elif [[ $kJinmeiyoKanji =~ U+ ]]; then
      kJinmeiyoKanji=${kJinmeiyoKanji#*+}
      linkBack[$kJinmeiyoKanji]=$cp
      cp+=" $kJinmeiyoKanji"
    fi
    if [[ -n $kJapaneseOn$kJapaneseKun ]]; then
      for link in $cp; do
        definition[$link]=$kDefinition
        on[$link]=$kJapaneseOn
        kun[$link]=$kJapaneseKun
        count+=1
      done
    fi
  done < <(grep -E "($onKunRegex|$jSourceRegex|$morohashiRegex)" $ucdFile)
  echo "Reading $count, Morohashi ${#morohashi[@]}, JSource ${#jSource[@]}"
}

function hasReading() {
  [[ -n ${on[$1]}${kun[$1]} ]]
}

function hasJapanID() {
  [[ -n ${morohashi[$1]} ]] || [[ -n ${jSource[$1]} ]]
}

# 'canLoadFrom': has exit status of 0 (true) if '$1' in refers to a kanji with
# an 'On' or 'Kun' reading or has a 'readingLink'. Thus 'readings' can be loaded
# from the '$1' if required.
function canLoadFrom() {
  hasReading $1 || [[ -n ${readingLink[$1]} ]]
}

# 'defTypePasses' controls the number of links separated by delimiters to check
# for in kDefinition. Set to '3' for now since '4', '5' or '6' didn't find more.
declare -r -i defTypePasses=3
# 'defType' arrays hold comma separated counts of definition links found per
# 'type' string. 'Utf' = links from UTF-8 characters and 'Uni' = links from
# Unicode values, i.e., U+ABCD.
declare -A defTypeUtf defTypeUni
# 'linkErrors' holds any errors found during link processing
declare -a linkErrors

function printDefinitionLinkCounts {
  local -i total=0 maxLen=0 utf uni i j
  local s types utfCounts uniCounts headers fmt2='  '
  local -a pass
  for s in "${!defTypeUtf[@]}"; do
    [[ ${#s} -gt $maxLen ]] && maxLen=${#s}
    [[ -z $types ]] && types=$s || types+="\n$s"
  done
  maxLen+=4 # add 4 to account for 2 space indent and wrapping in single quotes
  local -r fmt1="  %-${maxLen}s"
  while read -r s; do
    utfCounts=${defTypeUtf[$s]}
    uniCounts=${defTypeUni[$s]}
    i=0
    pass=()
    for ((j = 1; j <= defTypePasses; ++j)); do
      utf=$(echo $utfCounts | cut -d, -f$j)
      uni=$(echo $uniCounts | cut -d, -f$j)
      i+=$((utf + uni))
      pass+=("'$(printf '(%3d,%3d)' $utf $uni)'")
      if [[ $total -eq 0 ]]; then
        fmt2+='%-11s'
        headers+=" ' utf uni'"
      fi
    done
    if [[ $total -eq 0 ]]; then
      fmt2+='\n'
      eval printf "'${fmt1}Total$fmt2'" \"Types: ${#defTypeUtf[@]}\" $headers
    fi
    eval printf "'$fmt1 %4d$fmt2'" "\"  '$s'\"" $i "${pass[@]}"
    total+=$i
  done < <(echo -e "$types" | sort)
  printf "$fmt1 %4d\n" '  (manually set)' $((${#readingLink[@]} - total))
  if [[ ${#linkErrors[@]} -gt 0 ]]; then
    echo "  Errors: ${#linkErrors[@]}"
    for s in "${linkErrors[@]}"; do
      echo "    $s"
    done
  fi
}

function processUtfLinks() {
  if [[ -z ${readingLink[$1]} ]]; then
    local s link
    # some kanji have multiple links like 4640 (䙀) which has 'same as 綳繃'
    while read -r -n 1 s; do
      # get the Unicode (4 of 5 digit hex with caps) value from UTF-8 link
      if link=$(echo -n $s | iconv -s -f UTF-8 -t UTF-32BE | xxd -p); then
        if printf -v link '%04X' 0x$link 2>/dev/null; then
          hasReading $link && readingLink[$1]=$link && return 0
        else
          linkErrors+=("printf failed: cp=$1 link=$s 0x$link")
        fi
      else
        linkErrors+=("iconv failed: cp=$1 link=$s")
      fi
    done < <(echo -n $2)
  fi
  return 1
}

function processUniLink() {
  [[ -z ${readingLink[$1]} ]] && hasReading $2 && readingLink[$1]=$2
}

# 'findDefinitionLinksForType': finds links based on type ($1) string matches in
# 'kDefinition' field and updates 'defType' arrays.
function findDefinitionLinksForType() {
  local -r def="kDefinition=\"$2$1[-,a-z0-9 ]*" utf='[^ -~]' uni=[A-F0-9]
  local -r start='s/.* cp="\([^"]*\).*'$def end='*\).*/\1:\2/'
  local -i utfCount=0 uniCount=0
  local s
  # loop to handle strings like 'same as X' where X is a UTF-8 kanji
  for s in $(grep "$def$utf" $outFile | sed "$start\($utf$end"); do
    processUtfLinks ${s/:/ } && utfCount+=1
  done
  # loop to handle strings like 'same as U+ABCD ...' (can use Unicode directly)
  for s in $(grep "${def}U\+$uni*" $outFile | sed "${start}U+\($uni$end"); do
    processUniLink ${s/:/ } && uniCount+=1
  done
  defTypeUtf[$1]+=$utfCount,
  defTypeUni[$1]+=$uniCount,
}

# 'findDefinitionLinks': find links based on 'kDefinition' field. For example,
# if a record for 'cp' has '(same as X' in its definition then store a link from
# 'cp' to 'X'. This function populates 'readingLink' array. Links can occur more
# than once like '64DA (據)' which is a link for '3A3F (㨿)' and '3A40 (㩀)'.
function findDefinitionLinks() {
  log 'Find definition links ... ' -n
  local -r types="variant interchangeable same non-classical Variant standard \
simplified ancient"
  local link s='kDefinition="[^"]*('${types// /|}')[ ,]'
  outFilter+="|$s"
  # Use 'outFile' to temporarily hold the ~2K records that might have definition
  # links to speed up processing (instead of 'ucdFile' which has >150K records).
  # This reduces the time to run 3 passes on 8 types from ~120 secs to ~12 secs.
  grep -E "kRSUnicode=\"[0-9]*\..*$s" $ucdFile >$outFile
  # kDefinition can have multiple values separated by ';', but there are cases
  # where just brackets are used. For example, 4CFD (䳽) has:
  #   (non-classical form of 鸖) (same as 鶴) crane
  # Another example is '36B3 (㚶)':
  #   (same as 姒) wife of one's husband's elder brother; (in ancient China) the
  #   elder of twins; a Chinese family name, (same as 姬) a handsome girl; a
  #   charming girl; a concubine; a Chinese family name
  local -r delim=')'        # char used to split up definitions
  local -r end=[^$delim\"]* # regex that excludes 'delim' as well as "
  local -i i=0
  for link in $types; do
    s=
    # check the first 'defTypePasses' potential occurrences of 'link' value.
    for ((i = 0; i < defTypePasses; ++i)); do
      findDefinitionLinksForType $link "$end$s"
      s+=[$delim]$end # one 'delim' followed by non-delim (ie 'end')
    done
  done
  # Pull in some Kentei and Nelson ID kanji that are missing On/Kun (links have
  # the same definitions and expected readings).
  readingLink[3D4E]=6F97 # link 㵎 to 澗
  readingLink[5ECF]=5ED0 # link 廏 to 廐
  readingLink[9D25]=9D2A # link 鴥 to 鴪
  readingLink[6AA8]=69D8 # link 檨 to 様 (Nelson 2363)
  echo "found ${#readingLink[@]}"
  printDefinitionLinkCounts
}

declare -i strokes vstrokes

# 'getStrokesFromAdobeRef': tries to find the correct stroke count for Japanese
# characters by looking at the 'kRSAdobe_Japan1_6' property as well as 'radical'
# and 'kTotalStrokes' that are set by the 'setVars' function. Results are put in
# global 'strokes' and 'vstrokes' variables.
function getStrokesFromAdobeRef() {
  # Some characters have multiple adobe references so start with first one
  local s=${kRSAdobe_Japan1_6%%\ *}
  # Adobe refs have the form 'C+num+x.y.z' where 'x' is the radical number,
  # 'y' is number of strokes for the radical and 'z' is remaining strokes.
  s=${s##*+}               # remove the 'C+num+' prefix
  s=${s#*\.}               # get 'y.z' (by removing prefix to .)
  strokes=$((${s/\./ + })) # y + z
  # If there are multiple Adobe refs then check 'VStrokes' from the second in
  # the list. Some have more than two like 傑 (5091) which has kRSAdobe_Japan1_6
  # "C+1852+9.2.11 V+13433+9.2.11 V+13743+9.2.10". If strokes and vstrokes are
  # different then check to see if they should be swapped. Swapping fixes counts
  # for some characters but breaks it for others so only swap if 'vstrokes'
  # represents the same radical and count as 'kRSUnicode' (RS = Radical+Stroke).
  # This fixes some, but also isn't prefect. Limiting swapping to only happen if
  # vstrokes matches 'kTotalStrokes' also helps, but there are still 52 Joyo and
  # Jinmei Kanji that have different total stroke counts (without any swapping
  # there are 57 differences).
  if [[ ${kRSAdobe_Japan1_6} =~ ' ' ]]; then
    s=${kRSAdobe_Japan1_6#*\ } # remove the first adobe ref
    kTotalStrokes=${kTotalStrokes%% *}
    # Only allow flipping strokes up to the second entry for now. Strokes and
    # radicals mostly match official Joyo and Jinmei charts, but there are still
    # some differences and trying to get exact matches from ucd data may not be
    # possible (even some dictionaries disagree o radicals and stroke counts).
    local secondEntry=true
    for s in $s; do
      # Same as 'strokes' above, i.e, remove 'V+num+', get y.z and then add
      s=${s##*+}
      local -i vradical=${s%%\.*}
      s=${s#*\.}
      local -i newStrokes=$((${s/\./ + }))
      if [[ $strokes -ne $newStrokes ]]; then
        if $secondEntry && [[ $kRSUnicode = $vradical.${s#*\.} ]]; then
          if [[ $kTotalStrokes -eq $newStrokes ]]; then
            # flip strokes and vstrokes
            vstrokes=$strokes
            strokes=$newStrokes
          else
            vstrokes=$newStrokes
          fi
        else
          vstrokes=$newStrokes
        fi
        break
      fi
      secondEntry=false
    done
  fi
}

# 'getTraditionalLinks': sets 'linkTo' based on kTraditionalVariant field. More
# than one link can exist, but must satisfy the function passed in $1 (see
# 'getLinks' function below).
function getTraditionalLinks() {
  local s
  for s in $kTraditionalVariant; do
    s=${s#*+} # remove leading U+
    # Skip if kTraditionalVariant is the same as current record being processed.
    # For example, 'cp' 5413 (吓) has kTraditionalVariant="U+5413 U+5687".
    if [[ ! ,$cp,$linkTo, =~ ,$s, ]] && $1 $s; then
      linkType=Traditional
      # allow storing multiple traditional links
      [[ -z $linkTo ]] && linkTo=$s || linkTo+=,$s
    fi
  done
}

# 'getLinks': sets 'linkTo' to one on more values and sets 'linkType' if the
# links point to a value that satisfies the function passed in $1 ('canLoadFrom'
# or 'hasJapanID'). This function can only be used after 'setVars' and it also
# looks at 'resultOn' and 'resultKun'.
function getLinks() {
  local s
  # For non-Jouyou/non-Jinmei kanji, try to find meaningful links based on
  # kTraditionalVariant, kCompatibilityVariant or kSemanticVariant fields or
  # from 'readingLink' array (based on kDefinition field).
  getTraditionalLinks $1
  if [[ -z $linkType ]]; then
    for s in $kSimplifiedVariant; do
      linkTo=${s#*+} # remove leading U+
      # Skip if kSimplifiedVariant is the same as current record being processed.
      # For example, 'cp' 81E4 (臤) has kSimplifiedVariant="U+81E4 U+30021".
      [[ $cp != $linkTo ]] && $1 $linkTo && linkType=Simplified && break
    done
    # Only use Compatibility, Definition and Semantic links if they refer to
    # another kanji with an On/Kun reading, i.e., if 'canLoadFrom' is true.
    if [[ $1$linkType == canLoadFrom ]]; then
      if [[ $kCompatibilityVariant =~ U+ ]]; then
        # kCompatibilityVariant never has more than one value
        linkTo=${kCompatibilityVariant#*+} # remove leading U+
        $1 $linkTo && linkType=Compatibility
      fi
      if [[ -z $linkType ]]; then
        if [[ -n ${readingLink[$cp]} ]]; then
          linkTo=${readingLink[$cp]}
          linkType=Definition
        elif [[ -z $resultOn$resultKun ]]; then
          # Only use semantic links if there's no readings for the current kanji
          # being processed since they can result in linking together kanji that
          # don't seem related. kSemanticVariant can be Unicode like "U+8209" or
          # a compound like "U+71D0&lt;kMatthews U+7CA6&lt;kMatthews".
          for s in $kSemanticVariant; do
            s=${s#*+}       # remove leading U+
            linkTo=${s%%&*} # strip trailing &lt
            $1 $linkTo && linkType=Semantic && break
          done
        fi
      fi
    fi
  elif [[ $1 == canLoadFrom ]]; then
    # allow traditional links to kanji with a 'JapanID' if reading ones exist
    getTraditionalLinks hasJapanID
  fi
  if [[ -z $linkType ]]; then
    linkTo=
    return 1
  fi
}

# All entries have 'cp', 'age', 'blk', 'radical' and 'strokes' so no need to
# count them, but count some other optional fields of interest.
declare -i totalJoyo=0 totalJinmei=0 jinmeiLinks=0 traditionalLinks=0 \
  simplifiedLinks=0 compatibilityLinks=0 definitionLinks=0 semanticLinks=0 \
  totalMeaning=0 totalPinyin=0 totalReading=0 directReading=0 linkedReading=0

# 'countLinkType' increments totals for each link type (used in summary info)
function countLinkType() {
  case ${linkType/\*/} in
  Jinmei) jinmeiLinks+=1 ;;
  Traditional) traditionalLinks+=1 ;;
  Simplified) simplifiedLinks+=1 ;;
  Compatibility) compatibilityLinks+=1 ;;
  Definition) definitionLinks+=1 ;;
  Semantic) semanticLinks+=1 ;;
  *) [[ -n $linkType ]] && echo "unexpected linkType: $linkType" && exit 1 ;;
  esac
}

declare -A uniqueJSource uniqueMorohashi uniqueNelson

function processRecord() {
  # morohashi IDs stored in 'morohashi' can be different than kMorohashi due to
  # some processing (like removing zeroes). JSource values are not modified but
  # also use the values stored in the array to be consistent.
  local -r localMorohashi=${morohashi[$cp]} localJSource=${jSource[$cp]}
  local localDefinition=${definition[$cp]} loadFrom sources s
  setOnKun "${on[$cp]}" "${kun[$cp]}"
  # 'linkTo' and 'linkType' can be modified by 'getLinks' function
  linkTo=
  linkType=
  if [[ -n $kJoyoKanji ]]; then
    # unset kJoyoKanji if official version is the 'link' entry
    [[ $kJoyoKanji =~ U+ ]] && unset -v kJoyoKanji || totalJoyo+=1
  elif [[ -n $kJinmeiyoKanji ]]; then
    if [[ $kJinmeiyoKanji =~ U+ ]]; then
      loadFrom=${kJinmeiyoKanji#*+}
      [[ -z ${noLink[$cp]} ]] && linkTo=$loadFrom
    else
      linkTo=${linkBack[$cp]}
    fi
    [[ -z $linkTo ]] && totalJinmei+=1 || linkType=Jinmei
  else
    getLinks canLoadFrom && loadFrom=${linkTo%%,*} || getLinks hasJapanID
  fi
  if [[ -n $resultOn$resultKun ]]; then
    directReading+=1
  elif [[ -n $loadFrom ]]; then
    hasReading $loadFrom || loadFrom=${readingLink[$loadFrom]}
    localDefinition=${definition[$loadFrom]}
    resultOn=${on[$loadFrom]}
    resultKun=${kun[$loadFrom]}
    # This should never happen, but keep as a sanity check
    [[ -z $resultOn$resultKun ]] &&
      echo "ERROR: link not found for cp $cp (link $linkType $loadFrom)"
    # Loading readings from a link can potentially lead to incorrect and/or
    # confusing results for some less common kanji so add a '*' to 'linkType'
    # to make it clear that the data is less trustworthy.
    linkType+=*
    linkedReading+=1
  else
    # Support cases where on/kun is missing for Kentei kanji or kanji with
    # Nelson IDs. 4BC2 is not Kentei, but it's in 'wiki-stokes.txt' file. UCD
    # data doesn't have entries for Nelson IDs: 125, 149, 489 and 1639
    case $cp in
    34E4) setOnKun KATSU ;;                          # 㓤 (Nelson 677)
    3C7E) setOnKun KAI ;;                            # 㱾 (Nelson 2453)
    3C83) setOnKun KYUU ;;                           # 㲃 (Nelson 2459)
    4BC2) setOnKun SHIN ;;                           # 䯂
    6479) setOnKun BO MO ;;                          # 摹 (Nelson 4035)
    6532) setOnKun KI 'KATAMUKU SOBADATERU' ;;       # 攲 (Nelson 2041)
    6663) setOnKun 'SEI SETSU' 'AKIRAKA KASHIKOI' ;; # 晣
    69D4) setOnKun KOU HANETSURUBE ;;                # 槔
    7B53) setOnKun KEI 'KOUGAI KANZASHI' ;;          # 筓
    7CF1) setOnKun GETSU 'KOUJI MOYASHI' ;;          # 糱
    83C6) setOnKun SHU ;;                            # 菆 (Nelson 3961)
    # if no readings (and no Morohashi ID or JSource) then skip this record
    *) [[ -n $localMorohashi$localJSource ]] || return 1 ;;
    esac
  fi
  vstrokes=0
  if [[ -z $kRSAdobe_Japan1_6 ]]; then
    kTotalStrokes=${kTotalStrokes%% *}
    strokes=$kTotalStrokes
  else
    getStrokesFromAdobeRef
  fi
  [[ -n $localDefinition ]] && totalMeaning+=1
  [[ -n $resultOn$resultKun ]] && totalReading+=1
  if [[ -n $kMandarin ]]; then
    # for kanji with multiple Pinyin readings just keep the first for now
    kMandarin=${kMandarin%% *}
    totalPinyin+=1
  fi
  [[ -n $localJSource ]] && uniqueJSource[$localJSource]=1
  [[ -n $localMorohashi ]] && uniqueMorohashi[$localMorohashi]=1
  if [[ -n $kNelson ]]; then
    # remove leading 0's from all Nelson Ids in the list
    kNelson=$(echo $kNelson | sed -e 's/^0*//' -e 's/ 0*/ /g')
    for s in $kNelson; do
      uniqueNelson[$s]=1
    done
  fi
  for s in G H J K T V; do
    eval [[ -n \$kIRG_${s}Source ]] && sources+=$s
  done
  countLinkType
  # don't print 'vstrokes' if it's 0
  echo -e "$cp\t\U$cp\t$blk\t$age\t$radical\t$strokes\t${vstrokes#0}\t\
$kMandarin\t$localMorohashi\t${kNelson// /,}\t$sources\t$localJSource\t\
${kJoyoKanji:+Y}\t${kJinmeiyoKanji:+Y}\t$linkTo\t${linkTo:+\U${linkTo//,/,\\U}}\
\t$linkType\t$localDefinition\t$resultOn\t$resultKun\t$kJapanese" >>$outFile
}

function printResults() {
  log "Print results to '$outFile' ... " -n
  echo -e "Code\tName\tBlock\tVersion\tRadical\tStrokes\tVStrokes\tPinyin\t\
MorohashiId\tNelsonIds\tSources\tJSource\tJoyo\tJinmei\tLinkCodes\tLinkNames\t\
LinkType\tMeaning\tOn\tKun\tJapanese" >$outFile
  local s
  local -i count=0
  while read -r s; do
    # No need to unset fields that are in every record such as 'cp', 'age' and
    # 'blk', but need to unset all the optional fields used in 'processRecord'.
    setVars "$s" kTraditionalVariant kSimplifiedVariant kCompatibilityVariant \
      kSemanticVariant kRSAdobe_Japan1_6 kMandarin kNelson &&
      processRecord "$s" && count+=1
    # kIRG_xSource fields are also included in all Unihan records ("" if empty)
  done < <(grep -E "($outFilter)" $ucdFile)
  echo "wrote $count"
}

populateArrays
findDefinitionLinks
printResults

log Done

function checkTotal() {
  echo -n "$1: $2"
  [[ $2 -eq $3 ]] && echo "/$3" || echo " *** expected $3 ***"
}

# May need to update JSource and Morohashi expected totals when upgrading to a
# new Unicode version (below numbers are based on Unicode 15.1). Nelson could go
# up if any missing ones are added (see comments in 'processRecord'). Jouyou and
# Jinmei numbers should only change if the Japanese government makes changes.
checkTotal 'Unique JSource' ${#uniqueJSource[@]} 16222
checkTotal 'Unique Morohashi' ${#uniqueMorohashi[@]} 49019
checkTotal 'Unique Nelson ID' ${#uniqueNelson[@]} 5442
checkTotal 'Total Jouyou' $totalJoyo 2136
checkTotal 'Total Jinmei' $totalJinmei 633
checkTotal 'Jinmei Links' $jinmeiLinks 230

echo -e "Other Links:\n  Traditional $traditionalLinks, Simplified \
$simplifiedLinks, Compatibility $compatibilityLinks, Definition \
$definitionLinks, Semantic $semanticLinks"
echo -e "Other Fields:\n  Meaning $totalMeaning, Pinyin $totalPinyin, Reading \
$totalReading (Direct $directReading Linked $linkedReading Manual \
$((totalReading - directReading - linkedReading)))"

# More details on some fields:
#
# 'linkType' is followed by '*' if the first link in 'LinkName' list was used to
# pull in On/Kun values (only happens when the current entry has no readings).
#
# Location of 'ucd.all.flat.zip': https://unicode.org/Public/UCD/latest/ucdxml
# More info on UCD file: https://unicode.org/reports/tr38/
# This script was used to create 'data/ucd.txt'.
#
# There are over 140K characters in 'ucd.all.flat.xml' and most of them aren't
# relevant to the current functionality of this (kanji-tools) project so apply
# some filtering before parsing (also significantly reduces the time to parse).
#
# Filtering on kIRG_JSource being populated cuts the set to 16,226 entries (IRG
# = 'Ideographic Research Group') and 12,275 of these have at least one Japanese
# reading (On/Kun) or a value in kJoyoKanji. The kTotalStrokes field doesn't
# always work well for Japanese common characters so the kRSAdobe_Japan1_6 field
# is also used, i.e., there are over 300 differences in just the 3000 or so
# standard Jōyō + Jinmei. Examples: 4EE5 (以) and 4F3C (似) have kTotalStrokes
# of 4 and 6 respectively, but in Japanese they should be 5 and 7.
#
# In addition to on/kun and non-empty JSource, also pull in kanji that have a
# Morohashi ID to get a better set for 'lookup by Morohashi ID' functionality.
# Note, 'kMorohashi' has 18,168 entries (12,965 with On/Kun) and it covers a
# similar set of characters as 'kRSAdobe...'. Some counts as of Unicode 13.0:
# - has both Adobe and Morohashi: 12,447
# - has Morohashi, but not Adobe: 5,721
# - has Adobe, but not Morohashi: 1,010
#
# MorohashiId: Unicode 14.0 has 17,830 unique 'kMorohashi' values. This property
# has one or more index numbers into 'Dai Kan-Wa Jiten' (a huge Chinese-Japanese
# dictionary compiled by Tetsuji Morohashi). There are plans to cleanup/expand
# this property to cover ~50K entries by Unicode 16.0.
#
# NelsonIds: Unicode 14.0 has 5,442 unique 'kNelson' Ids. This property has one
# or more Ids from the 'Classic Nelson' Japanese-English Character Dictionary.
# 'Classic Nelson' was first published in 1962 and the Ids remained the same for
# the 'Second Revised Edition' from 1974 (including the Thirtieth Printing in
# 1989). These Ids don't match 'New Nelson' which was first published in 1997.
#
# Morohashi and Nelson Ids can be used for looking up kanji by the 'kanjiQuiz'
# program (see QuizLauncher.cpp or run 'kanjiQuiz -h' for details).
#
# Sources: a list of letters where each letter represents a 'kIRG_xSource' field
# that has a non-empty value to help determine the country where a character is
# used (skip KP, M, S, U, and UK for now since they either don't have many
# values or don't really represent an East Asian country). The 'kUnihanCore2020'
# field is similar, but it's missing values like 'V' and 'S' and it's also not
# always populated even if the source is populated since it's intended to be
# 'the minimal set of required ideographs for East Asia'. Letters included are:
#   G: People’s Republic of China and Singapore
#   H: Hong Kong
#   J: Japan, i.e., 'kIRG_JSource' has a non-empty value
#   K: Republic of Korea (South Korea)
#   T: Taiwan
#   V: Vietnam
#
# JSource (from kIRG_JSource) provides more details on the original source for
# Japanese characters. This field has the following syntax:
#   J[014]-[0-9A-F]{4}
#   | J3A?-[0-9A-F]{4}
#   | J13A?-[0-9A-F]{4}
#   | J14-[0-9A-F]{4}
#   | JA[34]?-[0-9A-F]{4}
#   | JARIB-[0-9A-F]{4}
#   | JH-(JT[ABC][0-9A-F]{3}S?|IB\d{4}|\d{6})
#   | JK-\d{5}
#   | JMJ-\d{6}
# With the following description (reformatted to 80 columns with leading -):
#   The IRG “J” source mapping for this character in hexadecimal or decimal. The
#   IRG “J” source consists of data from the following national standards and
#   lists from Japan.
#   - J0 JIS X 0208-1990
#   - J1 JIS X 0212-1990
#   - J4 JIS X 0213:2004 level-4
#   - J3 JIS X 0213:2004 level-3
#   - J3A JIS X 0213:2004 level-3 addendum from JIS X 0213:2000 level-3
#   - J13 JIS X 0213:2004 level-3 characters replacing J1 characters
#   - J13A JIS X 0213:2004 level-3 character addendum from JIS X 0213:2000
#     level-3 replacing J1 characters
#   - J14 JIS X 0213:2004 level-4 characters replacing J1 characters
#   - JA Unified Japanese IT Vendors Contemporary Ideographs, 1993
#   - JA3 JIS X 0213:2004 level-3 characters replacing JA characters
#   - JA4 JIS X 0213:2004 level-4 characters replacing JA characters
#   - JARIB Association of Radio Industries and Businesses (ARIB) ARIB STD-B24
#     Version 5.1, March 14 2007
#   - JH Hanyo-Denshi Program (汎用電子情報交換環境整備プログラム), 2002-2009
#   - JK Japanese KOKUJI Collection
#   - JMJ Moji Joho Kiban Project (文字情報基盤整備事業)
# Here are counts by prefix of kIRG_JSource as of Unicode 14.0:
#   J0: 6356
#   J1: 3058
#   J14: 1704
#   JMJ: 1647
#   J13: 1037
#   JK: 782
#   J4: 665
#   JA: 575
#   J3: 194
#   JH: 107
#   JA4: 67
#   JA3: 18
#   J3A: 8
#   JARIB: 6
#   J13A: 2
# These counts can be obtained via a command like the following:
# grep -o 'kIRG_JSource="[^"]*-' ~/ucd/14/ucd.all.flat.xml | sort | uniq -c |
#   sort -rn | sed 's/ *\([^ ]*\)[^"]*"\([^-]*\).*/#   \2: \1/'
#
# Here are some other 'Japan' type properties that aren't used by this script:
# - 'kJis0' has 6,356 (6,354 with On/Kun), but missed 4 Jōyō and 15 Jinmei.
# - 'kIRGDaiKanwaZiten' has 17,864 (12,942 with On/Kun). There's a proposal to
#   remove this property (and expand 'kMorohashi') so it's probably best not to
#   use it: https://www.unicode.org/L2/L2021/21032-unihan-kmorohashi.pdf
