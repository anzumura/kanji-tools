#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/utils/MBChar.h>

#include <cassert>

namespace kanji_tools {

Kanji::Kanji(const std::string& name, const OptString& compatibilityName, const Radical& radical, int strokes,
             const OptString& morohashiId, const NelsonIds& nelsonIds, const OptString& pinyin)
  : _name(name), _nonVariantName(MBChar::optionalWithoutVariationSelector(name)), _compatibilityName(compatibilityName),
    _radical(radical), _strokes(strokes), _morohashiId(morohashiId), _nelsonIds(nelsonIds), _pinyin(pinyin) {
  assert(MBChar::length(_name) == 1);
}

std::string Kanji::info(KanjiInfo fields) const {
  static const std::string Rad("Rad "), Strokes("Strokes "), Freq("Frq "), New("New "), Old("Old ");

  std::string result;
  const auto add = [&result](const auto& x) {
    if (!result.empty()) result += ", ";
    result += x;
  };
  if (hasValue(fields & KanjiInfo::Radical))
    add(Rad + radical().name() + '(' + std::to_string(radical().number()) + ')');
  if (hasValue(fields & KanjiInfo::Strokes) && strokes()) add(Strokes + std::to_string(strokes()));
  if (hasValue(fields & KanjiInfo::Pinyin) && pinyin()) add(*pinyin());
  if (hasValue(fields & KanjiInfo::Grade) && hasGrade()) add(toString(grade()));
  if (hasValue(fields & KanjiInfo::Level) && hasLevel()) add(toString(level()));
  if (hasValue(fields & KanjiInfo::Freq) && frequency()) add(Freq + std::to_string(*frequency()));
  // A kanji can possibly have a 'New' value (from a link) or an 'Old' value, but not both.
  if (newName()) {
    assert(oldNames().empty());
    if (hasValue(fields & KanjiInfo::New)) add(New + *newName() + (linkedReadings() ? "*" : ""));
  } else if (hasValue(fields & KanjiInfo::Old) && !oldNames().empty()) {
    std::string s;
    for (auto& i : oldNames()) {
      if (s.empty())
        s = i + (linkedReadings() ? "*" : "");
      else
        s += "／" + i;
    }
    add(Old + s);
  }
  if (hasValue(fields & KanjiInfo::Kyu) && hasKyu()) add(toString(kyu()));
  return result;
}

} // namespace kanji_tools
