#include <kanji/Data.h>
#include <kanji/Kanji.h>

#include <fstream>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

namespace {

const fs::path N1 = "n1.txt";
const fs::path N2 = "n2.txt";
const fs::path N3 = "n3.txt";
const fs::path N4 = "n4.txt";
const fs::path N5 = "n5.txt";
const fs::path Frequency = "frequency.txt";
const fs::path Radicals = "radicals.txt";
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

Data::Data(int argc, char** argv)
  : data(getDataDir(argc, argv)), n5(data / N5, Levels::N5), n4(data / N4, Levels::N4), n3(data / N3, Levels::N3),
    n2(data / N2, Levels::N2), n1(data / N1, Levels::N1), frequency(data / Frequency) {
  loadRadicals();
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

Types Data::getType(const std::string& name) const {
  auto i = _map.find(name);
  if (i == _map.end()) return Types::None;
  return i->second->type();
}

void Data::checkInsert(List& s, const Entry& i) {
  if (_map.insert(std::make_pair(i->name(), i)).second)
    s.push_back(i);
  else
    std::cerr << "ERROR --- failed to insert " << *i << " into map\n";
}

void Data::checkNotFound(const Entry& i) {
  if (_map.find(i->name()) != _map.end()) std::cerr << "ERROR --- " << *i << " already in map\n";
}

void Data::checkInsert(FileList::Set& s, const std::string& n) {
  if (!s.insert(n).second) std::cerr << "ERROR --- failed to insert " << n << " into set\n";
}

void Data::checkNotFound(const FileList::Set& s, const std::string& n) {
  if (s.find(n) != s.end()) std::cerr << "ERROR --- " << n + " already in set\n";
}

void Data::loadRadicals() {
  fs::path p(data / Radicals);
  if (!fs::is_regular_file(p)) usage(data.string() + " must contain " + p.string());
  std::ifstream f(p);
  std::string line;
  int numberCol = -1, radicalCol = -1, nameCol = -1, readingCol = -1;
  auto setCol = [&p](int& col, int pos) {
    if (col != -1) usage("column " + std::to_string(pos) + " has duplicate name in " + p.string());
    col = pos;
  };
  std::array<std::string, 4> cols;
  while (std::getline(f, line)) {
    int pos = 0;
    std::stringstream ss(line);
    if (numberCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Number")
          setCol(numberCol, pos);
        else if (token == "Radical")
          setCol(radicalCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "Reading")
          setCol(readingCol, pos);
        else
          usage("unrecognized column '" + token + "' in " + p.string());
      if (pos != cols.size()) usage("not enough columns in " + p.string());
    } else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        if (pos == cols.size()) usage("too many columns on line: " + line);
        cols[pos] = token;
      }
      if (pos != cols.size()) usage("not enough columns on line: " + line);
      std::stringstream radicals(cols[radicalCol]);
      Radical::AltForms altForms;
      std::string radical, token;
      while (std::getline(radicals, token, ' '))
        if (radical.empty())
          radical = token;
        else
          altForms.emplace_back(token);
      _radicals.emplace(
        std::piecewise_construct, std::make_tuple(radical),
        std::make_tuple(FileListKanji::toInt(cols[numberCol]), radical, altForms, cols[nameCol], cols[readingCol]));
    }
  }
}

void Data::loadStrokes() {
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

void Data::checkStrokes() const {
  // there shouldn't be any entries in _strokes that are 'Other' or 'None' type, but instead of
  // asserting, print lists to help find any problems
  FileList::List strokesOther;
  FileList::List strokesNotFound;
  for (const auto& i : _strokes) {
    auto t = getType(i.first);
    if (t == Types::Other)
      strokesOther.push_back(i.first);
    else if (t == Types::None && !isOldName(i.first))
      strokesNotFound.push_back(i.first);
  }
  FileList::print(strokesOther, "Kanjis in 'Other' group", "strokes.txt", true);
  FileList::print(strokesNotFound, "Kanjis without other groups", "strokes.txt", true);
}

void Data::populateJouyou() {
  auto results = FileListKanji::fromFile(*this, Types::Jouyou, data / Jouyou);
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(i->grade() != Grades::None);
    checkInsert(_jouyouKanji, i);
    if (i->oldName().has_value()) checkInsert(_jouyouOldSet, *i->oldName());
  }
  // populate _linkedJinmeiKanji that are linked to Jouyou
  fs::path p(data / LinkedJinmei);
  if (!fs::is_regular_file(p)) usage(data.string() + " must contain " + p.string());
  std::ifstream f(p);
  std::string line;
  FileList::List found, notFound;
  int count = 0;
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    if (std::string jouyou, linked; std::getline(ss, jouyou, '\t') && std::getline(ss, linked, '\t')) {
      const auto i = _map.find(jouyou);
      if (i == _map.end())
        std::cerr << "ERROR --- can't find " << jouyou << " while processing " << p.string() << '\n';
      else {
        auto k = std::make_shared<LinkedJinmeiKanji>(*this, ++count, linked, i->second);
        checkInsert(_linkedJinmeiKanji, k);
        if (_jouyouOldSet.find(linked) == _jouyouOldSet.end())
          notFound.emplace_back(linked);
        else
          found.emplace_back(linked);
      }
    } else
      std::cerr << "ERROR --- bad line in " << p.string() << ": " << line << '\n';
  }
  FileList::print(found, "kanji that are 'old jouyou'", p.stem().string());
  FileList::print(notFound, "kanji that are not 'old jouyou'", p.stem().string());
  found.clear();
  // populate _linkedOldKanji (so old Jouyou that are not Linked Jinmei)
  count = 0;
  for (const auto& i : _jouyouKanji)
    if (i->oldName().has_value()) {
      if (_map.find(*i->oldName()) == _map.end()) {
        auto k = std::make_shared<LinkedOldKanji>(*this, ++count, *i->oldName(), i);
        checkInsert(_linkedOldKanji, k);
        found.push_back(*i->oldName());
      }
    }
  FileList::print(found, "'old jouyou' that are not " + p.stem().string());
}

