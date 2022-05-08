#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/utils/Utils.h>

#include <cassert>

namespace kanji_tools {

Kanji::Frequency Kanji::frequency() const { return 0; }

KanjiGrades Kanji::grade() const { return KanjiGrades::None; }

KenteiKyus Kanji::kyu() const { return KenteiKyus::None; }

JlptLevels Kanji::level() const { return JlptLevels::None; }

KanjiPtr Kanji::link() const { return {}; }

JinmeiReasons Kanji::reason() const { return JinmeiReasons::None; }

Kanji::Year Kanji::year() const { return 0; }

bool Kanji::linkedReadings() const { return false; }

Kanji::KanjiName::KanjiName(const std::string& name) : _name{name} {
  assert(MBChar::size(_name) == 1);
}

bool Kanji::KanjiName::isVariant() const {
  return MBChar::isMBCharWithVariationSelector(_name);
}

std::string Kanji::KanjiName::nonVariant() const {
  return MBChar::noVariationSelector(_name);
}

Kanji::Kanji(DataRef d, Name n, RadicalRef radical, Strokes strokes, UcdPtr u)
    : Kanji{n, d.getCompatibilityName(n), radical, strokes, Data::getPinyin(u),
          Data::getMorohashiId(u), Data::getNelsonIds(u)} {}

std::string Kanji::compatibilityName() const {
  return _compatibilityName.value_or(_name.name());
}

Kanji::Frequency Kanji::frequencyOrDefault(Frequency x) const {
  const auto f{frequency()};
  return f ? f : x;
}

Kanji::Frequency Kanji::frequencyOrMax() const {
  return frequencyOrDefault(std::numeric_limits<Frequency>::max());
}

std::string Kanji::info(KanjiInfo fields) const {
  static const std::string RadMsg{"Rad "}, StrokesMsg{"Strokes "},
      FreqMsg{"Frq "}, NewMsg{"New "}, OldMsg{"Old "};

  std::string result;
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
    std::string s;
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

std::string Kanji::qualifiedName() const {
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
                                   : vK1;
}

} // namespace kanji_tools
