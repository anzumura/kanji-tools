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

const fs::path N1 = "n1.txt";
const fs::path N2 = "n2.txt";
const fs::path N3 = "n3.txt";
const fs::path N4 = "n4.txt";
const fs::path N5 = "n5.txt";
const fs::path Frequency = "frequency.txt";
const fs::path Strokes = "strokes.txt";
const fs::path Jouyou = "jouyou.txt";
const fs::path Jinmei = "jinmei.txt";
const fs::path LinkedJinmei = "linked-jinmei.txt";
const fs::path Extra = "extra.txt";

} // namespace

const char* toString(Grades x) {
  switch (x) {
  case Grades::S: return "S";
  case Grades::G6: return "G6";
  case Grades::G5: return "G5";
  case Grades::G4: return "G4";
  case Grades::G3: return "G3";
  case Grades::G2: return "G2";
  case Grades::G1: return "G1";
  default: return "None";
  }
}

const char* toString(Levels x) {
  switch (x) {
  case Levels::N1: return "N1";
  case Levels::N2: return "N2";
  case Levels::N3: return "N3";
  case Levels::N4: return "N4";
  case Levels::N5: return "N5";
  default: return "None";
  }
}

const char* toString(Types x) {
  switch (x) {
  case Types::Jouyou: return "Jouyou";
  case Types::Jinmei: return "Jinmei";
  case Types::LinkedJinmei: return "LinkedJinmei";
  case Types::LinkedOld: return "LinkedOld";
  case Types::Other: return "Other";
  case Types::Extra: return "Extra";
  default: return "None";
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

void KanjiList::print(const List& l, const std::string& type, const std::string& group, bool isError) {
  if (!l.empty()) {
    std::cout << (isError ? "ERROR ---" : ">>>") << " Found " << l.size() << ' ' << type << " in " << group << ':';
    for (const auto& i : l)
      std::cout << ' ' << i;
    std::cout << '\n';
  }
}

KanjiLists::KanjiLists(int argc, char** argv)
  : data(getDataDir(argc, argv)), n5(data / N5, Levels::N5), n4(data / N4, Levels::N4), n3(data / N3, Levels::N3),
    n2(data / N2, Levels::N2), n1(data / N1, Levels::N1), frequency(data / Frequency) {
  loadStrokes();
  populateJouyou();
  populateJinmei();
  populateExtra();
  processList(n5);
  processList(n4);
  processList(n3);
  processList(n2);
  processList(n1);
  processList(frequency);
  checkStrokes();
}

void KanjiLists::checkInsert(Map& s, const Entry& i) {
  if (!s.insert(std::make_pair(i->name(), i)).second) std::cerr << "ERROR --- failed to insert " << *i << " into map\n";
}

void KanjiLists::checkInsert(KanjiList::Set& s, const std::string& n) {
  if (!s.insert(n).second) std::cerr << "ERROR --- failed to insert " << n << " into set\n";
}

void KanjiLists::checkNotFound(const Map& s, const Entry& i) {
  if (s.find(i->name()) != s.end()) std::cerr << "ERROR --- " << *i << " already in map\n";
}

void KanjiLists::checkNotFound(const KanjiList::Set& s, const std::string& n) {
  if (s.find(n) != s.end()) std::cerr << "ERROR --- " << n + " already in set\n";
}

void KanjiLists::loadStrokes() {
  fs::path p(data / Strokes);
  if (!fs::is_regular_file(p)) usage(data.string() + " must contain " + p.string());
  std::ifstream f(p);
  std::string line;
  int strokes = 0;
  while (std::getline(f, line))
    if (std::isdigit(line[0])) {
      auto newStrokes = std::stoi(line);
      assert(newStrokes > strokes);
      strokes = newStrokes;
    } else {
      assert(strokes != 0); // first line must have a stroke count
      if (!_strokes.insert(std::pair(line, strokes)).second)
        std::cerr << "duplicate entry in " << p.string() << ": " << line << '\n';
    }
}

void KanjiLists::checkStrokes() const {
  // there shouldn't be any entries in _strokes that are 'Other' or 'None' type, but instead of
  // asserting, print lists to help find any problems
  KanjiList::List strokesOther;
  KanjiList::List strokesNotFound;
  for (const auto& i : _strokes) {
    auto t = getType(i.first);
    if (t == Types::Other)
      strokesOther.push_back(i.first);
    else if (t == Types::None && !isOldName(i.first))
      strokesNotFound.push_back(i.first);
  }
  KanjiList::print(strokesOther, "Kanjis in 'Other' group", "strokes.txt", true);
  KanjiList::print(strokesNotFound, "Kanjis without other groups", "strokes.txt", true);
}

void KanjiLists::populateJouyou() {
  auto results = FileListKanji::fromFile(*this, Types::Jouyou, data / Jouyou);
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(i->grade() != Grades::None);
    checkInsert(_jouyouMap, i);
    if (i->oldName().has_value()) checkInsert(_jouyouOldSet, *i->oldName());
    _jouyouKanji.push_back(i);
  }
  // populate _linkedJinmeiKanji that are linked to Jouyou
  fs::path p(data / LinkedJinmei);
  if (!fs::is_regular_file(p)) usage(data.string() + " must contain " + p.string());
  std::ifstream f(p);
  std::string line;
  KanjiList::List notInOld;
  int count = 0;
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    if (std::string jouyou, linked; std::getline(ss, jouyou, '\t') && std::getline(ss, linked, '\t')) {
      const auto i = _jouyouMap.find(jouyou);
      if (i == _jouyouMap.end())
        std::cerr << "ERROR --- can't find " << jouyou << " while processing " << p.string() << '\n';
      else {
        auto k = std::make_shared<LinkedJinmeiKanji>(*this, ++count, linked, i->second);
        checkInsert(_linkedJinmeiMap, k);
        _linkedJinmeiKanji.push_back(k);
        if (_jouyouOldSet.find(linked) == _jouyouOldSet.end()) notInOld.emplace_back(linked);
      }
    } else
      std::cerr << "ERROR --- bad line in " << p.string() << ": " << line << '\n';
  }
  KanjiList::print(notInOld, "kanji that are not 'old jouyou'", p.stem().string());
  // populate _linkedOldKanji (so old Jouyou that are not Linked Jinmei)
  count = 0;
  for (const auto& i : _jouyouKanji)
    if (i->oldName().has_value()) {
      auto k = std::make_shared<LinkedOldKanji>(*this, ++count, *i->oldName(), i);
      checkInsert(_linkedOldMap, k);
      _linkedOldKanji.push_back(k);
    }
}

