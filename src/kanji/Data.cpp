#include <kanji_tools/kanji/CustomFileKanji.h>
#include <kanji_tools/kanji/LinkedKanji.h>
#include <kanji_tools/kanji/UcdFileKanji.h>
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

KanjiTypes Data::getType(const std::string& name) const {
  auto i = findKanjiByName(name);
  return i.has_value() ? (**i).type() : KanjiTypes::None;
}

Kanji::NelsonIds Data::getNelsonIds(const Ucd* u) const {
  if (u && !u->nelsonIds().empty()) {
    Kanji::NelsonIds ids;
    std::stringstream ss(u->nelsonIds());
    int id;
    while (ss >> id)
      ids.push_back(id);
    return ids;
  }
  return _emptyNelsonIds;
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

bool Data::checkInsert(const Entry& kanji) {
  if (_kanjiNameMap.insert(std::make_pair(kanji->name(), kanji)).second) {
    auto error = [this, &kanji](const std::string& s) {
      std::string v;
      if (kanji->variant()) v = " (non-variant: " + kanji->nonVariantName() + ")";
      printError(kanji->name() + " [" + toUnicode(kanji->name()) + "] " + v + " " + s + " in _ucd");
    };
    if (kanji->frequency() >= _maxFrequency) _maxFrequency = kanji->frequency() + 1;
    auto type = kanji->type();
    auto ucd = _ucd.find(kanji->name());
    if (!ucd)
      error("not found");
    else if (type == KanjiTypes::Jouyou && !ucd->joyo())
      error("not marked as 'Joyo'");
    else if (type == KanjiTypes::Jinmei && !ucd->jinmei())
      error("not marked as 'Jinmei'");
    else if (type == KanjiTypes::LinkedJinmei && !ucd->jinmei())
      error("with link not marked as 'Jinmei'");
    else if (type == KanjiTypes::LinkedJinmei && !ucd->hasLinks())
      error("missing 'JinmeiLink' for " + ucd->codeAndName());
    // skipping radical and strokes checks for now
    if (kanji->variant() &&
        !_compatibilityNameMap.insert(std::make_pair(kanji->compatibilityName(), kanji->name())).second)
      printError("failed to insert variant " + kanji->name() + " into map");
    if (kanji->morohashiId().has_value()) _morohashiMap[*kanji->morohashiId()].push_back(kanji);
    for (int id : kanji->nelsonIds())
      _nelsonMap[id].push_back(kanji);
    return true;
  }
  printError("failed to insert " + kanji->name() + " into map");
  return false;
}

bool Data::checkInsert(List& s, const Entry& kanji) {
  if (!checkInsert(kanji)) return false;
  s.push_back(kanji);
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

void Data::loadFrequencyReadings(const fs::path& file) {
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
      if (!_frequencyReadings.insert(std::make_pair(cols[nameCol], cols[readingCol])).second) error("duplicate name");
    }
  }
}

void Data::populateJouyou() {
  auto results = CustomFileKanji::fromFile(*this, KanjiTypes::Jouyou, DataFile::getFile(_dataDir, JouyouFile));
  for (const auto& i : results) {
    // all Jouyou Kanji must have a grade
    assert(i->grade() != KanjiGrades::None);
    if (checkInsert(i)) _grades[i->grade()].push_back(i);
  }
  _types.insert(std::make_pair(KanjiTypes::Jouyou, std::move(results)));
  // populate _linkedJinmeiKanji that are linked to Jouyou
  fs::path file = DataFile::getFile(_dataDir, LinkedJinmeiFile);
  std::ifstream f(file);
  auto& linkedJinmei = _types[KanjiTypes::LinkedJinmei];
  for (std::string line; std::getline(f, line);) {
    std::stringstream ss(line);
    if (std::string jouyou, linked; std::getline(ss, jouyou, '\t') && std::getline(ss, linked, '\t')) {
      const auto i = _kanjiNameMap.find(jouyou);
      if (i == _kanjiNameMap.end())
        printError("can't find " + jouyou + " while processing " + file.string());
      else {
        auto k = std::make_shared<LinkedJinmeiKanji>(*this, linked, i->second);
        checkInsert(linkedJinmei, k);
      }
    } else
      printError("bad line in " + file.string() + ": " + line);
  }
  // create 'LinkedOld' type kanji (these are the 'old Jouyou' that are not LinkedJinmei created above)
  auto& linkedOld = _types[KanjiTypes::LinkedOld];
  for (const auto& i : _kanjiNameMap)
    for (auto& j : i.second->oldNames())
      if (!findKanjiByName(j)) {
        auto k = std::make_shared<LinkedOldKanji>(*this, j, i.second);
        checkInsert(linkedOld, k);
      }
}

void Data::populateJinmei() {
  auto results = CustomFileKanji::fromFile(*this, KanjiTypes::Jinmei, DataFile::getFile(_dataDir, JinmeiFile));
  auto& linkedJinmei = _types[KanjiTypes::LinkedJinmei];
  for (const auto& i : results) {
    checkInsert(i);
    for (auto& j : i->oldNames()) {
      auto k = std::make_shared<LinkedJinmeiKanji>(*this, j, i);
      checkInsert(linkedJinmei, k);
    }
  }
  _types.insert(std::make_pair(KanjiTypes::Jinmei, std::move(results)));
}

void Data::populateExtra() {
  auto results = CustomFileKanji::fromFile(*this, KanjiTypes::Extra, DataFile::getFile(_dataDir, ExtraFile));
  for (const auto& i : results)
    checkInsert(i);
  _types.insert(std::make_pair(KanjiTypes::Extra, std::move(results)));
}