void Data::populateJinmei() {
  auto results = FileListKanji::fromFile(*this, Types::Jinmei, data / Jinmei);
  for (const auto& i : results) {
    checkInsert(_jinmeiKanji, i);
    checkNotFound(_jouyouOldSet, i->name());
    if (i->oldName().has_value()) {
      checkInsert(_jinmeiOldSet, *i->oldName());
      auto k = std::make_shared<LinkedJinmeiKanji>(*this, _linkedJinmeiKanji.size(), *i->oldName(), i);
      checkInsert(_linkedJinmeiKanji, k);
    }
  }
}

void Data::populateExtra() {
  auto results = FileListKanji::fromFile(*this, Types::Extra, data / Extra);
  for (const auto& i : results)
    checkInsert(_extraKanji, i);
}

void Data::processList(const FileList& l) {
  FileList::List jouyouOld, jinmeiOld, other;
  std::map<Types, FileList::List> found;
  auto count = 0;
  for (const auto& i : l.list()) {
    // keep track of any 'old' kanji in a level or top frequency list
    if (_jouyouOldSet.find(i) != _jouyouOldSet.end())
      jouyouOld.push_back(i);
    else if (_jinmeiOldSet.find(i) != _jinmeiOldSet.end())
      jinmeiOld.push_back(i);
    auto j = _map.find(i);
    if (j != _map.end()) {
      if (j->second->type() != Types::Jouyou) found[j->second->type()].emplace_back(i);
    } else {
      // kanji wasn't already in _map so it only exists in the 'frequency.txt' file
      auto k = std::make_shared<Kanji>(*this, ++count, i, l.level);
      _map.insert(std::make_pair(i, k));
      _otherKanji.push_back(k);
      other.emplace_back(i);
    }
  }
  FileList::print(jouyouOld, "Jouyou Old", l.name);
  FileList::print(jinmeiOld, "Jinmei Old", l.name);
  FileList::print(found[Types::LinkedOld], "Linked Old", l.name);
  FileList::print(other, std::string("non-Jouyou/non-Jinmei") + (l.level == Levels::None ? "/non-JLPT" : ""), l.name);
  // l.level is None when processing 'frequency.txt' file (so not a JLPT level file)
  if (l.level == Levels::None) {
    std::vector lists = {std::make_pair(&found[Types::Jinmei], ""),
                         std::make_pair(&found[Types::LinkedJinmei], "Linked ")};
    for (const auto& i : lists) {
      FileList::List jlptJinmei, otherJinmei;
      for (const auto& j : *i.first)
        if (getLevel(j) != Levels::None)
          jlptJinmei.emplace_back(j);
        else
          otherJinmei.emplace_back(j);
      FileList::print(jlptJinmei, std::string("JLPT ") + i.second + "Jinmei", l.name);
      FileList::print(otherJinmei, std::string("non-JLPT ") + i.second + "Jinmei", l.name);
    }
  } else {
    FileList::print(found[Types::Jinmei], "Jinmei", l.name);
    FileList::print(found[Types::LinkedJinmei], "Linked Jinmei", l.name);
  }
}

fs::path Data::getDataDir(int argc, char** argv) {
  if (argc < 2) usage("please specify data directory");
  fs::path f(argv[1]);
  if (!fs::is_directory(f)) usage(f.string() + " is not a valid directory");
  return f;
}

Levels Data::getLevel(const std::string& k) const {
  if (n1.exists(k)) return Levels::N1;
  if (n2.exists(k)) return Levels::N2;
  if (n3.exists(k)) return Levels::N3;
  if (n4.exists(k)) return Levels::N4;
  if (n5.exists(k)) return Levels::N5;
  return Levels::None;
}

} // namespace kanji
