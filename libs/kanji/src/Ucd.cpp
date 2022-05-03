#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/UnicodeBlock.h>
#include <kanji_tools/utils/Utils.h>

namespace kanji_tools {

namespace {

enum SourcesBits : unsigned char {
  GSource = 1,
  HSource = 2,
  JSource = 4,
  KSource = 8,
  TSource = 16,
  VSource = 32,
  Joyo = 64,
  Jinmei = 128
};

const std::map SourceLetterMap{std::pair{'G', GSource}, std::pair{'H', HSource},
    std::pair{'J', JSource}, std::pair{'K', KSource}, std::pair{'T', TSource},
    std::pair{'V', VSource}};

} // namespace

UcdEntry::UcdEntry(Code code, const std::string& name) : _name{name} {
  if (!isKanji(name))
    throw std::domain_error{"name '" + name + "' isn't a recognized Kanji"};
  if (const auto c{getCode(name)}; code != c)
    throw std::domain_error{
        "code '" + toUnicode(code) + "' doesn't match '" + toUnicode(c) + "'"};
}

Code UcdEntry::code() const { return getCode(_name); }

std::string UcdEntry::codeAndName() const {
  return toUnicode(code(), BracketType::Square) + ' ' + _name;
}

std::string Ucd::linkCodeAndNames() const {
  std::string result;
  for (auto& i : _links) {
    if (!result.empty()) result += ", ";
    result += toUnicode(i.code(), BracketType::Square) + ' ' + i.name();
  }
  return result;
}

Ucd::Ucd(const UcdEntry& entry, const std::string& block,
    const std::string& version, Radical::Number radical, Strokes strokes,
    const std::string& pinyin, const std::string& morohashiId,
    const std::string& nelsonIds, const std::string& sources,
    const std::string& jSource, bool joyo, bool jinmei, Links links,
    UcdLinkTypes linkType, Meaning meaning, Reading onReading,
    Reading kunReading)
    : _entry{entry}, _block{block}, _version{version}, _pinyin{pinyin},
      _sources{getSources(sources, joyo, jinmei)}, _linkType{linkType},
      _radical{radical}, _strokes{strokes}, _morohashiId{morohashiId},
      _links{std::move(links)}, _nelsonIds{nelsonIds}, _jSource{jSource},
      _meaning{meaning}, _onReading{onReading}, _kunReading{kunReading} {}

bool Ucd::linkedReadings() const {
  return _linkType < UcdLinkTypes::Compatibility;
}

std::string Ucd::sources() const {
  std::string result;
  for (auto& i : SourceLetterMap)
    if (_sources & i.second) result += i.first;
  return result;
}

bool Ucd::joyo() const { return _sources & Joyo; }

bool Ucd::jinmei() const { return _sources & Jinmei; }

bool Ucd::hasLinks() const { return !_links.empty(); }

bool Ucd::hasTraditionalLinks() const {
  return _linkType == UcdLinkTypes::Traditional ||
         _linkType == UcdLinkTypes::Traditional_R;
}

bool Ucd::hasNonTraditionalLinks() const {
  return hasLinks() && _linkType != UcdLinkTypes::Traditional &&
         _linkType != UcdLinkTypes::Traditional_R;
}

std::string Ucd::codeAndName() const { return _entry.codeAndName(); }

unsigned char Ucd::getSources(
    const std::string& sources, bool joyo, bool jinmei) {
  const auto error{[&sources](const auto& msg) {
    throw std::domain_error{"sources '" + sources + "' " + msg};
  }};
  if (sources.size() > SourceLetterMap.size()) error("exceeds max size");
  unsigned char result{};
  for (auto i : sources) {
    if (const auto j{SourceLetterMap.find(i)}; j != SourceLetterMap.end()) {
      if (result & j->second) error(std::string{"has duplicate value: "} + i);
      result |= j->second;
    } else
      error(std::string{"has unrecognized value: "} + i);
  }
  if (joyo) result |= Joyo;
  if (jinmei) result |= Jinmei;
  return result;
}

} // namespace kanji_tools
