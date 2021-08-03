#!/usr/bin/env bash

declare -r program="parseUcdAllFlat.sh"

# This script searches the Unicode 'ucd.all.flat.xml' file for characters that
# have a Japanese reading (On or Kun) and prints out a tab-separated line with
# the following 12 values:
# - Code: Unicode code point (4 or 5 digit hex code)
# - Name: character in utf8
# - Radical: radical number (1 to 214)
# - Strokes: total strokes (including the radical)
# - VStrokes: total strokes for first variant (blank if no variants)
# - Joyo: 'Y' if part of Jōyō list or blank
# - Jinmei: 'Y' if part of Jinmeiyō list or blank
# - LinkCode: 230 Jinmei (of the 863 total) are variants of other Jōyō/Jinmei
# - LinkName: link character in utf8
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

# 'get' gets a tag ($1) from an XML string ($2) - the value is put in a variable
# with the same name as the tag.
function get() {
  unset -v $1
  eval $(echo "$2" | grep -o " $1=\"[^\"]*\"")
}
# 'getFirst' is similar to 'get', but it will only take up to the first space if
# value has spaces (so it gets the value up to the first space)
function getFirst() {
  unset -v $1
  eval $(echo "$2" | grep -o " $1=\"[^\" ]*")\"
}

# declare arrays to help support links for variant and compat kanjis
declare -A definition on kun linkBack variantLink

defStart='kDefinition=\"\('
variants=()
variants+=('a variant of ')
variants+=('interchangeable ')
variants+=('same as ')
variants+=('non-classical ')

for type in "${variants[@]}"; do
  secondEntry=false
  for i in $(grep -E "$defStart$type.*" $1 |
    sed "s/.*cp=\"\([^\"]*\).*($type[-a-z0-9UA-Z+ ]*\([^ ,)]*\).*/\1 \2/"); do
    if $secondEntry; then
      # get the Unicode (4 of 5 digit hex with caps) value from UTF-8 kanji
      s=$(echo -n ${i:0:1} | iconv -f UTF-8 -t UTF-32BE | xxd -p |
        sed -r 's/^0+/0x/')
      s=$(printf "%04X\n" $s)
      # cp (for the variant) is unique, but the original kanji 's' can occur more
      # than once, i.e., if there are multiple variants for 's'. This is true for
      # 64DA (據) which has variants 3A3F (㨿) and 3A40 (㩀).
      variantLink[$cp]=$s
      linkBack[$s]=$cp # only needs a value (doesn't need to be unique)
      secondEntry=false
    else
      cp=$i
      secondEntry=true
    fi
  done
done

# There are over 140K characters in 'ucd.all.flat.txt' and most of them aren't
# relevant to the current functionality of this 'kanji' project so apply some
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
# In addition to Adobe refs, also pull in any kanji that are compatibility or
# variants of any kanji that had an on/kun reading.
#
# Here are other 'Japan' type source tags that are not used by this script:
# - 'kNelson' (Classic 'Nelson Japanese-English Character Dictionary') has 5,398
#   entries (5,330 with On/Kun), but missed 7 Jōyō and 48 Jinmei Kanji.
# - 'kJis0' has 6,356 (6,354 with On/Kun), but missed 4 Jōyō and 15 Jinmei.
# - 'kMorohashi' has 18,168 (12,965 with On/Kun) so this isn't much different
#   compared to just using On/Kun filtering.
# - 'kIRGDaiKanwaZiten' has 17,864 (12,942 with On/Kun). There's also a proposal
#   to remove this property (and expand 'kMorohashi') so it's probably best not
#   to use it: https://www.unicode.org/L2/L2021/21032-unihan-kmorohashi.pdf

