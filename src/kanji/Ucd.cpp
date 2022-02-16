#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/MBUtils.h>

namespace kanji_tools {

const std::string Ucd::EmptyString = "";

std::string Ucd::codeAndName() const { return toUnicode(_code, BracketType::Square) + ' ' + _name; }

std::string Ucd::linkCodeAndNames() const {
  std::string result;
  for (auto& i : _links) {
    if (!result.empty()) result += ", ";
    result += toUnicode(i.code(), BracketType::Square) + ' ' + i.name();
  }
  return result;
}

} // namespace kanji_tools
