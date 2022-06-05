#include <kanji_tools/kanji/OfficialKanji.h>

#include <sstream>

namespace kanji_tools {

// NumberedKanji

Kanji::OptString NumberedKanji::extraTypeInfo() const {
  return '#' + std::to_string(_number);
}

Kanji::Name NumberedKanji::name(File f) { return f.get(NameCol); }

NumberedKanji::NumberedKanji(KanjiDataRef data, File f, Name name,
    Strokes strokes, Meaning meaning, OldNames oldNames, UcdPtr u)
    : LoadedKanji{data, name, data.getRadicalByName(f.get(RadicalCol)), strokes,
          meaning, f.get(ReadingCol), u},
      _kyu{data.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

NumberedKanji::NumberedKanji(KanjiDataRef data, File f, Name name,
    OldNames oldNames, UcdPtr u) // LCOV_EXCL_LINE
    : LoadedKanji{data, name, data.getRadicalByName(f.get(RadicalCol)),
          f.get(ReadingCol), u},
      _kyu{data.kyu(name)}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

// OfficialKanji

Kanji::OptString OfficialKanji::extraTypeInfo() const {
  return _year ? OptString{*NumberedKanji::extraTypeInfo() + ' ' +
                           std::to_string(_year)}
               : NumberedKanji::extraTypeInfo();
}

OfficialKanji::OfficialKanji(KanjiDataRef data, File f, Name name, UcdPtr u)
    : NumberedKanji{data, f, name, getOldNames(f), u},
      _frequency{data.frequency(name)}, _level{data.level(name)},
      _year{f.isEmpty(YearCol) ? Year{} : f.getU16(YearCol)} {}

OfficialKanji::OfficialKanji(KanjiDataRef data, File f, Name name,
    Strokes strokes, Meaning meaning) // LCOV_EXCL_LINE
    : NumberedKanji{data, f, name, strokes, meaning, getOldNames(f),
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

JinmeiKanji::JinmeiKanji(KanjiDataRef data, File f)
    : OfficialKanji{data, f, name(f), data.findUcd(f.get(NameCol))},
      _reason{AllJinmeiReasons.fromString(f.get(ReasonCol))} {}

Kanji::OptString JinmeiKanji::extraTypeInfo() const {
  return *OfficialKanji::extraTypeInfo() + " [" + toString(_reason) + ']';
}

// JouyouKanji

JouyouKanji::JouyouKanji(KanjiDataRef data, File f)
    : OfficialKanji{data, f, name(f), Strokes{f.getU8(StrokesCol)},
          f.get(MeaningCol)},
      _grade{getGrade(f.get(GradeCol))} {}

KanjiGrades JouyouKanji::getGrade(const String& s) {
  return AllKanjiGrades.fromString(s.starts_with("S") ? s : "G" + s);
}

// ExtraKanji

ExtraKanji::ExtraKanji(KanjiDataRef data, File f)
    : ExtraKanji{data, f, name(f)} {}

ExtraKanji::ExtraKanji(KanjiDataRef data, File f, Name name)
    : ExtraKanji{data, f, name, data.findUcd(name)} {}

ExtraKanji::ExtraKanji(KanjiDataRef data, File f, Name name, UcdPtr u)
    : NumberedKanji{data, f, name, Strokes{f.getU8(StrokesCol)},
          f.get(MeaningCol),
          u && u->hasTraditionalLinks() ? linkNames(u) : EmptyLinkNames, u},
      _newName{u && u->hasNonTraditionalLinks()
                   ? OptString{u->links()[0].name()}
                   : std::nullopt} {}

// OfficialLinkedKanji

Kanji::Meaning OfficialLinkedKanji::meaning() const { return _link->meaning(); }

Kanji::Reading OfficialLinkedKanji::reading() const { return _link->reading(); }

Kanji::OptString OfficialLinkedKanji::newName() const { return _link->name(); }

OfficialLinkedKanji::OfficialLinkedKanji(
    KanjiDataRef data, Name name, const KanjiPtr& link, UcdPtr u)
    : Kanji{data, name, data.ucdRadical(name, u), data.ucdStrokes(name, u), u},
      _frequency{data.frequency(name)}, _kyu{data.kyu(name)}, _link{link} {}

Kanji::Name OfficialLinkedKanji::check(
    Name name, const Kanji& link, bool isOld) {
  if (const auto t{link.type()};
      t != KanjiTypes::Jouyou && (isOld || t != KanjiTypes::Jinmei))
    throw DomainError{
        "OfficialLinkedKanji " + name + " wanted type '" +
        toString(KanjiTypes::Jouyou) +
        (isOld ? EmptyString
               : String{"' or '"} + toString(KanjiTypes::Jinmei)) +
        "' for link " + link.name() + ", but got '" + toString(t) + "'"};
  return name;
}

LinkedJinmeiKanji::LinkedJinmeiKanji(
    KanjiDataRef data, Name name, const KanjiPtr& link)
    : OfficialLinkedKanji{
          data, check(name, *link, false), link, data.findUcd(name)} {}

LinkedOldKanji::LinkedOldKanji(
    KanjiDataRef data, Name name, const KanjiPtr& link)
    : OfficialLinkedKanji{
          data, check(name, *link, true), link, data.findUcd(name)} {}

} // namespace kanji_tools
