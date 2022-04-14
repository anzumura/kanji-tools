#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/Utils.h>

namespace kanji_tools {

std::string UcdEntry::codeAndName() const {
  return toUnicode(_code, BracketType::Square) + ' ' + _name;
}

std::string UcdLinks::codeAndNames() const {
  std::string result;
  for (auto& i : _links) {
    if (!result.empty()) result += ", ";
    result += toUnicode(i.code(), BracketType::Square) + ' ' + i.name();
  }
  return result;
}

bool Ucd::hasLinks() const { return !_links.empty(); }

bool Ucd::hasTraditionalLinks() const {
  return linkType() == UcdLinkTypes::Traditional;
}

bool Ucd::hasNonTraditionalLinks() const {
  return hasLinks() && linkType() != UcdLinkTypes::Traditional;
}

bool Ucd::hasVariantStrokes() const { return _variantStrokes != 0; }

std::string Ucd::codeAndName() const { return _entry.codeAndName(); }

std::string Ucd::linkCodeAndNames() const { return _links.codeAndNames(); }

} // namespace kanji_tools
