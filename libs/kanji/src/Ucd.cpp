#include <kanji_tools/kanji/Ucd.h>
#include <kanji_tools/utils/UnicodeBlock.h>

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

Ucd::Entry::Entry(Code code, const String& name) : _name{name} {
  if (!isKanji(name))
    throw std::domain_error{"name '" + name + "' isn't a recognized Kanji"};
  if (const auto c{getCode(name)}; code != c)
    throw std::domain_error{
        "code '" + toUnicode(code) + "' doesn't match '" + toUnicode(c) + "'"};
}

Code Ucd::Entry::code() const { return getCode(_name); }

String Ucd::Entry::codeAndName() const {
  return toUnicode(code(), BracketType::Square) + ' ' + _name;
}

String Ucd::linkCodeAndNames() const {
  String result;
  for (auto& i : _links) {
    if (!result.empty()) result += ", ";
    result += toUnicode(i.code(), BracketType::Square) + ' ' + i.name();
  }
  return result;
}

Ucd::Ucd(const Entry& entry, const String& block, const String& version,
    Radical::Number radical, Strokes strokes, const String& pinyin,
    const String& morohashiId, const String& nelsonIds, const String& sources,
    const String& jSource, bool joyo, bool jinmei, Links links,
    LinkTypes linkType, Meaning meaning, Reading onReading, Reading kunReading)
    : _entry{entry}, _block{block}, _version{version}, _pinyin{pinyin},
      _sources{getSources(sources, joyo, jinmei)}, _linkType{linkType},
      _radical{radical}, _strokes{strokes}, _morohashiId{morohashiId},
      _links{std::move(links)}, _nelsonIds{nelsonIds}, _jSource{jSource},
      _meaning{meaning}, _onReading{onReading}, _kunReading{kunReading} {}

bool Ucd::linkedReadings() const {
  return _linkType < LinkTypes::Compatibility;
}

String Ucd::sources() const {
  String result;
  for (auto& i : SourceLetterMap)
    if (_sources & i.second) result += i.first;
  return result;
}

bool Ucd::joyo() const { return _sources & Joyo; }

bool Ucd::jinmei() const { return _sources & Jinmei; }

bool Ucd::hasLinks() const { return !_links.empty(); }

bool Ucd::hasTraditionalLinks() const {
  return _linkType == LinkTypes::Traditional ||
         _linkType == LinkTypes::Traditional_R;
}

bool Ucd::hasNonTraditionalLinks() const {
  return hasLinks() && _linkType != LinkTypes::Traditional &&
         _linkType != LinkTypes::Traditional_R;
}

String Ucd::codeAndName() const { return _entry.codeAndName(); }

unsigned char Ucd::getSources(const String& sources, bool joyo, bool jinmei) {
  const auto error{[&sources](const auto& msg) {
    throw std::domain_error{"sources '" + sources + "' " + msg};
  }};
  if (sources.size() > SourceLetterMap.size()) error("exceeds max size");
  unsigned char result{};
  for (auto i : sources) {
    if (const auto j{SourceLetterMap.find(i)}; j != SourceLetterMap.end()) {
      if (result & j->second) error(String{"has duplicate value: "} + i);
      result |= j->second;
    } else
      error(String{"has unrecognized value: "} + i);
  }
  if (joyo) result |= Joyo;
  if (jinmei) result |= Jinmei;
  return result;
}

} // namespace kanji_tools
