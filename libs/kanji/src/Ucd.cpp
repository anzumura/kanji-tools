#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/Utils.h>

namespace kanji_tools {

namespace {

enum SourcesBits {
  GSource,
  HSource,
  JSource,
  KSource,
  TSource,
  VSource,
  Joyo,
  Jinmei
};

const std::map SourceLetterMap{std::pair{'G', GSource}, std::pair{'H', HSource},
    std::pair{'J', JSource}, std::pair{'K', KSource}, std::pair{'T', TSource},
    std::pair{'V', VSource}};

} // namespace

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

Ucd::Ucd(const UcdEntry& entry, const std::string& block,
    const std::string& version, Radical::Number radical, Strokes strokes,
    Strokes variantStrokes, const std::string& pinyin,
    const std::string& morohashiId, const std::string& nelsonIds,
    const std::string& sources, const std::string& jSource, bool joyo,
    bool jinmei, const UcdLinks& links, Meaning meaning, Reading onReading,
    Reading kunReading)
    : _entry{entry}, _block{block}, _version{version}, _radical{radical},
      _strokes{strokes}, _variantStrokes{variantStrokes}, _pinyin{pinyin},
      _morohashiId{morohashiId}, _nelsonIds{nelsonIds}, _jSource{jSource},
      _links{links}, _meaning{meaning}, _onReading{onReading},
      _kunReading{kunReading}, _sources{getSources(sources, joyo, jinmei)} {}

std::string Ucd::sources() const {
  std::string result;
  for (auto& i : SourceLetterMap)
    if (_sources[i.second]) result += i.first;
  return result;
}

bool Ucd::joyo() const { return _sources[Joyo]; }

bool Ucd::jinmei() const { return _sources[Jinmei]; }

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

std::bitset<Ucd::SourcesSize> Ucd::getSources(
    const std::string& sources, bool joyo, bool jinmei) {
  const auto error{[&sources](const auto& msg) {
    throw std::domain_error{"sources '" + sources + "' " + msg};
  }};
  if (sources.size() > SourcesSize - 2) error("exceeds max size");
  std::bitset<SourcesSize> result;
  for (auto i : sources) {
    if (const auto j{SourceLetterMap.find(i)}; j != SourceLetterMap.end()) {
      if (result[j->second]) error(std::string{"has duplicate value: "} + i);
      result.set(j->second);
    } else
      error(std::string{"has unrecognized value: "} + i);
  }
  result.set(Joyo, joyo);
  result.set(Jinmei, jinmei);
  return result;
}

} // namespace kanji_tools
