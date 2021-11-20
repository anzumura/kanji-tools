#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/MBUtils.h>

namespace kanji_tools {

const std::string Ucd::EmptyString = "";

std::string Ucd::codeAndName() const {
  return toHex(_code, true, true) + " " + _name;
}

std::string Ucd::linkCodeAndName() const {
  return hasLink() ? toHex(_linkCode, true, true) + " " + _linkName : EmptyString;
}

} // namespace kanji_tools
