#include <kanji/Kanji.h>
#include <kanji/MBChar.h>

#include <fstream>
#include <numeric>
#include <sstream>

namespace kanji {

int Data::_maxFrequency;
const Data::List Data::_emptyList;

namespace fs = std::filesystem;

namespace {

const fs::path JouyouFile = "jouyou.txt";
const fs::path JinmeiFile = "jinmei.txt";
const fs::path LinkedJinmeiFile = "linked-jinmei.txt";
const fs::path ExtraFile = "extra.txt";
const fs::path UcdFile = "ucd.txt";

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

const Data::Ucd* Data::findUcd(const std::string& s) const {
  std::string r = s;
  if (MBChar::isMBCharWithVariationSelector(s)) {
    auto nonVariant = MBChar::withoutVariationSelector(s);
    // check for linked Jinmei variant first
    auto i = _ucdLinkedJinmei.find(nonVariant);
    if (i == _ucdLinkedJinmei.end()) {
      auto j = _ucdLinkedOther.find(nonVariant);
      if (j == _ucdLinkedOther.end()) return nullptr;
      r = j->second;
    } else
      r = i->second;
  }
  auto i = _ucdMap.find(r);
  return i == _ucdMap.end() ? nullptr : &i->second;
}

fs::path Data::getDataDir(int argc, const char** argv) {
  std::optional<fs::path> found = {};
  for (int i = 1; !found.has_value() && i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-data") {
      if (i + 1 < argc) {
        auto data = fs::path(argv[i + 1]);
        if (fs::is_directory(data))
          found = data;
        else
          usage(data.string() + " is not a valid directory");
      } else
        usage("'-data' must be followed by a directory name");
    }
  }
  // If '-data' wasn't provided then search up directories for 'data' and make sure
  // it contains at least one of the required files (jouyou.txt).
  if (!found.has_value()) {
    if (!argc) usage("need at least one argument, argv[0], to check for a relative 'data' directory");
    auto oldParent = fs::absolute(fs::path(argv[0])).lexically_normal();
    auto dataDir = fs::path("data");
    do {
      auto parent = oldParent.parent_path();
      // 'has_parent_path' seems to always return true, i.e., the parent of '/' is
      // '/' so break if new 'parent' is equal to 'oldParent'.
      if (parent == oldParent) break;
      auto data = parent / dataDir;
      if (fs::is_directory(data) && fs::is_regular_file(data / JouyouFile))
        found = data;
      else
        oldParent = parent;
    } while (!found.has_value());
  }
  if (!found.has_value()) usage("couldn't find valid 'data' directory");
  return *found;
}

bool Data::getDebug(int argc, const char** argv) {
  for (int i = 1; i < argc; ++i)
    if (std::string(argv[i]) == "-debug") return true;
  return false;
}

int Data::nextArg(int argc, const char** argv, int currentArg) {
  int result = currentArg + 1;
  if (result < argc) {
    std::string arg = argv[result];
    // '-data' should be followed by a 'path' so increment by 2. If -data isn't followed
    // by a path then an earlier call to 'getDataDir' would have failed with a call to
    // 'usage' which ends the program.
    if (arg == "-data") return nextArg(argc, argv, result + 1);
    if (arg == "-debug") return nextArg(argc, argv, result);
  }
  return result;
}

bool Data::checkInsert(const Entry& i) {
  if (_map.insert(std::make_pair(i->name(), i)).second) {
    auto error = [this, &i](const std::string& s) {
      std::string v;
      if (i->variant()) v = " (non-variant: " + i->nonVariantName() + ")";
      printError(i->name() + v + " " + s + " in _ucd");
    };
    if (i->frequency() >= _maxFrequency) _maxFrequency = i->frequency() + 1;
    auto t = i->type();
    auto j = findUcd(i->name());
    if (!j)
      error("not found");
    else {
      if (t == Types::Jouyou && !j->joyo())
        error("not marked as 'Joyo'");
      else if (t == Types::Jinmei && !j->jinmei())
        error("not marked as 'Jinmei'");
      else if (t == Types::LinkedJinmei && !j->jinmei())
        error("with link not marked as 'Jinmei'");
      else if (t == Types::LinkedJinmei && !j->hasLink())
        error("missing 'JinmeiLink'");
      // skipping radical and strokes checks for now
    }
    return true;
  }
  printError("failed to insert " + i->name() + " into map");
  return false;
}

bool Data::checkInsert(List& s, const Entry& i) {
  if (!checkInsert(i)) return false;
  s.push_back(i);
  return true;
}

bool Data::checkNotFound(const Entry& i) const {
  if (_map.find(i->name()) == _map.end()) return true;
  printError(i->name() + " already in map");
  return false;
}

bool Data::checkInsert(FileList::Set& s, const std::string& n) const {
  if (s.insert(n).second) return true;
  printError("failed to insert " + n + " into set");
  return false;
}

bool Data::checkNotFound(const FileList::Set& s, const std::string& n) const {
  if (s.find(n) == s.end()) return true;
  printError(n + " already in set");
  return false;
}

void Data::printError(const std::string& msg) const {
  static int count = 0;
  _err << "ERROR[" << std::setfill('0') << std::setw(4) << ++count << "] --- " << msg << std::setfill(' ') << '\n';
}

void Data::loadUcdData() {
  auto file = _dataDir / UcdFile;
  int lineNum = 1, codeCol = -1, nameCol = -1, radicalCol = -1, strokesCol = -1, variantStrokesCol = -1, joyoCol = -1,
      jinmeiCol = -1, linkCodeCol = -1, linkNameCol = -1, meaningCol = -1, onCol = -1, kunCol = -1;
  auto error = [&lineNum, &file](const std::string& s, bool printLine = true) {
    usage(s + (printLine ? " - line: " + std::to_string(lineNum) : "") + ", file: " + file.string());
  };
  auto getWchar = [&error](const std::string& col, const auto& s, bool allowEmpty = false) -> wchar_t {
    if (s.empty() && allowEmpty) return 0;
    if (s.length() != 4 && s.length() != 5) error(col + " length must be 4 or 5 '" + s + "'");
    for (char c : s)
      if (c < '0' || c > 'F' || (c < 'A' && c > '9')) error("invalid '" + col + "' string '" + s + "'");
    return std::strtol(s.c_str(), nullptr, 16);
  };
  auto getBool = [&error](const std::string& col, const auto& s) {
    if (s == "Y") return true;
    if (!s.empty()) error("unrecognized '" + col + "' value '" + s + "'");
    return false;
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name");
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 12> cols;
  for (std::string line; std::getline(f, line); ++lineNum) {
    int pos = 0;
    std::stringstream ss(line);
    if (codeCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Code")
          setCol(codeCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "Radical")
          setCol(radicalCol, pos);
        else if (token == "Strokes")
          setCol(strokesCol, pos);
        else if (token == "VStrokes")
          setCol(variantStrokesCol, pos);
        else if (token == "Joyo")
          setCol(joyoCol, pos);
        else if (token == "Jinmei")
          setCol(jinmeiCol, pos);
        else if (token == "LinkCode")
          setCol(linkCodeCol, pos);
        else if (token == "LinkName")
          setCol(linkNameCol, pos);
        else if (token == "Meaning")
          setCol(meaningCol, pos);
        else if (token == "On")
          setCol(onCol, pos);
        else if (token == "Kun")
          setCol(kunCol, pos);
        else
          error("unrecognized column '" + token + "'", false);
      if (pos != cols.size()) error("not enough columns", false);
    } else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        if (pos == cols.size()) error("too many columns");
        cols[pos] = token;
      }
      if (pos == cols.size() - 1)
        cols[pos] = "";
      else if (pos != cols.size())
        error("not enough columns - got " + std::to_string(pos) + ", wanted " + std::to_string(cols.size()));
      const wchar_t code = getWchar("Unicode", cols[codeCol]);
      const auto& name = cols[nameCol];
      if (name.length() > 4) error("name greater than 4");
      const int radical = FileListKanji::toInt(cols[radicalCol]);
      if (radical < 1 || radical > 214) error("radical out of range");
      const int strokes = FileListKanji::toInt(cols[strokesCol]);
      if (strokes < 1 || strokes > 33) error("strokes out of range");
      const int variantStrokes = cols[variantStrokesCol].empty() ? 0 : FileListKanji::toInt(cols[variantStrokesCol]);
      if (variantStrokes < 0 || variantStrokes == 1 || variantStrokes > 33) error("variant strokes out of range");
      const bool joyo = getBool("Joyo", cols[joyoCol]);
      const bool jinmei = getBool("Jinmei", cols[jinmeiCol]);
      const wchar_t linkCode = getWchar("LinkCode", cols[linkCodeCol], true);
      if (linkCode > 0 && cols[linkNameCol].empty()) error("missing link name");
      // meaning is empty for some entries like 乁, 乣, 乴, etc., but it shouldn't be empty for a Joyo
      if (joyo && cols[meaningCol].empty()) error("meaning is empty for Joyo Kanji");
      if (cols[onCol].empty() && cols[kunCol].empty()) error("one of 'on' or 'kun' must be populated");
      if (!_ucdMap
             .emplace(std::piecewise_construct, std::make_tuple(name),
                      std::make_tuple(code, name, radical, strokes, variantStrokes, joyo, jinmei, linkCode,
                                      cols[linkNameCol], cols[meaningCol], cols[onCol], cols[kunCol]))
             .second)
        error("duplicate entry '" + name + "'");
      if (linkCode > 0) {
        if (jinmei) {
          auto i = _ucdLinkedJinmei.insert(std::make_pair(cols[linkNameCol], name));
          if (!i.second) error("jinmei link " + cols[linkNameCol] + " to " + name + " failed - has " + i.first->second);
        } else {
          auto i = _ucdLinkedOther.insert(std::make_pair(cols[linkNameCol], name));
          if (!i.second) error("link " + cols[linkNameCol] + " to " + name + " failed - has " + i.first->second);
        }
      }
    }
  }
}

