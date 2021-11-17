#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/utils/MBChar.h>

namespace kanji_tools {

const char* toString(Grades x) {
  switch (x) {
  case Grades::S: return "S";
  case Grades::G6: return "G6";
  case Grades::G5: return "G5";
  case Grades::G4: return "G4";
  case Grades::G3: return "G3";
  case Grades::G2: return "G2";
  case Grades::G1: return "G1";
  default: return "None";
  }
}

const char* toString(Types x) {
  switch (x) {
  case Types::Jouyou: return "Jouyou";
  case Types::Jinmei: return "Jinmei";
  case Types::LinkedJinmei: return "LinkedJinmei";
  case Types::LinkedOld: return "LinkedOld";
  case Types::Other: return "Other";
  case Types::Extra: return "Extra";
  case Types::Kentei: return "Kentei";
  default: return "None";
  }
}

Kanji::Kanji(int number, const std::string& name, const std::string& compatibilityName, const Radical& radical,
             int strokes, const OptString& pinyin, Levels level, Kyus kyu, int frequency)
  : _number(number), _name(name), _variant(MBChar::isMBCharWithVariationSelector(name)),
    _nonVariantName(MBChar::withoutVariationSelector(name)), _compatibilityName(compatibilityName), _radical(radical),
    _strokes(strokes), _pinyin(pinyin), _level(level), _kyu(kyu), _frequency(frequency) {
  assert(MBChar::length(_name) == 1);
}

std::string Kanji::info(int infoFields) const {
  static const std::string Pinyin("Pinyin "), Rad("Rad "), Strokes("Strokes "), Grade("Grade "), Level("Level "),
    Freq("Freq "), New("New "), Old("Old "), Kyu("Kyu ");
  std::string result;
  auto add = [&result](const auto& x) {
    if (!result.empty()) result += ", ";
    result += x;
  };
  auto t = type();
  if (infoFields & RadicalField) add(Rad + radical().name() + ':' + std::to_string(radical().number()));
  if (infoFields & StrokesField && strokes()) add(Strokes + std::to_string(strokes()));
  if (infoFields & PinyinField && pinyin().has_value()) add(Pinyin + *pinyin());
  if (infoFields & GradeField && hasGrade()) add(Grade + toString(grade()));
  if (infoFields & LevelField && hasLevel()) add(Level + toString(level()));
  if (infoFields & FreqField && frequency()) add(Freq + std::to_string(frequency()));
  // A kanji can possibly have a 'New' value (from a link) or an 'Old' value, but not both. Check for
  // linked types first (since oldName is a top level optional field on all kanji).
  if (hasLink(t)) {
    assert(oldNames().empty());
    if (infoFields & NewField) add(New + *newName());
  } else if (infoFields & OldField && !oldNames().empty()) {
    std::string s;
    for (auto& i : oldNames()) {
      if (!s.empty()) s += "／";
      s += i;
    }
    add(Old + s);
  }
  if (infoFields & KyuField && hasKyu()) add(Kyu + toString(kyu()));
  return result;
}

} // namespace kanji_tools
