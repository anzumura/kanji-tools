#ifndef KANJI_TOOLS_KANJI_OTHER_KANJI_H
#define KANJI_TOOLS_KANJI_OTHER_KANJI_H

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'OtherKanji' is for kanji pulled in from frequency.txt and is also the base class for 'KenteiKanji'
// and 'ExtraKanji'
class OtherKanji : public Kanji {
public:
  // constructor used for 'Other' kanji with readings from 'other-readings.txt'
  OtherKanji(const Data& d, int number, const std::string& name, const std::string& reading)
    : Kanji(number, name, d.getCompatibilityName(name), d.ucdRadical(name), d.getStrokes(name), d.getPinyin(name),
            Levels::None, d.getKyu(name), d.getFrequency(name)),
      _meaning(d.ucd().getMeaning(name)), _reading(reading) {}
  // constructor used for 'Other' kanji without a reading (will look up from UCD instead)
  OtherKanji(const Data& d, int number, const std::string& name)
    : OtherKanji(d, number, name, d.ucd().getReadingsAsKana(name)) {}

  Types type() const override { return Types::Other; }
  const std::string& meaning() const override { return _meaning; }
  const std::string& reading() const override { return _reading; }
protected:
  // protected constructors used by derived ExtraKanji class (allows controlling strokes and frequency)
  OtherKanji(const Data& d, int number, const std::string& name, const Radical& radical, const std::string& meaning,
             const std::string& reading, int strokes, bool findFrequency)
    : Kanji(number, name, d.getCompatibilityName(name), radical, strokes, d.getPinyin(name), d.getLevel(name),
            d.getKyu(name), findFrequency ? d.getFrequency(name) : 0),
      _meaning(meaning), _reading(reading) {}
  OtherKanji(const Data& d, int number, const std::string& name, const Radical& radical, const std::string& reading,
             int strokes, bool findFrequency)
    : OtherKanji(d, number, name, radical, d.ucd().getMeaning(name), reading, strokes, findFrequency) {}
private:
  const std::string _meaning;
  const std::string _reading;
};

// 'KenteiKanji' is for kanji in kentei/k*.txt files that aren't already pulled in from other files
class KenteiKanji : public OtherKanji {
public:
  KenteiKanji(const Data& d, int number, const std::string& name) : OtherKanji(d, number, name) {}

  Types type() const override { return Types::Kentei; }
};

// 'ExtraKanji' is used for kanji loaded from 'extra.txt'. It is also the base class for 'OfficialKanji'
// - Each file contains the same first 4 columns: 'Number', 'Name', 'Radical' and 'Reading'
// - Jouyou and Extra files contain 'Strokes' column, Jinmei strokes come from 'strokes.txt' or 'ucd.txt'
class ExtraKanji : public OtherKanji {
public:
  ExtraKanji(const Data& d) : ExtraKanji(d, Data::toInt(columns[StrokesCol]), columns[MeaningCol]) {}

  Types type() const override { return Types::Extra; }

  // 'fromString' is a factory method that creates a list of kanjis of the given 'type' from the given 'file'
  // - 'type' must be Jouyou, Jinmei or Extra
  // - 'file' must have tab separated lines that have the right number of columns for the given type
  // - the first line of 'file' should have column header names that match the names in the 'Columns' enum
  static Data::List fromFile(const Data&, Types type, const std::filesystem::path& file);
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

  ExtraKanji(const Data& d, int strokes, bool findFrequency = true)
    : OtherKanji(d, Data::toInt(columns[NumberCol]), columns[NameCol], d.getRadicalByName(columns[RadicalCol]),
                 columns[ReadingCol], strokes, findFrequency) {}
  ExtraKanji(const Data& d, int strokes, const std::string& meaning, bool findFrequency = true)
    : OtherKanji(d, Data::toInt(columns[NumberCol]), columns[NameCol], d.getRadicalByName(columns[RadicalCol]), meaning,
                 columns[ReadingCol], strokes, findFrequency) {}
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
};

// 'OfficialKanji' contains attributes shared by Jouyou and Jinmei kanji, i.e., optional 'Old' and 'Year' values
class OfficialKanji : public ExtraKanji {
public:
  using OptInt = std::optional<int>;

  const OldNames& oldNames() const override { return _oldNames; }
  OptInt year() const { return _year; }
protected:
  OfficialKanji(const Data& d, int s) : ExtraKanji(d, s), _oldNames(getOldNames()), _year(optInt(columns[YearCol])) {}
  OfficialKanji(const Data& d, int s, const std::string& meaning)
    : ExtraKanji(d, s, meaning), _oldNames(getOldNames()), _year(optInt(columns[YearCol])) {}
private:
  static OldNames getOldNames();

  static OptInt optInt(const std::string& s) {
    if (s.empty()) return {};
    return Data::toInt(s);
  }

  const OldNames _oldNames;
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
  JinmeiKanji(const Data& d)
    : OfficialKanji(d, d.getStrokes(columns[NameCol])), _reason(getReason(columns[ReasonCol])) {}

  Types type() const override { return Types::Jinmei; }
  Reasons reason() const { return _reason; }
private:
  static Reasons getReason(const std::string&);
  const Reasons _reason;
};

class JouyouKanji : public OfficialKanji {
public:
  JouyouKanji(const Data& d)
    : OfficialKanji(d, Data::toInt(columns[StrokesCol]), columns[MeaningCol]), _grade(getGrade(columns[GradeCol])) {}

  Types type() const override { return Types::Jouyou; }
  Grades grade() const override { return _grade; }
private:
  static Grades getGrade(const std::string&);
  const Grades _grade;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_OTHER_KANJI_H
