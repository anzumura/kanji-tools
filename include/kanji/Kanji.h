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
// Type 'Extra' is for kanjis loaded from 'extra.txt' file whereas 'Other' is for any other
// kanji found in the 'frequency.txt' file that isn't one of the first 3 types.
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
  // constructor for Kanji found in frequency.txt that weren't found in one of the other 3 type files
  Kanji(const KanjiLists& k, int n, const std::string& s, Levels l)
    : number(n), name(s), grade(Grades::None), level(l), frequency(k.getFrequency(s)) {}
  virtual ~Kanji() = default;
  Kanji(const Kanji&) = delete;
  Kanji& operator=(const Kanji&) = delete;

  virtual Types type() const { return Types::Other; }

  const int number;
  const std::string name;
  const Grades grade;
  const Levels level;
  const int frequency;

protected:
  Kanji(const KanjiLists& k, int n, const std::string& s, Grades g)
    : number(n), name(s), grade(g), level(k.getLevel(s)), frequency(k.getFrequency(s)) {}
};

class JinmeiKanji : public Kanji {
public:
  using OptString = std::optional<std::string>;
  enum Columns { Name, OldName, MaxIndex };
  JinmeiKanji(const KanjiLists& k, int n, const KanjiList::List& l)
    : Kanji(k, n, l[Name], Grades::None), oldName(optString(l, OldName)) {}

  Types type() const override { return Types::Jinmei; }

  const OptString oldName;

protected:
  JinmeiKanji(const KanjiLists& k, int n, const std::string& s, OptString o = {}, Grades g = Grades::None)
    : Kanji(k, n, s, g), oldName(o) {}
  static OptString optString(const KanjiList::List& l, int i) {
    return l.size() <= i || l[i].empty() ? std::nullopt : std::optional(l[i]);
  }
};

class ExtraKanji : public JinmeiKanji {
public:
  enum Columns { Name, Radical, Strokes, Meaning, Reading, MaxIndex };
  ExtraKanji(const KanjiLists& k, int n, const KanjiList::List& l)
    : JinmeiKanji(k, n, l[Name]), radical(l[Radical]), strokes(toInt(l, Strokes)), meaning(l[Meaning]),
      reading(l[Reading]) {}

  Types type() const override { return Types::Extra; }

  const std::string radical;
  const int strokes;
  const std::string meaning;
  const std::string reading;

protected:
  ExtraKanji(const KanjiLists& k, int n, const std::string& s, OptString o, Grades g, const std::string& r, int st,
             const std::string& m, const std::string& re)
    : JinmeiKanji(k, n, s, o, g), radical(r), strokes(st), meaning(m), reading(re) {}
  static int toInt(const KanjiList::List&, int);
};

class JouyouKanji : public ExtraKanji {
public:
  using OptInt = std::optional<int>;
  enum Columns { Number, Name, OldName, Radical, Strokes, Grade, Year, Meaning, Reading, MaxIndex };
  JouyouKanji(const KanjiLists& k, const KanjiList::List& l)
    : ExtraKanji(k, toInt(l, Number), l[Name], optString(l, OldName), getGrade(l[Grade]), l[Radical], toInt(l, Strokes),
                 l[Meaning], l[Reading]),
      year(l[Year].empty() ? std::nullopt : std::optional(toInt(l, Year))) {}

  Types type() const override { return Types::Jouyou; }

  const OptInt year;

private:
  static Grades getGrade(const std::string&);
};

} // namespace kanji

#endif // KANJI_KANJI_H
