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
// Types represents the type of Kanji:
// - Jouyou: 2136 official Jouyou kanji
// - Jinmei: 633 official Jinmei kanji
// - LinkedJinmei: 230 more Jinmei kanji that are old/alternative forms of Jouyou (212) or Jinmei (18)
// - LinkedOld: old/variant Jouyou kanji that aren't in 'LinkedJinmei'
// - Other: kanji that are in the top 2501 frequency list, but not one of the first 4 types
// - Extra: kanji loaded from 'extra.txt' - shouldn't be any of the above types
// - None: used as a type for a kanji that hasn't been loaded
enum class Types { Jouyou, Jinmei, LinkedJinmei, LinkedOld, Other, Extra, None };

const char* toString(Grades);
const char* toString(Levels);
const char* toString(Types);
inline std::ostream& operator<<(std::ostream& os, const Grades& x) { return os << toString(x); }
inline std::ostream& operator<<(std::ostream& os, const Levels& x) { return os << toString(x); }
inline std::ostream& operator<<(std::ostream& os, const Types& x) { return os << toString(x); }

// helper functions to get the string length in encoded charaters instead of bytes
inline size_t length(const char* s) {
  size_t len = 0;
  while (*s) len += (*s++ & 0xc0) != 0x80;
  return len;
}
inline size_t length(const std::string& s) { return length(s.c_str()); }

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
  static void print(const List&, const std::string& type, const std::string& group, bool isError = false);
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
  using Map = std::map<std::string, Entry>;
  const List& jouyouKanji() const { return _jouyouKanji; }
  const List& jinmeiKanji() const { return _jinmeiKanji; }
  const List& linkedJinmeiKanji() const { return _linkedJinmeiKanji; }
  const List& linkedOldKanji() const { return _linkedOldKanji; }
  const List& otherKanji() const { return _otherKanji; }
  const List& extraKanji() const { return _extraKanji; }

  int getFrequency(const std::string& name) const { return frequency.get(name); }
  Levels getLevel(const std::string&) const;
  int getStrokes(const std::string& name) const {
    auto i = _strokes.find(name);
    return i == _strokes.end() ? 0 : i->second;
  }
  Types getType(const std::string& name) const {
    if (_jouyouMap.find(name) != _jouyouMap.end()) return Types::Jouyou;
    if (_jinmeiMap.find(name) != _jinmeiMap.end()) return Types::Jinmei;
    if (_linkedJinmeiMap.find(name) != _linkedJinmeiMap.end()) return Types::LinkedJinmei;
    if (_linkedOldMap.find(name) != _linkedOldMap.end()) return Types::LinkedOld;
    if (_otherMap.find(name) != _otherMap.end()) return Types::Other;
    if (_extraMap.find(name) != _extraMap.end()) return Types::Extra;
    return Types::None;
  }
  bool isOldJouyou(const std::string& name) const { return _jouyouOldSet.find(name) != _jouyouOldSet.end(); }
  bool isOldJinmei(const std::string& name) const { return _jinmeiOldSet.find(name) != _jinmeiOldSet.end(); }
  bool isOldName(const std::string& name) const { return isOldJouyou(name) || isOldJinmei(name); }
private:
  static std::filesystem::path getDataDir(int, char**);
  static void checkInsert(Map&, const Entry&);
  static void checkInsert(KanjiList::Set&, const std::string&);
  static void checkNotFound(const Map&, const Entry&);
  static void checkNotFound(const KanjiList::Set&, const std::string&);
  void loadStrokes();
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const KanjiList&);
  void checkStrokes() const;

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
  List _linkedJinmeiKanji;
  List _linkedOldKanji;
  List _otherKanji;
  List _extraKanji;
  // allow lookup by name
  Map _jouyouMap;
  Map _jinmeiMap;
  Map _linkedJinmeiMap;
  Map _linkedOldMap;
  Map _otherMap;
  Map _extraMap;
  // sets to help during loading (detecting duplicates, print diagnostics, etc.)
  KanjiList::Set _jouyouOldSet;
  KanjiList::Set _jinmeiOldSet;
};

class Kanji {
public:
  using OptString = std::optional<std::string>;
  // constructor for Kanji found in frequency.txt that weren't found in one of the other files
  Kanji(const KanjiLists& k, int number, const std::string& name, Levels level = Levels::None)
    : _number(number), _name(name), _strokes(k.getStrokes(name)), _level(level), _frequency(k.getFrequency(name)) {}
  virtual ~Kanji() = default;

  virtual Types type() const { return Types::Other; }
  virtual Grades grade() const { return Grades::None; }
  virtual OptString oldName() const { return {}; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }
  int strokes() const { return _strokes; } // may be zero for kanjis only loaded from frequency.txt
  Levels level() const { return _level; }
  int frequency() const { return _frequency; }

  // helper functions for getting information on 'oldValue' (旧字体) kanjis
  int oldFrequency(const KanjiLists& k) const {
    auto i = oldName();
    if (i.has_value()) return k.getFrequency(*i);
    return 0;
  }
  Levels oldLevel(const KanjiLists& k) const {
    auto i = oldName();
    if (i.has_value()) return k.getLevel(*i);
    return Levels::None;
  }
  int oldStrokes(const KanjiLists& k) const {
    auto i = oldName();
    if (i.has_value()) return k.getStrokes(*i);
    return 0;
  }
  Types oldType(const KanjiLists& k) const {
    auto i = oldName();
    if (i.has_value()) return k.getType(*i);
    return Types::None;
  }
protected:
  // helper constructor for derived classes (can avoid looking up frequency for 'extra' kanji)
  Kanji(const KanjiLists& k, int number, const std::string& name, int strokes, bool findFrequency)
    : _number(number), _name(name), _strokes(strokes), _level(k.getLevel(name)),
      _frequency(findFrequency ? k.getFrequency(name) : 0) {}
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
  LinkedKanji(const KanjiLists& k, int number, const std::string& name, const KanjiLists::Entry& kanji)
    : Kanji(k, number, name), _kanji(kanji) {}

  const KanjiLists::Entry& kanji() const { return _kanji; }
private:
  const KanjiLists::Entry _kanji;
};

class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(const KanjiLists& k, int number, const std::string& name, const KanjiLists::Entry& kanji)
    : LinkedKanji(k, number, name, kanji) {}

  Types type() const override { return Types::LinkedJinmei; }
};

class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(const KanjiLists& k, int number, const std::string& name, const KanjiLists::Entry& kanji)
    : LinkedKanji(k, number, name, kanji) {}

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
  static KanjiLists::List fromFile(const KanjiLists&, Types type, const std::filesystem::path& file);

  const std::string& radical() const { return _radical; }
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

  FileListKanji(const KanjiLists& k, int strokes, bool findFrequency = true)
    : Kanji(k, toInt(columns[Number]), columns[Name], strokes, findFrequency), _radical(columns[Radical]) {}
private:
  static std::map<std::string, int> ColumnMap; // maps column names to Column enum values
  const std::string _radical;
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
