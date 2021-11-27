#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/utils/MBChar.h>

namespace kanji_tools {

Kanji::Kanji(int number, const std::string& name, const std::string& compatibilityName, const Radical& radical,
             int strokes, const OptString& pinyin, const NelsonIds& nelsonIds, JlptLevels level, KenteiKyus kyu,
             int frequency)
  : _number(number), _name(name), _variant(MBChar::isMBCharWithVariationSelector(name)),
    _nonVariantName(MBChar::withoutVariationSelector(name)), _compatibilityName(compatibilityName), _radical(radical),
    _strokes(strokes), _pinyin(pinyin), _nelsonIds(nelsonIds), _level(level), _kyu(kyu), _frequency(frequency) {
  assert(MBChar::length(_name) == 1);
}

std::string Kanji::info(int infoFields) const {
  static const std::string Rad("Rad "), Strokes("Strokes "), Freq("Frq "), New("New "), Old("Old ");
  std::string result;
  auto add = [&result](const auto& x) {
    if (!result.empty()) result += ", ";
    result += x;
  };
  auto t = type();
  if (infoFields & RadicalField) add(Rad + radical().name() + '(' + std::to_string(radical().number()) + ')');
  if (infoFields & StrokesField && strokes()) add(Strokes + std::to_string(strokes()));
  if (infoFields & PinyinField && pinyin().has_value()) add(*pinyin());
  if (infoFields & GradeField && hasGrade()) add(toString(grade()));
  if (infoFields & LevelField && hasLevel()) add(toString(level()));
  if (infoFields & FreqField && frequency()) add(Freq + std::to_string(frequency()));
  // A kanji can possibly have a 'New' value (from a link) or an 'Old' value, but not both. Check for
  // linked types first (since oldName is a top level optional field on all kanji).
  auto optNewName = newName();
  if (optNewName.has_value()) {
    assert(oldNames().empty());
    if (infoFields & NewField) add(New + *optNewName);
  } else if (infoFields & OldField && !oldNames().empty()) {
    std::string s;
    for (auto& i : oldNames()) {
      if (!s.empty()) s += "／";
      s += i;
    }
    add(Old + s);
  }
  if (infoFields & KyuField && hasKyu()) add(toString(kyu()));
  return result;
}

} // namespace kanji_tools
