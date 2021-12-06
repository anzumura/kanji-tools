#!/usr/bin/env bash

declare -r program="parseUcdAllFlat.sh"

# This script searches the Unicode 'ucd.all.flat.xml' file for characters that
# have a Japanese reading (On or Kun) and prints out a tab-separated line with
# the following 18 values:
# - Code: Unicode code point (4 or 5 digit hex code)
# - Name: character in UTF-8
# - Block: name of the Unicode block (from the 'blk' tag)
# - Version: Unicode version this character was added (from 'age' tag)
# - Radical: radical number (1 to 214)
# - Strokes: total strokes (including the radical)
# - VStrokes: strokes for first different 'adobe' count (blank if no diffs)
# - Pinyin: optional first pīnyīn (拼音) reading from 'kMandarin'
# - Morohashi: optional 'Dai Kan-Wa Jiten (大漢和辞典)' index number
# - Nelson: optional list of space-separated 'Classic Nelson' ids
# - Joyo: 'Y' if part of Jōyō list or blank
# - Jinmei: 'Y' if part of Jinmeiyō list or blank
# - LinkCode: optional list of comma-separated link values in Unicode
# - LinkName: optional list of comma-separated link values in UTF-8
# - LinkType: optional single value for the type of all the links
# - Meaning: optional semicolin separated English definitions
# - On: optional space-separated Japanese On readings (in all-caps Rōmaji)
# - Kun: optional space-separated Japanese Kun readings (in all-caps Rōmaji)
#
# Location of 'ucd.all.flat.zip': https://unicode.org/Public/UCD/latest/ucdxml
# More info on UCD file: https://unicode.org/reports/tr38/
# This script was used to create 'data/ucd.txt'.
#
# There are over 140K characters in 'ucd.all.flat.txt' and most of them aren't
# relevant to the current functionality of this (kanji-tools) project so apply
# some filtering before parsing (also significantly reduces the time to parse).
#
# Filtering on kIRG_JSource being populated cuts the set to 16,226 entries (IRG
# = 'Ideographic Research Group') and 12,275 of these have at least one Japanese
# reading (On/Kun) or a value in kJoyoKanji. However, kRSAdobe_Japan1_6 gets
# almost the same sized set (12,274, but has a few hundred different entries).
# The advantage of using Adobe is its value contains more accurate stroke count
# information. The kTotalStrokes field doesn't always work well for Japanese
# common characters, i.e., there are over 300 differences in just the 3000 or so
# standard Jōyō + Jinmei. Examples: 4EE5 (以) and 4F3C (似) have kTotalStrokes
# of 4 and 6 respectively, but in Japanese they should be 5 and 7.
#
# In addition to Adobe, also pull in kanji that have a Morohashi ID to help get
# compatibility/variants of kanji that have on/kun readings. Note, 'kMorohashi'
# has 18,168 entries (12,965 with On/Kun) which is more than Adobe so it pulls
# in a few hundred more entries (including some Kentei Kanji).
# Some counts as of Unicode 13.0:
# - has both Adobe and Morohashi: 12,447
# - has Morohashi, but not Adobe: 5,721
# - has Adobe, but not Morohashi: 1,010
#
# Morohashi: Unicode 14.0 has 17,830 unique values in the 'kMorohashi'. This
# This property has one or more index numbers into 'Dai Kan-Wa Jiten' (a massive
# Chinese-Japanese dictionary compiled by Tetsuji Morohashi). There are plans to
# cleanup/expand this property to cover ~50K entries by Unicode 16.0.
#
# Nelson: Unicode 14.0 has 5,442 unique ids in 'kNelson'. This property has one
# or more ids from the 'Classic Nelson' Japanese-English Character Dictionary.
# 'Classic Nelson' was first published in 1962 and the ids remained the same for
# the 'Second Revised Edition' from 1974 (including the Thirtieth Printing in
# 1989). These ids don't match 'New Nelson' which was first publised in 1997.
#
# Morohashi and Nelson ids can be used for looking up kanji by the 'kanjiQuiz'
# program (see Quiz.cpp or run 'kanjiQuiz -h' for details).
#
# Here are some other 'Japan' type properties that aren't used by this script:
# - 'kJis0' has 6,356 (6,354 with On/Kun), but missed 4 Jōyō and 15 Jinmei.
# - 'kIRGDaiKanwaZiten' has 17,864 (12,942 with On/Kun). There's a proposal to
#   remove this property (and expand 'kMorohashi') so it's probably best not to
#   use it: https://www.unicode.org/L2/L2021/21032-unihan-kmorohashi.pdf

