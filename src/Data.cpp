#include <kanji/Group.h>
#include <kanji/MBChar.h>

#include <fstream>
#include <numeric>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

namespace {

const fs::path JouyouFile = "jouyou.txt";
const fs::path JinmeiFile = "jinmei.txt";
const fs::path LinkedJinmeiFile = "linked-jinmei.txt";
const fs::path ExtraFile = "extra.txt";

// helper function for printing 'no-frequency' counts
void noFreq(int f, bool brackets = false) {
  if (f) {
    if (brackets)
      std::cout << " (";
    else
      std::cout << ' ';
    std::cout << "nf " << f;
    if (brackets) std::cout << ')';
  }
}

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

Types Data::getType(const std::string& name) const {
  auto i = _map.find(name);
  if (i == _map.end()) return Types::None;
  return i->second->type();
}

fs::path Data::getDataDir(int argc, char** argv) {
  if (argc < 2) usage("please specify data directory");
  fs::path f(argv[1]);
  if (!fs::is_directory(f)) usage(f.string() + " is not a valid directory");
  return f;
}

bool Data::getDebug(int argc, char** argv) {
  for (int i = 2; i < argc; ++i)
    if (std::string(argv[i]) == "-debug") return true;
  return false;
}

bool Data::checkInsert(const Entry& i) {
  if (_map.insert(std::make_pair(i->name(), i)).second) return true;
  printError("failed to insert " + i->name() + " into map");
  return false;
}

void Data::checkInsert(List& s, const Entry& i) {
  if (checkInsert(i)) s.push_back(i);
}

void Data::checkInsert(const std::string& name, GroupMap& groups, const GroupEntry& group) {
  auto i = groups.insert(std::make_pair(name, group));
  if (!i.second)
    printError(name + " from Group " + std::to_string(group->number()) + " already in group " +
               i.first->second->toString());
}

void Data::checkNotFound(const Entry& i) const {
  if (_map.find(i->name()) != _map.end()) printError(i->name() + " already in map");
}

void Data::checkInsert(FileList::Set& s, const std::string& n) {
  if (!s.insert(n).second) printError("failed to insert " + n + " into set");
}

void Data::checkNotFound(const FileList::Set& s, const std::string& n) {
  if (s.find(n) != s.end()) printError(n + " already in set");
}

void Data::printError(const std::string& msg) { std::cerr << "ERROR --- " << msg << '\n'; }

void Data::loadRadicals(const fs::path& file) {
  int lineNumber = 1, numberCol = -1, radicalCol = -1, nameCol = -1, readingCol = -1;
  auto error = [&lineNumber, &file](const std::string& s, bool printLine = true) {
    usage(s + (printLine ? " - line: " + std::to_string(lineNumber) : "") + ", file: " + file.string());
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name");
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 4> cols;
  for (std::string line; std::getline(f, line); ++lineNumber) {
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
          error("unrecognized column '" + token + "'", false);
      if (pos != cols.size()) error("not enough columns", false);
    } else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        if (pos == cols.size()) error("too many columns");
        cols[pos] = token;
      }
      if (pos != cols.size()) error("not enough columns");
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

void Data::loadStrokes(const fs::path& file) {
  std::ifstream f(file);
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
        printError("duplicate entry in " + file.string() + ": " + line);
    }
}

void Data::populateJouyou() {
  auto results = FileListKanji::fromFile(*this, Types::Jouyou, FileList::getFile(_dataDir, JouyouFile));
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(i->grade() != Grades::None);
    checkInsert(i);
    if (i->oldName().has_value()) checkInsert(_jouyouOldSet, *i->oldName());
  }
  _lists.insert(std::make_pair(Types::Jouyou, std::move(results)));
  // populate _linkedJinmeiKanji that are linked to Jouyou
  fs::path file = FileList::getFile(_dataDir, LinkedJinmeiFile);
  std::ifstream f(file);
  FileList::List found, notFound;
  int count = 0;
  auto& linkedJinmei = _lists[Types::LinkedJinmei];
  for (std::string line; std::getline(f, line);) {
    std::stringstream ss(line);
    if (std::string jouyou, linked; std::getline(ss, jouyou, '\t') && std::getline(ss, linked, '\t')) {
      const auto i = _map.find(jouyou);
      if (i == _map.end())
        printError("can't find " + jouyou + " while processing " + file.string());
      else {
        auto k = std::make_shared<LinkedJinmeiKanji>(*this, ++count, linked, i->second);
        checkInsert(linkedJinmei, k);
        if (_debug) {
          if (_jouyouOldSet.find(linked) == _jouyouOldSet.end())
            notFound.emplace_back(linked);
          else
            found.emplace_back(linked);
        }
      }
    } else
      printError("bad line in " + file.string() + ": " + line);
  }
  FileList::print(found, "kanji that are 'old jouyou'", file.stem().string());
  FileList::print(notFound, "kanji that are not 'old jouyou'", file.stem().string());
  found.clear();
  // populate _linkedOldKanji (so old Jouyou that are not Linked Jinmei)
  count = 0;
  auto& linkedOld = _lists[Types::LinkedOld];
  for (const auto& i : _map)
    if (i.second->oldName().has_value()) {
      if (_map.find(*i.second->oldName()) == _map.end()) {
        auto k = std::make_shared<LinkedOldKanji>(*this, ++count, *i.second->oldName(), i.second);
        checkInsert(linkedOld, k);
        if (_debug) found.push_back(*i.second->oldName());
      }
    }
  FileList::print(found, "'old jouyou' that are not " + file.stem().string());
}

