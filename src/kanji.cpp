#include <kanji/Kanji.h>

#include <fstream>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

Data::List FileListKanji::fromFile(const Data& data, Types type, const fs::path& file) {
  assert(type == Types::Jouyou || type == Types::Jinmei || type == Types::Extra);
  if (!fs::is_regular_file(file)) usage("can't find file: " + file.string());
  std::ifstream f(file);
  std::string line;
  std::map<int, int> colMap;
  Data::List results;
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    int pos = 0;
    // first line should be headers, don't catch exceptions for first line since
    // whole file would be a problem
    if (colMap.empty())
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        auto i = ColumnMap.find(token);
        if (i == ColumnMap.end()) throw std::domain_error("unrecognized column: " + token);
        if (!colMap.insert(std::pair(pos, i->second)).second) throw std::domain_error("duplicate column: " + token);
      }
    else
      try {
        for (std::string token; std::getline(ss, token, '\t'); ++pos) {
          auto i = colMap.find(pos);
          if (i == colMap.end()) throw std::out_of_range("too many columns");
          columns[i->second] = token;
        }
        switch (type) {
        case Types::Jouyou: results.push_back(std::make_shared<JouyouKanji>(data)); break;
        case Types::Jinmei: results.push_back(std::make_shared<JinmeiKanji>(data)); break;
        default: results.push_back(std::make_shared<ExtraKanji>(data)); break;
        }
      } catch (const std::exception& e) {
        std::cerr << "got exception: " << e.what() << " while processing " << file.string() << " line: " << line
                  << '\n';
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
