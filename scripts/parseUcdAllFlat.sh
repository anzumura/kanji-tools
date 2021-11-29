#!/usr/bin/env bash

declare -r program="parseUcdAllFlat.sh"

# This script searches the Unicode 'ucd.all.flat.xml' file for characters that
# have a Japanese reading (On or Kun) and prints out a tab-separated line with
# the following 17 values:
# - Code: Unicode code point (4 or 5 digit hex code)
# - Name: character in utf8
# - Block: name of the Unicode block (from the 'blk' tag)
# - Version: Unicode version this character was added (from 'age' tag)
# - Radical: radical number (1 to 214)
# - Strokes: total strokes (including the radical)
# - VStrokes: strokes for first different 'adobe' count (blank if no diffs)
# - Pinyin: most customary pīnyīn (拼音) reading (from 'kMandarin' tag)
# - Morohashi: optional 'Dai Kan-Wa Jiten (大漢和辞典)' index number
# - Nelson: optional list of space-separated 'Classic Nelson' ids
# - Joyo: 'Y' if part of Jōyō list or blank
# - Jinmei: 'Y' if part of Jinmeiyō list or blank
# - LinkCode: 230 Jinmei (of the 863 total) are variants of other Jōyō/Jinmei
# - LinkName: link character in utf8
# - Meaning: semicolin separated English definitions
# - On: space-separated Japanese On readings (in all-caps Rōmaji)
# - Kun: space-separated Japanese Kun readings (in all-caps Rōmaji)
#
# Location of 'ucd.all.flat.zip': https://unicode.org/Public/UCD/latest/ucdxml
# More info on UCD file: https://unicode.org/reports/tr38/
# This script was used to create 'data/ucd.txt'.
#
# There are over 140K characters in 'ucd.all.flat.txt' and most of them aren't
# relevant to the current functionality of 'kanji-tools' project so apply some
# filtering before parsing (also significantly reduces the time to parse).
#
# Filtering on kIRG_JSource being populated cuts the set to 16,226 entries (IRG
# = 'Ideographic Research Group') and 12,275 of these have at least one Japanese
# reading (On/Kun) or a value in kJoyoKanji. However, kRSAdobe_Japan1_6 gets
# almost the same sized set (12,274, but has a few hundred different entries).
# The advantage of using Adobe is it's reference contains more accurate stroke
# count information. The kTotalStrokes field doesn't work that well for Japanese
# common characters, i.e., there are over 300 differences in just the 3000 or so
# standard Jōyō + Jinmei. Examples: 4EE5 (以) and 4F3C (似) have kTotalStrokes of
# 4 and 6 respectively, but in Japanese they should be 5 and 7.
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
# Nelson: As of Unicode 14.0 there are 5,399 entries with a non-empty 'kNelson'
# (Classic 'Nelson Japanese-English Character Dictionary') field. Load these ids
# to show in 'review mode' and also for looking up from the command-line (see
# kanjiQuiz -h for details).
#
# Morohashi: As of Unicode 14.0 there are 18,169 entries with a non-empty
# 'kMorohashi' field. This property holds index numbers into Dai Kan-Wa Jiten (a
# massive Chinese-Japanese dictionary compiled by Tetsuji Morohashi). There are
# plans to cleanup/expand this property to cover ~50K entries by Unicode 16.0.
#
# Here are other 'Japan' type source tags that are not used by this script:
# - 'kJis0' has 6,356 (6,354 with On/Kun), but missed 4 Jōyō and 15 Jinmei.
# - 'kIRGDaiKanwaZiten' has 17,864 (12,942 with On/Kun). There's also a proposal
#   to remove this property (and expand 'kMorohashi') so it's probably best not
#   to use it: https://www.unicode.org/L2/L2021/21032-unihan-kmorohashi.pdf

