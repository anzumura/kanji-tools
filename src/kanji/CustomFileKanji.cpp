#include <kanji_tools/kanji/CustomFileKanji.h>

#include <sstream>

namespace kanji_tools {

Data::List CustomFileKanji::fromFile(const Data& data, KanjiTypes kanjiType,
                                     const std::filesystem::path& file) {
  ColumnFile::Columns columns(RequiredColumns);
  const auto add = [&columns](auto& x) {
    for (auto& i : x) columns.emplace_back(i);
  };
  switch (kanjiType) {
  case KanjiTypes::Jouyou: add(JouyouRequiredColumns); break;
  case KanjiTypes::Jinmei: add(JinmeiRequiredColumns); break;
  case KanjiTypes::Extra: add(ExtraRequiredColumns); break;
  default:
    throw std::domain_error(std::string("fromFile got invalid type: ") +
                            toString(kanjiType));
  }
  Data::List results;
  for (ColumnFile f(file, columns); f.nextRow();) switch (kanjiType) {
    case KanjiTypes::Jouyou:
      results.push_back(std::make_shared<JouyouKanji>(data, f));
      break;
    case KanjiTypes::Jinmei:
      results.push_back(std::make_shared<JinmeiKanji>(data, f));
      break;
    default: results.push_back(std::make_shared<ExtraKanji>(data, f)); break;
    }
  return results;
}

Kanji::LinkNames OfficialKanji::getOldNames(const ColumnFile& f) {
  LinkNames result;
  std::stringstream ss(f.get(OldNamesCol));
  for (std::string token; std::getline(ss, token, ',');)
    result.push_back(token);
  return result;
}

} // namespace kanji_tools
