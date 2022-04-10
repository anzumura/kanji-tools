#include <kanji_tools/kanji/CustomFileKanji.h>

#include <sstream>

namespace kanji_tools {

// CustomFileKanji

Kanji::OptString CustomFileKanji::extraTypeInfo() const {
  return '#' + std::to_string(_number);
}

CustomFileKanji::CustomFileKanji(const Data& data, const ColumnFile& f,
    // LCOV_EXCL_START
    const std::string& name, Strokes strokes, const std::string& meaning,
    // LCOV_EXCL_STOP
    const LinkNames& oldNames, const Ucd* u)
    : NonLinkedKanji{data, name, data.getRadicalByName(f.get(RadicalCol)),
          strokes, meaning, f.get(ReadingCol), u},
      _kyu{data.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

CustomFileKanji::CustomFileKanji(const Data& data, const ColumnFile& f,
    const std::string& name, Strokes strokes, const LinkNames& oldNames,
    const Ucd* u) // LCOV_EXCL_LINE
    : NonLinkedKanji{data, name, data.getRadicalByName(f.get(RadicalCol)),
          strokes, f.get(ReadingCol), u},
      _kyu{data.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

// OfficialKanji

Kanji::OptString OfficialKanji::extraTypeInfo() const {
  return _year ? OptString{*CustomFileKanji::extraTypeInfo() + ' ' +
                           std::to_string(*_year)}
               : CustomFileKanji::extraTypeInfo();
}

OfficialKanji::OfficialKanji(const Data& data, const ColumnFile& f,
    const std::string& name, const Ucd* u)
    : CustomFileKanji{data, f, name, data.ucdStrokes(name, u), getOldNames(f),
          u},
      _frequency{data.frequency(name)}, _level{data.level(name)},
      _year{f.getOptU16(YearCol)} {}

OfficialKanji::OfficialKanji(const Data& data, const ColumnFile& f,
    // LCOV_EXCL_START
    const std::string& name, Strokes strokes, const std::string& meaning)
    // LCOV_EXCL_STOP
    : CustomFileKanji{data, f, name, strokes, meaning, getOldNames(f),
          data.findUcd(name)},
      _frequency{data.frequency(name)}, _level{data.level(name)},
      _year{f.getOptU16(YearCol)} {}

Kanji::LinkNames OfficialKanji::getOldNames(const ColumnFile& f) {
  LinkNames result;
  std::stringstream ss{f.get(OldNamesCol)};
  for (std::string token; std::getline(ss, token, ',');)
    result.emplace_back(token);
  return result;
}

// JinmeiKanji

JinmeiKanji::JinmeiKanji(const Data& data, const ColumnFile& f)
    : OfficialKanji{data, f, f.get(NameCol), data.findUcd(f.get(NameCol))},
      _reason{AllJinmeiKanjiReasons.fromString(f.get(ReasonCol))} {}

Kanji::OptString JinmeiKanji::extraTypeInfo() const {
  return *OfficialKanji::extraTypeInfo() + " [" + toString(_reason) + ']';
}

// JouyouKanji

JouyouKanji::JouyouKanji(const Data& data, const ColumnFile& f)
    : OfficialKanji{data, f, f.get(NameCol), f.getU8(StrokesCol),
          f.get(MeaningCol)},
      _grade{getGrade(f.get(GradeCol))} {}

KanjiGrades JouyouKanji::getGrade(const std::string& s) {
  return AllKanjiGrades.fromString(s.starts_with("S") ? s : "G" + s);
}

// ExtraKanji

ExtraKanji::ExtraKanji(const Data& data, const ColumnFile& f)
    : ExtraKanji{data, f, f.get(NameCol)} {}

ExtraKanji::ExtraKanji(
    const Data& data, const ColumnFile& f, const std::string& name)
    : ExtraKanji{data, f, name, data.findUcd(name)} {}

ExtraKanji::ExtraKanji(const Data& data, const ColumnFile& f,
    const std::string& name, const Ucd* u)
    : CustomFileKanji{data, f, name, f.getU8(StrokesCol), f.get(MeaningCol),
          u && u->hasTraditionalLinks() ? linkNames(u) : EmptyLinkNames, u},
      _newName{u && u->hasNonTraditionalLinks()
                   ? OptString{u->links()[0].name()}
                   : std::nullopt} {}

} // namespace kanji_tools
