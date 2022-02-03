#ifndef KANJI_TOOLS_KANJI_DATA_H
#define KANJI_TOOLS_KANJI_DATA_H

#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/RadicalData.h>
#include <kanji_tools/kanji/UcdData.h>
#include <kanji_tools/utils/DataFile.h>

#include <optional>

namespace kanji_tools {

// 'Data' provides methods used by 'Kanji' classes during loading and is the base class for KanjiData
class Data {
public:
  using Entry = std::shared_ptr<Kanji>;
  using OptEntry = std::optional<const Entry>;
  using List = std::vector<Entry>;
  using Map = std::map<std::string, Entry>;

  // 'DebugMode' is controlled by command-line options:
  //   -debug: sets '_debugMode' to 'Full' to print all debug output
  //   -info:  sets '_debugMode' to 'Info' to print some summary debug output
  enum class DebugMode { Full, Info, None };

  static void usage(const std::string& msg) { DataFile::usage(msg); }
  inline static const auto orderByQualifiedName = [](const Entry& a, const Entry& b) {
    return a->orderByQualifiedName(*b);
  };

  Data(const std::filesystem::path& dataDir, DebugMode, std::ostream& out = std::cout, std::ostream& err = std::cerr);
  virtual ~Data() = default;
  Data(const Data&) = delete;

  [[nodiscard]] auto& ucd() const { return _ucd; }
  [[nodiscard]] auto findUcd(const std::string& kanjiName) const { return _ucd.find(kanjiName); }

  // Functions used by 'Kanji' classes during construction, each takes a kanji name.
  [[nodiscard]] virtual Kanji::OptInt getFrequency(const std::string&) const = 0;
  [[nodiscard]] virtual JlptLevels getLevel(const std::string&) const = 0;
  [[nodiscard]] virtual KenteiKyus getKyu(const std::string&) const = 0;
  [[nodiscard]] virtual const Radical& ucdRadical(const std::string& kanjiName, const Ucd* u) const {
    if (u) return _radicals.find(u->radical());
    // 'throw' should never happen - every 'Kanji' class instance should have also exist in the
    // data loaded from Unicode.
    throw std::domain_error("UCD entry not found: " + kanjiName);
  }

  // 'getRadicalByName' is used by 'ExtraKanji' classes during construction. It will return
  // the Radical for the given 'radicalName' (like 二, 木, 言, etc.).
  [[nodiscard]] virtual const Radical& getRadicalByName(const std::string& radicalName) const {
    return _radicals.find(radicalName);
  }

  // 'getPinyin' returns an optional string since not all Kanji have a Pinyin reading.
  [[nodiscard]] auto getPinyin(const Ucd* u) const {
    return u && !u->pinyin().empty() ? Kanji::OptString(u->pinyin()) : std::nullopt;
  }

  // 'getMorohashiId' returns an optional 'Dai Kan-Wa Jiten' index number (see comments in scripts/parseUcdAllFlat.sh)
  [[nodiscard]] auto getMorohashiId(const Ucd* u) const {
    return u && !u->morohashiId().empty() ? Kanji::OptString(u->morohashiId()) : std::nullopt;
  }

  // 'getNelsonIds' returns a vector of 0 or more 'Classic Nelson' ids
  Kanji::NelsonIds getNelsonIds(const Ucd*) const;

  // 'getCompatibilityName' returns the UCD compatibility code for the given 'kanjiName' if it
  // exists (_ucd.find method takes care of checking whether kanjiName has a variation selector).
  [[nodiscard]] auto getCompatibilityName(const std::string& kanjiName) const {
    const auto u = _ucd.find(kanjiName);
    return u && u->name() != kanjiName ? Kanji::OptString(u->name()) : std::nullopt;
  }

  [[nodiscard]] auto getStrokes(const std::string& kanjiName, const Ucd* u, bool variant = false,
                                bool onlyUcd = false) const {
    if (!onlyUcd) {
      const auto i = _strokes.find(kanjiName);
      if (i != _strokes.end()) return i->second;
    }
    return u ? u->getStrokes(variant) : 0;
  }
  [[nodiscard]] auto getStrokes(const std::string& kanjiName) const {
    return getStrokes(kanjiName, findUcd(kanjiName));
  }