void Data::processList(const DataFile& list) {
  const bool kenteiList = list.kyu() != KenteiKyus::None;
  DataFile::List created;
  std::map<KanjiTypes, DataFile::List> found;
  auto& newKanji = _types[kenteiList ? KanjiTypes::Kentei : KanjiTypes::Frequency];
  for (const auto& i : list.list()) {
    auto j = findKanjiByName(i);
    Entry kanji;
    if (j.has_value()) {
      kanji = *j;
      if (_debug && !kenteiList && kanji->type() != KanjiTypes::Jouyou) found[kanji->type()].push_back(i);
    } else {
      if (kenteiList)
        kanji = std::make_shared<KenteiKanji>(*this, i);
      else {
        // kanji wasn't already in _kanjiNameMap so it only exists in the 'frequency.txt' file - these kanjis
        // are considered 'Frequency' type and by definition are not part of Jouyou or Jinmei (so also
        // not part of JLPT levels)
        auto reading = _frequencyReadings.find(i);
        if (reading != _frequencyReadings.end())
          kanji = std::make_shared<FrequencyKanji>(*this, i, reading->second);
        else
          kanji = std::make_shared<FrequencyKanji>(*this, i);
      }
      checkInsert(newKanji, kanji);
      // don't print out kentei 'created' since there more than 2,000 outside of the other types
      if (_debug && !kenteiList) created.push_back(i);
    }
    if (kenteiList) {
      assert(kanji->kyu() == list.kyu());
      _kyus[list.kyu()].push_back(kanji);
    } else if (list.level() == JlptLevels::None) {
      assert(kanji->frequency() != 0);
      int index = (kanji->frequency() - 1) / FrequencyBucketEntries;
      _frequencies[index < FrequencyBuckets ? index : FrequencyBuckets - 1].push_back(kanji);
    } else {
      assert(kanji->level() == list.level());
      _levels[list.level()].push_back(kanji);
    }
  }
  DataFile::print(found[KanjiTypes::LinkedOld], "Linked Old", list.name());
  DataFile::print(created, std::string("non-Jouyou/Jinmei") + (list.level() == JlptLevels::None ? "/JLPT" : ""),
                  list.name());
  // list.level is None when processing 'frequency.txt' file (so not a JLPT level file)
  if (!kenteiList && list.level() == JlptLevels::None) {
    std::vector lists = {std::make_pair(&found[KanjiTypes::Jinmei], ""),
                         std::make_pair(&found[KanjiTypes::LinkedJinmei], "Linked ")};
    for (const auto& i : lists) {
      DataFile::List jlptJinmei, otherJinmei;
      for (const auto& j : *i.first)
        if (getLevel(j) != JlptLevels::None)
          jlptJinmei.emplace_back(j);
        else
          otherJinmei.emplace_back(j);
      DataFile::print(jlptJinmei, std::string("JLPT ") + i.second + "Jinmei", list.name());
      DataFile::print(otherJinmei, std::string("non-JLPT ") + i.second + "Jinmei", list.name());
    }
  } else {
    DataFile::print(found[KanjiTypes::Jinmei], "Jinmei", list.name());
    DataFile::print(found[KanjiTypes::LinkedJinmei], "Linked Jinmei", list.name());
  }
}

void Data::processUcd() {
  auto& newKanji = _types[KanjiTypes::Ucd];
  // Calling 'findKanjiByName' also checks for a 'variation selector' version of the given 'name'
  // so use it instead of just checking for a match in _kanjiNameMap directly (this avoids 52
  // redundant kanji getting created when processing 'ucd.txt').
  for (auto& i : _ucd.map())
    if (!findKanjiByName(i.second.name()).has_value())
      checkInsert(newKanji, std::make_shared<UcdKanji>(*this, i.second));
}

void Data::checkStrokes() const {
  DataFile::List strokesFrequency, strokesNotFound, strokeDiffs, vStrokeDiffs, missingDiffs, missingUcd;
  for (const auto& i : _strokes) {
    const Ucd* u = findUcd(i.first);
    const int ucdStrokes = getStrokes(i.first, u, false, true);
    auto k = findKanjiByName(i.first);
    if (ucdStrokes) {
      // If a Kanji object exists, prefer to use its 'strokes' since this it's more accurate, i.e.,
      // there are some incorrect stroke counts in 'wiki-strokes.txt', but they aren't used because
      // the actual stroke count comes from 'jouyou.txt', 'jinmei.txt' or 'extra.txt'
      if (k.has_value()) {
        if ((**k).variant()) {
          if ((**k).strokes() != getStrokes(i.first, u, true, true)) vStrokeDiffs.push_back(i.first);
        } else if ((**k).strokes() != ucdStrokes)
          strokeDiffs.push_back(i.first);
      } else if (i.second != ucdStrokes)
        missingDiffs.push_back(i.first);
    } else
      missingUcd.push_back(i.first);
    if (k.has_value()) {
      if ((**k).type() == KanjiTypes::Frequency) strokesFrequency.push_back(i.first);
    } else
      strokesNotFound.push_back(i.first);
  }
  if (_debug) {
    DataFile::print(strokesFrequency, "Kanji in 'Frequency' group", "_strokes");
    DataFile::print(strokesNotFound, "Kanji not loaded", "_strokes");
    DataFile::print(strokeDiffs, "Kanji with differrent strokes", "_ucd");
    DataFile::print(vStrokeDiffs, "Variant kanji with differrent strokes", "_ucd");
    DataFile::print(missingDiffs, "'_stokes only' Kanji with differrent strokes", "_ucd");
    DataFile::print(missingUcd, "Kanji in _strokes, but not found", "_ucd");
  }
}

} // namespace kanji_tools
