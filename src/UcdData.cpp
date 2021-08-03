#include <kanji/Data.h>
#include <kanji/Kanji.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

#include <fstream>
#include <sstream>

namespace kanji {

const Ucd* UcdData::find(const std::string& s) const {
  std::string r = s;
  if (MBChar::isMBCharWithVariationSelector(s)) {
    auto nonVariant = MBChar::withoutVariationSelector(s);
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

std::string UcdData::getReadingsAsKana(const std::string& s) const {
  const Ucd* u = find(s);
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
  int lineNum = 1, codeCol = -1, nameCol = -1, radicalCol = -1, strokesCol = -1, variantStrokesCol = -1, joyoCol = -1,
      jinmeiCol = -1, linkCodeCol = -1, linkNameCol = -1, meaningCol = -1, onCol = -1, kunCol = -1;
  auto error = [&lineNum, &file](const std::string& s, bool printLine = true) {
    Data::usage(s + (printLine ? " - line: " + std::to_string(lineNum) : Ucd::EmptyString) +
                ", file: " + file.string());
  };
  auto getWchar = [&error](const std::string& col, const auto& s, bool allowEmpty = false) -> wchar_t {
    if (s.empty() && allowEmpty) return 0;
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
  std::array<std::string, 12> cols;
  for (std::string line; std::getline(f, line); ++lineNum) {
    int pos = 0;
    std::stringstream ss(line);
    if (codeCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Code")
          setCol(codeCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "Radical")
          setCol(radicalCol, pos);
        else if (token == "Strokes")
          setCol(strokesCol, pos);
        else if (token == "VStrokes")
          setCol(variantStrokesCol, pos);
        else if (token == "Joyo")
          setCol(joyoCol, pos);
        else if (token == "Jinmei")
          setCol(jinmeiCol, pos);
        else if (token == "LinkCode")
          setCol(linkCodeCol, pos);
        else if (token == "LinkName")
          setCol(linkNameCol, pos);
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
      const int variantStrokes = cols[variantStrokesCol].empty() ? 0 : Data::toInt(cols[variantStrokesCol]);
      if (variantStrokes < 0 || variantStrokes == 1 || variantStrokes > 33) error("variant strokes out of range");
      const bool joyo = getBool("Joyo", cols[joyoCol]);
      const bool jinmei = getBool("Jinmei", cols[jinmeiCol]);
      if (joyo && jinmei) error("can't be both joyo and jinmei");
      const wchar_t linkCode = getWchar("LinkCode", cols[linkCodeCol], true);
      if (linkCode > 0) {
        if (cols[linkNameCol].empty()) error("missing link name");
        // Joyo are standard Kanji so they shouldn't have a link back to a standard form. However,
        // Some Jinmei do have links since they are 'officially allowed variants/old forms'.
        if (joyo) error("joyo shouldn't have a link");
      }
      // meaning is empty for some entries like 乁, 乣, 乴, etc., but it shouldn't be empty for a Joyo
      if (joyo && cols[meaningCol].empty()) error("meaning is empty for Joyo Kanji");
      if (cols[onCol].empty() && cols[kunCol].empty()) error("one of 'on' or 'kun' must be populated");
      if (!_map
             .emplace(std::piecewise_construct, std::make_tuple(name),
                      std::make_tuple(code, name, radical, strokes, variantStrokes, joyo, jinmei, linkCode,
                                      cols[linkNameCol], cols[meaningCol], cols[onCol], cols[kunCol]))
             .second)
        error("duplicate entry '" + name + "'");
      if (linkCode > 0) {
        if (jinmei) {
          auto i = _linkedJinmei.insert(std::make_pair(cols[linkNameCol], name));
          if (!i.second) error("jinmei link " + cols[linkNameCol] + " to " + name + " failed - has " + i.first->second);
        } else
          _linkedOther[cols[linkNameCol]].push_back(name);
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
    void add(const Ucd& k) {
      ++count;
      if (k.hasLink()) ++link;
      if (k.hasVariantStrokes()) ++variantStrokes;
      if (!k.meaning().empty()) ++meaning;
      if (!k.onReading().empty()) ++onReading;
      if (!k.kunReading().empty()) ++kunReading;
    }
  };
  auto print = [&data](const char* s, int x, int y, int z) {
    data.log() << "  " << s << ": " << x + y + z << " (Jouyou " << x << ", Jinmei " << y << ", Other " << z << ")\n";
  };
  Count joyo;
  Count jinmei;
  Count other;
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
  data.log() << "  Standard Kanji with 'Variation Selectors' vs UCD Variants:\n";
  int count = 0;
  data.log() << "    #      Standard Kanji with Selector    UCD Compatibility Kanji\n";
  data.log() << "    -      ----------------------------    -----------------------\n";
  for (const auto& i : data.map()) {
    const Kanji& k = *i.second;
    if (k.variant()) {
      data.log() << "    " << std::left << std::setfill(' ') << std::setw(3) << ++count << "    ["
                 << toUnicode(k.name()) << "] " << k.name() << " variant of " << k.nonVariantName() << "    ";
      auto u = find(k.name());
      if (u) {
        data.out() << u->codeAndName();
        if (u->hasLink()) data.out() << " variant of " << u->linkCodeAndName();
      } else
        data.out() << "UCD not found";
      data.out() << '\n';
    }
  }
}

} // namespace kanji