  // get kanji lists
  [[nodiscard]] auto& jouyouKanji() const { return _types.at(KanjiTypes::Jouyou); }
  [[nodiscard]] auto& jinmeiKanji() const { return _types.at(KanjiTypes::Jinmei); }
  [[nodiscard]] auto& linkedJinmeiKanji() const { return _types.at(KanjiTypes::LinkedJinmei); }
  [[nodiscard]] auto& linkedOldKanji() const { return _types.at(KanjiTypes::LinkedOld); }
  [[nodiscard]] auto& frequencyKanji() const { return _types.at(KanjiTypes::Frequency); }
  [[nodiscard]] auto& extraKanji() const { return _types.at(KanjiTypes::Extra); }

  // get list by KanjiType
  [[nodiscard]] auto& typeList(KanjiTypes type) const {
    const auto i = _types.find(type);
    return i != _types.end() ? i->second : _emptyList;
  }
  [[nodiscard]] auto typeTotal(KanjiTypes type) const { return typeList(type).size(); }
  [[nodiscard]] KanjiTypes getType(const std::string& name) const;

  // get list by KanjiGrade
  [[nodiscard]] auto& gradeList(KanjiGrades grade) const {
    const auto i = _grades.find(grade);
    return i != _grades.end() ? i->second : _emptyList;
  }
  [[nodiscard]] auto gradeTotal(KanjiGrades grade) const { return gradeList(grade).size(); }

  // get list by JLPT Level
  [[nodiscard]] auto& levelList(JlptLevels level) const {
    const auto i = _levels.find(level);
    return i != _levels.end() ? i->second : _emptyList;
  }
  [[nodiscard]] auto levelTotal(JlptLevels level) const { return levelList(level).size(); }

  // get list by Kentei Kyu
  [[nodiscard]] auto& kyuList(KenteiKyus kyu) const {
    const auto i = _kyus.find(kyu);
    return i != _kyus.end() ? i->second : _emptyList;
  }
  [[nodiscard]] auto kyuTotal(KenteiKyus kyu) const { return kyuList(kyu).size(); }

  // See comment for '_frequencies' private data member for more details about frequency lists
  enum Values { FrequencyBuckets = 5, FrequencyBucketEntries = 500 };
  [[nodiscard]] auto& frequencyList(int range) const {
    return range >= 0 && range < FrequencyBuckets ? _frequencies[range] : _emptyList;
  }
  [[nodiscard]] auto frequencyTotal(int range) const { return frequencyList(range).size(); }

  // 'findKanjiByName' supports finding a Kanji by UTF-8 string including 'variation selectors', i.e., the
  // same result is returned for '侮︀ [4FAE FE00]' and '侮 [FA30]' (a single UTF-8 compatibility kanji).
  [[nodiscard]] OptEntry findKanjiByName(const std::string& s) const {
    const auto i = _compatibilityMap.find(s);
    const auto j = _kanjiNameMap.find(i != _compatibilityMap.end() ? i->second : s);
    if (j == _kanjiNameMap.end()) return {};
    return j->second;
  }

  // 'findKanjiByFrequency' returns the Kanji with the given 'frequency' (should be a value from 1 to 2501)
  [[nodiscard]] OptEntry findKanjiByFrequency(int frequency) const {
    if (frequency < 1 || frequency >= _maxFrequency) return {};
    auto bucket = --frequency / FrequencyBucketEntries;
    if (bucket == FrequencyBuckets) --bucket; // last bucket contains FrequencyBucketEntries + 1
    return _frequencies[bucket][frequency - bucket * FrequencyBucketEntries];
  }

  // 'findKanjisByMorohashiId' can return more than one entry. The ids are usually plain just numeric, but they can
  // also be an index number followed by a 'P'. For example, '4138' maps to 嗩 and '4138P' maps to 嘆.
  [[nodiscard]] auto& findKanjisByMorohashiId(const std::string& id) const {
    const auto i = _morohashiMap.find(id);
    return i != _morohashiMap.end() ? i->second : _emptyList;
  }

  // 'findKanjisByNelsonId' can return more than one entry. For example, 1491 maps to 㡡, 幮 and 𢅥.
  [[nodiscard]] auto& findKanjisByNelsonId(int id) const {
    const auto i = _nelsonMap.find(id);
    return i != _nelsonMap.end() ? i->second : _emptyList;
  }

