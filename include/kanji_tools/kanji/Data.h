#ifndef KANJI_TOOLS_KANJI_DATA_H
#define KANJI_TOOLS_KANJI_DATA_H

#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/RadicalData.h>
#include <kanji_tools/kanji/UcdData.h>

#include <memory>
#include <optional>

namespace kanji_tools {

// 'Data' provides methods used by 'Kanji' classes during loading and is the base class for KanjiData
class Data {
public:
  using Entry = std::shared_ptr<Kanji>;
  using OptEntry = std::optional<const Entry>;
  using List = std::vector<Entry>;
  using Map = std::map<std::string, Entry>;

  static void usage(const std::string& msg) { FileList::usage(msg); }

  Data(const std::filesystem::path& dataDir, bool debug, std::ostream& out = std::cout, std::ostream& err = std::cerr)
    : _dataDir(dataDir), _debug(debug), _out(out), _err(err) {
    // Clearing FileList static data is only needed to help test code, for example FileList tests can leave some
    // data in these sets before Quiz tests are run (leading to problems loading real files).
    FileList::clearUniqueCheckData();
    if (_debug) log(true) << "Begin Loading Data\n>>>\n";
  }
  virtual ~Data() = default;
  Data(const Data&) = delete;

  // Functions used by 'Kanji' classes during construction, each takes a kanji name.
  virtual int getFrequency(const std::string&) const = 0;
  virtual Levels getLevel(const std::string&) const = 0;
  virtual Kyus getKyu(const std::string&) const = 0;
  virtual const Radical& ucdRadical(const std::string& kanjiName) const {
    const Ucd* u = _ucd.find(kanjiName);
    if (u) return _radicals.find(u->radical());
    // 'throw' should never happen - every 'Kanji' class instance should have also exist in the
    // data loaded from Unicode.
    throw std::domain_error("UCD entry not found: " + kanjiName);
  }
  // 'getRadicalByName' is used by 'ExtraKanji' classes during construction. It will return
  // the Radical for the given 'radicalName' (like 二, 木, 言, etc.).
  virtual const Radical& getRadicalByName(const std::string& radicalName) const { return _radicals.find(radicalName); }
  // 'getPinyin' returns an optional string since not all Kanji have a Pinyin reading.
  std::optional<std::string> getPinyin(const std::string& kanjiName) const {
    const Ucd* u = _ucd.find(kanjiName);
    if (u && !u->pinyin().empty()) return u->pinyin();
    return {};
  }

  // 'getCompatibilityName' returns the UCD compatibility code for the given 'kanjiName' if it
  // exists (_ucd.find method takes care of checking whether kanjiName has a variation selector).
  const std::string& getCompatibilityName(const std::string& kanjiName) const {
    const Ucd* u = _ucd.find(kanjiName);
    if (u && u->name() != kanjiName) return u->name();
    return kanjiName;
  }

  const UcdData& ucd() const { return _ucd; }

  int getStrokes(const std::string& s, bool variant = false, bool onlyUcd = false) const {
    if (!onlyUcd) {
      auto i = _strokes.find(s);
      if (i != _strokes.end()) return i->second;
    }
    auto i = _ucd.find(s);
    return i ? i->getStrokes(variant) : 0;
  }

  // get kanji lists
  const List& jouyouKanji() const { return _types.at(Types::Jouyou); }
  const List& jinmeiKanji() const { return _types.at(Types::Jinmei); }
  const List& linkedJinmeiKanji() const { return _types.at(Types::LinkedJinmei); }
  const List& linkedOldKanji() const { return _types.at(Types::LinkedOld); }
  const List& otherKanji() const { return _types.at(Types::Other); }
  const List& extraKanji() const { return _types.at(Types::Extra); }

  const List& typeList(Types type) const {
    auto i = _types.find(type);
    return i != _types.end() ? i->second : _emptyList;
  }
  size_t typeTotal(Types type) const { return typeList(type).size(); }

  OptEntry findKanji(const std::string& s) const {
    auto i = _compatibilityNameMap.find(s);
    auto j = _map.find(i != _compatibilityNameMap.end() ? i->second : s);
    if (j == _map.end()) return {};
    return j->second;
  }
  Types getType(const std::string& name) const;

  const List& gradeList(Grades grade) const {
    auto i = _grades.find(grade);
    return i != _grades.end() ? i->second : _emptyList;
  }
  size_t gradeTotal(Grades grade) const { return gradeList(grade).size(); }

  const List& levelList(Levels level) const {
    auto i = _levels.find(level);
    return i != _levels.end() ? i->second : _emptyList;
  }
  size_t levelTotal(Levels level) const { return levelList(level).size(); }

  const List& kyuList(Kyus kyu) const {
    auto i = _kyus.find(kyu);
    return i != _kyus.end() ? i->second : _emptyList;
  }
  size_t kyuTotal(Kyus kyu) const { return kyuList(kyu).size(); }

