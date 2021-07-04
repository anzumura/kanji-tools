#include <kanji/Kanji.h>

#include <fstream>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

const std::string Kanji::EmptyString = "";

Data::List FileListKanji::fromFile(const Data& data, Types type, const fs::path& file) {
  assert(type == Types::Jouyou || type == Types::Jinmei || type == Types::Extra);
  int lineNumber = 1;
  auto error = [&lineNumber, &file](const std::string& s, bool printLine = true) {
    Data::usage(s + (printLine ? " - line: " + std::to_string(lineNumber) : "") + ", file: " + file.string());
  };
  std::ifstream f(file);
  std::bitset<MaxCol> found;
  std::array<int, MaxCol> colMap;
  colMap.fill(-1);
  Data::List results;
  for (std::string line; std::getline(f, line); ++lineNumber) {
    std::stringstream ss(line);
    int pos = 0;
    for (std::string token; std::getline(ss, token, '\t'); ++pos) {
      if (pos >= MaxCol) error("too many columns");
      if (lineNumber == 1) {
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
    if (lineNumber == 1) {
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
