#ifndef KANJI_KANJI_H
#define KANJI_KANJI_H

#include <kanji/Data.h>

namespace kanji {

// 'KanjiLegend' is meant to be used in output to briefly describe the suffix added to a kanji when
// using the 'qualifiedName' method. See comments for Kanji::qualifiedName for more details.
inline constexpr auto KanjiLegend = "Suffixes: '=JLPT \"=Freq ^=Jinmei ~=LinkedJinmei %=LinkedOld +=Extra *=Kentei";

class Kanji {
public:
  using OptString = std::optional<std::string>;
  static bool hasLink(Types t) { return t == Types::LinkedJinmei || t == Types::LinkedOld; }

  // Public constructor for Kanji found in frequency.txt that weren't found in one of the other
  // files. This constructor is also used by LinkedKanji derived class to avoid 'getLevel' call
  // done by the protected constructor.
  Kanji(const Data& d, int number, const std::string& name, Levels level = Levels::None)
    : Kanji(d, number, name, d.ucdRadical(name), d.getStrokes(name), true, level) {}
  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;

  virtual Types type() const = 0;
  virtual const std::string& meaning() const = 0;
  virtual const std::string& reading() const = 0;
  virtual Grades grade() const { return Grades::None; }
  virtual OptString oldName() const { return {}; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }
  bool variant() const { return _variant; } // true if _name includes a Unicode 'variation selector'
  const std::string& nonVariantName() const { return _nonVariantName; }
  const Radical& radical() const { return _radical; }
  int strokes() const { return _strokes; } // may be zero for kanjis only loaded from frequency.txt
  const OptString& pinyin() const { return _pinyin; }
  Levels level() const { return _level; }
  Kyus kyu() const { return _kyu; }
  int frequency() const { return _frequency; }
  int frequencyOrDefault(int x) const { return _frequency ? _frequency : x; }

  bool is(Types t) const { return type() == t; }
  bool hasLevel() const { return _level != Levels::None; }
  bool hasKyu() const { return _kyu != Kyus::None; }
  bool hasGrade() const { return grade() != Grades::None; }
  bool hasMeaning() const { return !meaning().empty(); }
  bool hasReading() const { return !reading().empty(); }

  // 'InfoFields' members can be used to select which fields are printed by 'info'
  // method. For example 'GradeField | LevelField | FreqField' will print grade and
  // level fields and 'AllFields ^ StrokesField' will print all except for strokes.
  enum InfoFields {
    RadicalField = 1,
    StrokesField,
    PinyinField = 4,
    GradeField = 8,
    LevelField = 16,
    FreqField = 32,
    NewField = 64,
    OldField = 128,
    KyuField = 256,
    AllFields = 511
  };

  // 'info' returns a comma separated string with extra info (if present) including:
  //   Radical, Strokes, Grade, Level, Freq, New, Old
  // 'infoFields' can be used to control inclusion of fields (include all by default).
  // Note: some Jouyou and Jinmei kanji have multiple old/variant forms, but at most
  // one will be displayed. 'New' is for 'Linked' type kanji and will show the official
  // 'standard' form in the Jouyou or Jinmei list.
  std::string info(int infoFields = AllFields) const;

  // 'qualifiedName' returns 'name' plus an extra marker to show additional information:
  // space = Jouyou         : all 2136 Jouyou (use space since this is the most common type)
  //     ' = JLPT           : 251 Jinmei in JLPT (out of 2222 total - the other 1971 are Jouyou)
  //     " = Top Frequency  : 296 top frequency not in Jouyou or JLPT
  //     ^ = Jinmei         : 224 Jinmei not already covered by the above types
  //     ~ = Linked Jinmei  : 218 Linked Jinmei (with no frequency)
  //     % = Linked Old     : 211 'no-frequency' Linked Old (with no frequency)
  //     + = Extra          : all kanji loaded from Extra file
  //     * = Kentei         : 2823 Kentei Kanji (that aren't in any of the above categories)
  std::string qualifiedName() const {
    auto t = type();
    return _name +
      (t == Types::Jouyou           ? ' '
         : hasLevel()               ? '\''
         : _frequency               ? '"'
         : t == Types::Jinmei       ? '^'
         : t == Types::LinkedJinmei ? '~'
         : t == Types::LinkedOld    ? '%'
         : t == Types::Extra        ? '+'
                                    : '*');
  }

