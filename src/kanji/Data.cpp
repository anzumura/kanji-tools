#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/kanji/OtherKanji.h>
#include <kanji_tools/utils/MBUtils.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path JouyouFile = "jouyou.txt";
const fs::path JinmeiFile = "jinmei.txt";
const fs::path LinkedJinmeiFile = "linked-jinmei.txt";
const fs::path ExtraFile = "extra.txt";

} // namespace

Types Data::getType(const std::string& name) const {
  auto i = findKanji(name);
  return i.has_value() ? (**i).type() : Types::None;
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
      printError(i->name() + " [" + toUnicode(i->name()) + "] " + v + " " + s + " in _ucd");
    };
    if (i->frequency() >= _maxFrequency) _maxFrequency = i->frequency() + 1;
    auto t = i->type();
    auto j = _ucd.find(i->name());
    if (!j)
      error("not found");
    else if (t == Types::Jouyou && !j->joyo())
      error("not marked as 'Joyo'");
    else if (t == Types::Jinmei && !j->jinmei())
      error("not marked as 'Jinmei'");
    else if (t == Types::LinkedJinmei && !j->jinmei())
      error("with link not marked as 'Jinmei'");
    else if (t == Types::LinkedJinmei && !j->hasLink())
      error("missing 'JinmeiLink' for " + j->codeAndName());
    // skipping radical and strokes checks for now
    if (i->variant()) {
      auto k = _compatibilityNameMap.insert(std::make_pair(i->compatibilityName(), i->name()));
      if (!k.second) printError("failed to insert variant " + i->name() + " into map");
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

void Data::printError(const std::string& msg) const {
  static int count = 0;
  _err << "ERROR[" << std::setfill('0') << std::setw(4) << ++count << "] --- " << msg << std::setfill(' ') << '\n';
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
  auto results = ExtraKanji::fromFile(*this, Types::Jouyou, FileList::getFile(_dataDir, JouyouFile));
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(i->grade() != Grades::None);
    if (checkInsert(i)) _grades[i->grade()].push_back(i);
  }
  _types.insert(std::make_pair(Types::Jouyou, std::move(results)));
  // populate _linkedJinmeiKanji that are linked to Jouyou
  fs::path file = FileList::getFile(_dataDir, LinkedJinmeiFile);
  std::ifstream f(file);
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
      }
    } else
      printError("bad line in " + file.string() + ": " + line);
  }
  // create 'LinkedOld' type kanji (these are the 'old Jouyou' that are not LinkedJinmei created above)
  count = 0;
  auto& linkedOld = _types[Types::LinkedOld];
  for (const auto& i : _map)
    for (auto& j : i.second->oldNames())
      if (!findKanji(j)) {
        auto k = std::make_shared<LinkedOldKanji>(*this, ++count, j, i.second);
        checkInsert(linkedOld, k);
      }
}

void Data::populateJinmei() {
  auto results = ExtraKanji::fromFile(*this, Types::Jinmei, FileList::getFile(_dataDir, JinmeiFile));
  auto& linkedJinmei = _types[Types::LinkedJinmei];
  for (const auto& i : results) {
    checkInsert(i);
    for (auto& j : i->oldNames()) {
      auto k = std::make_shared<LinkedJinmeiKanji>(*this, linkedJinmei.size(), j, i);
      checkInsert(linkedJinmei, k);
    }
  }
  _types.insert(std::make_pair(Types::Jinmei, std::move(results)));
}

void Data::populateExtra() {
  auto results = ExtraKanji::fromFile(*this, Types::Extra, FileList::getFile(_dataDir, ExtraFile));
  for (const auto& i : results)
    checkInsert(i);
  _types.insert(std::make_pair(Types::Extra, std::move(results)));
}

void Data::processList(const FileList& list) {
  const bool kenteiList = list.kyu() != Kyus::None;
  FileList::List created;
  std::map<Types, FileList::List> found;
  auto& newKanji = _types[kenteiList ? Types::Kentei : Types::Other];
  auto count = newKanji.size();
  for (const auto& i : list.list()) {
    auto j = findKanji(i);
    Entry kanji;
    if (j.has_value()) {
      kanji = *j;
      if (_debug && !kenteiList && kanji->type() != Types::Jouyou) found[kanji->type()].push_back(i);
    } else {
      if (kenteiList)
        kanji = std::make_shared<KenteiKanji>(*this, ++count, i);
      else {
        // kanji wasn't already in _map so it only exists in the 'frequency.txt' file - these kanjis
        // are considered 'Other' type and by definition are not part of Jouyou or Jinmei (so also
        // not part of JLPT levels)
        auto reading = _otherReadings.find(i);
        if (reading != _otherReadings.end())
          kanji = std::make_shared<OtherKanji>(*this, ++count, i, reading->second);
        else
          kanji = std::make_shared<OtherKanji>(*this, ++count, i);
      }
      _map.insert(std::make_pair(i, kanji));
      newKanji.push_back(kanji);
      // don't print out kentei 'created' since there more than 2,000 outside of the other types
      if (_debug && !kenteiList) created.push_back(i);
    }
    if (kenteiList) {
      assert(kanji->kyu() == list.kyu());
      _kyus[list.kyu()].push_back(kanji);
    } else if (list.level() == Levels::None) {
      assert(kanji->frequency() != 0);
      int index = (kanji->frequency() - 1) / 500;
      _frequencies[index < FrequencyBuckets ? index : FrequencyBuckets - 1].push_back(kanji);
    } else {
      assert(kanji->level() == list.level());
      _levels[list.level()].push_back(kanji);
    }
  }
  FileList::print(found[Types::LinkedOld], "Linked Old", list.name());
  FileList::print(created, std::string("non-Jouyou/Jinmei") + (list.level() == Levels::None ? "/JLPT" : ""),
                  list.name());
  // list.level is None when processing 'frequency.txt' file (so not a JLPT level file)
  if (!kenteiList && list.level() == Levels::None) {
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
    FileList::print(strokeDiffs, "Kanji with differrent strokes", "_ucd");
    FileList::print(vStrokeDiffs, "Variant kanji with differrent strokes", "_ucd");
    FileList::print(missingDiffs, "'_stokes only' Kanji with differrent strokes", "_ucd");
    FileList::print(missingUcd, "Kanji in _strokes, but not found", "_ucd");
  }
}

} // namespace kanji_tools
