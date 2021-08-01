#include <kanji/MBUtils.h>
#include <kanji/Ucd.h>

namespace kanji {

const std::string Ucd::EmptyString = "";

std::string Ucd::codeAndName() const {
  return toHex(_code, true, true) + " " + _name;
}

std::string Ucd::linkCodeAndName() const {
  return hasLink() ? toHex(_linkCode, true, true) + " " + _linkName : EmptyString;
}

} // namespace kanji
