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

std::ostream& operator<<(std::ostream&, const Grades&);
std::ostream& operator<<(std::ostream&, const Levels&);

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
  const List& otherKanji() const { return _otherKanji; }

  int getFrequency(const std::string& k) const { return frequency.get(k); }
  Levels getLevel(const std::string&) const;

private:
  static std::filesystem::path getDataDir(int, char**);
  void populateJouyou();
  void populateJinmei();
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
  List _otherKanji;
  KanjiList::Set _jouyouSet;
  KanjiList::Set _jinmeiSet;
  KanjiList::Set _otherSet;
};

class Kanji {
public:
  using OptString = std::optional<std::string>;
  // constructor for non-Jouyou JLPT Kanji
  Kanji(const KanjiLists& k, int n, const std::string& s, Levels l)
    : number(n), name(s), grade(Grades::None), level(l), frequency(k.getFrequency(s)) {}
  // constructor for Jinmei Kanji
  Kanji(const KanjiLists& k, int n, const KanjiList::List& l)
    : number(n), name(l[0]), oldName(optString(l, 1)), grade(Grades::None), level(k.getLevel(name)),
      frequency(k.getFrequency(name)) {}
  Kanji(const Kanji&) = delete;
  Kanji& operator=(const Kanji&) = delete;
  virtual ~Kanji() = default;

  virtual bool isJouyou() const { return false; }
  const int number;
  const std::string name;
  const OptString oldName;
  const Grades grade;
  const Levels level;
  const int frequency;

protected:
  Kanji(const KanjiLists& k, int n, const std::string& s, const OptString& o, Grades g)
    : number(n), name(s), oldName(o), grade(g), level(k.getLevel(s)), frequency(k.getFrequency(s)) {}
  static OptString optString(const KanjiList::List& l, int i) {
    return l.size() <= i || l[i].empty() ? std::nullopt : std::optional(l[i]);
  }
};

class JouyouKanji : public Kanji {
public:
  enum Columns { Number = 0, Name, OldName, Radical, Strokes, Grade, Year, Meaning, Reading, MaxIndex };
  JouyouKanji(const KanjiLists& k, const KanjiList::List& l)
    : Kanji(k, toInt(l, Number), l[Name], optString(l, OldName), getGrade(l[Grade])), radical(l[Radical]),
      strokes(toInt(l, Strokes)), year(l[Year].empty() ? std::nullopt : std::optional(toInt(l, Year))),
      meaning(l[Meaning]), readings(l[Reading]) {}

  bool isJouyou() const override { return true; }
  const std::string radical;
  const int strokes;
  const std::optional<int> year;
  const std::string meaning;
  const std::string readings;

private:
  static Grades getGrade(const std::string&);
  static int toInt(const KanjiList::List&, int);
};

} // namespace kanji

#endif // KANJI_KANJI_H
