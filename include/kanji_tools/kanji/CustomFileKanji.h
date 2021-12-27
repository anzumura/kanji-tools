#ifndef KANJI_TOOLS_KANJI_CUSTOM_FILE_KANJI_H
#define KANJI_TOOLS_KANJI_CUSTOM_FILE_KANJI_H

#include <kanji_tools/kanji/NonLinkedKanji.h>
#include <kanji_tools/utils/ColumnFile.h>

namespace kanji_tools {

// 'CustomFileKanji' is the base class for ExtraKanji and OfficialKanji and supports loading data from
// column based customized local files.
// - Each file contains the same first 4 columns: 'Number', 'Name', 'Radical' and 'Reading'
// - Jouyou and Extra files contain 'Strokes' column, Jinmei strokes come from 'strokes.txt' or 'ucd.txt'
class CustomFileKanji : public NonLinkedKanji {
public:
  KenteiKyus kyu() const override { return _kyu; }
  OptString extraTypeInfo() const override { return '#' + std::to_string(_number); }
  const LinkNames& oldNames() const override { return _oldNames; }

  int number() const { return _number; }

  // 'fromString' is a factory method that creates a list of kanjis of the given 'type' from the given 'file'
  // - 'type' must be Jouyou, Jinmei or Extra
  // - 'file' must have tab separated lines that have the right number of columns for the given type
  // - first line of 'file' must have header names that match the static 'Column' instances below
  static Data::List fromFile(const Data&, KanjiTypes type, const std::filesystem::path& file);
protected:
  inline static ColumnFile::Column NumberCol{"Number"}, NameCol{"Name"}, RadicalCol{"Radical"}, OldNamesCol{"OldNames"},
    YearCol{"Year"}, StrokesCol{"Strokes"}, GradeCol{"Grade"}, MeaningCol{"Meaning"}, ReadingCol{"Reading"},
    ReasonCol{"Reason"};

  static const Ucd* findUcd(const Data& d, const std::string& name) { return d.findUcd(name); }

  // Constructor used by 'CustomFileKanji' and 'ExtraKanji': calls base with 'meaning' field
  CustomFileKanji(const Data& d, const ColumnFile& f, const std::string& name, int strokes, const std::string& meaning,
                  const LinkNames& oldNames, const Ucd* u)
    : NonLinkedKanji(d, name, d.getRadicalByName(f.get(RadicalCol)), meaning, f.get(ReadingCol), strokes, u),
      _kyu(d.getKyu(name)), _number(f.getInt(NumberCol)), _oldNames(oldNames) {}

  // Constructor used by 'OfficialKanji': calls base without 'meaning' field
  CustomFileKanji(const Data& d, const ColumnFile& f, const std::string& name, int strokes, const LinkNames& oldNames)
    : NonLinkedKanji(d, name, d.getRadicalByName(f.get(RadicalCol)), f.get(ReadingCol), strokes, findUcd(d, name)),
      _kyu(d.getKyu(name)), _number(f.getInt(NumberCol)), _oldNames(oldNames) {}
private:
  // all kanji files must have at least the following columns
  inline static const std::vector RequiredColumns{NumberCol, NameCol, RadicalCol, ReadingCol};

  // specific types require additional columns
  inline static const std::vector JouyouRequiredColumns{OldNamesCol, YearCol, StrokesCol, GradeCol, MeaningCol};
  inline static const std::vector JinmeiRequiredColumns{OldNamesCol, YearCol, ReasonCol};
  inline static const std::vector ExtraRequiredColumns{StrokesCol, MeaningCol};

  const KenteiKyus _kyu;
  const int _number;
  const LinkNames _oldNames;
};

// 'OfficialKanji' contains attributes shared by Jouyou and Jinmei kanji, i.e., optional 'Old' and 'Year' values
class OfficialKanji : public CustomFileKanji {
public:
  OptString extraTypeInfo() const override {
    return _year ? std::optional(*CustomFileKanji::extraTypeInfo() + ' ' + std::to_string(*_year))
                 : CustomFileKanji::extraTypeInfo();
  }