void Data::loadRadicals(const fs::path& file) {
  int lineNum = 1, numberCol = -1, nameCol = -1, longNameCol = -1, readingCol = -1;
  auto error = [&lineNum, &file](const std::string& s, bool printLine = true) {
    usage(s + (printLine ? " - line: " + std::to_string(lineNum) : "") + ", file: " + file.string());
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name");
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 4> cols;
  for (std::string line; std::getline(f, line); ++lineNum) {
    int pos = 0;
    std::stringstream ss(line);
    if (numberCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Number")
          setCol(numberCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "LongName")
          setCol(longNameCol, pos);
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
      if (pos != cols.size())
        error("not enough columns - got " + std::to_string(pos) + ", wanted " + std::to_string(cols.size()));
      const int radicalNumber = FileListKanji::toInt(cols[numberCol]);
      if (radicalNumber + 1 != lineNum) error("radicals must be ordered by 'number'");
      std::stringstream radicals(cols[nameCol]);
      Radical::AltForms altForms;
      std::string name, token;
      while (std::getline(radicals, token, ' '))
        if (name.empty())
          name = token;
        else
          altForms.emplace_back(token);
      _radicals.emplace_back(radicalNumber, name, altForms, cols[longNameCol], cols[readingCol]);
    }
  }
  for (auto& i : _radicals)
    _radicalMap[i.name()] = i.number() - 1;
}

void Data::loadStrokes(const fs::path& file, bool checkDuplicates) {
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
      std::stringstream ss(line);
      for (std::string token; std::getline(ss, token, ' ');) {
        auto i = _strokes.insert(std::pair(token, strokes));
        if (!i.second) {
          if (checkDuplicates)
            printError("duplicate entry in " + file.string() + ": " + token);
          else if (i.first->second != strokes)
            printError("found entry with different count in " + file.string() + ": " + token);
        }
      }
    }
}

