#include <kanji/Kanji.h>

#include <fstream>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

namespace {

void usage(const std::string& msg) {
  std::cerr << msg << '\n';
  exit(1);
}

const fs::path Jouyou = "jouyou.txt";
const fs::path Jinmei = "jinmei.txt";
const fs::path Frequency = "frequency.txt";
const fs::path N1 = "n1.txt";
const fs::path N2 = "n2.txt";
const fs::path N3 = "n3.txt";
const fs::path N4 = "n4.txt";
const fs::path N5 = "n5.txt";

} // namespace

const char* toString(Grades g) {
  switch (g) {
  case Grades::S:
    return "S";
  case Grades::G6:
    return "G6";
  case Grades::G5:
    return "G5";
  case Grades::G4:
    return "G4";
  case Grades::G3:
    return "G3";
  case Grades::G2:
    return "G2";
  case Grades::G1:
    return "G1";
  default:
    return "None";
  }
}

const char* toString(Levels l) {
  switch (l) {
  case Levels::N1:
    return "N1";
  case Levels::N2:
    return "N2";
  case Levels::N3:
    return "N3";
  case Levels::N4:
    return "N4";
  case Levels::N5:
    return "N5";
  default:
    return "None";
  }
}

KanjiList::Set KanjiList::uniqueNames;

KanjiList::KanjiList(const fs::path& p, Levels l)
  : name(l == Levels::None ? std::string("Top Frequency") : std::string("JLPT ") + toString(l)), level(l) {
  if (!fs::is_regular_file(p)) usage("can't open " + p.string());
  int count = 0;
  std::ifstream f(p);
  std::string line;
  KanjiList::List good;
  KanjiList::List dups;
  while (std::getline(f, line)) {
    assert(_map.find(line) == _map.end());
    if (l != Levels::None) {
      auto i = uniqueNames.insert(line);
      if (!i.second) {
        dups.emplace_back(*i.first);
        continue;
      }
      good.emplace_back(*i.first);
    }
    _list.emplace_back(line);
    // map value count starts at 1, i.e., the first kanji has 'frequency 1' (not 0)
    _map[line] = ++count;
  }
  if (!dups.empty()) {
    std::cerr << ">>> found " << dups.size() << " duplicates in JLPT list " << name << ":";
    for (const auto& i : dups)
      std::cerr << ' ' << i;
    fs::path newFile(p);
    newFile.replace_extension(fs::path("new"));
    std::cerr << "\n>>> saving " << good.size() << " unique entries to: " << newFile.string() << '\n';
    std::ofstream of(newFile);
    for (const auto& i : good)
      of << i << '\n';
  }
}

void KanjiList::print(const List& l, const std::string& type, const std::string& group) {
  if (!l.empty()) {
    std::cout << ">>> Found " << l.size() << ' ' << type << " in " << group << ':';
    for (const auto& i : l)
      std::cout << ' ' << i;
    std::cout << '\n';
  }
}

KanjiLists::KanjiLists(int argc, char** argv)
  : data(getDataDir(argc, argv)), n5(data / N5, Levels::N5), n4(data / N4, Levels::N4), n3(data / N3, Levels::N3),
    n2(data / N2, Levels::N2), n1(data / N1, Levels::N1), frequency(data / Frequency) {
  populateJouyou();
  populateJinmei();
  processList(n5);
  processList(n4);
  processList(n3);
  processList(n2);
  processList(n1);
  processList(frequency);
}

void KanjiLists::populateJouyou() {
  fs::path p(data / Jouyou);
  if (!fs::is_regular_file(p)) usage(data.string() + " must contain " + p.string());
  std::set<std::string> found;
  std::ifstream f(p);
  int number = 1;
  for (std::string line; std::getline(f, line); ++number) {
    std::stringstream ss(line);
    KanjiList::List tokens;
    for (std::string token; std::getline(ss, token, '\t');)
      tokens.emplace_back(token);
    if (tokens.size() == JouyouKanji::MaxIndex) {
      try {
        auto k = std::make_shared<JouyouKanji>(*this, tokens);
        _jouyouSet.insert(k->name);
        _jouyouKanji.push_back(k);
      } catch (const std::exception& e) {
        std::cerr << "got exception: " << e.what() << " while processing line: " << line << '\n';
      }
    } else
      std::cerr << "got " << tokens.size() << " tokens (wanted " << JouyouKanji::MaxIndex << ") line: " << line << '\n';
  }
}

