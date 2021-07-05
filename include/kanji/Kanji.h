#ifndef KANJI_KANJI_H
#define KANJI_KANJI_H

#include <kanji/Data.h>

namespace kanji {

class Kanji {
public:
  using OptString = std::optional<std::string>;
  // Public constructor for Kanji found in frequency.txt that weren't found in one of the other
  // files. This constructor is also used by LinkedKanji derived class to avoid 'getLevel' call
  // done by the protected constructor.
  Kanji(const Data& d, int number, const std::string& name, Levels level = Levels::None)
    : _number(number), _name(name), _strokes(d.getStrokes(name)), _level(level), _frequency(d.getFrequency(name)) {}
  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;

  virtual Types type() const { return Types::Other; }
  virtual Grades grade() const { return Grades::None; }
  virtual OptString oldName() const { return {}; }
  virtual const std::string& meaning() const { return EmptyString; }
  virtual const std::string& reading() const { return EmptyString; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }
  int strokes() const { return _strokes; } // may be zero for kanjis only loaded from frequency.txt
  Levels level() const { return _level; }
  int frequency() const { return _frequency; }
  int frequencyOrDefault(int x) const { return _frequency ? _frequency : x; }

  bool is(Types t) const { return type() == t; }
  bool hasLevel() const { return _level != Levels::None; }
  bool hasGrade() const { return grade() != Grades::None; }
  bool hasMeaning() const { return !meaning().empty(); }
  bool hasReading() const { return !reading().empty(); }
  // 'qualifiedName' returns 'name' plus an extra marker to show additional information:
  // space = Jouyou         : all 2136 Jouyou (use space since this is the most common type)
  //     ' = JLPT           : 251 Jinmei in JLPT (out of 2222 total - the other 1971 are Jouyou)
  //     " = Top Frequency  : 296 top frequency not in Jouyou or JLPT
  //     ^ = Jinmei         : 224 Jinmei not already covered by the above types
  //     ~ = Linked Jinmei  : 218 Linked Jinmei (with no frequency)
  //     + = Extra          : all kanji loaded from Extra file
  //     * = anything else  : currently includes 211 'no-frequency' Linked Old
  std::string qualifiedName() const {
    auto t = type();
    return _name +
      (t == Types::Jouyou           ? ' '
         : hasLevel()               ? '\''
         : _frequency               ? '"'
         : t == Types::Jinmei       ? '^'
         : t == Types::LinkedJinmei ? '~'
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
  Kanji(const Data& d, int number, const std::string& name, int strokes, bool findFrequency)
    : _number(number), _name(name), _strokes(strokes), _level(d.getLevel(name)),
      _frequency(findFrequency ? d.getFrequency(name) : 0) {}
private:
  // Not all 'Other' type Kanji have 'readings' and only 'Jouyou' and 'Extra' type kanji
  // currently have English 'meaning' - for these cases use 'EmptyString'.
  static const std::string EmptyString;
  const int _number;
  const std::string _name;
  const int _strokes;
  const Levels _level;
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

// 'ReadingKanji' is the base class for FileListKanji and is also the class used for 'Other'
// type kanji that have a reading (pulled in from other-readings.txt).
class ReadingKanji : public Kanji {
public:
  // public constructor is used for 'Other' kanjis with readings
  ReadingKanji(const Data& d, int number, const std::string& name, const std::string& reading)
    : Kanji(d, number, name), _reading(reading) {}
  const std::string& reading() const override { return _reading; }
protected:
  // protected constructor is used by derived FileListKanji class (allows controlling strokes and frequency)
  ReadingKanji(const Data& d, int number, const std::string& name, const std::string& reading, int strokes,
               bool findFrequency)
    : Kanji(d, number, name, strokes, findFrequency), _reading(reading) {}
private:
  const std::string _reading;
};

// FileListKanji is the base class for kanjis loaded from 'jouyou.txt', 'jinmei.txt' and 'extra.txt' files
// - Each file contains the same first 4 columns: 'Number', 'Name', 'Radical' and 'Reading'
// - Jouyou and Extra files contain 'Strokes' column, Jinmei strokes come from a separate 'strokes.txt' file.
class FileListKanji : public ReadingKanji {
public:
  // 'fromString' is a factory method that creates a list of kanjis of the given 'type' from the given 'file'
  // - 'type' must be Jouyou, Jinmei or Extra
  // - 'file' must have tab separated lines that have the right number of columns for the given type
  // - the first line of 'file' should have column header names that match the names in the 'Columns' enum
  static Data::List fromFile(const Data&, Types type, const std::filesystem::path& file);
  static int toInt(const std::string& s) {
    try {
      return std::stoi(s);
    } catch (...) {
      throw std::invalid_argument("failed to convert to int: " + s);
    }
  }
  const Radical& radical() const { return _radical; }
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
    : ReadingKanji(d, toInt(columns[NumberCol]), columns[NameCol], columns[ReadingCol], strokes, findFrequency),
      _radical(d.getRadical(columns[RadicalCol])) {}
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

  const Radical _radical;
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
private:
  static OptString optString(const std::string& s) { return s.empty() ? std::nullopt : std::optional(s); }
  static OptInt optInt(const std::string& s) {
    if (s.empty()) return {};
    return toInt(s);
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

// currently only Jouyou and Extra kanji include and English 'meaning'
class Meaning {
protected:
  Meaning(const std::string meaning) : _meaning(meaning) {}
  const std::string _meaning;
};

class ExtraKanji : public FileListKanji, public Meaning {
public:
  ExtraKanji(const Data& d) : FileListKanji(d, toInt(columns[StrokesCol]), false), Meaning(columns[MeaningCol]) {}
  Types type() const override { return Types::Extra; }
  const std::string& meaning() const override { return _meaning; }
};

class JouyouKanji : public OfficialListKanji, public Meaning {
public:
  JouyouKanji(const Data& d)
    : OfficialListKanji(d, toInt(columns[StrokesCol])), Meaning(columns[MeaningCol]),
      _grade(getGrade(columns[GradeCol])) {}
  Types type() const override { return Types::Jouyou; }
  const std::string& meaning() const override { return _meaning; }
  Grades grade() const override { return _grade; }
private:
  static Grades getGrade(const std::string&);
  const Grades _grade;
};

} // namespace kanji

#endif // KANJI_KANJI_H