void Data::loadOtherReadings(const fs::path& file) {
  int lineNum = 1, nameCol = -1, readingCol = -1;
  auto error = [&lineNum, &file](const std::string& s, bool printLine = true) {
    usage(s + (printLine ? " - line: " + std::to_string(lineNum) : "") + ", file: " + file.string());
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name");
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 2> cols;
  for (std::string line; std::getline(f, line); ++lineNum) {
    int pos = 0;
    std::stringstream ss(line);
    if (nameCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Name")
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
      if (!_otherReadings.insert(std::make_pair(cols[nameCol], cols[readingCol])).second) error("duplicate name");
    }
  }
}

void Data::populateJouyou() {
  auto results = FileListKanji::fromFile(*this, Types::Jouyou, FileList::getFile(_dataDir, JouyouFile));
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(i->grade() != Grades::None);
    if (checkInsert(i)) _grades[i->grade()].push_back(i);
    if (i->oldName().has_value()) checkInsert(_jouyouOldSet, *i->oldName());
  }
  _types.insert(std::make_pair(Types::Jouyou, std::move(results)));
  // populate _linkedJinmeiKanji that are linked to Jouyou
  fs::path file = FileList::getFile(_dataDir, LinkedJinmeiFile);
  std::ifstream f(file);
  FileList::List found, notFound;
  int count = 0;
  auto& linkedJinmei = _types[Types::LinkedJinmei];
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
  auto& linkedOld = _types[Types::LinkedOld];
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
  auto& linkedJinmei = _types[Types::LinkedJinmei];
  for (const auto& i : results) {
    checkInsert(i);
    checkNotFound(_jouyouOldSet, i->name());
    if (i->oldName().has_value()) {
      checkInsert(_jinmeiOldSet, *i->oldName());
      auto k = std::make_shared<LinkedJinmeiKanji>(*this, linkedJinmei.size(), *i->oldName(), i);
      checkInsert(linkedJinmei, k);
    }
  }
  _types.insert(std::make_pair(Types::Jinmei, std::move(results)));
}

