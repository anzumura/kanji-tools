#ifndef KANJI_KANJI_H
#define KANJI_KANJI_H

#include <kanji/Data.h>

namespace kanji {

class Kanji {
public:
  using OptString = std::optional<std::string>;
  // constructor for Kanji found in frequency.txt that weren't found in one of the other files
  Kanji(const Data& d, int number, const std::string& name, Levels level = Levels::None)
    : _number(number), _name(name), _strokes(d.getStrokes(name)), _level(level), _frequency(d.getFrequency(name)) {}
  virtual ~Kanji() = default;

  virtual Types type() const { return Types::Other; }
  virtual Grades grade() const { return Grades::None; }
  virtual OptString oldName() const { return {}; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }
  int strokes() const { return _strokes; } // may be zero for kanjis only loaded from frequency.txt
  Levels level() const { return _level; }
  bool hasLevel() const { return _level != Levels::None; }
  int frequency() const { return _frequency; }
  // 'qualifiedName' returns 'name' plus an extra marker to indicate additional useful information:
  //   space = Jouyou (use space since this is the most common type)
  //       ' = JLPT
  //       " = Top Frequency
  //       j = Jinmei (not already covered by the above types)
  //       e = Extra
  //       * = anything else
  std::string qualifiedName() const {
    auto t = type();
    return _name +
      (t == Types::Jouyou     ? ' '
         : hasLevel()         ? '\''
         : _frequency         ? '"'
         : t == Types::Jinmei ? 'j'
         : t == Types::Extra  ? 'e'
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
  const int _number;
  const std::string _name;
  const int _strokes;
  const Levels _level;
  const int _frequency;
};

inline std::ostream& operator<<(std::ostream& os, const Kanji& k) { return os << k.name(); }

class LinkedKanji : public Kanji {
protected:
  LinkedKanji(const Data& d, int number, const std::string& name, const Data::Entry& kanji)
    : Kanji(d, number, name), _kanji(kanji) {}

  const Data::Entry& kanji() const { return _kanji; }
private:
  const Data::Entry _kanji;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(const Data& d, int number, const std::string& name, const Data::Entry& kanji)
    : LinkedKanji(d, number, name, kanji) {}

  Types type() const override { return Types::LinkedJinmei; }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(const Data& d, int number, const std::string& name, const Data::Entry& kanji)
    : LinkedKanji(d, number, name, kanji) {}

  Types type() const override { return Types::LinkedOld; }
};

// FileListKanji is the base class for kanjis loaded from 'jouyou.txt', 'jinmei.txt' and 'extra.txt' files
// - Each file contains the same first 3 columns: 'Number', 'Name', 'Radical'
// - Jouyou and Extra files contain 'Strokes' column, Jinmei strokes come from a separate 'strokes.txt' file.
class FileListKanji : public Kanji {
public:
  // 'fromString' is a factory method that creates a list of kanjis of the given 'type' from the given 'file'
  // - 'type' must be Jouyou, Jinmei or Extra
  // - 'file' must have tab separated lines that have the right number of columns for the given type
  // - the first line of 'file' should have column header names that match the names in the 'Columns' enum
  static Data::List fromFile(const Data&, Types type, const std::filesystem::path& file);

  const Radical& radical() const { return _radical; }
  static int toInt(const std::string& s) {
    try {
      return std::stoi(s);
    } catch (...) {
      throw std::invalid_argument("failed to convert to int: " + s);
    }
  }
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
    : Kanji(d, toInt(columns[NumberCol]), columns[NameCol], strokes, findFrequency),
      _radical(d.getRadical(columns[RadicalCol])) {}
private:
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

class MeaningAndReading {
public:
  const std::string& meaning() const { return _meaning; }
  const std::string& reading() const { return _reading; }
protected:
  MeaningAndReading(const std::string meaning, const std::string& reading) : _meaning(meaning), _reading(reading) {}
private:
  const std::string _meaning;
  const std::string _reading;
};

class ExtraKanji : public FileListKanji, public MeaningAndReading {
public:
  ExtraKanji(const Data& d)
    : FileListKanji(d, toInt(columns[StrokesCol]), false), MeaningAndReading(columns[MeaningCol], columns[ReadingCol]) {
  }

  Types type() const override { return Types::Extra; }
};

class JouyouKanji : public OfficialListKanji, public MeaningAndReading {
public:
  JouyouKanji(const Data& d)
    : OfficialListKanji(d, toInt(columns[StrokesCol])), MeaningAndReading(columns[MeaningCol], columns[ReadingCol]),
      _grade(getGrade(columns[GradeCol])) {}

  Types type() const override { return Types::Jouyou; }
  Grades grade() const override { return _grade; }
private:
  static Grades getGrade(const std::string&);

  const Grades _grade;
};

} // namespace kanji

#endif // KANJI_KANJI_H