  OptInt frequency() const override { return _frequency; }
  JlptLevels level() const override { return _level; }

  OptInt year() const { return _year; }
protected:
  // constructor used by 'JinmeiKanji' calls base without 'meaning' field
  OfficialKanji(const Data& d, const ColumnFile& f, const std::string& name)
    : CustomFileKanji(d, f, name, d.getStrokes(name), getOldNames(f)), _frequency(d.getFrequency(name)),
      _level(d.getLevel(name)), _year(f.getOptInt(YearCol)) {}

  // constructor used by 'JouyouKanji' calls base with 'meaning' field
  OfficialKanji(const Data& d, const ColumnFile& f, const std::string& name, int s, const std::string& meaning)
    : CustomFileKanji(d, f, name, s, meaning, getOldNames(f), findUcd(d, name)), _frequency(d.getFrequency(name)),
      _level(d.getLevel(name)), _year(f.getOptInt(YearCol)) {}
private:
  static LinkNames getOldNames(const ColumnFile&);

  const OptInt _frequency;
  const JlptLevels _level;
  const OptInt _year;
};

class JinmeiKanji : public OfficialKanji {
public:
  // Reasons enum represents reason kanji was added to Jinmei list:
  // - Names: for use in names
  // - Print: for use in publications
  // - Variant: allowed variant form (異体字)
  // - Moved: moved out of Jouyou into Jinmei
  // - Other: reason listed as その他
  enum class Reasons { Names, Print, Variant, Moved, Other };
  static constexpr const char* toString(Reasons x) {
    switch (x) {
    case Reasons::Names: return "Names";
    case Reasons::Print: return "Print";
    case Reasons::Variant: return "Variant";
    case Reasons::Moved: return "Moved";
    default: return "Other";
    }
  }

  JinmeiKanji(const Data& d, const ColumnFile& f)
    : OfficialKanji(d, f, f.get(NameCol)), _reason(getReason(f.get(ReasonCol))) {}

  KanjiTypes type() const override { return KanjiTypes::Jinmei; }
  OptString extraTypeInfo() const override {
    return std::optional(*OfficialKanji::extraTypeInfo() + " [" + toString(_reason) + ']');
  }
  Reasons reason() const { return _reason; }
private:
  static Reasons getReason(const std::string&);
  const Reasons _reason;
};

class JouyouKanji : public OfficialKanji {
public:
  JouyouKanji(const Data& d, const ColumnFile& f)
    : OfficialKanji(d, f, f.get(NameCol), f.getInt(StrokesCol), f.get(MeaningCol)), _grade(getGrade(f.get(GradeCol))) {}

  KanjiTypes type() const override { return KanjiTypes::Jouyou; }
  KanjiGrades grade() const override { return _grade; }
private:
  static KanjiGrades getGrade(const std::string&);
  const KanjiGrades _grade;
};

// 'ExtraKanji' is used for kanji loaded from 'extra.txt'. 'extra.txt' is meant to hold 'fairly common'
// kanji, but kanji that are outside of the official lists (Jouyou, Jinmei and their linked kanji). They
// should also not be in 'frequency.txt' or have a JLPT level.
class ExtraKanji : public CustomFileKanji {
public:
  ExtraKanji(const Data& d, const ColumnFile& f) : ExtraKanji(d, f, f.get(NameCol)) {}

  KanjiTypes type() const override { return KanjiTypes::Extra; }
  OptString newName() const override { return _newName; }
private:
  ExtraKanji(const Data& d, const ColumnFile& f, const std::string& name) : ExtraKanji(d, f, name, findUcd(d, name)) {}
  ExtraKanji(const Data& d, const ColumnFile& f, const std::string& name, const Ucd* u)
    : CustomFileKanji(d, f, name, f.getInt(StrokesCol), f.get(MeaningCol),
                      u && u->hasTraditionalLinks() ? getLinkNames(u) : EmptyLinkNames, u),
      _newName(u && u->hasNonTraditionalLinks() ? OptString(u->links()[0].name()) : std::nullopt) {}
  const OptString _newName;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_CUSTOM_FILE_KANJI_H
