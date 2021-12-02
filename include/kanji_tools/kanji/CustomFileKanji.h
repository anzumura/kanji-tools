#ifndef KANJI_TOOLS_KANJI_CUSTOM_FILE_KANJI_H
#define KANJI_TOOLS_KANJI_CUSTOM_FILE_KANJI_H

#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

// 'CustomFileKanji' is the base class for ExtraKanji and OfficialKanji and supports loading data from
// column based customized local files.
// - Each file contains the same first 4 columns: 'Number', 'Name', 'Radical' and 'Reading'
// - Jouyou and Extra files contain 'Strokes' column, Jinmei strokes come from 'strokes.txt' or 'ucd.txt'
class CustomFileKanji : public NonLinkedKanji {
public:
  const LinkNames& oldNames() const override { return _oldNames; }
  // 'fromString' is a factory method that creates a list of kanjis of the given 'type' from the given 'file'
  // - 'type' must be Jouyou, Jinmei or Extra
  // - 'file' must have tab separated lines that have the right number of columns for the given type
  // - the first line of 'file' should have column header names that match the names in the 'Columns' enum
  static Data::List fromFile(const Data&, KanjiTypes type, const std::filesystem::path& file);
protected:
  // list of all supported columns in files
  enum Columns {
    NumberCol,
    NameCol,
    RadicalCol,
    OldNamesCol,
    YearCol,
    StrokesCol,
    GradeCol,
    MeaningCol,
    ReadingCol,
    ReasonCol,
    MaxCol
  };
  // 'columns' contains list of values for each column after parsing a line (used by 'fromString' method)
  static std::array<std::string, MaxCol> columns;
  // Constructor used by 'ExtraKanji'
  CustomFileKanji(const Data& d, int strokes, const std::string& meaning, const LinkNames& oldNames, const Ucd* u)
    : NonLinkedKanji(d, Data::toInt(columns[NumberCol]), columns[NameCol], d.getRadicalByName(columns[RadicalCol]),
                     meaning, columns[ReadingCol], strokes, u, false, false),
      _oldNames(oldNames) {}
  // Constructors used by 'OfficialKanji'
  CustomFileKanji(const Data& d, int strokes, const LinkNames& oldNames)
    : NonLinkedKanji(d, Data::toInt(columns[NumberCol]), columns[NameCol], d.getRadicalByName(columns[RadicalCol]),
                     columns[ReadingCol], strokes, d.findUcd(columns[NameCol])),
      _oldNames(oldNames) {}
  CustomFileKanji(const Data& d, int strokes, const std::string& meaning, const LinkNames& oldNames)
    : NonLinkedKanji(d, Data::toInt(columns[NumberCol]), columns[NameCol], d.getRadicalByName(columns[RadicalCol]),
                     meaning, columns[ReadingCol], strokes, d.findUcd(columns[NameCol])),
      _oldNames(oldNames) {}
private:
  // all kanji files must have at least the following columns
  static constexpr std::array requiredColumns{NumberCol, NameCol, RadicalCol, ReadingCol};

  // specific types require additional columns
  static constexpr std::array jouyouRequiredColumns{OldNamesCol, YearCol, StrokesCol, GradeCol, MeaningCol};
  static constexpr std::array jinmeiRequiredColumns{OldNamesCol, YearCol, ReasonCol};
  static constexpr std::array extraRequiredColumns{StrokesCol, MeaningCol};
  static constexpr std::array ColumnNames{"Number",  "Name",  "Radical", "OldNames", "Year",
                                          "Strokes", "Grade", "Meaning", "Reading",  "Reason"};
  static std::pair<std::string, int> colPair(int x) { return std::make_pair(ColumnNames[x], x); }
  static std::map<std::string, int> ColumnMap; // maps column names to Column enum values

  const LinkNames _oldNames;
};

// 'ExtraKanji' is used for kanji loaded from 'extra.txt'. 'extra.txt' is meant to hold 'fairly common'
// kanji, but kanji that are outside of the official lists (Jouyou, Jinmei and their linked kanji). They
// should also not be in 'frequency.txt' or have a JLPT level.
class ExtraKanji : public CustomFileKanji {
public:
  ExtraKanji(const Data& d) : ExtraKanji(d, d.findUcd(columns[NameCol])) {}

  KanjiTypes type() const override { return KanjiTypes::Extra; }
  OptString newName() const override { return _newName; }
private:
  ExtraKanji(const Data& d, const Ucd* u)
    : CustomFileKanji(d, Data::toInt(columns[StrokesCol]), columns[MeaningCol],
                      u && u->hasTraditionalLinks() ? getLinkNames(u) : EmptyLinkNames, u),
      _newName(u && u->hasNonTraditionalLinks() ? OptString(u->links()[0].name()) : std::nullopt) {}
  const OptString _newName;
};

// 'OfficialKanji' contains attributes shared by Jouyou and Jinmei kanji, i.e., optional 'Old' and 'Year' values
class OfficialKanji : public CustomFileKanji {
public:
  using OptInt = std::optional<int>;

  OptInt year() const { return _year; }
protected:
  OfficialKanji(const Data& d, int s) : CustomFileKanji(d, s, getOldNames()), _year(optInt(columns[YearCol])) {}
  OfficialKanji(const Data& d, int s, const std::string& meaning)
    : CustomFileKanji(d, s, meaning, getOldNames()), _year(optInt(columns[YearCol])) {}
private:
  static LinkNames getOldNames();

  static OptInt optInt(const std::string& s) {
    if (s.empty()) return {};
    return Data::toInt(s);
  }

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

  JinmeiKanji(const Data& d)
    : OfficialKanji(d, d.getStrokes(columns[NameCol])), _reason(getReason(columns[ReasonCol])) {}

  KanjiTypes type() const override { return KanjiTypes::Jinmei; }
  OptString extraTypeInfo() const override {
    // year should always be populated for Jinmei kanji
    return year().has_value() ? std::optional(std::to_string(*year()) + ' ' + toString(_reason)) : std::nullopt;
  }
  Reasons reason() const { return _reason; }
private:
  static Reasons getReason(const std::string&);
  const Reasons _reason;
};

class JouyouKanji : public OfficialKanji {
public:
  JouyouKanji(const Data& d)
    : OfficialKanji(d, Data::toInt(columns[StrokesCol]), columns[MeaningCol]), _grade(getGrade(columns[GradeCol])) {}

  KanjiTypes type() const override { return KanjiTypes::Jouyou; }
  OptString extraTypeInfo() const override {
    return year().has_value() ? std::optional(std::to_string(*year())) : std::nullopt;
  }
  KanjiGrades grade() const override { return _grade; }
private:
  static KanjiGrades getGrade(const std::string&);
  const KanjiGrades _grade;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_CUSTOM_FILE_KANJI_H
