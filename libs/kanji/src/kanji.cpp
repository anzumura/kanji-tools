#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/utils/Utils.h>

#include <cassert>

namespace kanji_tools {

Kanji::Kanji(const std::string& name, const OptString& compatibilityName,
    // LCOV_EXCL_START: gcov bug
    const Radical& radical, Strokes strokes, const OptString& morohashiId,
    // LCOV_EXCL_STOP
    const NelsonIds& nelsonIds, const OptString& pinyin)
    : _name{name}, _nonVariantName{MBChar::optNoVariationSelector(name)},
      _compatibilityName{compatibilityName}, _radical{radical},
      _strokes{strokes}, _morohashiId{morohashiId},
      _nelsonIds{nelsonIds}, _pinyin{pinyin} {
  assert(MBChar::size(_name) == 1);
}

bool Kanji::variant() const { return _nonVariantName.has_value(); }

std::string Kanji::nonVariantName() const {
  return _nonVariantName.value_or(_name);
}

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
  return _name + QualifiedNames[qualifiedNameRank()];
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

u_int8_t Kanji::qualifiedNameRank() const {
  const auto t{type()};
  // Note: '7' is for non-K1 Kentei, '8' is for K1 Kentei and '9' is for Ucd
  // (so the least common)
  return t == KanjiTypes::Jouyou         ? 0
         : hasLevel()                    ? 1
         : frequency()                   ? 2
         : t == KanjiTypes::Jinmei       ? 3
         : t == KanjiTypes::LinkedJinmei ? 4
         : t == KanjiTypes::LinkedOld    ? 5
         : t == KanjiTypes::Extra        ? 6
         : t == KanjiTypes::Ucd          ? 9
         : kyu() != KenteiKyus::K1       ? 7
                                         : 8;
}

} // namespace kanji_tools
