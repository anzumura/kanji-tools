#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/utils/Utils.h>

#include <cassert>

namespace kanji_tools {

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
    : Kanji{n, d.getCompatibilityName(n), radical, strokes, d.getMorohashiId(u),
          d.getNelsonIds(u), d.getPinyin(u)} {}

std::string Kanji::compatibilityName() const {
  return _compatibilityName.value_or(_name);
}

Kanji::Frequency Kanji::frequencyOrDefault(Frequency x) const {
  return frequency().value_or(x);
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
  if (hasValue(fields & KanjiInfo::Strokes) && strokes())
    add(StrokesMsg + std::to_string(strokes()));
  if (hasValue(fields & KanjiInfo::Pinyin) && pinyin()) add(*pinyin());
  if (hasValue(fields & KanjiInfo::Grade) && hasGrade()) add(toString(grade()));
  if (hasValue(fields & KanjiInfo::Level) && hasLevel()) add(toString(level()));
  if (hasValue(fields & KanjiInfo::Freq) && frequency())
    add(FreqMsg + std::to_string(*frequency()));
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

u_int16_t Kanji::qualifiedNameRank() const {
  // use an enum to avoid magic numbers, note 'Ucd' is the least common type
  enum Values { Jou, Jlpt, Freq, Jinmei, LJinmei, LOld, Extra, NonK1, K1, Ucd };
  const auto t{type()};
  return t == KanjiTypes::Jouyou         ? Jou
         : hasLevel()                    ? Jlpt
         : frequency()                   ? Freq
         : t == KanjiTypes::Jinmei       ? Jinmei
         : t == KanjiTypes::LinkedJinmei ? LJinmei
         : t == KanjiTypes::LinkedOld    ? LOld
         : t == KanjiTypes::Extra        ? Extra
         : t == KanjiTypes::Ucd          ? Ucd
         : kyu() != KenteiKyus::K1       ? NonK1
                                         : K1;
}

} // namespace kanji_tools
