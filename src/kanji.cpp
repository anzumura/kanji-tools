#include <kanji/Kanji.h>
#include <kanji/MBChar.h>

#include <fstream>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

Kanji::Kanji(const Data& d, int number, const std::string& name, const Radical& radical, int strokes,
             bool findFrequency, Levels level)
  : _number(number), _name(name), _variant(MBChar::isMBCharWithVariationSelector(name)),
    _nonVariantName(MBChar::withoutVariationSelector(name)), _compatibilityName(d.getCompatibilityName(name)),
    _radical(radical), _strokes(strokes), _pinyin(d.getPinyin(name)), _level(level),
    _frequency(findFrequency ? d.getFrequency(name) : 0), _kyu(d.getKyu(name)) {
  assert(MBChar::length(_name) == 1);
}

std::string Kanji::info(int infoFields) const {
  static const std::string Pinyin("Pinyin "), Rad("Rad "), Strokes("Strokes "), Grade("Grade "), Level("Level "),
    Freq("Freq "), New("New "), Old("Old "), Kyu("Kyu ");
  std::string result;
  auto add = [&result](const auto& x) {
    if (!result.empty()) result += ", ";
    result += x;
  };
  auto t = type();
  if (infoFields & RadicalField) add(Rad + radical().name() + ':' + std::to_string(radical().number()));
  if (infoFields & StrokesField && strokes()) add(Strokes + std::to_string(strokes()));
  if (infoFields & PinyinField && pinyin().has_value()) add(Pinyin + *pinyin());
  if (infoFields & GradeField && hasGrade()) add(Grade + toString(grade()));
  if (infoFields & LevelField && hasLevel()) add(Level + toString(level()));
  if (infoFields & FreqField && frequency()) add(Freq + std::to_string(frequency()));
  // A kanji can possibly have a 'New' value (from a link) or an 'Old' value, but not both. Check for
  // linked types first (since oldName is a top level optional field on all kanji).
  if (hasLink(t)) {
    assert(!oldName().has_value());
    if (infoFields & NewField) add(New + static_cast<const LinkedKanji&>(*this).link()->name());
  } else if (infoFields & OldField && oldName().has_value())
    add(Old + *oldName());
  if (infoFields & KyuField && hasKyu()) add(Kyu + toString(kyu()));
  return result;
}

Data::List FileListKanji::fromFile(const Data& data, Types type, const fs::path& file) {
  assert(type == Types::Jouyou || type == Types::Jinmei || type == Types::Extra);
  int lineNum = 1;
  auto error = [&lineNum, &file](const std::string& s, bool printLine = true) {
    Data::usage(s + (printLine ? " - line: " + std::to_string(lineNum) : "") + ", file: " + file.string());
  };
  std::ifstream f(file);
  std::bitset<MaxCol> found;
  std::array<int, MaxCol> colMap;
  colMap.fill(-1);
  Data::List results;
  for (std::string line; std::getline(f, line); ++lineNum) {
    std::stringstream ss(line);
    int pos = 0;
    for (std::string token; std::getline(ss, token, '\t'); ++pos) {
      if (pos >= MaxCol) error("too many columns");
      if (lineNum == 1) {
        const auto i = ColumnMap.find(token);
        if (i == ColumnMap.end()) error("unrecognized column: " + token, false);
        if (found[i->second]) error("duplicate column: " + token, false);
        found.flip(i->second);
        colMap[pos] = i->second;
      } else if (colMap[pos] == -1)
        error("too many columns");
      else
        columns[colMap[pos]] = token;
    }
    if (lineNum == 1) {
      auto check = [&found, &error](const auto& x) {
        for (auto i : x)
          if (!found[i]) error(std::string("missing required column: ") + ColumnNames[i], false);
      };
      check(requiredColumns);
      switch (type) {
      case Types::Jouyou: check(jouyouRequiredColumns); break;
      case Types::Jinmei: check(jinmeiRequiredColumns); break;
      default: check(extraRequiredColumns);
      }
    } else {
      if (pos < MaxCol && colMap[pos] != -1) error("not enough columns");
      try {
        switch (type) {
        case Types::Jouyou: results.push_back(std::make_shared<JouyouKanji>(data)); break;
        case Types::Jinmei: results.push_back(std::make_shared<JinmeiKanji>(data)); break;
        default: results.push_back(std::make_shared<ExtraKanji>(data)); break;
        }
      } catch (const std::exception& e) {
        error(std::string("got exception while creating kanji '") + e.what() + "'");
      }
    }
  }
  return results;
}

std::array<std::string, FileListKanji::MaxCol> FileListKanji::columns;
std::map<std::string, int> FileListKanji::ColumnMap = {
  colPair(NumberCol),  colPair(NameCol),  colPair(RadicalCol), colPair(OldNameCol), colPair(YearCol),
  colPair(StrokesCol), colPair(GradeCol), colPair(MeaningCol), colPair(ReadingCol), colPair(ReasonCol)};

JinmeiKanji::Reasons JinmeiKanji::getReason(const std::string& s) {
  if (s == "Names") return Reasons::Names;
  if (s == "Print") return Reasons::Print;
  if (s == "Moved") return Reasons::Moved;
  if (s == "Variant") return Reasons::Variant;
  return Reasons::Other;
}

Grades JouyouKanji::getGrade(const std::string& s) {
  if (s == "S") return Grades::S;
  if (s == "6") return Grades::G6;
  if (s == "5") return Grades::G5;
  if (s == "4") return Grades::G4;
  if (s == "3") return Grades::G3;
  if (s == "2") return Grades::G2;
  if (s == "1") return Grades::G1;
  return Grades::None;
}

} // namespace kanji
