#include <kanji_tools/kanji/CustomFileKanji.h>

#include <sstream>

namespace kanji_tools {

// CustomFileKanji

Kanji::OptString CustomFileKanji::extraTypeInfo() const {
  return '#' + std::to_string(_number);
}

Kanji::Name CustomFileKanji::name(ColumnFileRef f) { return f.get(NameCol); }

CustomFileKanji::CustomFileKanji(DataRef data, ColumnFileRef f, Name name,
    Strokes strokes, Meaning meaning, OldNames oldNames, UcdPtr u)
    : NonLinkedKanji{data, name, data.getRadicalByName(f.get(RadicalCol)),
          strokes, meaning, f.get(ReadingCol), u},
      _kyu{data.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

CustomFileKanji::CustomFileKanji(
    // LCOV_EXCL_START: gcov bug
    DataRef data, ColumnFileRef f, Name name, OldNames oldNames, UcdPtr u)
    // LCOV_EXCL_STOP
    : NonLinkedKanji{data, name, data.getRadicalByName(f.get(RadicalCol)),
          f.get(ReadingCol), u},
      _kyu{data.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

// OfficialKanji

Kanji::OptString OfficialKanji::extraTypeInfo() const {
  return _year ? OptString{*CustomFileKanji::extraTypeInfo() + ' ' +
                           std::to_string(*_year)}
               : CustomFileKanji::extraTypeInfo();
}

OfficialKanji::OfficialKanji(DataRef data, ColumnFileRef f, Name name, UcdPtr u)
    : CustomFileKanji{data, f, name, getOldNames(f), u},
      _frequency{data.frequency(name)}, _level{data.level(name)},
      _year{f.getOptU16(YearCol)} {}

OfficialKanji::OfficialKanji(
    DataRef data, ColumnFileRef f, Name name, Strokes strokes, Meaning meaning)
    : CustomFileKanji{data, f, name, strokes, meaning, getOldNames(f),
          data.findUcd(name)},
      _frequency{data.frequency(name)}, _level{data.level(name)},
      _year{f.getOptU16(YearCol)} {}

Kanji::LinkNames OfficialKanji::getOldNames(ColumnFileRef f) {
  LinkNames result;
  std::stringstream ss{f.get(OldNamesCol)};
  for (std::string token; std::getline(ss, token, ',');)
    result.emplace_back(token);
  return result;
}

// JinmeiKanji

JinmeiKanji::JinmeiKanji(DataRef data, ColumnFileRef f)
    : OfficialKanji{data, f, name(f), data.findUcd(f.get(NameCol))},
      _reason{AllJinmeiKanjiReasons.fromString(f.get(ReasonCol))} {}

Kanji::OptString JinmeiKanji::extraTypeInfo() const {
  return *OfficialKanji::extraTypeInfo() + " [" + toString(_reason) + ']';
}

// JouyouKanji

JouyouKanji::JouyouKanji(DataRef data, ColumnFileRef f)
    : OfficialKanji{data, f, name(f), f.getU8(StrokesCol), f.get(MeaningCol)},
      _grade{getGrade(f.get(GradeCol))} {}

KanjiGrades JouyouKanji::getGrade(const std::string& s) {
  return AllKanjiGrades.fromString(s.starts_with("S") ? s : "G" + s);
}

// ExtraKanji

ExtraKanji::ExtraKanji(DataRef data, ColumnFileRef f)
    : ExtraKanji{data, f, name(f)} {}

ExtraKanji::ExtraKanji(DataRef data, ColumnFileRef f, Name name)
    : ExtraKanji{data, f, name, data.findUcd(name)} {}

ExtraKanji::ExtraKanji(DataRef data, ColumnFileRef f, Name name, UcdPtr u)
    : CustomFileKanji{data, f, name, f.getU8(StrokesCol), f.get(MeaningCol),
          u && u->hasTraditionalLinks() ? linkNames(u) : EmptyLinkNames, u},
      _newName{u && u->hasNonTraditionalLinks()
                   ? OptString{u->links()[0].name()}
                   : std::nullopt} {}

} // namespace kanji_tools
