#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/utils/ColumnFile.h>
#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/MBUtils.h>

#include <sstream>

namespace kanji_tools {

namespace {

// 'PrintCount' is used for debug printing. Some combinations are prevented by 'load' function (like Joyo
// with a link or missing meaning), but count all cases for completeness.
struct PrintCount {
  int count = 0;
  int link = 0;
  int variantStrokes = 0;
  int meaning = 0;
  int onReading = 0;
  int kunReading = 0;
  int morohashi = 0;
  int nelson = 0;
  void add(const Ucd& k) {
    ++count;
    if (k.hasLinks()) ++link;
    if (k.hasVariantStrokes()) ++variantStrokes;
    if (!k.meaning().empty()) ++meaning;
    if (!k.onReading().empty()) ++onReading;
    if (!k.kunReading().empty()) ++kunReading;
    if (!k.morohashiId().empty()) ++morohashi;
    if (!k.nelsonIds().empty()) ++nelson;
  }
};

} // namespace

const Ucd* UcdData::find(const std::string& kanjiName) const {
  std::string r = kanjiName;
  if (MBChar::isMBCharWithVariationSelector(kanjiName)) {
    auto nonVariant = MBChar::withoutVariationSelector(kanjiName);
    // check for linked Jinmei variant first
    if (auto i = _linkedJinmei.find(nonVariant); i == _linkedJinmei.end()) {
      auto j = _linkedOther.find(nonVariant);
      if (j == _linkedOther.end()) return nullptr;
      // if j exists it should never by an empty vector
      assert(!j->second.empty());
      // if there are more than one variant, just return the first one for now.
      r = j->second[0];
    } else
      r = i->second;
  }
  auto i = _map.find(r);
  return i == _map.end() ? nullptr : &i->second;
}

std::string UcdData::getReadingsAsKana(const Ucd* u) const {
  if (u) {
    std::string reading = u->onReading();
    std::replace(reading.begin(), reading.end(), ' ', ',');
    std::string result = _converter.convert(CharType::Romaji, reading, CharType::Katakana);
    reading = u->kunReading();
    if (!reading.empty()) {
      std::replace(reading.begin(), reading.end(), ' ', ',');
      // if there are both 'on' and 'kun' readings then separate them with a comma
      if (!result.empty()) reading = ',' + reading;
      result += _converter.convert(CharType::Romaji, reading, CharType::Hiragana);
    }
    return result;
  }
  return Ucd::EmptyString;
}

void UcdData::load(const std::filesystem::path& file) {
  const ColumnFile::Column codeCol("Code"), nameCol("Name"), blockCol("Block"), versionCol("Version"),
    radicalCol("Radical"), strokesCol("Strokes"), vStrokesCol("VStrokes"), pinyinCol("Pinyin"),
    morohashiCol("Morohashi"), nelsonIdsCol("NelsonIds"), joyoCol("Joyo"), jinmeiCol("Jinmei"),
    linkCodesCol("LinkCodes"), linkNamesCol("LinkNames"), linkTypeCol("LinkType"), meaningCol("Meaning"), onCol("On"),
    kunCol("Kun");
  for (ColumnFile f(
         file,
         {codeCol, nameCol, blockCol, versionCol, radicalCol, strokesCol, vStrokesCol, pinyinCol, morohashiCol,
          nelsonIdsCol, joyoCol, jinmeiCol, linkCodesCol, linkNamesCol, linkTypeCol, meaningCol, onCol, kunCol});
       f.nextRow();) {
    if (f.isEmpty(onCol) && f.isEmpty(kunCol) && f.isEmpty(morohashiCol))
      f.error("one of 'On', 'Kun' or 'Morohashi' must be populated");
    const auto& name = f.get(nameCol);
    if (name.length() > 4) f.error("name greater than 4");
    if (f.get(vStrokesCol) == "0") f.error("VStrokes shouldn't be 0");

    const int radical = f.getInt(radicalCol), strokes = f.getInt(strokesCol),
              vStrokes = f.isEmpty(vStrokesCol) ? 0 : f.getInt(vStrokesCol);
    if (radical < 1 || radical > 214) f.error("radical out of range");
    // 9F98 (龘) has 48 strokes
    if (strokes < 1 || strokes > 48) f.error("strokes out of range");
    if (vStrokes < 0 || vStrokes == 1 || vStrokes > 33) f.error("variant strokes out of range");

    const bool joyo = f.getBool(joyoCol), jinmei = f.getBool(jinmeiCol);
    if (joyo) {
      if (jinmei) f.error("can't be both joyo and jinmei");
      // meaning is empty for some entries like 乁, 乣, 乴, etc., but it shouldn't be empty for Joyo
      if (f.isEmpty(meaningCol)) f.error("meaning is empty for Joyo Kanji");
    }

    Ucd::Links links;
    if (!f.isEmpty(linkNamesCol)) {
      std::stringstream names(f.get(linkNamesCol)), codes(f.get(linkCodesCol));
      for (std::string linkName; std::getline(names, linkName, ',');)
        if (std::string linkCode; std::getline(codes, linkCode, ','))
          links.emplace_back(f.getWChar(linkCodesCol, linkCode), linkName);
        else
          f.error("LinkName has more values than LinkCode");
      // Joyo are standard Kanji so they shouldn't have a link back to a standard form. However,
      // Some Jinmei do have links since they are 'officially allowed variants/old forms'. There
      // are links in raw XML data for joyo, but the parse script ignores them.
      if (joyo) f.error("joyo shouldn't have links");
      if (f.isEmpty(linkTypeCol)) f.error("LinkName has a value, but LinkType is empty");
    } else if (!f.isEmpty(linkTypeCol))
      f.error("LinkType has a value, but LinkName is empty");
    else if (!f.isEmpty(linkCodesCol))
      f.error("LinkCode has a value, but LinkName is empty");

    std::string linkType = f.get(linkTypeCol);
    const bool linkedReadings = linkType.ends_with("*");
    if (linkedReadings) linkType.pop_back();

    if (!_map
           .emplace(
             std::piecewise_construct, std::make_tuple(name),
             std::make_tuple(f.getWChar(codeCol), name, f.get(blockCol), f.get(versionCol), radical, strokes, vStrokes,
                             f.get(pinyinCol), f.get(morohashiCol), f.get(nelsonIdsCol), joyo, jinmei, links,
                             Ucd::toLinkType(linkType), linkedReadings, f.get(meaningCol), f.get(onCol), f.get(kunCol)))
           .second)
      f.error("duplicate entry '" + name + "'");
    for (const auto& link : links)
      if (!jinmei)
        _linkedOther[link.name()].push_back(name);
      else if (auto i = _linkedJinmei.emplace(link.name(), name); !i.second)
        f.error("jinmei link " + link.name() + " to " + name + " failed - has " + i.first->second);
  }
}

void UcdData::print(const Data& data) const {
  auto print = [&data](const char* s, int x, int y, int z) {
    data.log() << "  " << s << ": " << x + y + z << " (Jouyou " << x << ", Jinmei " << y << ", Other " << z << ")\n";
  };
  PrintCount joyo, jinmei, other;
  data.log() << "Kanji Loaded from Unicode 'ucd' file:\n";
  for (const auto& i : _map)
    if (const auto& k = i.second; k.joyo())
      joyo.add(k);
    else if (k.jinmei())
      jinmei.add(k);
    else
      other.add(k);
  print("Total", joyo.count, jinmei.count, other.count);
  print("Links", joyo.link, jinmei.link, other.link);
  print("VStrokes", joyo.variantStrokes, jinmei.variantStrokes, other.variantStrokes);
  print("Meanings", joyo.meaning, jinmei.meaning, other.meaning);
  print("On Readdings", joyo.onReading, jinmei.onReading, other.onReading);
  print("Kun Readings", joyo.kunReading, jinmei.kunReading, other.kunReading);
  print("Morohashi Ids", joyo.morohashi, jinmei.morohashi, other.morohashi);
  print("Nelson Ids", joyo.nelson, jinmei.nelson, other.nelson);
  auto printLinks = [this, &data](const std::string& name, const auto& list) {
    auto count = std::count_if(list.begin(), list.end(), [this](const auto& i) {
      auto j = _map.find(i->name());
      return j != _map.end() && j->second.hasLinks();
    });
    data.log() << name << " Kanji with links " << count << ":\n";
    for (auto& i : list)
      if (auto j = _map.find(i->name()); j != _map.end()) {
        if (j->second.hasLinks())
          data.out() << "  " << j->second.codeAndName() << " -> " << j->second.linkCodeAndNames() << ' '
                     << j->second.linkType() << '\n';
      } else
        data.out() << "  ERROR: " << i->name() << " not found in UCD\n";
  };
  printLinks("Frequency", data.frequencyKanji());
  printLinks("Extra", data.extraKanji());
  printVariationSelectorKanji(data);
}

void UcdData::printVariationSelectorKanji(const Data& data) const {
  data.log() << "  Standard Kanji with 'Variation Selectors' vs UCD Variants:\n";
  data.log() << "    #      Standard Kanji with Selector    UCD Compatibility Kanji\n";
  data.log() << "    -      ----------------------------    -----------------------\n";
  for (auto count = 0; auto& i : data.kanjiNameMap())
    if (auto& k = *i.second; k.variant()) {
      data.log() << "    " << std::left << std::setfill(' ') << std::setw(3) << ++count << "    "
                 << toUnicode(k.name(), BracketType::Square) << ' ' << k.name() << " variant of " << k.nonVariantName()
                 << "    ";
      auto u = find(k.name());
      if (u) {
        data.out() << u->codeAndName();
        if (u->hasLinks()) data.out() << " variant of " << u->linkCodeAndNames();
      } else
        data.out() << "UCD not found";
      data.out() << '\n';
    }
}

} // namespace kanji_tools