if [[ $# -lt 2 ]]; then
  echo "usage: parseUcdAllFlat.sh 'input file' 'output file'"
  echo "  input file: ucd.all.flat.xml file (see script comments for details)"
  echo "  output file: where to write output (for example, data/ucd.txt)"
  exit 1
fi
declare -r ucdFile=$1 outFile=$2

function log() {
  echo "$(date +%T): $1"
}

log "Begin parsing UCD data from '$ucdFile'"

function setOnKun() {
  resultOn="$1"
  resultKun="$2"
}

# global arrays to help support links for variant and compat kanjis
declare -A definition on kun linkBack noLink variantLink

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
declare -r onKun='kJapanese[OK].*n="[^"]'

# 'printResults' loop uses 'onKun' as well as the following (for variants):
declare -r morohashi='kMorohashi="[^"]'    # has a Morohashi ID
declare -r nelson='kNelson="[^"]'          # has a Nelson ID
declare -r adobe='kRSAdobe_Japan1_6="[^"]' # has an Adobe ID

printResulsFilter="$onKun|$morohashi|$nelson|$adobe"

# 'findVairantLinks': find links based on 'kDefinition' field. For example, if
# the field starts with '(same as X' then store a link from 'cp' to 'X'.
function findVariantLinks() {
  local -r def='kDefinition=\"[(]*'
  local -r nonAscii='[^\x00-\x7F]{1,}'
  for type in 'a variant of' 'interchangeable' 'same as' 'non-classical' \
    'Variant of'; do
    local secondEntry=false
    printResulsFilter="$printResulsFilter|$type "
    for i in $(grep -E "$def$type ($nonAscii\)|U\+[A-F0-9]{4,5} $nonAscii)" \
      $ucdFile | sed 's/ U+[A-F0-9]*//g' |
      sed "s/.*cp=\"\([^\"]*\).*$type \([^ ,)]*\).*/\1 \2/"); do
      if $secondEntry; then
        # get the Unicode (4 of 5 digit hex with caps) value from UTF-8 kanji
        local s=$(echo -n ${i:0:1} | iconv -f UTF-8 -t UTF-32BE | xxd -p)
        # cp (for variant) is unique, but the original kanji 's' can occur more
        # than once, i.e., if there are multiple variants for 's'. This is true
        # for 64DA (據) which has variants 3A3F (㨿) and 3A40 (㩀).
        variantLink[$cp]=$(printf '%04X' 0x$s)
        secondEntry=false
      else
        local cp=$i
        secondEntry=true
      fi
    done
  done
  # Pull in some Kentei kanji that are missing on/kun via links (the links have
  # the same definitions and expected on/kun).
  variantLink[3D4E]=6F97 # link 㵎 to 澗
  variantLink[5ECF]=5ED0 # link 廏 to 廐
  variantLink[9D25]=9D2A # link 鴥 to 鴪
  variantLink[6AA8]=69D8 # link 檨 to 様 (Nelson 2363)
}

# 'populateOnKun': populates arrays for all kanji having 'on' or 'kun' readings.
# Note, a link can point to an entry later in the file like 5DE2 (巢) which
# links to 5DE3 (巣) so populate on/kun first before calling 'printResults'.
function populateOnKun() {
  while read -r i; do
    unset -v kJoyoKanji kJinmeiyoKanji kDefinition kJapaneseOn kJapaneseKun
    i=${i:6}
    eval ${i%/>}
    if [[ -n $kJoyoKanji ]]; then
      # There are 4 entries with a Joyo link: 5265, 53F1, 586B and 982C. Store
      # definition/on/kun since since they are missing for some linked Jinmeiyo
      # Kanji as well as Joyo 𠮟 (U+20B9F) which replaces 叱 (53F1) しか-る.
      [[ $kJoyoKanji =~ U+ ]] && cp="$cp ${kJoyoKanji#U+}"
    else
      if [[ $kJinmeiyoKanji =~ U+ ]]; then
        kJinmeiyoKanji=${kJinmeiyoKanji#*+}
        linkBack[$kJinmeiyoKanji]=$cp
        cp="$cp $kJinmeiyoKanji"
      fi
    fi
    for link in $cp; do
      definition[$link]="$kDefinition"
      on[$link]="$kJapaneseOn"
      kun[$link]="$kJapaneseKun"
    done
  done < <(grep $onKun $ucdFile)
}

# keep a counts of entries written to 'outFile'
declare -i total=0 jouyou=0 jinmei=0 linkedJinmei=0

function printResults() {
  echo -e "Code\tName\tBlock\tVersion\tRadical\tStrokes\tVStrokes\tPinyin\t\
Morohashi\tNelson\tJoyo\tJinmei\tLinkCode\tLinkName\tMeaning\tOn\tKun" >$outFile
  while read -r i; do
    # No need to unset fields that are in every record such as 'cp', 'age' and
    # 'blk', but need to unset all of the optional fields used below.
    unset -v kJoyoKanji kJinmeiyoKanji kCompatibilityVariant kSemanticVariant \
      kTraditionalVariant kRSAdobe_Japan1_6 kMandarin kMorohashi kNelson
    i=${i:6}
    eval ${i%/>}
    local localDef=${definition[$cp]}
    # 'resultOn' and 'resultKun' come from 'on' and 'kun' arrays, but can also
    # be set by the 'setOnKun' function so they can't be local variables.
    resultOn=${on[$cp]}
    resultKun=${kun[$cp]}
    local loadFrom=
    local linkTo=
    if [[ -n $kJoyoKanji ]]; then
      if [[ $kJoyoKanji =~ U+ ]]; then
        # Need to unset kJoyoKanji since official version is the 'link' entry.
        unset -v kJoyoKanji
      fi
    elif [[ -n $kJinmeiyoKanji ]]; then
      if [[ $kJinmeiyoKanji =~ U+ ]]; then
        loadFrom=${kJinmeiyoKanji#*+}
        [[ -z ${noLink[$cp]} ]] && linkTo=$loadFrom
      else
        linkTo=${linkBack[$cp]}
      fi
    elif [[ -z $resultOn$resultKun ]]; then
      if [[ $kCompatibilityVariant =~ U+ ]]; then
        loadFrom=${kCompatibilityVariant#*+}
        linkTo=$loadFrom
      elif [[ -n ${variantLink[$cp]} ]]; then
        loadFrom=${variantLink[$cp]}
        linkTo=$loadFrom
      else
        if [[ -z $kSemanticVariant ]]; then
          kSemanticVariant="$kTraditionalVariant"
        fi
        if [[ $kSemanticVariant =~ U+ ]]; then
          kSemanticVariant=${kSemanticVariant##*+} # remove leading U+
          loadFrom=${kSemanticVariant%%&*}         # remove any trailing &lt ...
          linkTo=$loadFrom
        fi
      fi
    fi
    if [[ -n $loadFrom ]]; then
      localDef=${definition[$loadFrom]}
      resultOn=${on[$loadFrom]}
      resultKun=${kun[$loadFrom]}
    fi
    if [[ -z $resultOn$resultKun ]]; then
      # Support cases where UCD is missing on/kun for Kentei kanji or kanji with
      # Nelson IDs. If a special case is used then also clear any 'linkTo' value
      # since that value didn't result in an on/kun getting loaded. 4BC2 is not
      # Kentei, but it's in 'wiki-stokes.txt' file.
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
      *) continue ;;                                   # skip if no readings
      esac
      # UCD data doesn't have entries for these Nelson IDs: 125, 149, 489, 1639
      linkTo=
    fi
    #
    # Radical and Strokes
    #
    kRSUnicode=${kRSUnicode%% *}
    # some kRSUnicode entries have ' after the radical like 4336 (䌶): 120'.3
    kRSUnicode=${kRSUnicode/\'/}
    local -i radical=${kRSUnicode%\.*} vstrokes=0
    if [[ -z ${kRSAdobe_Japan1_6} ]]; then
      kTotalStrokes=${kTotalStrokes%% *}
      local -i strokes=$kTotalStrokes
    else
      # Some characters have multiple adobe references so start with first one
      local s=${kRSAdobe_Japan1_6%%\ *}
      # Adobe refs have the form 'C+num+x.y.z' where 'x' is the radical number,
      # 'y' is number of strokes for the radical and 'z' is remaining strokes.
      s=${s##*+}                        # remove the 'C+num+' prefix
      s=${s#*\.}                        # get 'y.z' (by removing prefix to .)
      local -i strokes=$((${s/\./ + })) # y + z
      # If there are mutiple Adobe refs then check 'VStrokes' from the second
      # one in the list. Some have more than two refs like 傑 (5091) having:
      # -  kRSAdobe_Japan1_6="C+1852+9.2.11 V+13433+9.2.11 V+13743+9.2.10"
      # If strokes and vstrokes aren't the same then check to see if they should
      # be swapped. Swapping fixes stroke count for some charaters and breaks it
      # for others so only swap if 'vstrokes' represents the same radical and
      # count as 'kRSUnicode' (RS = Radical+Stroke). This fixes some, but also
      # isn't a prefect solution. Further limiting swapping to only happen if
      # vstrokes matches 'kTotalStrokes' also helps, but there are still 52
      # Joyo/Jinmei Kanji that have different total stroke counts (without any
      # swapping there are 57 differences).
      if [[ ${kRSAdobe_Japan1_6} =~ ' ' ]]; then
        s=${kRSAdobe_Japan1_6#*\ } # remove the first adobe ref
        kTotalStrokes=${kTotalStrokes%% *}
        # Only allow flipping strokes up to the second entry for now. Strokes
        # and radicals mostly match official Joyo and Jinmei charts, but there
        # are still some differences and trying to get exact matches from ucd
        # data may not be possible (even some dictionaries disagree about
        # radicals and total stroke counts here are there).
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
    fi
    # put utf-8 version of 'linkTo' code into 's' if 'linkTo' is populated
    [[ -n $linkTo ]] && s="\U$linkTo" || s=
    kMandarin=${kMandarin%% *}
    # Remove leading 0's and change single quotes to 'P' (Prime) so that 04138
    # changes to 4138 (maps to 嗩) and 04138' changes to 4138P (maps to 嘆).
    [[ -n $kMorohashi ]] && kMorohashi=$(echo $kMorohashi |
      sed -e 's/^0*//' -e "s/'/P/g")
    # remove leading 0's from Nelson ids
    [[ -n $kNelson ]] && kNelson=$(echo $kNelson |
      sed -e 's/^0*//' -e 's/ 0*/ /g')
    # don't print 'vstrokes' if it's 0
    echo -e "$cp\t\U$cp\t$blk\t$age\t$radical\t$strokes\t${vstrokes#0}\t\
$kMandarin\t$kMorohashi\t$kNelson\t${kJoyoKanji:+Y}\t${kJinmeiyoKanji:+Y}\t\
$linkTo\t$s\t$localDef\t$resultOn\t$resultKun" >>$outFile
    total=$((total + 1))
    [[ -n $kJoyoKanji ]] && jouyou=$((jouyou + 1))
    if [[ -n $kJinmeiyoKanji ]]; then
      [[ -n $linkTo ]] && linkedJinmei=$((linkedJinmei + 1)) || jinmei=$((jinmei + 1))
    fi
  done < <(grep -E "($printResulsFilter)" $ucdFile)
}

log "Find variant links"
findVariantLinks
log "Populate On/Kun arrays"
populateOnKun
log "Print results to '$outFile'"
printResults
log "Total $total, Jouyou $jouyou, Jinmei $jinmei, LinkedJinmei $linkedJinmei"
