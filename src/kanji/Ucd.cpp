#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/MBUtils.h>

namespace kanji_tools {

namespace {

static const std::string Compatibility("Compatibility"), Definition("Definition"), Jinmei("Jinmei"),
  Semantic("Semantic"), Simplified("Simplified"), Traditional("Traditional"), None("None");

} // namespace

const std::string Ucd::EmptyString = "";

Ucd::LinkTypes Ucd::toLinkType(const std::string& s) {
  if (s == Compatibility) return LinkTypes::Compatibility;
  if (s == Definition) return LinkTypes::Definition;
  if (s == Jinmei) return LinkTypes::Jinmei;
  if (s == Semantic) return LinkTypes::Semantic;
  if (s == Simplified) return LinkTypes::Simplified;
  if (s == Traditional) return LinkTypes::Traditional;
  if (!s.empty() && s != None) throw std::invalid_argument(s + " is not a recognized LinkTypes value");
  return LinkTypes::None;
}

const std::string& Ucd::toString(LinkTypes x) {
  switch (x) {
  case LinkTypes::Compatibility: return Compatibility;
  case LinkTypes::Definition: return Definition;
  case LinkTypes::Jinmei: return Jinmei;
  case LinkTypes::Semantic: return Semantic;
  case LinkTypes::Simplified: return Simplified;
  case LinkTypes::Traditional: return Traditional;
  case LinkTypes::None: return None;
  }
  __builtin_unreachable(); // prevent gcc 'control reaches end ...' warning
}

std::string Ucd::codeAndName() const { return toHex(_code, true, true) + " " + _name; }

std::string Ucd::linkCodeAndNames() const {
  std::string result;
  for (auto& i : _links) {
    if (!result.empty()) result += ", ";
    result += toHex(i.code(), true, true) + " " + i.name();
  }
  return result;
}

} // namespace kanji_tools
