#ifndef KANJI_KANJI_H
#define KANJI_KANJI_H

#include <array>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace kanji {

enum class Grades { G1, G2, G3, G4, G5, G6, S, None }; // S=secondary school, None=not jouyou
enum class Levels { N5, N4, N3, N2, N1, None };
// Type 'Extra' is for kanjis loaded from 'extra.txt' file whereas 'Other' is for other
// kanjis in 'frequency.txt' file that isn't one of the first 3 types. The extra.txt file
// should only contain kanji that are not in jouyou.txt, jinmei.txt or frequency.txt.
enum class Types { Jouyou, Jinmei, Extra, Other };

const char* toString(Grades);
const char* toString(Levels);
const char* toString(Types);
inline std::ostream& operator<<(std::ostream& os, const Grades& x) { return os << toString(x); }
inline std::ostream& operator<<(std::ostream& os, const Levels& x) { return os << toString(x); }
inline std::ostream& operator<<(std::ostream& os, const Types& x) { return os << toString(x); }

class KanjiList {
public:
  using List = std::vector<std::string>;
  using Map = std::map<std::string, int>;
  using Set = std::set<std::string>;

  KanjiList(const std::filesystem::path&, Levels = Levels::None);
  bool exists(const std::string& s) const { return _map.find(s) != _map.end(); }
  // return 0 for 'not found'
  int get(const std::string& name) const {
    auto i = _map.find(name);
    return i != _map.end() ? i->second : 0;
  }
  const List& list() const { return _list; }
  const std::string name;
  const Levels level;
  static void print(const List&, const std::string& type, const std::string& group);
private:
  static Set uniqueNames; // populated and used by lists that specify a non-None level
  List _list;
  Map _map;
};

class KanjiLists {
public:
  KanjiLists(int argc, char** argv);
  using Entry = std::shared_ptr<class Kanji>;
  using List = std::vector<Entry>;
  const List& jouyouKanji() const { return _jouyouKanji; }
  const List& jinmeiKanji() const { return _jinmeiKanji; }
  const List& extraKanji() const { return _extraKanji; }
  const List& otherKanji() const { return _otherKanji; }

  int getFrequency(const std::string& name) const { return frequency.get(name); }
  Levels getLevel(const std::string&) const;
  int getStrokes(const std::string& name) const {
    auto i = _strokes.find(name);
    return i == _strokes.end() ? 0 : i->second;
  }
private:
  static std::filesystem::path getDataDir(int, char**);
  static void checkInsert(KanjiList::Set&, const std::string&);
  static void checkNotFound(const KanjiList::Set&, const std::string&);
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const KanjiList&);

  const std::filesystem::path data;
  // 'n1-n5' and 'frequency' lists are loaded from simple files with one kanji per line
  const KanjiList n5;
  const KanjiList n4;
  const KanjiList n3;
  const KanjiList n2;
  const KanjiList n1;
  const KanjiList frequency;
  // '_strokes' is populated from strokes.txt and is meant to supplement jinmei kanji (file doesn't
  // have a 'Strokes' column) as well as old kanjis from both jouyou and jinmei files. This file
  // contains stroke counts followed by one or more lines each with a single kanji that has the given
  // number of strokes.
  std::map<std::string, int> _strokes;
  // 4 lists of kanjis (lists correspond to 'Types' enum)
  List _jouyouKanji;
  List _jinmeiKanji;
  List _extraKanji;
  List _otherKanji;
  // sets to help during loading (detecting duplicates, print diagnostics, etc.)
  KanjiList::Set _jouyouSet;
  KanjiList::Set _jinmeiSet;
  KanjiList::Set _extraSet;
  KanjiList::Set _otherSet;
};

class Kanji {
public:
  using OptString = std::optional<std::string>;
  // constructor for Kanji found in frequency.txt that weren't found in one of the other 3 type files
  Kanji(const KanjiLists& k, int number, const std::string& name, Levels level)
    : _number(number), _name(name), _level(level), _frequency(k.getFrequency(name)) {}
  virtual ~Kanji() = default;

  virtual Types type() const { return Types::Other; }
  virtual Grades grade() const { return Grades::None; }
  virtual OptString oldName() const { return {}; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }
  Levels level() const { return _level; }
  int frequency() const { return _frequency; }
protected:
  // helper constructor for derived classes (can avoid looking up frequency for 'extra' kanji)
  Kanji(const KanjiLists& k, int number, const std::string& name, bool findFrequency)
    : _number(number), _name(name), _level(k.getLevel(name)), _frequency(findFrequency ? k.getFrequency(name) : 0) {}
private:
  const int _number;
  const std::string _name;
  const Levels _level;
  const int _frequency;
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
  static KanjiLists::List fromFile(const KanjiLists&, Types type, const std::filesystem::path& file);

  const std::string& radical() const { return _radical; }
  int strokes() const { return _strokes; }
protected:
  static int toInt(const std::string& s) {
    try {
      return std::stoi(s);
    } catch (...) {
      throw std::invalid_argument("failed to convert to int: " + s);
    }
  }
  // list of all supported columns in files
  enum Columns { Number, Name, Radical, OldName, Year, Strokes, Grade, Meaning, Reading, Reason, MaxIndex };
  // 'columns' contains list of values for each column after parsing a line (used by 'fromString' method)
  static std::array<std::string, MaxIndex> columns;

  FileListKanji(const KanjiLists& k, int s, bool findFrequency = true)
    : Kanji(k, toInt(columns[Number]), columns[Name], findFrequency), _radical(columns[Radical]), _strokes(s) {}
private:
  static std::map<std::string, int> ColumnMap; // maps column names to Column enum values
  const std::string _radical;
  const int _strokes;
};

// OfficialListKanji contains attributes shared by Jouyou and Jinmei kanji, i.e., optional 'Old' and 'Year' values
class OfficialListKanji : public FileListKanji {
public:
  using OptInt = std::optional<int>;
  OptString oldName() const override { return _oldName; }
  OptInt year() const { return _year; }
protected:
  OfficialListKanji(const KanjiLists& k, int s)
    : FileListKanji(k, s), _oldName(optString(columns[OldName])), _year(optInt(columns[Year])) {}
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
  JinmeiKanji(const KanjiLists& k)
    : OfficialListKanji(k, k.getStrokes(columns[Name])), _reason(getReason(columns[Reason])) {}

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
  ExtraKanji(const KanjiLists& k)
    : FileListKanji(k, toInt(columns[Strokes]), false), MeaningAndReading(columns[Meaning], columns[Reading]) {}

  Types type() const override { return Types::Extra; }
};

class JouyouKanji : public OfficialListKanji, public MeaningAndReading {
public:
  JouyouKanji(const KanjiLists& k)
    : OfficialListKanji(k, toInt(columns[Strokes])), MeaningAndReading(columns[Meaning], columns[Reading]),
      _grade(getGrade(columns[Grade])) {}

  Types type() const override { return Types::Jouyou; }
  Grades grade() const override { return _grade; }
private:
  static Grades getGrade(const std::string&);

  const Grades _grade;
};

} // namespace kanji

#endif // KANJI_KANJI_H