  void printError(const std::string&) const;

  [[nodiscard]] auto debug() const { return _debugMode != DebugMode::None; }
  [[nodiscard]] auto fullDebug() const { return _debugMode == DebugMode::Full; }
  [[nodiscard]] auto infoDebug() const { return _debugMode == DebugMode::Info; }

  [[nodiscard]] auto& out() const { return _out; }
  [[nodiscard]] auto& err() const { return _err; }
  [[nodiscard]] auto& dataDir() const { return _dataDir; }
  [[nodiscard]] auto& kanjiNameMap() const { return _kanjiNameMap; }

  // 'log' can be used for putting a standard prefix to output messages (used for some debug messages)
  [[nodiscard]] auto& log(bool heading = false) const { return heading ? _out << ">>>\n>>> " : _out << ">>> "; }

  [[nodiscard]] static auto maxFrequency() { return _maxFrequency; }

  // 'nextArg' will return 'currentArg + 1' if argv[currentArg + 1] is not used by this
  // class (ie getDataDir or getDebug). If currentArg + 1 is used by this class then
  // a larger increment is returned to 'skip over' the args, for example:
  //     for (auto i = Data::nextArg(argc, argv); i < argc; i = Data::nextArg(argc, argv, i))
  [[nodiscard]] static size_t nextArg(size_t argc, const char* const* argv, size_t currentArg = 0);
protected:
  // 'getDataDir' looks for a directory called 'data' containing 'jouyou.txt' based on
  // checking directories starting at 'argv[0]' (the program name) and working up parent
  // directories. Therefore argc must be at least 1. '-data' followed by a directory
  // name can also be used as an override.
  [[nodiscard]] static std::filesystem::path getDataDir(size_t argc, const char** argv);

  // 'getDebugMode' looks for '-debug' or '-info' flags in 'argv' list (see 'DebugMode' above)
  [[nodiscard]] static DebugMode getDebugMode(size_t argc, const char** argv);

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
  // 'populateLinkedKanji' is called by 'populateJouyou' function. It reads data from 'linked-jinmei.txt'
  // and creates either a LinkedJinmei or a LinkedOld kanji for each entry.
  void populateLinkedKanji();

  // helper functions for checking and inserting into '_kanjiNameMap'
  bool checkInsert(const Entry&);
  bool checkInsert(List&, const Entry&);
  void insertSanityChecks(const Entry&) const;

  const std::filesystem::path _dataDir;
  const DebugMode _debugMode;
  std::ostream& _out;
  std::ostream& _err;

  // '_compatibilityMap' maps from a UCD 'compatibility' code name to a 'variation selector'
  // style name. This map only has entries for recognized kanji that were loaded with a selector.
  std::map<std::string, std::string> _compatibilityMap;

  // '_frequencyReadings' holds readings loaded from frequency-readings.txt - these are for Top Frequency
  // kanji that aren't part of any other group (so not Jouyou or Jinmei).
  std::map<std::string, std::string> _frequencyReadings;

  // lists of kanji corresponding to Levels, Grades and Kyus (excluding the 'None' enum values)
  std::map<JlptLevels, List> _levels;
  std::map<KanjiGrades, List> _grades;
  std::map<KenteiKyus, List> _kyus;

  // Lists of kanji grouped into 5 frequency ranges: 1-500, 501-1000, 1001-1500, 1501-2000, 2001-2501.
  // The last list is one longer in order to hold the full frequency list (of 2501 kanji).
  std::array<List, FrequencyBuckets> _frequencies;

  Map _kanjiNameMap;                         // allow lookup by UTF-8 name
  std::map<std::string, List> _morohashiMap; // allow lookup by Dai Kan-Wa Jiten index number (aka Morohashi ID)
  std::map<int, List> _nelsonMap;            // allow lookup by Nelson ID

  // 'maxFrequency' is set to 1 larger than the highest frequency of any kanji put into '_kanjiNameMap'
  inline static int _maxFrequency = 0;

  inline static const std::string dataArg = "-data", debugArg = "-debug", infoArg = "-info";
  inline static const List _emptyList;
  inline static const Kanji::NelsonIds _emptyNelsonIds;
};

using DataPtr = std::shared_ptr<const Data>;

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_DATA_H