void Data::populateJinmei() {
  auto results = FileListKanji::fromFile(*this, Types::Jinmei, FileList::getFile(_dataDir, JinmeiFile));
  auto& linkedJinmei = _lists[Types::LinkedJinmei];
  for (const auto& i : results) {
    checkInsert(i);
    checkNotFound(_jouyouOldSet, i->name());
    if (i->oldName().has_value()) {
      checkInsert(_jinmeiOldSet, *i->oldName());
      auto k = std::make_shared<LinkedJinmeiKanji>(*this, linkedJinmei.size(), *i->oldName(), i);
      checkInsert(linkedJinmei, k);
    }
  }
  _lists.insert(std::make_pair(Types::Jinmei, std::move(results)));
}

void Data::populateExtra() {
  auto results = FileListKanji::fromFile(*this, Types::Extra, FileList::getFile(_dataDir, ExtraFile));
  for (const auto& i : results)
    checkInsert(i);
  _lists.insert(std::make_pair(Types::Extra, std::move(results)));
}

void Data::processList(const FileList& list) {
  FileList::List jouyouOld, jinmeiOld, other;
  std::map<Types, FileList::List> found;
  auto count = 0;
  auto& otherKanji = _lists[Types::Other];
  for (const auto& i : list.list()) {
    // keep track of any 'old' kanji in a level or top frequency list
    if (_debug) {
      if (_jouyouOldSet.find(i) != _jouyouOldSet.end())
        jouyouOld.push_back(i);
      else if (_jinmeiOldSet.find(i) != _jinmeiOldSet.end())
        jinmeiOld.push_back(i);
    }
    auto j = _map.find(i);
    if (j != _map.end()) {
      if (_debug && j->second->type() != Types::Jouyou) found[j->second->type()].emplace_back(i);
    } else {
      // kanji wasn't already in _map so it only exists in the 'frequency.txt' file
      auto k = std::make_shared<Kanji>(*this, ++count, i, list.level());
      _map.insert(std::make_pair(i, k));
      otherKanji.push_back(k);
      if (_debug) other.emplace_back(i);
    }
  }
  FileList::print(jouyouOld, "Jouyou Old", list.name());
  FileList::print(jinmeiOld, "Jinmei Old", list.name());
  FileList::print(found[Types::LinkedOld], "Linked Old", list.name());
  FileList::print(other, std::string("non-Jouyou/Jinmei") + (list.level() == Levels::None ? "/JLPT" : ""), list.name());
  // list.level is None when processing 'frequency.txt' file (so not a JLPT level file)
  if (list.level() == Levels::None) {
    std::vector lists = {std::make_pair(&found[Types::Jinmei], ""),
                         std::make_pair(&found[Types::LinkedJinmei], "Linked ")};
    for (const auto& i : lists) {
      FileList::List jlptJinmei, otherJinmei;
      for (const auto& j : *i.first)
        if (getLevel(j) != Levels::None)
          jlptJinmei.emplace_back(j);
        else
          otherJinmei.emplace_back(j);
      FileList::print(jlptJinmei, std::string("JLPT ") + i.second + "Jinmei", list.name());
      FileList::print(otherJinmei, std::string("non-JLPT ") + i.second + "Jinmei", list.name());
    }
  } else {
    FileList::print(found[Types::Jinmei], "Jinmei", list.name());
    FileList::print(found[Types::LinkedJinmei], "Linked Jinmei", list.name());
  }
}

