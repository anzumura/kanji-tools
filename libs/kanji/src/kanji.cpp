#include <kt_kana/Utf8Char.h>
#include <kt_kanji/OfficialKanji.h>

#include <algorithm>
#include <cassert>

namespace kanji_tools {

// Kanji public methods

// base implementations of virtual functions return default values

Kanji::Frequency Kanji::frequency() const { return 0; }
KanjiGrades Kanji::grade() const { return KanjiGrades::None; }
KenteiKyus Kanji::kyu() const { return KenteiKyus::None; }
JlptLevels Kanji::level() const { return JlptLevels::None; }
Kanji::Link Kanji::link() const {
  static const KanjiPtr EmptyLink;
  return EmptyLink;
}
JinmeiReasons Kanji::reason() const { return JinmeiReasons::None; }
Kanji::Year Kanji::year() const { return 0; }
bool Kanji::linkedReadings() const { return false; }

String Kanji::compatibilityName() const {
  return _compatibilityName.value_or(_name.name());
}

Kanji::Frequency Kanji::frequencyOrDefault(Frequency x) const {
  const auto f{frequency()};
  return f ? f : x;
}

Kanji::Frequency Kanji::frequencyOrMax() const {
  return frequencyOrDefault(std::numeric_limits<Frequency>::max());
}

bool Kanji::is(KanjiTypes t) const { return type() == t; }
bool Kanji::hasGrade() const { return hasValue(grade()); }
bool Kanji::hasKyu() const { return hasValue(kyu()); }
bool Kanji::hasLevel() const { return hasValue(level()); }
bool Kanji::hasMeaning() const { return !meaning().empty(); }
bool Kanji::hasNelsonIds() const { return !_nelsonIds.empty(); }
bool Kanji::hasReading() const { return !reading().empty(); }

String Kanji::info(Info fields) const {
  static const String RadMsg{"Rad "}, StrokesMsg{"Strokes "}, FreqMsg{"Frq "},
      NewMsg{"New "}, OldMsg{"Old "};

  String result;
  const auto add{[&result](const auto& x) {
    if (!result.empty()) result += ", ";
    result += x;
  }};
  if (hasValue(fields & Info::Radical))
    add(RadMsg + radical().name() + '(' + std::to_string(radical().number()) +
        ')');
  if (hasValue(fields & Info::Strokes)) add(StrokesMsg + strokes().toString());
  if (hasValue(fields & Info::Pinyin) && _pinyin) add(_pinyin.name());
  if (hasValue(fields & Info::Grade) && hasGrade()) add(toString(grade()));
  if (hasValue(fields & Info::Level) && hasLevel()) add(toString(level()));
  if (hasValue(fields & Info::Freq) && frequency())
    add(FreqMsg + std::to_string(frequency()));
  // kanji can have a 'New' value (from a link) or an 'Old' value,but not both.
  if (newName()) {
    assert(oldNames().empty());
    if (hasValue(fields & Info::New))
      add(NewMsg + *newName() + (linkedReadings() ? "*" : ""));
  } else if (hasValue(fields & Info::Old) && !oldNames().empty()) {
    String s;
    for (auto& i : oldNames()) {
      if (s.empty())
        s = i + (linkedReadings() ? "*" : "");
      else
        s += "／" + i;
    }
    add(OldMsg + s);
  }
  if (hasValue(fields & Info::Kyu) && hasKyu()) add(toString(kyu()));
  return result;
}

String Kanji::qualifiedName() const {
  return name() + QualifiedNames[qualifiedNameRank()];
}

bool Kanji::orderByQualifiedName(const Kanji& x) const {
  return qualifiedNameRank() < x.qualifiedNameRank() ||
         qualifiedNameRank() == x.qualifiedNameRank() && orderByStrokes(x);
}

bool Kanji::orderByStrokes(const Kanji& x) const {
  return strokes() < x.strokes() ||
         strokes() == x.strokes() &&
             (frequencyOrMax() < x.frequencyOrMax() ||
                 frequencyOrMax() == x.frequencyOrMax() &&
                     toUnicode(compatibilityName()) <
                         toUnicode(x.compatibilityName()));
}

bool Kanji::operator==(const Kanji& x) const { return name() == x.name(); }

// Kanji protected methods

Kanji::Kanji(CtorParams params, RadicalRef radical, Strokes strokes)
    : Kanji{params.name(), params.data().getCompatibilityName(params.name()),
          radical, strokes, KanjiData::getPinyin(params.ucd()),
          KanjiData::getMorohashiId(params.ucd()),
          KanjiData::getNelsonIds(params.ucd())} {}

Kanji::Kanji(Name name, const OptString& compatibilityName, RadicalRef radical,
    Strokes strokes, const Pinyin& pinyin, const MorohashiId& morohashiId,
    NelsonIds nelsonIds)
    : _name{name}, _compatibilityName{compatibilityName}, _radical{radical},
      _strokes{strokes}, _pinyin{pinyin}, _morohashiId{morohashiId},
      _nelsonIds{std::move(nelsonIds)} {}

// Kanji private methods

// chained ':?' causes clang-tidy 'cognitive complexity' warning, but this
// style seems nicer than multiple returns (in 'if' and 'switch' blocks)
uint16_t Kanji::qualifiedNameRank() const { // NOLINT
  using enum KanjiTypes;
  // use an enum to avoid magic numbers, note 'vUcd' is the least common type
  enum Vals { vJou, vJlpt, vFreq, vJin, vLnkJ, vLnkO, vExt, vNoK1, vK1, vUcd };
  const auto t{type()};
  return t == Jouyou               ? vJou
         : hasLevel()              ? vJlpt
         : frequency()             ? vFreq
         : t == Jinmei             ? vJin
         : t == LinkedJinmei       ? vLnkJ
         : t == LinkedOld          ? vLnkO
         : t == Extra              ? vExt
         : t == Ucd                ? vUcd
         : kyu() != KenteiKyus::K1 ? vNoK1
                                   : vK1;
}

// Kanji::CtorParams

Kanji::CtorParams::CtorParams(KanjiDataRef data, Name name) noexcept
    : CtorParams{data, name, data.findUcd(name)} {}

Kanji::CtorParams::CtorParams(KanjiDataRef data, Name name, UcdPtr ucd) noexcept
    : _data{data}, _name{name}, _ucd{ucd} {}

Kanji::Frequency Kanji::CtorParams::frequency() const {
  return _data.frequency(_name);
}

bool Kanji::CtorParams::hasNonTraditionalLinks() const {
  return _ucd && _ucd->hasNonTraditionalLinks();
}

bool Kanji::CtorParams::hasTraditionalLinks() const {
  return _ucd && _ucd->hasTraditionalLinks();
}

KenteiKyus Kanji::CtorParams::kyu() const { return _data.kyu(_name); }

JlptLevels Kanji::CtorParams::level() const { return _data.level(_name); }

bool Kanji::CtorParams::linkedReadings() const {
  return _ucd && _ucd->linkedReadings();
}

RadicalRef Kanji::CtorParams::radical() const {
  return _data.ucdRadical(_name, _ucd);
}

String Kanji::CtorParams::reading() const {
  return _data.ucd().getReadingsAsKana(_ucd);
}

Strokes Kanji::CtorParams::strokes() const {
  return _data.ucdStrokes(_name, _ucd);
}

// Kanji::KanjiName

Kanji::KanjiName::KanjiName(const String& name) : _name{name} {
  assert(Utf8Char::size(_name) == 1);
}

bool Kanji::KanjiName::isVariant() const {
  return Utf8Char::isCharWithVariationSelector(_name);
}

String Kanji::KanjiName::nonVariant() const {
  return Utf8Char::noVariationSelector(_name);
}

// LoadedKanji

Kanji::LinkNames LoadedKanji::linkNames(UcdPtr u) {
  LinkNames result;
  if (u && u->hasLinks())
    std::transform(u->links().begin(), u->links().end(),
        std::back_inserter(result), [](const auto& i) { return i.name(); });
  return result;
}

LoadedKanji::LoadedKanji(CtorParams params, RadicalRef radical, Reading reading,
    Strokes strokes, Meaning meaning)
    : Kanji{params, radical, strokes}, _meaning{meaning}, _reading{reading} {}

LoadedKanji::LoadedKanji(CtorParams params, RadicalRef radical, Reading reading)
    : LoadedKanji{params, radical, reading, params.strokes(),
          UcdData::getMeaning(params.ucd())} {}

// OtherKanji

const Kanji::LinkNames& OtherKanji::oldNames() const {
  return _hasOldLinks ? _linkNames : EmptyLinkNames;
}

Kanji::OptString OtherKanji::newName() const {
  return _linkNames.empty() || _hasOldLinks ? std::nullopt
                                            : OptString{_linkNames[0]};
}

OtherKanji::OtherKanji(CtorParams params, Reading reading)
    : LoadedKanji{params, params.radical(), reading},
      _hasOldLinks{params.hasTraditionalLinks()},
      _linkNames{linkNames(params.ucd())}, _linkedReadings{
                                               params.linkedReadings()} {}

OtherKanji::OtherKanji(CtorParams params)
    : OtherKanji{params, params.reading()} {}

StandardKanji::StandardKanji(KanjiDataRef data, Name name, Reading reading)
    : OtherKanji{{data, name}, reading}, _kyu{data.kyu(name)} {}

StandardKanji::StandardKanji(KanjiDataRef data, Name name)
    : StandardKanji{data, name, data.kyu(name)} {}

StandardKanji::StandardKanji(KanjiDataRef data, Name name, KenteiKyus kyu)
    : OtherKanji{{data, name}}, _kyu{kyu} {}

FrequencyKanji::FrequencyKanji(
    KanjiDataRef data, Name name, Frequency frequency)
    : StandardKanji{data, name}, _frequency{frequency} {}

FrequencyKanji::FrequencyKanji(
    KanjiDataRef data, Name name, Reading reading, Frequency frequency)
    : StandardKanji{data, name, reading}, _frequency{frequency} {}

KenteiKanji::KenteiKanji(KanjiDataRef data, Name name, KenteiKyus kyu)
    : StandardKanji{data, name, kyu} {}

UcdKanji::UcdKanji(KanjiDataRef data, const Ucd& u)
    : OtherKanji{{data, u.name(), &u}} {}

} // namespace kanji_tools