void KanjiLists::populateJinmei() {
  fs::path p(data / Jinmei);
  if (!fs::is_regular_file(p)) usage(data.string() + " must contain " + p.string());
  std::set<std::string> found;
  std::ifstream f(p);
  int count = _jouyouKanji.size();
  for (std::string line; std::getline(f, line); ++count) {
    std::stringstream ss(line);
    KanjiList::List tokens;
    for (std::string token; std::getline(ss, token, '\t');)
      tokens.emplace_back(token);
    if (tokens.size() && tokens.size() < 3) {
      try {
        auto k = std::make_shared<Kanji>(*this, ++count, tokens);
        // Jinmei kanji should not be part of Jouyou set
        assert(_jouyouSet.find(k->name) == _jouyouSet.end());
        _jinmeiSet.insert(k->name);
        _jinmeiKanji.push_back(k);
      } catch (const std::exception& e) {
        std::cerr << "got exception: " << e.what() << " while processing line: " << line << '\n';
      }
    } else
      std::cerr << "got " << tokens.size() << " tokens (wanted 1 or 2) line: " << line << '\n';
  }
}

void KanjiLists::processList(const KanjiList& l) {
  KanjiList::List other;
  KanjiList::List jinmei;
  auto count = _jouyouKanji.size() + _jinmeiKanji.size() + _otherSet.size();
  for (const auto& i : l.list()) {
    if (_jouyouSet.find(i) == _jouyouSet.end()) {
      if (_jinmeiSet.find(i) == _jinmeiSet.end()) {
        auto k = _otherSet.insert(i);
        if (k.second) {
          _otherKanji.push_back(std::make_shared<Kanji>(*this, ++count, i, l.level));
          other.emplace_back(i);
        }
      } else
        jinmei.emplace_back(i);
    }
  }
  KanjiList::print(other, std::string("non-Jouyou/non-Jinmei") + (l.level == Levels::None ? "/non-JLPT" : ""), l.name);
  if (l.level == Levels::None) {
    KanjiList::List jlptJinmei, otherJinmei;
    for (const auto& i : jinmei)
      if (getLevel(i) != Levels::None)
        jlptJinmei.emplace_back(i);
      else
        otherJinmei.emplace_back(i);
    KanjiList::print(jlptJinmei, "JLPT Jinmei", l.name);
    KanjiList::print(otherJinmei, "non-JLPT Jinmei", l.name);
  } else
    KanjiList::print(jinmei, "Jinmei", l.name);
}

fs::path KanjiLists::getDataDir(int argc, char** argv) {
  if (argc < 2) usage("please specify data directory");
  fs::path f(argv[1]);
  if (!fs::is_directory(f)) usage(f.string() + " is not a valid directory");
  return f;
}

Levels KanjiLists::getLevel(const std::string& k) const {
  if (n1.exists(k)) return Levels::N1;
  if (n2.exists(k)) return Levels::N2;
  if (n3.exists(k)) return Levels::N3;
  if (n4.exists(k)) return Levels::N4;
  if (n5.exists(k)) return Levels::N5;
  return Levels::None;
}

Grades JouyouKanji::getGrade(const std::string& g) {
  if (g == "S") return Grades::S;
  if (g == "6") return Grades::G6;
  if (g == "5") return Grades::G5;
  if (g == "4") return Grades::G4;
  if (g == "3") return Grades::G3;
  if (g == "2") return Grades::G2;
  if (g == "1") return Grades::G1;
  return Grades::None;
}

int JouyouKanji::toInt(const KanjiList::List& l, int i) {
  const std::string& s = l[i];
  try {
    return std::stoi(s);
  } catch (...) {
    throw std::invalid_argument("failed to pasrse token " + std::to_string(i) + " - string was: " + s);
  }
}

} // namespace kanji
