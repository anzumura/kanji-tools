#include <kanji_tools/kanji/CustomFileKanji.h>

#include <bitset>
#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

Data::List CustomFileKanji::fromFile(const Data& data, KanjiTypes type, const fs::path& file) {
  assert(type == KanjiTypes::Jouyou || type == KanjiTypes::Jinmei || type == KanjiTypes::Extra);
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
      case KanjiTypes::Jouyou: check(jouyouRequiredColumns); break;
      case KanjiTypes::Jinmei: check(jinmeiRequiredColumns); break;
      default: check(extraRequiredColumns);
      }
    } else {
      if (pos < MaxCol && colMap[pos] != -1) error("not enough columns");
      try {
        switch (type) {
        case KanjiTypes::Jouyou: results.push_back(std::make_shared<JouyouKanji>(data)); break;
        case KanjiTypes::Jinmei: results.push_back(std::make_shared<JinmeiKanji>(data)); break;
        default: results.push_back(std::make_shared<ExtraKanji>(data)); break;
        }
      } catch (const std::exception& e) {
        error(std::string("got exception while creating kanji '") + e.what() + "'");
      }
    }
  }
  return results;
}

std::array<std::string, CustomFileKanji::MaxCol> CustomFileKanji::columns;
std::map<std::string, int> CustomFileKanji::ColumnMap = {
  colPair(NumberCol),  colPair(NameCol),  colPair(RadicalCol), colPair(OldNamesCol), colPair(YearCol),
  colPair(StrokesCol), colPair(GradeCol), colPair(MeaningCol), colPair(ReadingCol),  colPair(ReasonCol)};

Kanji::LinkNames OfficialKanji::getOldNames() {
  LinkNames result;
  std::stringstream ss(columns[OldNamesCol]);
  for (std::string token; std::getline(ss, token, ',');)
    result.push_back(token);
  return result;
}

JinmeiKanji::Reasons JinmeiKanji::getReason(const std::string& s) {
  if (s == "Names") return Reasons::Names;
  if (s == "Print") return Reasons::Print;
  if (s == "Moved") return Reasons::Moved;
  if (s == "Variant") return Reasons::Variant;
  return Reasons::Other;
}

KanjiGrades JouyouKanji::getGrade(const std::string& s) {
  if (s.length() == 1) switch (s[0]) {
    case '1': return KanjiGrades::G1;
    case '2': return KanjiGrades::G2;
    case '3': return KanjiGrades::G3;
    case '4': return KanjiGrades::G4;
    case '5': return KanjiGrades::G5;
    case '6': return KanjiGrades::G6;
    case 'S': return KanjiGrades::S;
    }
  return KanjiGrades::None;
}

} // namespace kanji_tools
