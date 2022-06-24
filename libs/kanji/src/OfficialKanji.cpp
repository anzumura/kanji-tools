#include <kt_kanji/OfficialKanji.h>

#include <sstream>

namespace kanji_tools {

// NumberedKanji

Kanji::OptString NumberedKanji::extraTypeInfo() const {
  return '#' + std::to_string(_number);
}

Kanji::Name NumberedKanji::name(File f) { return f.get(NameCol); }

NumberedKanji::NumberedKanji(CtorParams params, File f, Strokes strokes,
    Meaning meaning, OldNames oldNames)
    : LoadedKanji{params, params.data().getRadicalByName(f.get(RadicalCol)),
          f.get(ReadingCol), strokes, meaning},
      _kyu{params.kyu()}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

NumberedKanji::NumberedKanji(CtorParams params, File f, OldNames oldNames)
    : LoadedKanji{params, params.data().getRadicalByName(f.get(RadicalCol)),
          f.get(ReadingCol)},
      _kyu{params.kyu()}, _number{f.getU16(NumberCol)}, _oldNames{oldNames} {}

// OfficialKanji

Kanji::OptString OfficialKanji::extraTypeInfo() const {
  return _year ? OptString{*NumberedKanji::extraTypeInfo() + ' ' +
                           std::to_string(_year)}
               : NumberedKanji::extraTypeInfo();
}

OfficialKanji::OfficialKanji(CtorParams params, File f)
    : NumberedKanji{params, f, getOldNames(f)}, _frequency{params.frequency()},
      _level{params.level()}, _year{f.isEmpty(YearCol) ? Year{}
                                                       : f.getU16(YearCol)} {}

OfficialKanji::OfficialKanji(
    KanjiDataRef data, File f, Name name, Strokes strokes, Meaning meaning)
    : NumberedKanji{{data, name}, f, strokes, meaning, getOldNames(f)},
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
    : OfficialKanji{{data, name(f)}, f}, _reason{AllJinmeiReasons.fromString(
                                             f.get(ReasonCol))} {}

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
    : ExtraKanji{{data, name(f)}, f} {}

ExtraKanji::ExtraKanji(CtorParams params, File f)
    : NumberedKanji{params, f, Strokes{f.getU8(StrokesCol)}, f.get(MeaningCol),
          params.hasTraditionalLinks() ? linkNames(params.ucd())
                                       : EmptyLinkNames},
      _newName{params.hasNonTraditionalLinks()
                   ? OptString{params.ucd()->links()[0].name()}
                   : std::nullopt} {}

// OfficialLinkedKanji

Kanji::Meaning OfficialLinkedKanji::meaning() const { return _link->meaning(); }

Kanji::Reading OfficialLinkedKanji::reading() const { return _link->reading(); }

Kanji::OptString OfficialLinkedKanji::newName() const { return _link->name(); }

OfficialLinkedKanji::OfficialLinkedKanji(CtorParams params, Link link)
    : Kanji{params, params.radical(), params.strokes()},
      _frequency{params.frequency()}, _kyu{params.kyu()}, _link{link} {}

Kanji::CtorParams OfficialLinkedKanji::check(
    KanjiDataRef data, Name name, Link link, bool isOld) {
  if (const auto t{link->type()};
      t != KanjiTypes::Jouyou && (isOld || t != KanjiTypes::Jinmei))
    throw DomainError{
        "OfficialLinkedKanji " + name + " wanted type '" +
        toString(KanjiTypes::Jouyou) +
        (isOld ? emptyString()
               : String{"' or '"} + toString(KanjiTypes::Jinmei)) +
        "' for link " + link->name() + ", but got '" + toString(t) + "'"};
  return {data, name};
}

LinkedJinmeiKanji::LinkedJinmeiKanji(KanjiDataRef data, Name name, Link link)
    : OfficialLinkedKanji{check(data, name, link, false), link} {}

LinkedOldKanji::LinkedOldKanji(KanjiDataRef data, Name name, Link link)
    : OfficialLinkedKanji{check(data, name, link, true), link} {}

} // namespace kanji_tools
