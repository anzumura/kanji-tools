#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

void usage(const std::string& msg) {
  std::cerr << msg << '\n';
  exit(1);
}

using List = std::vector<std::string>;
using Map = std::map<std::string, int>;

namespace fs = std::filesystem;

class KanjiList {
public:
  KanjiList(const fs::path& p) {
    if (!fs::is_regular_file(p)) usage("can't open " + p.string());
    int count = 0;
    std::ifstream f(p);
    std::string line;
    while (std::getline(f, line)) {
      assert(_map.find(line) == _map.end());
      _list.emplace_back(line);
      // map value count starts at 1, i.e., the first kanji has 'frequency 1' (not 0)
      _map[line] = ++count;
    }
  }
  bool exists(const std::string& s) const { return _map.find(s) != _map.end(); }
  // return 0 for 'not found'
  int get(const std::string& s) const {
    auto i = _map.find(s);
    return i != _map.end() ? i->second : 0;
  }

private:
  List _list;
  Map _map;
};

enum class Grades { G1, G2, G3, G4, G5, G6, S, None }; // S=secondary school, None=not jouyou
enum class Levels { N5, N4, N3, N2, N1, None };

std::ostream& operator<<(std::ostream& os, const Grades& g) {
  switch (g) {
  case Grades::S:
    return os << "S";
  case Grades::G6:
    return os << "G6";
  case Grades::G5:
    return os << "G5";
  case Grades::G4:
    return os << "G4";
  case Grades::G3:
    return os << "G3";
  case Grades::G2:
    return os << "G2";
  case Grades::G1:
    return os << "G1";
  default:
    return os << "None";
  }
}

std::ostream& operator<<(std::ostream& os, const Levels& l) {
  switch (l) {
  case Levels::N1:
    return os << "N1";
  case Levels::N2:
    return os << "N2";
  case Levels::N3:
    return os << "N3";
  case Levels::N4:
    return os << "N4";
  case Levels::N5:
    return os << "N5";
  default:
    return os << "None";
  }
}

const fs::path Jouyou = "jouyou.txt";
const fs::path Frequency = "frequency.txt";
const fs::path N1 = "n1.txt";
const fs::path N2 = "n2.txt";
const fs::path N3 = "n3.txt";
const fs::path N4 = "n4.txt";
const fs::path N5 = "n5.txt";

class KanjiLists {
public:
  KanjiLists(const fs::path& p)
    : frequency(p / Frequency), n1(p / N1), n2(p / N2), n3(p / N3), n4(p / N4), n5(p / N5) {}

  int getFrequency(const std::string& k) const { return frequency.get(k); }
  Levels getLevel(const std::string& k) const {
    if (n1.exists(k)) return Levels::N1;
    if (n2.exists(k)) return Levels::N2;
    if (n3.exists(k)) return Levels::N3;
    if (n4.exists(k)) return Levels::N4;
    if (n5.exists(k)) return Levels::N5;
    return Levels::None;
  }

  const KanjiList frequency;
  const KanjiList n1;
  const KanjiList n2;
  const KanjiList n3;
  const KanjiList n4;
  const KanjiList n5;
};

class Kanji {
public:
  static Grades getGrade(const std::string& g) {
    if (g == "S") return Grades::S;
    if (g == "6") return Grades::G6;
    if (g == "5") return Grades::G5;
    if (g == "4") return Grades::G4;
    if (g == "3") return Grades::G3;
    if (g == "2") return Grades::G2;
    if (g == "1") return Grades::G1;
    return Grades::None;
  }
  static int toInt(const List& l, int i) {
    const std::string& s = l[i];
    try {
      return std::stoi(s);
    } catch (...) {
      throw std::invalid_argument("failed to pasrse token " + std::to_string(i) + " - string was: " + s);
    }
  }
  enum Values { Number = 0, Name, OldName, Radical, Strokes, Grade, Year, Meaning, Reading, MaxIndex };
  Kanji(const List& l, const KanjiLists& k)
    : number(toInt(l, Number)), kanji(l[Name]), oldKanji(l[OldName].empty() ? std::nullopt : std::optional(l[OldName])),
      radical(l[Radical]), strokes(toInt(l, Strokes)), grade(getGrade(l[Grade])),
      year(l[Year].empty() ? std::nullopt : std::optional(toInt(l, Year))), level(k.getLevel(kanji)),
      frequency(k.getFrequency(kanji)), meaning(l[Meaning]), readings(l[Reading]) {}

  const int number;
  const std::string kanji;
  const std::optional<std::string> oldKanji;
  const std::string radical;
  const int strokes;
  const Grades grade;
  const std::optional<int> year;
  const std::string meaning;
  const std::string readings;
  const Levels level;
  const int frequency;
};

int main(int argc, char** argv) {
  if (argc < 2) usage("please specify data directory");
  fs::path data(argv[1]);
  if (!fs::is_directory(data)) usage(data.string() + " is not a valid directory");
  KanjiLists l(data);
  // populate Jouyou Kanji
  fs::path p(data / Jouyou);
  if (!fs::is_regular_file(p)) usage(data.string() + " must contain " + p.string());
  std::vector<Kanji> kanji;
  std::ifstream f(p);
  for (std::string line; std::getline(f, line);) {
    std::stringstream ss(line);
    List tokens;
    for (std::string token; std::getline(ss, token, '\t');)
      tokens.emplace_back(token);
    if (tokens.size() == Kanji::MaxIndex) {
      try {
        kanji.emplace_back(tokens, l);
      } catch (const std::exception& e) {
        std::cerr << "got exception: " << e.what() << " while processing line: " << line << '\n';
      }
    } else
      std::cerr << "got " << tokens.size() << " tokens (expected " << Kanji::MaxIndex << ") line: " << line << '\n';
  }
  std::cout << "loaded " << kanji.size() << " jouyou kanji\n";
  for (auto i : kanji)
    std::cout << i.kanji << ": " << i.grade << ", " << i.level << ", " << i.frequency << '\n'; 
  return 0;
}