# First loop to populate links. A link can point to an entry later in the file
# like 5DE2 (巢) which links to 5DE3 (巣). Process kanji having 'on' or 'kun'
onKun='kJapanese[OK].*n="[^"]'
while read -r i; do
  get cp "$i"
  get kJoyoKanji "$i"
  if [ -n "$kJoyoKanji" ]; then
    # There are 4 entries with a Joyo link: 5265, 53F1, 586B and 982C. Store
    # definition/on/kun since since they are missing for some linked Jinmeiyo
    # Kanji as well as Joyo 𠮟 (U+20B9F) which replaces 叱 (53F1) しか-る.
    [[ $kJoyoKanji =~ U+ ]] && cp="$cp ${kJoyoKanji#U+}"
  else
    get kJinmeiyoKanji "$i"
    if [[ $kJinmeiyoKanji =~ U+ ]]; then
      linkBack[${kJinmeiyoKanji#*+}]=$cp
      cp="$cp ${kJinmeiyoKanji#*+}"
    elif [ -z "${linkBack[$cp]}" ]; then
      # only store linked info if there's a variant that links back to this one
      continue
    fi
  fi
  get kDefinition "$i"
  get kJapaneseOn "$i"
  get kJapaneseKun "$i"
  for link in $cp; do
    definition[$link]="$kDefinition"
    on[$link]="$kJapaneseOn"
    kun[$link]="$kJapaneseKun"
  done
done < <(grep $onKun $1)

echo -e "Code\tName\tRadical\tStrokes\tVStrokes\tJoyo\tJinmei\tLinkCode\t\
LinkName\tMeaning\tOn\tKun"

# Second loop picks up 'variant', 'onKun' as well as any of the following:
adobe='kRSAdobe_Japan1_6="[^"]'      # has an Adobe ID
compat='kCompatibilityVariant="[^"]' # is a compatibility variant (points back)
official='kJ.*yoKanji="[^"]'         # is Joyo or Jinmeiyo (to catch 𠮟)
for s in "${variants[@]}"; do
  condition="$condition|$defStart$s"
done
condition="($adobe|$onKun|$official|$compat$condition)"

# Second loop produces output
while read -r i; do
  get cp "$i"
  get kJoyoKanji "$i"
  get kJinmeiyoKanji "$i"
  get kDefinition "$i"
  get kJapaneseOn "$i"
  get kJapaneseKun "$i"
  loadFrom=
  linkTo=
  if [ -n "$kJoyoKanji" ]; then
    if [[ $kJoyoKanji =~ U+ ]]; then
      # Need to unset kJoyoKanji since the official version is the 'link' entry.
      unset -v kJoyoKanji
    else
      loadFrom=$cp
    fi
  elif [ -n "$kJinmeiyoKanji" ]; then
    if [[ $kJinmeiyoKanji =~ U+ ]]; then
      loadFrom=${kJinmeiyoKanji#*+}
      linkTo=$loadFrom
    else
      loadFrom=$cp
      linkTo=${linkBack[$cp]}
    fi
  else
    get kCompatibilityVariant "$i"
    if [[ "$kCompatibilityVariant" =~ U+ ]]; then
      loadFrom=${kCompatibilityVariant#*+}
      linkTo=$loadFrom
    elif [ -n "${variantLink[$cp]}" ]; then
      loadFrom=${variantLink[$cp]}
      linkTo=$loadFrom
    fi
  fi
  if [ -n "$loadFrom" ] && [ -n "${on[$loadFrom]}${kun[$loadFrom]}" ]; then
    kDefinition=${kDefinition:-${definition[$loadFrom]}}
    kJapaneseOn=${kJapaneseOn:-${on[$loadFrom]}}
    kJapaneseKun=${kJapaneseKun:-${kun[$loadFrom]}}
  fi
  # don't write a record if there are no Japanese readings
  [ -z "$kJapaneseOn$kJapaneseKun" ] && continue
  #
  # Radical and Strokes
  #
  getFirst kRSUnicode "$i"
  radical=${kRSUnicode%\.*}
  get kRSAdobe_Japan1_6 "$i"
  vstrokes= # only set if there's a variant 'adobe ref' with a different count
  if [ -z "${kRSAdobe_Japan1_6}" ]; then
    getFirst kTotalStrokes "$i"
    strokes=$kTotalStrokes
  else
    # Some characters have multiple adobe references so start with the first one
    s=${kRSAdobe_Japan1_6%%\ *}
    # Adobe refs have the form 'C+num+x.y.z' where 'x' is the radical number,
    # 'y' is number of strokes for the radical and 'z' is remaining strokes.
    s=${s##*+}               # remove the 'C+num+' prefix
    s=${s#*\.}               # get 'y.z' (by removing the shortest prefix to .)
    strokes=$((${s/\./ + })) # y + z
    # If there are mutiple Adobe refs then check get 'VStrokes' from the second
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
      getFirst kTotalStrokes "$i"
      secondEntry=true
      for s in $s; do
        # Same as 'strokes' above, i.e, remove 'V+num+', get y.z and then add
        s=${s##*+}
        vradical=${s%%\.*}
        s=${s#*\.}
        newStrokes=$((${s/\./ + }))
        if [ $strokes -ne $newStrokes ]; then
          if $secondEntry && [ $kRSUnicode = $vradical.${s#*\.} ]; then
            if [ $kTotalStrokes -eq $newStrokes ]; then
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
        # Only allow flipping strokes up to the second entry for now. Strokes and
        # radicals mostly match official Joyo and Jinmei charts, but there are
        # still some differences and trying to get exact matches in from ucd data
        # may not be possible (even some dictionaries disagree about radicals and
        # total stroke counts here are there).
        secondEntry=false
      done
    fi
  fi
  # put utf-8 version of 'linkTo' code into 's' if 'linkTo' is populated
  [ -n "$linkTo" ] && s="\U$linkTo" || s=
  echo -e "$cp\t\U$cp\t$radical\t$strokes\t$vstrokes\t${kJoyoKanji:+Y}\t\
${kJinmeiyoKanji:+Y}\t$linkTo\t$s\t$kDefinition\t$kJapaneseOn\t$kJapaneseKun"
done < <(grep -E "$condition" $1)
