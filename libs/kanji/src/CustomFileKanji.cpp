#include <kanji_tools/kanji/CustomFileKanji.h>

#include <sstream>

namespace kanji_tools {

Kanji::LinkNames OfficialKanji::getOldNames(const ColumnFile& f) {
  LinkNames result;
  std::stringstream ss{f.get(OldNamesCol)};
  for (std::string token; std::getline(ss, token, ',');)
    result.emplace_back(token);
  return result;
}

} // namespace kanji_tools
