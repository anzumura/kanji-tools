#include <kanji/Kanji.h>

#include <fstream>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

Data::List FileListKanji::fromFile(const Data& data, Types type, const fs::path& file) {
  assert(type == Types::Jouyou || type == Types::Jinmei || type == Types::Extra);
  int lineNumber = 1;
  auto error = [&lineNumber, &file](const std::string& s, bool printLine = true) {
    Data::usage(s + (printLine ? " - line: " + std::to_string(lineNumber) : "") + ", file: " + file.string());
  };
  std::ifstream f(file);
  std::map<int, int> colMap;
  Data::List results;
  for (std::string line; std::getline(f, line); ++lineNumber) {
    std::stringstream ss(line);
    int pos = 0;
    // first line should be headers, don't catch exceptions for first line since
    // whole file would be a problem
    if (colMap.empty())
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        auto i = ColumnMap.find(token);
        if (i == ColumnMap.end()) error("unrecognized column: " + token, false);
        if (!colMap.insert(std::pair(pos, i->second)).second) error("duplicate column: " + token, false);
      }
    else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        auto i = colMap.find(pos);
        if (i == colMap.end()) error("too many columns");
        columns[i->second] = token;
      }
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
  {"Number", NumberCol},   {"Name", NameCol},       {"Radical", RadicalCol}, {"OldName", OldNameCol},
  {"Year", YearCol},       {"Strokes", StrokesCol}, {"Grade", GradeCol},     {"Meaning", MeaningCol},
  {"Reading", ReadingCol}, {"Reason", ReasonCol}};

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
