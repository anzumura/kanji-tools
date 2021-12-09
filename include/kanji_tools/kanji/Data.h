#ifndef KANJI_TOOLS_KANJI_DATA_H
#define KANJI_TOOLS_KANJI_DATA_H

#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/RadicalData.h>
#include <kanji_tools/kanji/UcdData.h>
#include <kanji_tools/utils/DataFile.h>

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

  static void usage(const std::string& msg) { DataFile::usage(msg); }
  inline static auto orderByQualifiedName = [](const Entry& a, const Entry& b) { return a->orderByQualifiedName(*b); };

  Data(const std::filesystem::path& dataDir, bool debug, std::ostream& out = std::cout, std::ostream& err = std::cerr)
    : _dataDir(dataDir), _debug(debug), _out(out), _err(err) {
    // Clearing DataFile static data is only needed to help test code, for example DataFile tests can leave some
    // data in these sets before Quiz tests are run (leading to problems loading real files).
    DataFile::clearUniqueCheckData();
    if (_debug) log(true) << "Begin Loading Data\n>>>\n";
  }
  virtual ~Data() = default;
  Data(const Data&) = delete;

  const UcdData& ucd() const { return _ucd; }
  const Ucd* findUcd(const std::string& kanjiName) const { return _ucd.find(kanjiName); }
  // Functions used by 'Kanji' classes during construction, each takes a kanji name.
  virtual Kanji::OptInt getFrequency(const std::string&) const = 0;
  virtual JlptLevels getLevel(const std::string&) const = 0;
  virtual KenteiKyus getKyu(const std::string&) const = 0;
  virtual const Radical& ucdRadical(const std::string& kanjiName, const Ucd* u) const {
    if (u) return _radicals.find(u->radical());
    // 'throw' should never happen - every 'Kanji' class instance should have also exist in the
    // data loaded from Unicode.
    throw std::domain_error("UCD entry not found: " + kanjiName);
  }
  // 'getRadicalByName' is used by 'ExtraKanji' classes during construction. It will return
  // the Radical for the given 'radicalName' (like 二, 木, 言, etc.).
  virtual const Radical& getRadicalByName(const std::string& radicalName) const { return _radicals.find(radicalName); }
  // 'getPinyin' returns an optional string since not all Kanji have a Pinyin reading.
  Kanji::OptString getPinyin(const Ucd* u) const {
    return u && !u->pinyin().empty() ? Kanji::OptString(u->pinyin()) : std::nullopt;
  }
  // 'getMorohashiId' returns an optional 'Dai Kan-Wa Jiten' index number (see comments in scripts/parseUcdAllFlat.sh)
  Kanji::OptString getMorohashiId(const Ucd* u) const {
    return u && !u->morohashiId().empty() ? Kanji::OptString(u->morohashiId()) : std::nullopt;
  }
  // 'getNelsonIds' returns a vector of 0 or more 'Classic Nelson' ids
  Kanji::NelsonIds getNelsonIds(const Ucd*) const;
  // 'getCompatibilityName' returns the UCD compatibility code for the given 'kanjiName' if it
  // exists (_ucd.find method takes care of checking whether kanjiName has a variation selector).
  Kanji::OptString getCompatibilityName(const std::string& kanjiName) const {
    const Ucd* u = _ucd.find(kanjiName);
    return u && u->name() != kanjiName ? Kanji::OptString(u->name()) : std::nullopt;
  }

  int getStrokes(const std::string& kanjiName) const { return getStrokes(kanjiName, findUcd(kanjiName)); }
  int getStrokes(const std::string& kanjiName, const Ucd* u, bool variant = false, bool onlyUcd = false) const {
    if (!onlyUcd) {
      auto i = _strokes.find(kanjiName);
      if (i != _strokes.end()) return i->second;
    }
    return u ? u->getStrokes(variant) : 0;
  }

  // get kanji lists
  const List& jouyouKanji() const { return _types.at(KanjiTypes::Jouyou); }
  const List& jinmeiKanji() const { return _types.at(KanjiTypes::Jinmei); }
  const List& linkedJinmeiKanji() const { return _types.at(KanjiTypes::LinkedJinmei); }
  const List& linkedOldKanji() const { return _types.at(KanjiTypes::LinkedOld); }
  const List& frequencyKanji() const { return _types.at(KanjiTypes::Frequency); }
  const List& extraKanji() const { return _types.at(KanjiTypes::Extra); }

  // get list by KanjiType
  const List& typeList(KanjiTypes type) const {
    auto i = _types.find(type);
    return i != _types.end() ? i->second : _emptyList;
  }
  size_t typeTotal(KanjiTypes type) const { return typeList(type).size(); }
  KanjiTypes getType(const std::string& name) const;

  // get list by KanjiGrade
  const List& gradeList(KanjiGrades grade) const {
    auto i = _grades.find(grade);
    return i != _grades.end() ? i->second : _emptyList;
  }
  size_t gradeTotal(KanjiGrades grade) const { return gradeList(grade).size(); }

  // get list by JLPT Level
  const List& levelList(JlptLevels level) const {
    auto i = _levels.find(level);
    return i != _levels.end() ? i->second : _emptyList;
  }
  size_t levelTotal(JlptLevels level) const { return levelList(level).size(); }

  // get list by Kentei Kyu
  const List& kyuList(KenteiKyus kyu) const {
    auto i = _kyus.find(kyu);
    return i != _kyus.end() ? i->second : _emptyList;
  }
  size_t kyuTotal(KenteiKyus kyu) const { return kyuList(kyu).size(); }

  // See comment for '_frequencies' private data member for more details about frequency lists
  enum Values { FrequencyBuckets = 5, FrequencyBucketEntries = 500 };
  const List& frequencyList(int range) const {
    return range >= 0 && range < FrequencyBuckets ? _frequencies[range] : _emptyList;
  }
  size_t frequencyTotal(int range) const { return frequencyList(range).size(); }

  // 'findKanjiByName' supports finding a Kanji by UTF-8 string including 'variation selectors', i.e., the
  // same result is returned for '侮︀ [4FAE FE00]' and '侮 [FA30]' (a single UTF-8 compatibility kanji).
  OptEntry findKanjiByName(const std::string& s) const {
    auto i = _compatibilityNameMap.find(s);
    auto j = _kanjiNameMap.find(i != _compatibilityNameMap.end() ? i->second : s);
    if (j == _kanjiNameMap.end()) return {};
    return j->second;
  }
  // 'findKanjiByFrequency' returns the Kanji with the given 'frequency' (should be a value from 1 to 2501)
  OptEntry findKanjiByFrequency(int frequency) const {
    if (frequency < 1 || frequency >= _maxFrequency) return {};
    int bucket = --frequency / FrequencyBucketEntries;
    if (bucket == FrequencyBuckets) --bucket; // last bucket contains FrequencyBucketEntries + 1
    return _frequencies[bucket][frequency - bucket * FrequencyBucketEntries];
  }
  // 'findKanjisByMorohashiId' can return more than one entry. The ids are usually plain just numeric, but they can
  // also be an index number followed by a 'P'. For example, '4138' maps to 嗩 and '4138P' maps to 嘆.
  const List& findKanjisByMorohashiId(const std::string& id) const {
    auto i = _morohashiMap.find(id);
    return i != _morohashiMap.end() ? i->second : _emptyList;
  }
  // 'findKanjisByNelsonId' can return more than one entry. For example, 1491 maps to 㡡, 幮 and 𢅥.
  const List& findKanjisByNelsonId(int id) const {
    auto i = _nelsonMap.find(id);
    return i != _nelsonMap.end() ? i->second : _emptyList;
  }

  void printError(const std::string&) const;

  std::ostream& out() const { return _out; }
  std::ostream& err() const { return _err; }
  const std::filesystem::path& dataDir() const { return _dataDir; }
  bool debug() const { return _debug; }
  const Map& kanjiNameMap() const { return _kanjiNameMap; }

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

  // 'loadStrokes' and 'loadFrequencyReadings' must be called before calling 'populate Lists' functions
  void loadStrokes(const std::filesystem::path&, bool checkDuplicates = true);
  void loadFrequencyReadings(const std::filesystem::path&);

  // populate Lists (_types datastructure)
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const DataFile&);
  void processUcd(); // should be called after processing all other types

  // 'checkStrokes' should be called after all lists are populated. If debug is enabled (-debug)
  // then this function will print any entries in _strokes that are 'Frequency' type or not found.
  // It also compares strokes that were loaded from other files to strokes in 'ucd.txt'
  void checkStrokes() const;

  // '_radicals' holds the 214 official Kanji Radicals
  RadicalData _radicals;
  // '_ucd' is used to supplement Kanji attributes like radical, meaning and reading
  UcdData _ucd;
  // '_strokes' is populated from strokes.txt and is meant to supplement jinmei kanji (file doesn't
  // have a 'Strokes' column) as well as old kanjis from both jouyou and jinmei files. This file
  // contains stroke counts followed by one or more lines each with a single kanji that has the given
  // number of strokes.
  std::map<std::string, int> _strokes;
  std::map<KanjiTypes, List> _types;
