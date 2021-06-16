#ifndef KANJI_KANJI_H
#define KANJI_KANJI_H

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
  int get(const std::string& s) const {
    auto i = _map.find(s);
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

  int getFrequency(const std::string& k) const { return frequency.get(k); }
  Levels getLevel(const std::string&) const;

private:
  static std::filesystem::path getDataDir(int, char**);
  static void checkInsert(KanjiList::Set&, const std::string&);
  static void checkNotFound(const KanjiList::Set&, const std::string&);
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const KanjiList&);

  const std::filesystem::path data;
  const KanjiList n5;
  const KanjiList n4;
  const KanjiList n3;
  const KanjiList n2;
  const KanjiList n1;
  const KanjiList frequency;

  List _jouyouKanji;
  List _jinmeiKanji;
  List _extraKanji;
  List _otherKanji;
  KanjiList::Set _jouyouSet;
  KanjiList::Set _jinmeiSet;
  KanjiList::Set _extraSet;
  KanjiList::Set _otherSet;
};

class Kanji {
public:
  using OptString = std::optional<std::string>;
  // constructor for Kanji found in frequency.txt that weren't found in one of the other 3 type files
  Kanji(const KanjiLists& k, int n, const std::string& s, Levels l)
    : _number(n), _name(s), _level(l), _frequency(k.getFrequency(s)) {}
  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;
  Kanji& operator=(const Kanji&) = delete;

  virtual Types type() const { return Types::Other; }
  virtual Grades grade() const { return Grades::None; }
  virtual OptString oldName() const { return {}; }

  int number() const { return _number; }
  const std::string& name() const { return _name; }
  Levels level() const { return _level; }
  int frequency() const { return _frequency; }

protected:
  Kanji(const KanjiLists& k, int n, const std::string& s)
    : _number(n), _name(s), _level(k.getLevel(s)), _frequency(k.getFrequency(s)) {}

private:
  const int _number;
  const std::string _name;
  const Levels _level;
  const int _frequency;
};

class JinmeiKanji : public Kanji {
public:
  enum Columns { Name, OldName, MaxIndex };
  JinmeiKanji(const KanjiLists& k, int n, const KanjiList::List& l)
    : Kanji(k, n, l[Name]), _oldName(optString(l, OldName)) {}

  Types type() const override { return Types::Jinmei; }
  OptString oldName() const override { return _oldName; }

protected:
  static OptString optString(const KanjiList::List& l, int i) {
    return l.size() <= i || l[i].empty() ? std::nullopt : std::optional(l[i]);
  }
  JinmeiKanji(const KanjiLists& k, int n, const std::string& s, OptString o = {}) : Kanji(k, n, s), _oldName(o) {}

private:
  const OptString _oldName;
};

class ExtraKanji : public JinmeiKanji {
public:
  enum Columns { Name, Radical, Strokes, Meaning, Reading, MaxIndex };
  ExtraKanji(const KanjiLists& k, int n, const KanjiList::List& l)
    : JinmeiKanji(k, n, l[Name]), _radical(l[Radical]), _strokes(toInt(l, Strokes)), _meaning(l[Meaning]),
      _reading(l[Reading]) {}

  Types type() const override { return Types::Extra; }

  const std::string& radical() const { return _radical; }
  int strokes() const { return _strokes; }
  const std::string& meaning() const { return _meaning; }
  const std::string& reading() const { return _reading; }

protected:
  static int toInt(const KanjiList::List&, int);
  ExtraKanji(const KanjiLists& k, int n, const std::string& s, OptString o, const std::string& r, int st,
             const std::string& m, const std::string& re)
    : JinmeiKanji(k, n, s, o), _radical(r), _strokes(st), _meaning(m), _reading(re) {}

private:
  const std::string _radical;
  const int _strokes;
  const std::string _meaning;
  const std::string _reading;
};

class JouyouKanji : public ExtraKanji {
public:
  using OptInt = std::optional<int>;
  enum Columns { Number, Name, OldName, Radical, Strokes, Grade, Year, Meaning, Reading, MaxIndex };
  JouyouKanji(const KanjiLists& k, const KanjiList::List& l)
    : ExtraKanji(k, toInt(l, Number), l[Name], optString(l, OldName), l[Radical], toInt(l, Strokes), l[Meaning],
                 l[Reading]),
      _grade(getGrade(l[Grade])), _year(l[Year].empty() ? std::nullopt : std::optional(toInt(l, Year))) {}

  Types type() const override { return Types::Jouyou; }
  Grades grade() const override { return _grade; }
  OptInt year() const { return _year; }

private:
  static Grades getGrade(const std::string&);

  const Grades _grade;
  const OptInt _year;
};

} // namespace kanji

#endif // KANJI_KANJI_H