void Data::loadGroup(const std::filesystem::path& file, GroupMap& groups, GroupList& list, GroupType type) {
  int lineNumber = 1, numberCol = -1, nameCol = -1, membersCol = -1;
  auto error = [&lineNumber, &file](const std::string& s, bool printLine = true) {
    usage(s + (printLine ? " - line: " + std::to_string(lineNumber) : "") + ", file: " + file.string());
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name", false);
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 3> cols;
  for (std::string line; std::getline(f, line); ++lineNumber) {
    int pos = 0;
    std::stringstream ss(line);
    if (numberCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Number")
          setCol(numberCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "Members")
          setCol(membersCol, pos);
        else
          error("unrecognized column '" + token + "'", false);
      if (pos != cols.size()) error("not enough columns", false);
    } else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        if (pos == cols.size()) error("too many columns");
        cols[pos] = token;
      }
      if (pos != cols.size()) error("not enough columns");
      std::string number(cols[numberCol]), name(cols[nameCol]), token;
      FileList::List kanjis;
      const bool peers = name.empty();
      if (type == GroupType::Meaning) {
        if (peers) error("Meaning group must have a name");
      } else if (!peers) // if populated, 'name colum' is the first member of a Pattern group
        kanjis.emplace_back(name);
      for (std::stringstream members(cols[membersCol]); std::getline(members, token, ',');)
        kanjis.emplace_back(token);
      List memberKanjis;
      for (const auto& i : kanjis) {
        const auto memberKanji = findKanji(i);
        if (memberKanji.has_value())
          memberKanjis.push_back(*memberKanji);
        else
          printError("failed to find member " + i + " in group " + number);
      }
      if (memberKanjis.empty()) error("group " + number + " has no valid members");
      GroupEntry group;
      if (type == GroupType::Meaning)
        group = std::make_shared<MeaningGroup>(FileListKanji::toInt(number), name, memberKanjis);
      else
        group = std::make_shared<PatternGroup>(FileListKanji::toInt(number), memberKanjis, peers);
      for (const auto& i : memberKanjis)
        checkInsert(i->name(), groups, group);
      list.push_back(group);
    }
  }
}

