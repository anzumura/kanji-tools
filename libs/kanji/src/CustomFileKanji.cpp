#include <kanji_tools/kanji/CustomFileKanji.h>

#include <sstream>

namespace kanji_tools {

// CustomFileKanji

Kanji::OptString CustomFileKanji::extraTypeInfo() const {
  return '#' + std::to_string(_number);
}

Kanji::Name CustomFileKanji::name(File f) { return f.get(NameCol); }

CustomFileKanji::CustomFileKanji(DataRef data, File f, Name name,
    Strokes strokes, Meaning meaning, OldNames oldNames, UcdPtr u)
    : NonLinkedKanji{data, name, data.getRadicalByName(f.get(RadicalCol)),
          strokes, meaning, f.get(ReadingCol), u},
      _kyu{data.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

CustomFileKanji::CustomFileKanji(
    // LCOV_EXCL_START: covered
    DataRef data, File f, Name name, OldNames oldNames, UcdPtr u)
    // LCOV_EXCL_STOP
    : NonLinkedKanji{data, name, data.getRadicalByName(f.get(RadicalCol)),
          f.get(ReadingCol), u},
      _kyu{data.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

// OfficialKanji

Kanji::OptString OfficialKanji::extraTypeInfo() const {
  return _year ? OptString{*CustomFileKanji::extraTypeInfo() + ' ' +
                           std::to_string(_year)}
               : CustomFileKanji::extraTypeInfo();
}

OfficialKanji::OfficialKanji(DataRef data, File f, Name name, UcdPtr u)
    : CustomFileKanji{data, f, name, getOldNames(f), u},
      _frequency{data.frequency(name)}, _level{data.level(name)},
      _year{f.isEmpty(YearCol) ? Year{} : f.getU16(YearCol)} {}

OfficialKanji::OfficialKanji(
    DataRef data, File f, Name name, Strokes strokes, Meaning meaning)
    : CustomFileKanji{data, f, name, strokes, meaning, getOldNames(f),
          data.findUcd(name)},
      _frequency{data.frequency(name)}, _level{data.level(name)},
      _year{f.isEmpty(YearCol) ? Year{} : f.getU16(YearCol)} {}

Kanji::LinkNames OfficialKanji::getOldNames(File f) {
  LinkNames result;
  std::stringstream ss{f.get(OldNamesCol)};
  for (String token; std::getline(ss, token, ',');) result.emplace_back(token);
  return result;
}

// JinmeiKanji

JinmeiKanji::JinmeiKanji(DataRef data, File f)
    : OfficialKanji{data, f, name(f), data.findUcd(f.get(NameCol))},
      _reason{AllJinmeiReasons.fromString(f.get(ReasonCol))} {}

Kanji::OptString JinmeiKanji::extraTypeInfo() const {
  return *OfficialKanji::extraTypeInfo() + " [" + toString(_reason) + ']';
}

// JouyouKanji

JouyouKanji::JouyouKanji(DataRef data, File f)
    : OfficialKanji{data, f, name(f), Strokes{f.getU8(StrokesCol)},
          f.get(MeaningCol)},
      _grade{getGrade(f.get(GradeCol))} {}

KanjiGrades JouyouKanji::getGrade(const String& s) {
  return AllKanjiGrades.fromString(s.starts_with("S") ? s : "G" + s);
}

// ExtraKanji

ExtraKanji::ExtraKanji(DataRef data, File f) : ExtraKanji{data, f, name(f)} {}

ExtraKanji::ExtraKanji(DataRef data, File f, Name name)
    : ExtraKanji{data, f, name, data.findUcd(name)} {}

ExtraKanji::ExtraKanji(DataRef data, File f, Name name, UcdPtr u)
    : CustomFileKanji{data, f, name, Strokes{f.getU8(StrokesCol)},
          f.get(MeaningCol),
          u && u->hasTraditionalLinks() ? linkNames(u) : EmptyLinkNames, u},
      _newName{u && u->hasNonTraditionalLinks()
                   ? OptString{u->links()[0].name()}
                   : std::nullopt} {}

// LinkedKanji

Kanji::Meaning LinkedKanji::meaning() const { return _link->meaning(); }

Kanji::Reading LinkedKanji::reading() const { return _link->reading(); }

Kanji::OptString LinkedKanji::newName() const { return _link->name(); }

LinkedKanji::LinkedKanji(
    DataRef data, Name name, const KanjiPtr& link, UcdPtr u)
    : Kanji{data, name, data.ucdRadical(name, u), data.ucdStrokes(name, u), u},
      _frequency{data.frequency(name)}, _kyu{data.kyu(name)}, _link{link} {}

Kanji::Name LinkedKanji::linkType(Name name, const Kanji& link, bool isJouyou) {
  if (const auto t{link.type()};
      t != KanjiTypes::Jouyou && (isJouyou || t != KanjiTypes::Jinmei))
    throw std::domain_error{
        "LinkedKanji " + name + " wanted type '" +
        toString(KanjiTypes::Jouyou) +
        (isJouyou ? EmptyString
                  : String{"' or '"} + toString(KanjiTypes::Jinmei)) +
        "' for link " + link.name() + ", but got '" + toString(t) + "'"};
  return name;
}

LinkedJinmeiKanji::LinkedJinmeiKanji(
    DataRef data, Name name, const KanjiPtr& link)
    : LinkedKanji{
          data, linkType(name, *link, false), link, data.findUcd(name)} {}

LinkedOldKanji::LinkedOldKanji(DataRef data, Name name, const KanjiPtr& link)
    : LinkedKanji{data, linkType(name, *link), link, data.findUcd(name)} {}

} // namespace kanji_tools
