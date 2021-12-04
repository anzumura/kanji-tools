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

# 'setVars' sets global vars for each property in the XML record passed in. It
# will also unset any vars specified after the XML string. The exit status is 0
# if the record should be processed.
function setVars() {
  local -r xml=${1:6} # strip leading '<char '
  shift               # more 'vars to unset' can be specified after $1 (xml arg)
  unset -v kJoyoKanji kJinmeiyoKanji kMorohashi $*
  eval ${xml%/>} # strip trailing '/>' and set vars from remaining XML string
  radical=${kRSUnicode%%[.\']*}
  # Simplified radicals, i.e., a value ending with a single quote, shouldn't be
  # processed unless the radical is 199 (麥 麦). For example, 4336 (䌶) has
  # kRSUnicode="120'.3" so it will result in a non-zero exit code.
  [[ $radical -eq 199 || ! $kRSUnicode =~ ^[0-9]*\' ]]
}

function setOnKun() {
  resultOn="$1"
  resultKun="$2"
}

# global arrays to help support links for variant and compat kanjis
declare -A morohashi definition on kun linkBack noLink readingLink morohashiLink

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

# 'onKun' is used as a filter by 'populateVariantLinks' since we only want to
# link to an entry that has an 'On' and/or 'Kun' reading.
declare -r onKunRegex='kJapanese[OK].*n="[^"]'

# 'printResults' loop uses 'onKun' as well as the following (for variants):
declare -r morohashiRegex='kMorohashi="[^"]'    # has a Morohashi ID
declare -r nelsonRegex='kNelson="[^"]'          # has a Nelson ID
declare -r adobeRegex='kRSAdobe_Japan1_6="[^"]' # has an Adobe ID

printResulsFilter="$onKunRegex|$morohashiRegex|$nelsonRegex|$adobeRegex"

# 'populateArrays': populates 'on', 'kun', 'definition' and 'morohashi' arrays.
# Arrays need to be populated before 'printResults' to handle links to entries
# later in the file like '5DE2 (巢)' which links to '5DE3 (巣)'.
function populateArrays() {
  log "Populate arrays ... " -n
  local -i count=0
  while read -r i; do
    setVars "$i" kDefinition kJapaneseOn kJapaneseKun || continue
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
      [[ $kJoyoKanji =~ U+ ]] && cp="$cp ${kJoyoKanji#U+}"
    elif [[ $kJinmeiyoKanji =~ U+ ]]; then
      kJinmeiyoKanji=${kJinmeiyoKanji#*+}
      linkBack[$kJinmeiyoKanji]=$cp
      cp="$cp $kJinmeiyoKanji"
    fi
    if [[ -n $kJapaneseOn$kJapaneseKun ]]; then
      for link in $cp; do
        definition[$link]="$kDefinition"
        on[$link]="$kJapaneseOn"
        kun[$link]="$kJapaneseKun"
        count=++count
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

# 'canLinkTo' has an exit status of 0 (true) if the value passed in can be used
# as a link, i.e., it refers to a kanji with an 'on' or 'kun' reading or refers
# to a 'readingLink' entry.
function canLinkTo() {
  hasReading $1 || [[ -n ${readingLink[$1]} ]]
}

# 'canLinkToMorohashi' has an exit status of 0 (true) if the value passed in has
# has a morohashi id or refers to a kanji with a 'morohashiLink' entry.
function canLinkToMorohashi() {
  hasMorohashi $1 || [[ -n ${morohashiLink[$1]} ]]
}

# 'findDefinitionLinksForType' finds links based on type ($1) string matches in
# 'kDefinition' field. The second arg should be 'true' or 'false' and indicates
# if the links should point at other kanji having a reading or not.
function findDefinitionLinksForType() {
  local -r def='kDefinition=\"[^\";,)]*' nonAscii='[^\x00-\x7F]{1,}'
  # Some kanji have multiple 'type' strings in their kDefinition. For now just
  # check the first 3. For example, 36B3 (㚶) has kDefinition:
  #   (same as 姒) wife of one's husband's elder brother; (in ancient China) the
  #   elder of twins; a Chinese family name, (same as 姬) a handsome girl; a
  #   charming girl; a concubine; a Chinese family name
  for i in '' '[^\"]*[;,)][^\";,)]*' '[^\"]*[;,)][;,)][^\";,)]*'; do
    for j in $(grep -E "$def$i$1 ($nonAscii|U[^ ]* $nonAscii)" $ucdFile |
      grep -v "kRSUnicode=\"[0-9]*'" | sed 's/ U+[A-F0-9]*//g' |
      sed "s/.*cp=\"\([^\"]*\).*$def$i$1 \([^ ;,)]*\).*/\2:\1/"); do
      local cp=${j#*:}
      if [[ -z ${readingLink[$cp]} && -z ${morohashiLink[$cp]} ]]; then
        # 's' can occur more than once if there are multiple variants for it.
        # This is true for '64DA (據)' which has '3A3F (㨿)' and '3A40 (㩀)'.
        local s=$(echo -n ${j:0:1} | iconv -f UTF-8 -t UTF-32BE | xxd -p)
        # get the Unicode (4 of 5 digit hex with caps) value from UTF-8 kanji
        s=$(printf '%04X' 0x$s)
        if $2; then
          hasReading $s && readingLink[$cp]=$s
        elif hasMorohashi $s; then
          morohashiLink[$cp]=$s
        fi
      fi
    done
  done
}

# 'findDefinitionLinks': find links based on 'kDefinition' field. For example,
# if the field starts with '(same as X' then store a link from 'cp' to 'X'. This
# function populates 'readingLink' and 'morohashiLink' arrays
function findDefinitionLinks() {
  log "Find definition links ... " -n
  for reading in true false; do
    for type in 'a variant of' interchangeable 'same as' non-classical \
      'Variant of'; do
      $reading && printResulsFilter="$printResulsFilter|$type "
      findDefinitionLinksForType "$type" $reading
    done
  done
  # Pull in some Kentei kanji that are missing on/kun via links (the links have
  # the same definitions and expected on/kun).
  readingLink[3D4E]=6F97 # link 㵎 to 澗
  readingLink[5ECF]=5ED0 # link 廏 to 廐
  readingLink[9D25]=9D2A # link 鴥 to 鴪
  readingLink[6AA8]=69D8 # link 檨 to 様 (Nelson 2363)
  echo "found ${#readingLink[@]} reading and ${#morohashiLink[@]} morohashi"
}

declare -i strokes vstrokes

# 'getStrokesFromAdobeRef' tries to find the correct stroke count for Japanese
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

# 'getTraditionalLinks' sets 'linkTo' based on kTraditionalVariant field. More
# than one link can exist, but must satisfy the function passed in $1 (see
# 'getLinks' function below).
function getTraditionalLinks() {
  for s in $kTraditionalVariant; do
    s=${s#*+} # remove leading U+
    # Skip if kTraditionalVariant is the same as current record being processed.
    # For example, 'cp' 5413 (吓) has kTraditionalVariant="U+5413 U+5687".
    if [[ $s != $cp && ! ,$linkTo, =~ ,$s, ]] && $1 $s; then
      linkType=Traditional
      # allow storing multiple traditional links
      [[ -z $linkTo ]] && linkTo=$s || linkTo=$linkTo,$s
    fi
  done
}

# 'getLinks' sets 'linkTo' to one on more link values and sets 'linkType' if the
# links point to a value that satisfies the function passed in $1 ('canLinkTo'
# or 'canLinkToMorohashi'). This function can only be used after 'setVars' and
# it also looks at 'resultOn' and 'resultKun'.
function getLinks() {
  # For non-Jouyou/non-Jinmei kanji, try to find meaningful links based on
  # kTraditionalVariant, kCompatibilityVariant or kSemanticVariant fields or
  # from 'readingLink' array (based on kDefinition field).
  getTraditionalLinks $1
  if [[ -z $linkType ]]; then
    for s in $kSimplifiedVariant; do
      linkTo=${s#*+} # remove leading U+
      $1 $linkTo && linkType=Simplified && break
    done
    if [[ -z $linkType ]]; then
      if [[ $kCompatibilityVariant =~ U+ ]]; then
        # kCompatibilityVariant never has more than one value
        linkTo=${kCompatibilityVariant#*+} # remove leading U+
        $1 $linkTo && linkType=Compatibility
      fi
      # Only use definition and semantic links if there's no reading since they
      # can result in linking together kanji that don't seem related. Also, only
      # support reading links (not morohashi).
      if [[ -z $linkType && -z $resultOn$resultKun && $1 == canLinkTo ]]; then
        if [[ -n ${readingLink[$cp]} ]]; then
          linkTo=${readingLink[$cp]}
          linkType=Definition
        else
          # kSemanticVariant can be Unicode like "U+8209" or a compound like
          # "U+71D0&lt;kMatthews U+7CA6&lt;kMatthews" so need to strip '&'
          for s in $kSemanticVariant; do
            s=${s#*+} # remove leading U+
            linkTo=${s%%&*}
            canLinkTo $linkTo && linkType=Semantic && break
          done
        fi
      fi
    fi
  elif [[ $1 == canLinkTo ]]; then
    # allow adding morohashi traditional links if reading ones are found
    getTraditionalLinks canLinkToMorohashi
  fi
  if [[ -z $linkType ]]; then
    linkTo=
    return 1
  fi
}

# All entries have 'cp', 'age', 'blk', 'radical' and 'strokes' as well as at
# least one of 'resultOn' or 'resultKun' so no need to count them, but count
# some other optional fields of interest.
declare -i totalJoyo=0 totalJinmei=0 jinmeiLinks=0 traditionalLinks=0 \
  simplifiedLinks=0 compatibilityLinks=0 definitionLinks=0 semanticLinks=0 \
  totalMeaning=0 totalPinyin=0 totalReading=0

# 'countLinkType' increments totals for each link type (used in summary info)
function countLinkType() {
  case $linkType in
  Jinmei) jinmeiLinks=++jinmeiLinks ;;
  Traditional) traditionalLinks=++traditionalLinks ;;
  Simplified) simplifiedLinks=++simplifiedLinks ;;
  Compatibility) compatibilityLinks=++compatibilityLinks ;;
  Definition) definitionLinks=++definitionLinks ;;
  Semantic) semanticLinks=++semanticLinks ;;
  *) ;;
  esac
}

declare -A uniqueMorohashi uniqueNelson

function processRecord() {
  local -r localMorohashi=${morohashi[$cp]}
  local localDefinition=${definition[$cp]} loadFrom
  # 'resultOn' and 'resultKun' come from 'on' and 'kun' arrays, but can also be
  # set by 'setOnKun' and are used by 'getLinks' so they can't be local
  resultOn=${on[$cp]}
  resultKun=${kun[$cp]}
  # 'linkTo' and 'linkType' can be modified by 'getLinks' function
  linkTo=
  linkType=
  if [[ -n $kJoyoKanji ]]; then
    # unset kJoyoKanji if official version is the 'link' entry
    [[ $kJoyoKanji =~ U+ ]] && unset -v kJoyoKanji || totalJoyo=++totalJoyo
  elif [[ -n $kJinmeiyoKanji ]]; then
    if [[ $kJinmeiyoKanji =~ U+ ]]; then
      loadFrom=${kJinmeiyoKanji#*+}
      [[ -z ${noLink[$cp]} ]] && linkTo=$loadFrom
    else
      linkTo=${linkBack[$cp]}
    fi
    [[ -z $linkTo ]] && totalJinmei=++totalJinmei || linkType=Jinmei
  else
    getLinks canLinkTo && loadFrom=${linkTo%%,*} || getLinks canLinkToMorohashi
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
      34E4) setOnKun "KATSU" ;;                        # 㓤 (Nelson 677)
      3C7E) setOnKun "KAI" ;;                          # 㱾 (Nelson 2453)
      3C83) setOnKun "KYUU" ;;                         # 㲃 (Nelson 2459)
      4BC2) setOnKun "SHIN" ;;                         # 䯂
      6479) setOnKun "BO" "MO" ;;                      # 摹 (Nelson 4035)
      6532) setOnKun "KI" "KATAMUKU SOBADATERU" ;;     # 攲 (Nelson 2041)
      6FDB) setOnKun "BOU MOU" "KOSAME" ;;             # 濛
      6663) setOnKun "SEI SETSU" "AKIRAKA KASHIKOI" ;; # 晣
      69D4) setOnKun "KOU" "HANETSURUBE" ;;            # 槔
      6A94) setOnKun "TOU" ;;                          # 檔
      7B53) setOnKun "KEI" "KOUGAI KANZASHI" ;;        # 筓
      7CF1) setOnKun "GETSU" "KOUJI MOYASHI" ;;        # 糱
      83C6) setOnKun "SHU" ;;                          # 菆 (Nelson 3961)
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
  [[ -n $localDefinition ]] && totalMeaning=++totalMeaning
  [[ -n $resultOn$resultKun ]] && totalReading=++totalReading
  if [[ -n $kMandarin ]]; then
    # for kanji with multiple Pinyin readings just keep the first for now
    kMandarin=${kMandarin%% *}
    totalPinyin=++totalPinyin
  fi
  [[ -n $localMorohashi && -z ${uniqueMorohashi[$localMorohashi]} ]] &&
    uniqueMorohashi[$localMorohashi]=1
  if [[ -n $kNelson ]]; then
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
  local -i count=0
  echo -e "Code\tName\tBlock\tVersion\tRadical\tStrokes\tVStrokes\tPinyin\t\
Morohashi\tNelson\tJoyo\tJinmei\tLinkCode\tLinkName\tLinkType\tMeaning\tOn\t\
Kun" >$outFile
  while read -r i; do
    # No need to unset fields that are in every record such as 'cp', 'age' and
    # 'blk', but need to unset all the optional fields used in 'processRecord'.
    setVars "$i" kTraditionalVariant kSimplifiedVariant kCompatibilityVariant \
      kSemanticVariant kRSAdobe_Japan1_6 kMandarin kNelson || continue
    processRecord "$i" && count=++count
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

checkTotal "Unique Morohashi" ${#uniqueMorohashi[@]} 17830
checkTotal "Unique Nelson ID" ${#uniqueNelson[@]} 5442
checkTotal "Total Jouyou" $totalJoyo 2136
checkTotal "Total Jinmei" $totalJinmei 633
checkTotal "Jinmei Links" $jinmeiLinks 230

echo -e "Other Links:\n  Traditional $traditionalLinks, Simplified $simplifiedLinks, \
Compatibility $compatibilityLinks, Definition $definitionLinks, Semantic \
$semanticLinks"
echo -e "Other Fields:\n  Meaning $totalMeaning, Pinyin $totalPinyin, Reading \
$totalReading"