private:
  // helper functions for checking and inserting into '_kanjiNameMap'
  bool checkInsert(const Entry&);
  bool checkInsert(List&, const Entry&);

  const std::filesystem::path _dataDir;
  const bool _debug;
  std::ostream& _out;
  std::ostream& _err;

  // '_compatibilityNameMap' maps from a UCD 'compatibility' code name to a 'variation selector'
  // style name. This map only has entries for recognized kanji that were loaded with a selector.
  std::map<std::string, std::string> _compatibilityNameMap;

  // '_frequencyReadings' holds readings loaded from frequency-readings.txt - these are for Top Frequency
  // kanji that aren't part of any other group (so not Jouyou or Jinmei).
  std::map<std::string, std::string> _frequencyReadings;

  // lists of kanji corresponding to Levels, Grades, Types and Kyus (excluding the 'None' enum values)
  std::map<JlptLevels, List> _levels;
  std::map<KanjiGrades, List> _grades;
  std::map<KenteiKyus, List> _kyus;

  // Lists of kanji grouped into 5 frequency ranges: 1-500, 501-1000, 1001-1500, 1501-2000, 2001-2501.
  // The last list is one longer in order to hold the full frequency list (of 2501 kanji).
  std::array<List, FrequencyBuckets> _frequencies;

  Map _kanjiNameMap;                 // allow lookup by UTF-8 name
  std::map<std::string, List> _morohashiMap; // allow lookup by Dai Kan-Wa Jiten index number (aka Morohashi ID)
  std::map<int, List> _nelsonMap;    // allow lookup by Nelson ID

  // 'maxFrequency' is set to 1 larger than the highest frequency of any kanji put into '_kanjiNameMap'
  inline static int _maxFrequency = 0;

  inline static const List _emptyList;
  inline static const Kanji::NelsonIds _emptyNelsonIds;
};

using DataPtr = std::shared_ptr<const Data>;

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_DATA_H