# Ensure LANG is 'UTF-8' so things like ${j:0:1} get a single utf8 value instead
# of just a single byte.
if [[ ! $LANG =~ UTF-8 ]]; then
  for s in jp_JP en_US; do
    locale -a | grep -q $s.UTF-8 && export LANG=$s.UTF-8 && break
  done
  [[ ! $LANG =~ UTF-8 ]] && echo "failed to find a UTF-8 locale" && exit 1
fi

if [[ $# -lt 2 ]]; then
  echo "usage: parseUcdAllFlat.sh 'input file' 'output file'"
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
  eval ${xml%/>} # strip trailing '/>' and set vars from remaining XML string
  radical=${kRSUnicode%%[.\']*}
  # Simplified radicals, i.e., a value ending with a single quote, shouldn't be
  # processed unless the radical is 199 (麥 麦). For example, 4336 (䌶) has
  # kRSUnicode="120'.3" so it will result in a non-zero exit code.
  [[ $radical -eq 199 || ! $kRSUnicode =~ ^[0-9]*\' ]]
}

function setOnKun() {
  resultOn=$1
  resultKun=$2
}

# global arrays to help support links for variant and compat kanjis
declare -A morohashi definition on kun linkBack noLink readingLink

# there are 18 Jinmeiyō Kanji that link to other Jinmei, but unfortunately UCD
# data seems to have some mistakes (where the link points from the standard to
# the variant instead). For example 4E98 (亘) has kJinmeiyoKanji="2010:U+4E99"
# and 4E99 (亙) has kJinmeiyoKanji="2010". This contradicts the description of
# the field (since 4E98 is the standard form). Here's the official description:
#   The version year is either 2010 (861 ideographs), 2015 (one ideograph), or
#   2017 (one ideograph), and 230 ideographs are variants for which the code
#   point of the standard Japanese form is specified.
# For now, store the 18 kanji in 'noLink' to block setting links.
while read i; do
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
declare -r morohashiRegex='kMorohashi="[^"]'    # has a Morohashi ID
declare -r nelsonRegex='kNelson="[^"]'          # has a Nelson ID
declare -r adobeRegex='kRSAdobe_Japan1_6="[^"]' # has an Adobe ID

printResulsFilter="$onKunRegex|$morohashiRegex|$nelsonRegex|$adobeRegex"

# 'populateArrays': populates 'on', 'kun', 'definition' and 'morohashi' arrays.
# Arrays need to be populated before 'printResults' to handle links to entries
# later in the file like '5DE2 (巢)' which links to '5DE3 (巣)'.
function populateArrays() {
  log 'Populate arrays ... ' -n
  local -i count=0
  local s
  while read -r s; do
    setVars "$s" kDefinition kJapaneseOn kJapaneseKun || continue
    if [[ -n $kMorohashi ]]; then
      # Remove leading 0's and change single quotes to 'P' (Prime) so that 04138
      # changes to 4138 (maps to 嗩) and 04138' changes to 4138P (maps to 嘆).
      kMorohashi=$(echo $kMorohashi | sed -e 's/^0*//' -e "s/'/P/g")
      # There are a few kMorohashi values that are all 0's so check if non-empty
      # again before setting global array.
      [[ -n $kMorohashi ]] && morohashi[$cp]=$kMorohashi
    fi
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
  done < <(grep -E "($onKunRegex|$morohashiRegex)" $ucdFile)
  echo "set $count reading and ${#morohashi[@]} morohashi"
}

function hasReading() {
  [[ -n ${on[$1]}${kun[$1]} ]]
}

function hasMorohashi() {
  [[ -n ${morohashi[$1]} ]]
}

# 'canLoadFrom': has exit status of 0 (true) if '$1' in refers to a kanji with
# an 'On' or 'Kun' reading or has a 'readingLink'. Thus 'readings' can be loaded
# from the '$1' if required.
function canLoadFrom() {
  hasReading $1 || [[ -n ${readingLink[$1]} ]]
}

declare -A defTypeUtf defTypeCode
declare -a linkErrors

# 'findDefinitionLinksForType': finds links based on type ($1) string matches in
# 'kDefinition' field and updates 'defType' arrays.
function findDefinitionLinksForType() {
  # kDefinition can have multiple values separated by ';', but there are cases
  # where just brackets or commas are used. For example, 4CFD (䳽) has:
  #   (non-classical form of 鸖) (same as 鶴) crane
  local -r recordFilter="kRSUnicode=\"[0-9]*'" delim=';,)' unicode=[A-F0-9] \
    sedEnd='*\).*/linkFrom=\1:link=\2/' start='[^\"]*' nonAscii='[^ -~]'
  local -r sedStart="s/.*cp=\"\($start\).*" sep=[$delim] end=[^$delim'\"']*
  local -r definitionStart='kDefinition=\"'$end
  # Some kanji have multiple 'type' strings in their kDefinition. For now just
  # check the first 3. For example, 36B3 (㚶) has kDefinition:
  #   (same as 姒) wife of one's husband's elder brother; (in ancient China) the
  #   elder of twins; a Chinese family name, (same as 姬) a handsome girl; a
  #   charming girl; a concubine; a Chinese family name
  # Note: 'link' can occur more than once if there are multiple variants for it.
  # This is true for '64DA (據)' which has '3A3F (㨿)' and '3A40 (㩀)'.
  local -i utf=0 code=0
  local i j k s linkFrom link
  for i in '' $start$sep$end $start$sep$sep$end; do
    s="$definitionStart$i$1[-\'a-z0-9 ]*"
    # loop to handle strings like 'same as X' where X is a UTF-8 kanji
    for j in $(grep -E "$s$nonAscii{1,}" $ucdFile | grep -v $recordFilter |
      sed "$sedStart$s\($nonAscii$sedEnd"); do
      eval ${j/:/ }
      if [[ -z $link ]]; then
        linkErrors+=("[UTF cp=$linkFrom]")
      elif [[ -z ${readingLink[$linkFrom]} ]]; then
        # some kanji have multiple links like 4640 (䙀) which has 'same as 綳繃'
        while read -r -n 1 k; do
          # get the Unicode (4 of 5 digit hex with caps) value from UTF-8 link
          if link=$(echo -n $k | iconv -s -f UTF-8 -t UTF-32BE | xxd -p); then
            printf -v link '%04X' 0x$link 2>/dev/null && hasReading $link &&
              readingLink[$linkFrom]=$link && utf+=1 && break ||
              linkErrors+=("[cp=$linkFrom link=$k iconv=$link")
          else
            linkErrors+=("[cp=$linkFrom link=$k]")
          fi
        done < <(echo -n $link)
      fi
    done
    # loop to handle strings like 'same as U+ABCD ...' (so no conversion needed)
    for j in $(grep -E "${s}U\+$unicode*" $ucdFile | grep -v $recordFilter |
      sed "$sedStart${s}U+\($unicode$sedEnd"); do
      eval ${j/:/ }
      if [[ -z $link ]]; then
        linkErrors+=("[CODE cp=$linkFrom]")
      elif [[ -z ${readingLink[$linkFrom]} ]]; then
        hasReading $link && readingLink[$linkFrom]=$link && code+=1
      fi
    done
  done
  defTypeUtf[$1]=$utf
  defTypeCode[$1]=$code
}

function printDefinitionLinkCounts {
  local -i total=0 len=0 utf code count
  local s types
  for s in "${!defTypeUtf[@]}" "${!defTypeCode[@]}"; do
    [[ ${#s} -gt $len ]] && len=${#s}
    [[ -z $types ]] && types=$s || types+="\n$s"
  done
  len+=2
  while read -r s; do
    utf=${defTypeUtf[$s]}
    code=${defTypeCode[$s]}
    count=$((utf + code))
    printf "  %-${len}s : %4d (utf8 %4d, code %d)\n" "'$s'" $count $utf $code
    total+=count
  done < <(echo -e "$types" | sort -u)
  printf "  %-${len}s : %4d (manually set)\n" '' $((${#readingLink[@]} - total))
  [[ ${#convertErrors[@]} -gt 0 ]] && printf "  %-${len}s : %4d %s\n" \
    'Convert Errors' ${#convertErrors[@]} "${convertErrors[@]}"
}

# 'findDefinitionLinks': find links based on 'kDefinition' field. For example,
# if the field starts with '(same as X' then store a link from 'cp' to 'X'. This
# function populates 'readingLink' array.
function findDefinitionLinks() {
  log 'Find definition links ... ' -n
  local s
  for s in 'a variant' interchangeable 'same as' non-classical Variant; do
    printResulsFilter+="|$s "
    findDefinitionLinksForType "$s"
  done
  # Pull in some Kentei kanji that are missing on/kun via links (the links have
  # the same definitions and expected on/kun).
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
  # If there are mutiple Adobe refs then check 'VStrokes' from the second one in
  # the list. Some have more than two like 傑 (5091) which has kRSAdobe_Japan1_6
  # "C+1852+9.2.11 V+13433+9.2.11 V+13743+9.2.10". If strokes and vstrokes are
  # different then check to see if they should be swapped. Swapping fixes stroke
  # count for some charaters and breaks it for others so only swap if 'vstrokes'
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
# or 'hasMorohashi'). This function can only be used after 'setVars' and
# it also looks at 'resultOn' and 'resultKun'.
function getLinks() {
  local s
  # For non-Jouyou/non-Jinmei kanji, try to find meaningful links based on
  # kTraditionalVariant, kCompatibilityVariant or kSemanticVariant fields or
  # from 'readingLink' array (based on kDefinition field).
  getTraditionalLinks $1
  if [[ -z $linkType ]]; then
    for s in $kSimplifiedVariant; do
      linkTo=${s#*+} # remove leading U+
      $1 $linkTo && linkType=Simplified && break
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
    # allow adding morohashi traditional links if reading ones are found
    getTraditionalLinks hasMorohashi
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
  totalMeaning=0 totalPinyin=0 totalReading=0

# 'countLinkType' increments totals for each link type (used in summary info)
function countLinkType() {
  case $linkType in
  Jinmei) jinmeiLinks+=1 ;;
  Traditional) traditionalLinks+=1 ;;
  Simplified) simplifiedLinks+=1 ;;
  Compatibility) compatibilityLinks+=1 ;;
  Definition) definitionLinks+=1 ;;
  Semantic) semanticLinks+=1 ;;
  *) ;;
  esac
}

declare -A uniqueMorohashi uniqueNelson

function processRecord() {
  local -r localMorohashi=${morohashi[$cp]}
  local localDefinition=${definition[$cp]} loadFrom
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
    getLinks canLoadFrom && loadFrom=${linkTo%%,*} || getLinks hasMorohashi
  fi
  if [[ -z $resultOn$resultKun ]]; then
    if [[ -n $loadFrom ]]; then
      hasReading $loadFrom || loadFrom=${readingLink[$loadFrom]}
      localDefinition=${definition[$loadFrom]}
      resultOn=${on[$loadFrom]}
      resultKun=${kun[$loadFrom]}
      # This should never happen, but keep as a sanity check
      [[ -z $resultOn$resultKun ]] &&
        echo "ERROR: link not found for cp $cp (link $linkType $loadFrom)"
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
      6FDB) setOnKun 'BOU MOU' KOSAME ;;               # 濛
      6663) setOnKun 'SEI SETSU' 'AKIRAKA KASHIKOI' ;; # 晣
      69D4) setOnKun KOU HANETSURUBE ;;                # 槔
      6A94) setOnKun TOU ;;                            # 檔
      7B53) setOnKun KEI 'KOUGAI KANZASHI' ;;          # 筓
      7CF1) setOnKun GETSU 'KOUJI MOYASHI' ;;          # 糱
      83C6) setOnKun SHU ;;                            # 菆 (Nelson 3961)
      # if there are no readings and no Morohashi ID then skip this record
      *) [[ -n $localMorohashi ]] || return 1 ;;
      esac
    fi
  fi
  vstrokes=0
  if [[ -z ${kRSAdobe_Japan1_6} ]]; then
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
  [[ -n $localMorohashi && -z ${uniqueMorohashi[$localMorohashi]} ]] &&
    uniqueMorohashi[$localMorohashi]=1
  if [[ -n $kNelson ]]; then
    local s
    # remove leading 0's from all Nelson ids in the list
    kNelson=$(echo $kNelson | sed -e 's/^0*//' -e 's/ 0*/ /g')
    for s in $kNelson; do
      [[ -z ${uniqueNelson[$s]} ]] && uniqueNelson[$s]=1
    done
  fi
  countLinkType
  # don't print 'vstrokes' if it's 0
  echo -e "$cp\t\U$cp\t$blk\t$age\t$radical\t$strokes\t${vstrokes#0}\t\
$kMandarin\t$localMorohashi\t$kNelson\t${kJoyoKanji:+Y}\t${kJinmeiyoKanji:+Y}\t\
$linkTo\t${linkTo:+\U${linkTo//,/,\\U}}\t$linkType\t$localDefinition\t\
$resultOn\t$resultKun" >>$outFile
}

function printResults() {
  log "Print results to '$outFile' ... " -n
  echo -e "Code\tName\tBlock\tVersion\tRadical\tStrokes\tVStrokes\tPinyin\t\
Morohashi\tNelson\tJoyo\tJinmei\tLinkCode\tLinkName\tLinkType\tMeaning\tOn\t\
Kun" >$outFile
  local s
  local -i count=0
  while read -r s; do
    # No need to unset fields that are in every record such as 'cp', 'age' and
    # 'blk', but need to unset all the optional fields used in 'processRecord'.
    setVars "$s" kTraditionalVariant kSimplifiedVariant kCompatibilityVariant \
      kSemanticVariant kRSAdobe_Japan1_6 kMandarin kNelson &&
      processRecord "$s" && count+=1
  done < <(grep -E "($printResulsFilter)" $ucdFile)
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

checkTotal 'Unique Morohashi' ${#uniqueMorohashi[@]} 17830
checkTotal 'Unique Nelson ID' ${#uniqueNelson[@]} 5442
checkTotal 'Total Jouyou' $totalJoyo 2136
checkTotal 'Total Jinmei' $totalJinmei 633
checkTotal 'Jinmei Links' $jinmeiLinks 230

echo -e "Other Links:\n  Traditional $traditionalLinks, Simplified \
$simplifiedLinks, Compatibility $compatibilityLinks, Definition \
$definitionLinks, Semantic $semanticLinks"
echo -e "Other Fields:\n  Meaning $totalMeaning, Pinyin $totalPinyin, Reading \
$totalReading"