  // helper functions for getting information on 'oldValue' (旧字体) kanjis
  Types oldType(const Data& d) const {
    auto i = oldName();
    if (i.has_value()) return d.getType(*i);
    return Types::None;
  }
  int oldStrokes(const Data& d) const {
    auto i = oldName();
    if (i.has_value()) return d.getStrokes(*i);
    return 0;
  }
  Levels oldLevel(const Data& d) const {
    auto i = oldName();
    if (i.has_value()) return d.getLevel(*i);
    return Levels::None;
  }
  int oldFrequency(const Data& d) const {
    auto i = oldName();
    if (i.has_value()) return d.getFrequency(*i);
    return 0;
  }
protected:
  // helper constructor for derived classes (can avoid looking up frequency for 'extra' kanji)
  Kanji(const Data& d, int number, const std::string& name, const Radical& radical, int strokes, bool findFrequency)
    : Kanji(d, number, name, radical, strokes, findFrequency, d.getLevel(name)) {}
  Kanji(const Data& d, int number, const std::string& name, const Radical& radical, int strokes, bool findFrequency,
        Levels level);
private:
  const int _number;
  const std::string _name;
  const bool _variant;
  const std::string _nonVariantName; // same as _name if _variant is false
  const Radical _radical;
  const int _strokes;
  const OptString _pinyin;
  const Levels _level;
  const Kyus _kyu;
  const int _frequency;
};

inline std::ostream& operator<<(std::ostream& os, const Kanji& k) { return os << k.name(); }

class LinkedKanji : public Kanji {
public:
  const std::string& meaning() const override { return _link->meaning(); }
  const std::string& reading() const override { return _link->reading(); }
  const Data::Entry& link() const { return _link; }
protected:
  LinkedKanji(const Data& d, int number, const std::string& name, const Data::Entry& link)
    : Kanji(d, number, name), _link(link) {}

  // linkedOldKanji must link back to Jouyou and LinkedJinmeiKanji can link to either Jouyou or Jinmei
  static const std::string& checkType(const std::string& name, const Data::Entry& link, bool isJinmei = false) {
    Types t = link->type();
    if (t != Types::Jouyou && (!isJinmei || t != Types::Jinmei))
      throw std::domain_error("LinkedKanji " + name + " wanted type '" + toString(Types::Jouyou) +
                              (isJinmei ? std::string("' or '") + toString(Types::Jinmei) : std::string()) +
                              "' for link " + link->name() + ", but got '" + toString(t) + "'");
    return name;
  }
private:
  const Data::Entry _link;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(const Data& d, int number, const std::string& name, const Data::Entry& link)
    : LinkedKanji(d, number, checkType(name, link, true), link) {}

  Types type() const override { return Types::LinkedJinmei; }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(const Data& d, int number, const std::string& name, const Data::Entry& link)
    : LinkedKanji(d, number, checkType(name, link), link) {}

  Types type() const override { return Types::LinkedOld; }
};

// 'NonLinkedKanji' is the base class for KenteiKanji and FileListKanji and is also the class
// used for 'Other' type kanji (pulled in from frequency.txt).
class NonLinkedKanji : public Kanji {
public:
  // public constructor used for 'Other' kanji with readings from 'other-readings.txt'
  NonLinkedKanji(const Data& d, int number, const std::string& name, const std::string& reading)
    : Kanji(d, number, name), _meaning(d.ucd().getMeaning(name)), _reading(reading) {}
  // public constructor used for 'Other' kanji without a reading (will look up from UCD instead)
  NonLinkedKanji(const Data& d, int number, const std::string& name)
    : NonLinkedKanji(d, number, name, d.ucd().getReadingsAsKana(name)) {}

  Types type() const override { return Types::Other; }
  const std::string& meaning() const override { return _meaning; }
  const std::string& reading() const override { return _reading; }
protected:
  // protected constructors used by derived FileListKanji class (allows controlling strokes and frequency)
  NonLinkedKanji(const Data& d, int number, const std::string& name, const Radical& radical, const std::string& meaning,
                 const std::string& reading, int strokes, bool findFrequency)
    : Kanji(d, number, name, radical, strokes, findFrequency), _meaning(meaning), _reading(reading) {}
  NonLinkedKanji(const Data& d, int number, const std::string& name, const Radical& radical, const std::string& reading,
                 int strokes, bool findFrequency)
    : NonLinkedKanji(d, number, name, radical, d.ucd().getMeaning(name), reading, strokes, findFrequency) {}
private:
  const std::string _meaning;
  const std::string _reading;
};

// KenteiKanji is for kanji in kentei/k*.txt files that aren't already pulled in from other files.
class KenteiKanji : public NonLinkedKanji {
public:
  KenteiKanji(const Data& d, int number, const std::string& name) : NonLinkedKanji(d, number, name) {}