void Data::populateExtra() {
  auto results = FileListKanji::fromFile(*this, Types::Extra, FileList::getFile(_dataDir, ExtraFile));
  for (const auto& i : results)
    checkInsert(i);
  _types.insert(std::make_pair(Types::Extra, std::move(results)));
}

void Data::processList(const FileList& list) {
  FileList::List jouyouOld, jinmeiOld, other;
  std::map<Types, FileList::List> found;
  auto count = 0;
  auto& otherKanji = _types[Types::Other];
  for (const auto& i : list.list()) {
    // keep track of any 'old' kanji in a level or top frequency list
    if (_debug) {
      if (_jouyouOldSet.find(i) != _jouyouOldSet.end())
        jouyouOld.push_back(i);
      else if (_jinmeiOldSet.find(i) != _jinmeiOldSet.end())
        jinmeiOld.push_back(i);
    }
    auto j = _map.find(i);
    Entry kanji;
    if (j != _map.end()) {
      kanji = j->second;
      if (_debug && j->second->type() != Types::Jouyou) found[j->second->type()].emplace_back(i);
    } else {
      // kanji wasn't already in _map so it only exists in the 'frequency.txt' file - these kanjis
      // are considered 'Other' type and by definition are not part of Jouyou or Jinmei (so also
      // not part of JLPT levels)
      auto reading = _otherReadings.find(i);
      if (reading != _otherReadings.end())
        kanji = std::make_shared<ReadingKanji>(*this, ++count, i, reading->second);
      else
        kanji = std::make_shared<Kanji>(*this, ++count, i);
      _map.insert(std::make_pair(i, kanji));
      otherKanji.push_back(kanji);
      if (_debug) other.emplace_back(i);
    }
    if (list.level() == Levels::None) {
      assert(kanji->frequency() != 0);
      int index = (kanji->frequency() - 1) / 500;
      _frequencies[index < FrequencyBuckets ? index : FrequencyBuckets - 1].push_back(kanji);
    } else {
      assert(kanji->level() == list.level());
      _levels[list.level()].push_back(kanji);
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

void Data::checkStrokes() const {
  FileList::List strokesOther, strokesNotFound, strokeDiffs, vStrokeDiffs, missingDiffs, missingUcd;
  for (const auto& i : _strokes) {
    const int ucdStrokes = getStrokes(i.first, false, true);
    auto k = findKanji(i.first);
    if (ucdStrokes) {
      // If a Kanji object exists, prefer to use its 'strokes' since this it's more accurate, i.e.,
      // there are some incorrect stroke counts in 'wiki-strokes.txt', but they aren't used because
      // the actual stroke count comes from 'jouyou.txt', 'jinmei.txt' or 'extra.txt'
      if (k.has_value()) {
        if ((**k).variant()) {
          if ((**k).strokes() != getStrokes(i.first, true, true)) vStrokeDiffs.push_back(i.first);
        } else if ((**k).strokes() != ucdStrokes)
          strokeDiffs.push_back(i.first);
      } else if (i.second != ucdStrokes)
        missingDiffs.push_back(i.first);
    } else
      missingUcd.push_back(i.first);
    if (k.has_value()) {
      if ((**k).type() == Types::Other) strokesOther.push_back(i.first);
    } else
      strokesNotFound.push_back(i.first);
  }
  if (_debug) {
    FileList::print(strokesOther, "Kanji in 'Other' group", "_strokes");
    FileList::print(strokesNotFound, "Kanji without other groups", "_strokes");
    FileList::print(strokeDiffs, "Kanji with differrent strokes", "_ucdMap");
    FileList::print(vStrokeDiffs, "Variant kanji with differrent strokes", "_ucdMap");
    FileList::print(missingDiffs, "'_stokes only' Kanji with differrent strokes", "_ucdMap");
    FileList::print(missingUcd, "Kanji in _strokes, but not found", "_ucdMap");
  }
}

} // namespace kanji