void Data::checkStrokes() const {
  FileList::List strokesOther, strokesNotFound;
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

template<typename T> void Data::printCount(const std::string& name, T pred) const {
  std::vector<std::pair<Types, int>> counts;
  int total = 0;
  for (const auto& l : _lists) {
    const int count = std::count_if(l.second.begin(), l.second.end(), pred);
    if (count) {
      counts.emplace_back(l.first, count);
      total += count;
    }
  }
  if (total) {
    std::cout << ">>> " << name << ' ' << total << " (";
    for (const auto& i : counts) {
      std::cout << i.first << ' ' << i.second;
      total -= i.second;
      if (total) std::cout << ", ";
    }
    std::cout << ")\n";
  }
}

void Data::printStats() const {
  std::cout << ">>> Loaded " << _map.size() << " Kanji (";
  for (const auto& i : _lists) {
    if (i != *_lists.begin()) std::cout << ' ';
    std::cout << i.first << ' ' << i.second.size();
  }
  std::cout << ")\n";
  printCount("  Has JLPT level", [](const auto& x) { return x->hasLevel(); });
  printCount("  Has frequency and not in Jouyou or JLPT",
             [](const auto& x) { return x->frequency() && x->type() != Types::Jouyou && !x->hasLevel(); });
  printCount("  Jinmei with no frequency and not JLPT",
             [](const auto& x) { return x->type() == Types::Jinmei && !x->frequency() && !x->hasLevel(); });
  printCount("  NF (no-frequency)", [](const auto& x) { return !x->frequency(); });
  printCount("  Has Strokes", [](const auto& x) { return x->strokes() != 0; });
  printCount("Old Forms", [](const auto& x) { return x->oldName().has_value(); });
  // some old kanjis have a non-zero frequency
  printCount("  Old Has Frequency", [this](const auto& x) { return x->oldFrequency(*this) != 0; });
  // some old kanjis have stroke counts
  printCount("  Old Has Strokes", [this](const auto& x) { return x->oldStrokes(*this) != 0; });
  // no old kanjis should have a JLPT level, i.e.: they all should have Level 'None'
  printCount("  Old Has Level", [this](const auto& x) { return x->oldLevel(*this) != Levels::None; });
  // old kanjis should only have types of LinkedJinmei, Other or None
  for (auto i : AllTypes)
    printCount(std::string("  Old is type ") + toString(i),
               [this, i](const auto& x) { return x->oldName().has_value() && x->oldType(*this) == i; });
}

void Data::printGrades() const {
  std::cout << ">>> Grade breakdown:\n";
  int all = 0;
  const auto& jouyou = _lists.at(Types::Jouyou);
  for (auto i : AllGrades) {
    auto grade = [i](const auto& x) { return x->grade() == i; };
    auto gradeCount = std::count_if(jouyou.begin(), jouyou.end(), grade);
    if (gradeCount) {
      all += gradeCount;
      std::cout << ">>>   Total for grade " << i << ": " << gradeCount;
      noFreq(
        std::count_if(jouyou.begin(), jouyou.end(), [&grade](const auto& x) { return grade(x) && !x->frequency(); }),
        true);
      std::cout << " (";
      for (auto level : AllLevels) {
        const auto gradeLevelCount = std::count_if(
          jouyou.begin(), jouyou.end(), [&grade, level](const auto& x) { return grade(x) && x->level() == level; });
        if (gradeLevelCount) {
          gradeCount -= gradeLevelCount;
          std::cout << level << ' ' << gradeLevelCount;
          if (gradeCount) std::cout << ", ";
        }
      }
      std::cout << ")\n";
    }
  }
  std::cout << ">>>   Total for all grades: " << all << '\n';
}

void Data::printLevels() const {
  std::cout << ">>> Level breakdown:\n";
  int total = 0;
  for (auto level : AllLevels) {
    std::vector<std::pair<Types, int>> counts;
    int levelTotal = 0;
    for (const auto& l : _lists) {
      int count =
        std::count_if(l.second.begin(), l.second.end(), [level](const auto& x) { return x->level() == level; });
      if (count) {
        counts.emplace_back(l.first, count);
        levelTotal += count;
      }
    }
    if (levelTotal) {
      total += levelTotal;
      std::cout << ">>>   Total for level " << level << ": " << levelTotal << " (";
      for (const auto& j : counts) {
        std::cout << j.first << ' ' << j.second;
        const auto& l = _lists.at(j.first);
        noFreq(
          std::count_if(l.begin(), l.end(), [level](const auto& x) { return x->level() == level && !x->frequency(); }));
        levelTotal -= j.second;
        if (levelTotal) std::cout << ", ";
      }
      std::cout << ")\n";
    }
  }
  std::cout << ">>>   Total for all levels: " << total << '\n';
}

void Data::printRadicals() const {
  std::cout << ">>> Radical breakdown - total count for each radical is followed by (Jouyou Jinmei Extra) counts:\n";
  std::map<Radical, Data::List> radicals;
  for (const auto& i : _lists) {
    if (hasRadical(i.first)) {
      Data::List sorted(i.second);
      std::sort(sorted.begin(), sorted.end(), [](const auto& x, const auto& y) { return x->strokes() - y->strokes(); });
      for (const auto& j : sorted)
        radicals[static_cast<const FileListKanji&>(*j).radical()].push_back(j);
    }
  }
  int jouyou = 0, jinmei = 0, extra = 0;
  for (const auto& i : radicals) {
    int jo = 0, ji = 0, ex = 0;
    for (const auto& j : i.second)
      switch (j->type()) {
      case Types::Jouyou: ++jo; break;
      case Types::Jinmei: ++ji; break;
      default: ++ex; break;
      }
    auto counts = std::to_string(jo) + ' ' + std::to_string(ji) + ' ' + std::to_string(ex) + ')';
    std::cout << i.first << ':' << std::setfill(' ') << std::right << std::setw(4) << i.second.size() << " ("
              << std::left << std::setw(9) << counts << ':';
    jouyou += jo;
    jinmei += ji;
    extra += ex;
    Types oldType = i.second[0]->type();
    for (const auto& j : i.second) {
      if (j->type() != oldType) {
        std::cout << "、";
        oldType = j->type();
      }
      std::cout << ' ' << *j;
    }
    std::cout << '\n';
  }
  std::cout << ">>>   Total for " << radicals.size() << " radicals: " << jouyou + jinmei + extra << " (Jouyou "
            << jouyou << " Jinmei " << jinmei << " Extra " << extra << ")\n";
  std::vector<Radical> missingRadicals;
  for (const auto& i : _radicals)
    if (radicals.find(i.second) == radicals.end()) missingRadicals.push_back(i.second);
  if (!missingRadicals.empty()) {
    std::cout << ">>>   Found " << missingRadicals.size() << " radicals with no kanji:";
    for (const auto& i : missingRadicals)
      std::cout << ' ' << i;
    std::cout << '\n';
  }
}

void Data::printGroups(const GroupMap& groups, const GroupList& groupList) const {
  std::cout << ">>> Loaded " << groups.size() << " kanji into " << groupList.size() << " groups\n"
            << ">>> Jouyou kanji have no suffix, otherwise '=JLPT \"=Freq ^=Jinmei ~=Linked Jinmei +=Extra *=...:\n";
  for (const auto& i : groupList) {
    if (i->type() == GroupType::Meaning) {
      auto len = MBChar::length(i->name());
      std::cout << '[' << i->name()
                << (len == 1     ? "　　"
                      : len == 2 ? "　"
                                 : "")
                << ' ' << std::setw(2) << std::setfill(' ') << i->members().size() << "] :";
      for (const auto& j : i->members())
        std::cout << ' ' << j->qualifiedName();
    } else {
      std::cout << '[' << std::setw(3) << std::setfill('0') << i->number() << "] ";
      for (const auto& j : i->members())
        if (j == i->members()[0]) {
          if (i->peers())
            std::cout << "　 : " << j->qualifiedName();
          else
            std::cout << j->qualifiedName() << ':';
        } else
          std::cout << ' ' << j->qualifiedName();
    }
    std::cout << '\n';
  }
}

} // namespace kanji