  Types type() const override { return Types::Kentei; }
};

// FileListKanji is the base class for kanjis loaded from 'jouyou.txt', 'jinmei.txt' and 'extra.txt' files
// - Each file contains the same first 4 columns: 'Number', 'Name', 'Radical' and 'Reading'
// - Jouyou and Extra files contain 'Strokes' column, Jinmei strokes come from a separate 'strokes.txt' file.
class FileListKanji : public NonLinkedKanji {
public:
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
    OldNameCol,
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

  FileListKanji(const Data& d, int strokes, bool findFrequency = true)
    : NonLinkedKanji(d, Data::toInt(columns[NumberCol]), columns[NameCol], d.getRadicalByName(columns[RadicalCol]),
                     columns[ReadingCol], strokes, findFrequency) {}
  FileListKanji(const Data& d, int strokes, const std::string& meaning, bool findFrequency = true)
    : NonLinkedKanji(d, Data::toInt(columns[NumberCol]), columns[NameCol], d.getRadicalByName(columns[RadicalCol]),
                     meaning, columns[ReadingCol], strokes, findFrequency) {}
private:
  // all kanji files must have at least the following columns
  static constexpr std::array requiredColumns{NumberCol, NameCol, RadicalCol, ReadingCol};

  // specific types require additional columns
  static constexpr std::array jouyouRequiredColumns{OldNameCol, YearCol, StrokesCol, GradeCol, MeaningCol};
  static constexpr std::array jinmeiRequiredColumns{OldNameCol, YearCol, ReasonCol};
  static constexpr std::array extraRequiredColumns{StrokesCol, MeaningCol};
  static constexpr std::array ColumnNames{"Number",  "Name",  "Radical", "OldName", "Year",
                                          "Strokes", "Grade", "Meaning", "Reading", "Reason"};
  static std::pair<std::string, int> colPair(int x) { return std::make_pair(ColumnNames[x], x); }
  static std::map<std::string, int> ColumnMap; // maps column names to Column enum values
};

// OfficialListKanji contains attributes shared by Jouyou and Jinmei kanji, i.e., optional 'Old' and 'Year' values
class OfficialListKanji : public FileListKanji {
public:
  using OptInt = std::optional<int>;

  OptString oldName() const override { return _oldName; }
  OptInt year() const { return _year; }
protected:
  OfficialListKanji(const Data& d, int s)
    : FileListKanji(d, s), _oldName(optString(columns[OldNameCol])), _year(optInt(columns[YearCol])) {}
  OfficialListKanji(const Data& d, int s, const std::string& meaning)
    : FileListKanji(d, s, meaning), _oldName(optString(columns[OldNameCol])), _year(optInt(columns[YearCol])) {}
private:
  static OptString optString(const std::string& s) { return s.empty() ? std::nullopt : std::optional(s); }

  static OptInt optInt(const std::string& s) {
    if (s.empty()) return {};
    return Data::toInt(s);
  }

  const OptString _oldName;
  const OptInt _year;
};

class JinmeiKanji : public OfficialListKanji {
public:
  // Reasons enum represents reason kanji was added to Jinmei list:
  // - Names: for use in names
  // - Print: for use in publications
  // - Variant: allowed variant form (異体字)
  // - Moved: moved out of Jouyou into Jinmei
  // - Other: reason listed as その他
  enum class Reasons { Names, Print, Variant, Moved, Other };
  JinmeiKanji(const Data& d)
    : OfficialListKanji(d, d.getStrokes(columns[NameCol])), _reason(getReason(columns[ReasonCol])) {}

  Types type() const override { return Types::Jinmei; }
  Reasons reason() const { return _reason; }
private:
  static Reasons getReason(const std::string&);
  const Reasons _reason;
};

class ExtraKanji : public FileListKanji {
public:
  ExtraKanji(const Data& d) : FileListKanji(d, Data::toInt(columns[StrokesCol]), columns[MeaningCol], false) {}

  Types type() const override { return Types::Extra; }
};

class JouyouKanji : public OfficialListKanji {
public:
  JouyouKanji(const Data& d)
    : OfficialListKanji(d, Data::toInt(columns[StrokesCol]), columns[MeaningCol]), _grade(getGrade(columns[GradeCol])) {
  }

  Types type() const override { return Types::Jouyou; }
  Grades grade() const override { return _grade; }
private:
  static Grades getGrade(const std::string&);
  const Grades _grade;
};

} // namespace kanji

#endif // KANJI_KANJI_H