void KanjiLists::populateJinmei() {
  auto results = FileListKanji::fromFile(*this, Types::Jinmei, data / Jinmei);
  for (const auto& i : results) {
    checkInsert(_jinmeiMap, i);
    checkNotFound(_jouyouMap, i);
    checkNotFound(_jouyouOldSet, i->name());
    if (i->oldName().has_value()) {
      checkInsert(_jinmeiOldSet, *i->oldName());
      auto k = std::make_shared<LinkedJinmeiKanji>(*this, _linkedJinmeiMap.size(), *i->oldName(), i);
      checkInsert(_linkedJinmeiMap, k);
      _linkedJinmeiKanji.push_back(k);
    }
    _jinmeiKanji.push_back(i);
  }
}

void KanjiLists::populateExtra() {
  auto results = FileListKanji::fromFile(*this, Types::Extra, data / Extra);
  for (const auto& i : results) {
    checkInsert(_extraMap, i);
    checkNotFound(_jouyouMap, i);
    checkNotFound(_jinmeiMap, i);
    checkNotFound(_linkedJinmeiMap, i);
    checkNotFound(_linkedOldMap, i);
    _extraKanji.push_back(i);
  }
}

void KanjiLists::processList(const KanjiList& l) {
  KanjiList::List jouyouOld;
  KanjiList::List jinmeiOld;
  KanjiList::List other;
  KanjiList::List jinmei;
  KanjiList::List linkedJinmei;
  KanjiList::List linkedOld;
  auto count = 0;
  for (const auto& i : l.list()) {
    if (_jouyouMap.find(i) == _jouyouMap.end()) {
      // keep track of any 'old' kanji in a level or top frequency list
      if (_jouyouOldSet.find(i) != _jouyouOldSet.end())
        jouyouOld.push_back(i);
      else if (_jinmeiOldSet.find(i) != _jinmeiOldSet.end())
        jinmeiOld.push_back(i);
      // only add to _otherKanji/_otherMap if not in any other collections
      if (_jinmeiMap.find(i) != _jinmeiMap.end())
        jinmei.emplace_back(i);
      else if (_linkedJinmeiMap.find(i) != _linkedJinmeiMap.end())
        linkedJinmei.emplace_back(i);
      else if (_linkedOldMap.find(i) != _linkedOldMap.end())
        linkedOld.emplace_back(i);
      else if (_otherMap.find(i) == _otherMap.end()) {
        auto k = std::make_shared<Kanji>(*this, ++count, i, l.level);
        // Kanjis in lists (n1 - n5 and frequency) shouldn't be in the '_extraSet'
        checkNotFound(_extraMap, k);
        _otherMap.insert(std::make_pair(i, k));
        _otherKanji.push_back(k);
        other.emplace_back(i);
      }
    }
  }
  KanjiList::print(jouyouOld, "Jouyou Old", l.name);
  KanjiList::print(jinmeiOld, "Jinmei Old", l.name);
  KanjiList::print(linkedOld, "Linked Old", l.name);
  KanjiList::print(other, std::string("non-Jouyou/non-Jinmei") + (l.level == Levels::None ? "/non-JLPT" : ""), l.name);
  if (l.level == Levels::None) {
    std::vector lists = {std::make_pair(&jinmei, ""), std::make_pair(&linkedJinmei, "Linked ")};
    for (const auto& i : lists) {
      KanjiList::List jlptJinmei, otherJinmei;
      for (const auto& j : *i.first)
        if (getLevel(j) != Levels::None)
          jlptJinmei.emplace_back(j);
        else
          otherJinmei.emplace_back(j);
      KanjiList::print(jlptJinmei, std::string("JLPT ") + i.second + "Jinmei", l.name);
      KanjiList::print(otherJinmei, std::string("non-JLPT ") + i.second + "Jinmei", l.name);
    }
  } else {
    KanjiList::print(jinmei, "Jinmei", l.name);
    KanjiList::print(linkedJinmei, "Linked Jinmei", l.name);
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

KanjiLists::List FileListKanji::fromFile(const KanjiLists& k, Types type, const fs::path& file) {
  assert(type == Types::Jouyou || type == Types::Jinmei || type == Types::Extra);
  if (!fs::is_regular_file(file)) usage("can't find file: " + file.string());
  std::ifstream f(file);
  std::string line;
  std::map<int, int> colMap;
  KanjiLists::List results;
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    int pos = 0;
    // first line should be headers, don't catch exceptions for first line since
    // whole file would be a problem
    if (colMap.empty())
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        auto i = ColumnMap.find(token);
        if (i == ColumnMap.end()) throw std::domain_error("unrecognized column: " + token);
        if (!colMap.insert(std::pair(pos, i->second)).second) throw std::domain_error("duplicate column: " + token);
      }
    else
      try {
        for (std::string token; std::getline(ss, token, '\t'); ++pos) {
          auto i = colMap.find(pos);
          if (i == colMap.end()) throw std::out_of_range("too many columns");
          columns[i->second] = token;
        }
        switch (type) {
        case Types::Jouyou: results.push_back(std::make_shared<JouyouKanji>(k)); break;
        case Types::Jinmei: results.push_back(std::make_shared<JinmeiKanji>(k)); break;
        default: results.push_back(std::make_shared<ExtraKanji>(k)); break;
        }
      } catch (const std::exception& e) {
        std::cerr << "got exception: " << e.what() << " while processing " << file.string() << " line: " << line
                  << '\n';
      }
  }
  return results;
}

std::array<std::string, FileListKanji::MaxIndex> FileListKanji::columns;

std::map<std::string, int> FileListKanji::ColumnMap = {
  {"Number", Number},   {"Name", Name},   {"Radical", Radical}, {"OldName", OldName}, {"Year", Year},
  {"Strokes", Strokes}, {"Grade", Grade}, {"Meaning", Meaning}, {"Reading", Reading}, {"Reason", Reason}};

JinmeiKanji::Reasons JinmeiKanji::getReason(const std::string& s) {
  if (s == "Names") return Reasons::Names;
  if (s == "Print") return Reasons::Print;
  if (s == "Moved") return Reasons::Moved;
  if (s == "Variant") return Reasons::Variant;
  return Reasons::Other;
}

Grades JouyouKanji::getGrade(const std::string& s) {
  if (s == "S") return Grades::S;
  if (s == "6") return Grades::G6;
  if (s == "5") return Grades::G5;
  if (s == "4") return Grades::G4;
  if (s == "3") return Grades::G3;
  if (s == "2") return Grades::G2;
  if (s == "1") return Grades::G1;
  return Grades::None;
}

} // namespace kanji
