#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/kanji/CustomFileKanji.h>

#include <cassert>

namespace kanji_tools {

// base implementations of virtual functions return default values

Kanji::Frequency Kanji::frequency() const { return 0; }
KanjiGrades Kanji::grade() const { return KanjiGrades::None; }
KenteiKyus Kanji::kyu() const { return KenteiKyus::None; }
JlptLevels Kanji::level() const { return JlptLevels::None; }
KanjiPtr Kanji::link() const { return {}; }
JinmeiReasons Kanji::reason() const { return JinmeiReasons::None; }
Kanji::Year Kanji::year() const { return 0; }
bool Kanji::linkedReadings() const { return false; }

Kanji::KanjiName::KanjiName(const String& name) : _name{name} {
  assert(MBChar::size(_name) == 1);
}

bool Kanji::KanjiName::isVariant() const {
  return MBChar::isMBCharWithVariationSelector(_name);
}

String Kanji::KanjiName::nonVariant() const {
  return MBChar::noVariationSelector(_name);
}

Kanji::Kanji(DataRef d, Name n, RadicalRef radical, Strokes strokes, UcdPtr u)
    : Kanji{n, d.getCompatibilityName(n), radical, strokes, Data::getPinyin(u),
          Data::getMorohashiId(u), Data::getNelsonIds(u)} {}

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

String Kanji::info(KanjiInfo fields) const {
  static const String RadMsg{"Rad "}, StrokesMsg{"Strokes "}, FreqMsg{"Frq "},
      NewMsg{"New "}, OldMsg{"Old "};

  String result;
  const auto add{[&result](const auto& x) {
    if (!result.empty()) result += ", ";
    result += x;
  }};
  if (hasValue(fields & KanjiInfo::Radical))
    add(RadMsg + radical().name() + '(' + std::to_string(radical().number()) +
        ')');
  if (hasValue(fields & KanjiInfo::Strokes))
    add(StrokesMsg + strokes().toString());
  if (hasValue(fields & KanjiInfo::Pinyin) && _pinyin) add(_pinyin.name());
  if (hasValue(fields & KanjiInfo::Grade) && hasGrade()) add(toString(grade()));
  if (hasValue(fields & KanjiInfo::Level) && hasLevel()) add(toString(level()));
  if (hasValue(fields & KanjiInfo::Freq) && frequency())
    add(FreqMsg + std::to_string(frequency()));
  // kanji can have a 'New' value (from a link) or an 'Old' value,but not both.
  if (newName()) {
    assert(oldNames().empty());
    if (hasValue(fields & KanjiInfo::New))
      add(NewMsg + *newName() + (linkedReadings() ? "*" : ""));
  } else if (hasValue(fields & KanjiInfo::Old) && !oldNames().empty()) {
    String s;
    for (auto& i : oldNames()) {
      if (s.empty())
        s = i + (linkedReadings() ? "*" : "");
      else
        s += "／" + i;
    }
    add(OldMsg + s);
  }
  if (hasValue(fields & KanjiInfo::Kyu) && hasKyu()) add(toString(kyu()));
  return result;
}

String Kanji::qualifiedName() const {
  return name() + QualifiedNames[qualifiedNameRank()];
}

bool Kanji::orderByQualifiedName(const Kanji& x) const {
  return qualifiedNameRank() < x.qualifiedNameRank() ||
         qualifiedNameRank() == x.qualifiedNameRank() &&
             (strokes() < x.strokes() ||
                 strokes() == x.strokes() &&
                     (frequencyOrMax() < x.frequencyOrMax() ||
                         frequencyOrMax() == x.frequencyOrMax() &&
                             toUnicode(compatibilityName()) <
                                 toUnicode(x.compatibilityName())));
}

bool Kanji::operator==(const Kanji& x) const { return name() == x.name(); }

uint16_t Kanji::qualifiedNameRank() const { // NOLINT
  // use an enum to avoid magic numbers, note 'vUcd' is the least common type
  enum Vals { vJou, vJlpt, vFreq, vJin, vLnkJ, vLnkO, vExt, vNoK1, vK1, vUcd };
  const auto t{type()};
  // chained ':?' causes clang-tidy 'cognitive complexity' warning, but this
  // style seems nicer than multiple returns (in 'if' and 'switch' blocks)
  using enum KanjiTypes;
  return t == Jouyou               ? vJou
         : hasLevel()              ? vJlpt
         : frequency()             ? vFreq
         : t == Jinmei             ? vJin
         : t == LinkedJinmei       ? vLnkJ
         : t == LinkedOld          ? vLnkO
         : t == Extra              ? vExt
         : t == Ucd                ? vUcd
         : kyu() != KenteiKyus::K1 ? vNoK1
                                   : vK1; // LCOV_EXCL_LINE: covered
}

// NonLinkedKanji

Kanji::LinkNames NonLinkedKanji::linkNames(UcdPtr u) {
  LinkNames result;
  if (u && u->hasLinks())
    std::transform(u->links().begin(), u->links().end(),
        std::back_inserter(result), [](const auto& i) { return i.name(); });
  return result;
}

NonLinkedKanji::NonLinkedKanji(DataRef data, Name name, RadicalRef radical,
    Strokes strokes, Meaning meaning, Reading reading, UcdPtr u)
    : Kanji{data, name, radical, strokes, u}, _meaning{meaning}, _reading{
                                                                     reading} {}

NonLinkedKanji::NonLinkedKanji(
    DataRef data, Name name, RadicalRef radical, Reading reading, UcdPtr u)
    : NonLinkedKanji{data, name, radical, data.ucdStrokes(name, u),
          UcdData::getMeaning(u), reading, u} {}

// UcdFileKanji

const Kanji::LinkNames& UcdFileKanji::oldNames() const {
  return _hasOldLinks ? _linkNames : EmptyLinkNames;
}

Kanji::OptString UcdFileKanji::newName() const {
  return _linkNames.empty() || _hasOldLinks ? std::nullopt
                                            : OptString{_linkNames[0]};
}

UcdFileKanji::UcdFileKanji(DataRef data, Name name, Reading reading, UcdPtr u)
    : NonLinkedKanji{data, name, data.ucdRadical(name, u), reading, u},
      _hasOldLinks{u && u->hasTraditionalLinks()}, _linkNames{linkNames(u)},
      _linkedReadings{u && u->linkedReadings()} {}

UcdFileKanji::UcdFileKanji(DataRef data, Name name, UcdPtr u) // LCOV_EXCL_LINE
    : UcdFileKanji{data, name, data.ucd().getReadingsAsKana(u), u} {}

StandardKanji::StandardKanji(DataRef data, Name name, Reading reading)
    : UcdFileKanji{data, name, reading, data.findUcd(name)}, _kyu{data.kyu(
                                                                 name)} {}

StandardKanji::StandardKanji(DataRef data, Name name)
    : StandardKanji{data, name, data.kyu(name)} {}

StandardKanji::StandardKanji(DataRef data, Name name, KenteiKyus kyu)
    : UcdFileKanji{data, name, data.findUcd(name)}, _kyu{kyu} {}

FrequencyKanji::FrequencyKanji(DataRef data, Name name, Frequency frequency)
    : StandardKanji{data, name}, _frequency{frequency} {}

FrequencyKanji::FrequencyKanji(
    DataRef data, const String& name, Reading reading, Frequency frequency)
    : StandardKanji{data, name, reading}, _frequency{frequency} {}

KenteiKanji::KenteiKanji(DataRef data, Name name, KenteiKyus kyu)
    : StandardKanji{data, name, kyu} {}

UcdKanji::UcdKanji(DataRef data, const Ucd& u)
    : UcdFileKanji{data, u.name(), &u} {}

} // namespace kanji_tools