  // See comment for '_frequencies' private data member for more details about frequency lists
  enum Values { FrequencyBuckets = 5 };
  const List& frequencyList(int range) const {
    return range >= 0 && range < FrequencyBuckets ? _frequencies[range] : _emptyList;
  }
  size_t frequencyTotal(int range) const { return frequencyList(range).size(); }

  void printError(const std::string&) const;

  std::ostream& out() const { return _out; }
  std::ostream& err() const { return _err; }
  const std::filesystem::path& dataDir() const { return _dataDir; }
  bool debug() const { return _debug; }
  const Map& map() const { return _map; }

  // 'log' can be used for putting a standard prefix to output messages (used for some debug messages)
  std::ostream& log(bool heading = false) const { return heading ? _out << ">>>\n>>> " : _out << ">>> "; }

  // 'toInt' is a helper method used during file loading
  static int toInt(const std::string& s) {
    try {
      return std::stoi(s);
    } catch (...) {
      throw std::invalid_argument("failed to convert to int: " + s);
    }
  }

  static int maxFrequency() { return _maxFrequency; }

  // 'nextArg' will return 'currentArg + 1' if argv[currentArg + 1] is not used by this
  // class (ie getDataDir or getDebug). If currentArg + 1 is used by this class then
  // a larger increment is returned to 'skip over' the args, for example:
  //     for (int i = Data::nextArg(argc, argv); i < argc; i = Data::nextArg(argc, argv, i))
  static int nextArg(int argc, const char** argv, int currentArg = 0);
protected:
  // 'getDataDir' looks for a directory called 'data' containing 'jouyou.txt' based on
  // checking directories starting at 'argv[0]' (the program name) and working up parent
  // directories. Therefore argc must be at least 1. '-data' followed by a directory
  // name can also be used as an override.
  static std::filesystem::path getDataDir(int argc, const char** argv);

  // 'getDebug' looks for '-debug' flag in 'argv' list and returns true if it's found
  static bool getDebug(int argc, const char** argv);

  // helper functions for checking and inserting into '_map'
  bool checkInsert(const Entry&);
  bool checkInsert(List&, const Entry&);

  // 'loadStrokes' and 'loadOtherReadings' must be called before calling 'populate Lists' functions
  void loadStrokes(const std::filesystem::path&, bool checkDuplicates = true);
  void loadOtherReadings(const std::filesystem::path&);

  // populate Lists (_types datastructure)
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const FileList&);

  // 'checkStrokes' should be called after all lists are populated. If debug is enabled (-debug)
  // then this function will print any entries in _strokes that are 'Other' type or not found.
  // It also compares strokes that were loaded from other files to strokes in 'ucd.txt'
  void checkStrokes() const;

  std::ostream& _out;
  std::ostream& _err;
  const std::filesystem::path _dataDir;
  const bool _debug;

  // '_radicals' holds the 214 official Kanji Radicals
  RadicalData _radicals;

  // '_ucd' is used to supplement Kanji attributes like radical, meaning and reading
  UcdData _ucd;

  // '_compatibilityNameMap' maps from a UCD 'compatibility' code name to a 'variation selector'
  // style name. This map only has entries for recognized kanji that were loaded with a selector.
  std::map<std::string, std::string> _compatibilityNameMap;

  // 'otherReadings' holds readings loaded from other-readings.txt - these are for Top Frequency kanji
  // that aren't part of any other group (so not Jouyou or Jinmei).
  std::map<std::string, std::string> _otherReadings;

  // '_strokes' is populated from strokes.txt and is meant to supplement jinmei kanji (file doesn't
  // have a 'Strokes' column) as well as old kanjis from both jouyou and jinmei files. This file
  // contains stroke counts followed by one or more lines each with a single kanji that has the given
  // number of strokes.
  std::map<std::string, int> _strokes;

  // lists of kanji corresponding to 'Types', 'Grades', 'Levels' and 'Kyus' (excluding the 'None' enum values)
  std::map<Types, List> _types;
  std::map<Grades, List> _grades;
  std::map<Levels, List> _levels;
  std::map<Kyus, List> _kyus;

  // Lists of kanji grouped into 5 frequency ranges: 1-500, 501-1000, 1001-1500, 1501-2000, 2001-2501.
  // The last list is one longer in order to hold the full frequency list (of 2501 kanji).

  // allow lookup by name
  std::array<List, FrequencyBuckets> _frequencies;
  Map _map;

  // 'maxFrequency' is set to 1 larger than the highest frequency of any kanji put into '_map'
  inline static int _maxFrequency = 0;

  inline static const List _emptyList;
};

using DataPtr = std::shared_ptr<const Data>;

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_DATA_H
