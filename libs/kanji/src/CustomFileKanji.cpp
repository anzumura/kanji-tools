#include <kanji_tools/kanji/CustomFileKanji.h>

#include <sstream>

namespace kanji_tools {

// CustomFileKanji

Kanji::OptString CustomFileKanji::extraTypeInfo() const {
  return '#' + std::to_string(_number);
}

const Ucd* CustomFileKanji::findUcd(const Data& d, const std::string& name) {
  return d.findUcd(name);
}

CustomFileKanji::CustomFileKanji(const Data& d, const ColumnFile& f,
    const std::string& name, Strokes strokes, const std::string& meaning,
    const LinkNames& oldNames, const Ucd* u) // LCOV_EXCL_LINE
    : NonLinkedKanji{d, name, d.getRadicalByName(f.get(RadicalCol)), meaning,
          f.get(ReadingCol), strokes, u},
      _kyu{d.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

CustomFileKanji::CustomFileKanji(const Data& d, const ColumnFile& f,
    const std::string& name, Strokes strokes, const LinkNames& oldNames,
    const Ucd* u)
    : NonLinkedKanji{d, name, d.getRadicalByName(f.get(RadicalCol)),
          f.get(ReadingCol), strokes, u},
      _kyu{d.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

// OfficialKanji

Kanji::OptString OfficialKanji::extraTypeInfo() const {
  return _year ? OptString{*CustomFileKanji::extraTypeInfo() + ' ' +
                           std::to_string(*_year)}
               : CustomFileKanji::extraTypeInfo();
}

OfficialKanji::OfficialKanji(
    const Data& d, const ColumnFile& f, const std::string& name, const Ucd* u)
    : CustomFileKanji{d, f, name, d.ucdStrokes(name, u), getOldNames(f), u},
      _frequency{d.frequency(name)}, _level{d.level(name)}, _year{f.getOptU16(
                                                                YearCol)} {}

OfficialKanji::OfficialKanji(const Data& d, const ColumnFile& f,
    const std::string& name, Strokes s, const std::string& meaning)
    : CustomFileKanji{d, f, name, s, meaning, getOldNames(f), findUcd(d, name)},
      _frequency{d.frequency(name)}, _level{d.level(name)}, _year{f.getOptU16(
                                                                YearCol)} {}

Kanji::LinkNames OfficialKanji::getOldNames(const ColumnFile& f) {
  LinkNames result;
  std::stringstream ss{f.get(OldNamesCol)};
  for (std::string token; std::getline(ss, token, ',');)
    result.emplace_back(token);
  return result;
}

// JinmeiKanji

JinmeiKanji::JinmeiKanji(const Data& d, const ColumnFile& f)
    : OfficialKanji{d, f, f.get(NameCol), d.findUcd(f.get(NameCol))},
      _reason{AllJinmeiKanjiReasons.fromString(f.get(ReasonCol))} {}

Kanji::OptString JinmeiKanji::extraTypeInfo() const {
  return *OfficialKanji::extraTypeInfo() + " [" + toString(_reason) + ']';
}

// JouyouKanji

JouyouKanji::JouyouKanji(const Data& d, const ColumnFile& f)
    : OfficialKanji{d, f, f.get(NameCol), f.getU8(StrokesCol),
          f.get(MeaningCol)},
      _grade{getGrade(f.get(GradeCol))} {}

KanjiGrades JouyouKanji::getGrade(const std::string& s) {
  return AllKanjiGrades.fromString(s.starts_with("S") ? s : "G" + s);
}

// ExtraKanji

ExtraKanji::ExtraKanji(const Data& d, const ColumnFile& f)
    : ExtraKanji{d, f, f.get(NameCol)} {}

ExtraKanji::ExtraKanji(
    const Data& d, const ColumnFile& f, const std::string& name)
    : ExtraKanji{d, f, name, findUcd(d, name)} {}

ExtraKanji::ExtraKanji(
    const Data& d, const ColumnFile& f, const std::string& name, const Ucd* u)
    : CustomFileKanji{d, f, name, f.getU8(StrokesCol), f.get(MeaningCol),
          u && u->hasTraditionalLinks() ? linkNames(u) : EmptyLinkNames, u},
      _newName{u && u->hasNonTraditionalLinks()
                   ? OptString{u->links()[0].name()}
                   : std::nullopt} {}

} // namespace kanji_tools
