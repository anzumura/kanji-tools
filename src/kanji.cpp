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
const fs::path Frequency = "frequency.txt";
const fs::path N1 = "n1.txt";
const fs::path N2 = "n2.txt";
const fs::path N3 = "n3.txt";
const fs::path N4 = "n4.txt";
const fs::path N5 = "n5.txt";

} // namespace

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

KanjiList::Set KanjiList::uniqueNames;

KanjiList::KanjiList(const fs::path& p, Levels l) : name(p.stem().string()), level(l) {
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

KanjiLists::KanjiLists(int argc, char** argv)
  : data(getDataDir(argc, argv)), n5(data / N5, Levels::N5), n4(data / N4, Levels::N4), n3(data / N3, Levels::N3),
    n2(data / N2, Levels::N2), n1(data / N1, Levels::N1), frequency(data / Frequency) {
  populateJouyou();
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
  for (std::string line; std::getline(f, line);) {
    std::stringstream ss(line);
    KanjiList::List tokens;
    for (std::string token; std::getline(ss, token, '\t');)
      tokens.emplace_back(token);
    if (tokens.size() == JouyouKanji::MaxIndex) {
      try {
        auto k = std::make_unique<JouyouKanji>(tokens, *this);
        _jouyouSet.insert(k->name);
        _jouyou.emplace_back(std::move(k));
      } catch (const std::exception& e) {
        std::cerr << "got exception: " << e.what() << " while processing line: " << line << '\n';
      }
    } else
      std::cerr << "got " << tokens.size() << " tokens (wanted " << JouyouKanji::MaxIndex << ") line: " << line << '\n';
  }
}

void KanjiLists::processList(const KanjiList& l) {
  KanjiList::List jlptNonJouyou;
  for (const auto& i : l.list()) {
    if (_jouyouSet.find(i) == _jouyouSet.end()) {
      auto k = _nonJouyouSet.insert(i);
      if (k.second) {
        _nonJouyou.emplace_back(std::move(std::make_unique<Kanji>(i, *this, l.level)));
        if (l.level != Levels::None) jlptNonJouyou.emplace_back(i);
      }
    }
  }
  if (!jlptNonJouyou.empty()) {
    std::cout << ">>> found " << jlptNonJouyou.size() << " non-Jouyou in JLPT " << l.level << ':';
    for (const auto& i : jlptNonJouyou)
      std::cout << ' ' << i;
    std::cout << '\n';
  }
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
