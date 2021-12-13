#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/MBUtils.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

const Ucd* UcdData::find(const std::string& kanjiName) const {
  std::string r = kanjiName;
  if (MBChar::isMBCharWithVariationSelector(kanjiName)) {
    auto nonVariant = MBChar::withoutVariationSelector(kanjiName);
    // check for linked Jinmei variant first
    auto i = _linkedJinmei.find(nonVariant);
    if (i == _linkedJinmei.end()) {
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
  int lineNum = 1, codeCol = -1, nameCol = -1, blockCol = -1, versionCol = -1, radicalCol = -1, strokesCol = -1,
      variantStrokesCol = -1, pinyinCol = -1, morohashiCol = -1, nelsonIdsCol = -1, joyoCol = -1, jinmeiCol = -1,
      linkCodesCol = -1, linkNamesCol = -1, linkTypeCol = -1, meaningCol = -1, onCol = -1, kunCol = -1;
  auto error = [&lineNum, &file](const std::string& s, bool printLine = true) {
    Data::usage(s + (printLine ? " - line: " + std::to_string(lineNum) : Ucd::EmptyString) +
                ", file: " + file.string());
  };
  auto getWchar = [&error](const std::string& col, const auto& s) -> wchar_t {
    if (s.length() != 4 && s.length() != 5) error(col + " length must be 4 or 5 '" + s + "'");
    for (char c : s)
      if (c < '0' || c > 'F' || (c < 'A' && c > '9')) error("invalid '" + col + "' string '" + s + "'");
    return std::strtol(s.c_str(), nullptr, 16);
  };
  auto getBool = [&error](const std::string& col, const auto& s) {
    if (s == "Y") return true;
    if (!s.empty()) error("unrecognized '" + col + "' value '" + s + "'");
    return false;
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name");
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 18> cols;
  for (std::string line; std::getline(f, line); ++lineNum) {
    int pos = 0;
    std::stringstream ss(line);
    if (codeCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Code")
          setCol(codeCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "Block")
          setCol(blockCol, pos);
        else if (token == "Version")
          setCol(versionCol, pos);
        else if (token == "Radical")
          setCol(radicalCol, pos);
        else if (token == "Strokes")
          setCol(strokesCol, pos);
        else if (token == "VStrokes")
          setCol(variantStrokesCol, pos);
        else if (token == "Pinyin")
          setCol(pinyinCol, pos);
        else if (token == "Morohashi")
          setCol(morohashiCol, pos);
        else if (token == "NelsonIds")
          setCol(nelsonIdsCol, pos);
        else if (token == "Joyo")
          setCol(joyoCol, pos);
        else if (token == "Jinmei")
          setCol(jinmeiCol, pos);
        else if (token == "LinkCodes")
          setCol(linkCodesCol, pos);
        else if (token == "LinkNames")
          setCol(linkNamesCol, pos);
        else if (token == "LinkType")
          setCol(linkTypeCol, pos);
        else if (token == "Meaning")
          setCol(meaningCol, pos);
        else if (token == "On")
          setCol(onCol, pos);
        else if (token == "Kun")
          setCol(kunCol, pos);
        else
          error("unrecognized column '" + token + "'", false);
      if (pos != cols.size()) error("not enough columns", false);
    } else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        if (pos == cols.size()) error("too many columns");
        cols[pos] = token;
      }
      if (pos == cols.size() - 1)
        cols[pos] = "";
      else if (pos != cols.size())
        error("not enough columns - got " + std::to_string(pos) + ", wanted " + std::to_string(cols.size()));
      const wchar_t code = getWchar("Unicode", cols[codeCol]);
      const auto& name = cols[nameCol];
      if (name.length() > 4) error("name greater than 4");
      const int radical = Data::toInt(cols[radicalCol]);
      if (radical < 1 || radical > 214) error("radical out of range");
      const int strokes = Data::toInt(cols[strokesCol]);
      // 9F98 (龘) has 48 strokes
      if (strokes < 1 || strokes > 48) error("strokes out of range");
      if (cols[variantStrokesCol] == "0") error("VStrokes shouldn't be 0");
      const int variantStrokes = cols[variantStrokesCol].empty() ? 0 : Data::toInt(cols[variantStrokesCol]);
      if (variantStrokes < 0 || variantStrokes == 1 || variantStrokes > 33) error("variant strokes out of range");
      const bool joyo = getBool("Joyo", cols[joyoCol]);
      const bool jinmei = getBool("Jinmei", cols[jinmeiCol]);
      if (joyo && jinmei) error("can't be both joyo and jinmei");
      Ucd::Links links;
      if (!cols[linkNamesCol].empty()) {
        std::stringstream names(cols[linkNamesCol]);
        std::stringstream codes(cols[linkCodesCol]);
        for (std::string linkName; std::getline(names, linkName, ',');) {
          if (std::string linkCode; std::getline(codes, linkCode, ','))
            links.emplace_back(getWchar("LinkCode", linkCode), linkName);
          else
            error("LinkName has more values than LinkCode");
        }
        // Joyo are standard Kanji so they shouldn't have a link back to a standard form. However,
        // Some Jinmei do have links since they are 'officially allowed variants/old forms'. There
        // are links in raw XML data for joyo, but the parse script ignores them.
        if (joyo) error("joyo shouldn't have links");
        if (cols[linkTypeCol].empty()) error("LinkName has a value, but LinkType is empty");
      } else if (!cols[linkTypeCol].empty())
        error("LinkType has a value, but LinkName is empty");
      else if (!cols[linkCodesCol].empty())
        error("LinkCode has a value, but LinkName is empty");
      const bool linkedReadings = cols[linkTypeCol].ends_with("*");
      if (linkedReadings) cols[linkTypeCol].pop_back();
      // meaning is empty for some entries like 乁, 乣, 乴, etc., but it shouldn't be empty for a Joyo
      if (joyo && cols[meaningCol].empty()) error("meaning is empty for Joyo Kanji");
      if (cols[onCol].empty() && cols[kunCol].empty() && cols[morohashiCol].empty())
        error("one of 'On', 'Kun' or 'Morohashi' must be populated");
      if (!_map
             .emplace(std::piecewise_construct, std::make_tuple(name),
                      std::make_tuple(code, name, cols[blockCol], cols[versionCol], radical, strokes, variantStrokes,
                                      cols[pinyinCol], cols[morohashiCol], cols[nelsonIdsCol], joyo, jinmei, links,
                                      Ucd::toLinkType(cols[linkTypeCol]), linkedReadings, cols[meaningCol], cols[onCol],
                                      cols[kunCol]))
             .second)
        error("duplicate entry '" + name + "'");
      for (const auto& link : links) {
        if (jinmei) {
          auto i = _linkedJinmei.insert(std::make_pair(link.name(), name));
          if (!i.second) error("jinmei link " + link.name() + " to " + name + " failed - has " + i.first->second);
        } else
          _linkedOther[link.name()].push_back(name);
      }
    }
  }
}

void UcdData::print(const Data& data) const {
  // Some combinations are prevented by 'load' function (like Joyo with a link or missing
  // meaning), but count all cases here for completeness.
  struct Count {
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
  auto print = [&data](const char* s, int x, int y, int z) {
    data.log() << "  " << s << ": " << x + y + z << " (Jouyou " << x << ", Jinmei " << y << ", Other " << z << ")\n";
  };
  Count joyo, jinmei, other;
  data.log() << "Kanji Loaded from Unicode 'ucd' file:\n";
  for (const auto& i : _map) {
    const auto& k = i.second;
    if (k.joyo())
      joyo.add(k);
    else {
      if (k.jinmei())
        jinmei.add(k);
      else
        other.add(k);
    }
  }
  print("Total", joyo.count, jinmei.count, other.count);
  print("Links", joyo.link, jinmei.link, other.link);
  print("VStrokes", joyo.variantStrokes, jinmei.variantStrokes, other.variantStrokes);
  print("Meanings", joyo.meaning, jinmei.meaning, other.meaning);
  print("On Readdings", joyo.onReading, jinmei.onReading, other.onReading);
  print("Kun Readings", joyo.kunReading, jinmei.kunReading, other.kunReading);
  print("Morohashi Ids", joyo.morohashi, jinmei.morohashi, other.morohashi);
  print("Nelson Ids", joyo.nelson, jinmei.nelson, other.nelson);
  auto printLinks = [this, &data](const std::string& name, const auto& list) {
    int count = std::count_if(list.begin(), list.end(), [this](const auto& i) {
      auto j = _map.find(i->name());
      return j != _map.end() && j->second.hasLinks();
    });
    data.log() << name << " Kanji with links " << count << ":\n";
    for (auto& i : list) {
      auto j = _map.find(i->name());
      if (j != _map.end()) {
        if (j->second.hasLinks())
          data.out() << "  " << j->second.codeAndName() << " -> " << j->second.linkCodeAndNames() << ' '
                     << j->second.linkType() << '\n';
      } else
        data.out() << "  ERROR: " << i->name() << " not found in UCD\n";
    }
  };
  printLinks("Frequency", data.frequencyKanji());
  printLinks("Extra", data.extraKanji());
  data.log() << "  Standard Kanji with 'Variation Selectors' vs UCD Variants:\n";
  int count = 0;
  data.log() << "    #      Standard Kanji with Selector    UCD Compatibility Kanji\n";
  data.log() << "    -      ----------------------------    -----------------------\n";
  for (auto& i : data.kanjiNameMap()) {
    const Kanji& k = *i.second;
    if (k.variant()) {
      data.log() << "    " << std::left << std::setfill(' ') << std::setw(3) << ++count << "    ["
                 << toUnicode(k.name()) << "] " << k.name() << " variant of " << k.nonVariantName() << "    ";
      auto u = find(k.name());
      if (u) {
        data.out() << u->codeAndName();
        if (u->hasLinks()) data.out() << " variant of " << u->linkCodeAndNames();
      } else
        data.out() << "UCD not found";
      data.out() << '\n';
    }
  }
}

} // namespace kanji_tools
