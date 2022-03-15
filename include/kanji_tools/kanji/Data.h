#pragma once

#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/RadicalData.h>
#include <kanji_tools/kanji/UcdData.h>
#include <kanji_tools/utils/DataFile.h>
#include <kanji_tools/utils/EnumMap.h>

#include <optional>

namespace kanji_tools {

// 'Data' provides methods used by 'Kanji' classes during loading and is the
// base class for 'KanjiData'
class Data {
public:
  using Entry = std::shared_ptr<Kanji>;
  using OptEntry = std::optional<const Entry>;
  using List = std::vector<Entry>;
  using Map = std::map<std::string, Entry>;
  template<typename T> using EnumList = EnumMap<T, List>;

  // 'DebugMode' is controlled by command-line options:
  //   -debug: sets '_debugMode' to 'Full' to print all debug output
  //   -info:  sets '_debugMode' to 'Info' to print some summary debug output
  enum class DebugMode { Full, Info, None };

  static void usage(const std::string& msg) { DataFile::usage(msg); }
  static constexpr auto orderByQualifiedName{
    [](const Entry& a, const Entry& b) { return a->orderByQualifiedName(*b); }};

  Data(const std::filesystem::path& dataDir, DebugMode,
       std::ostream& out = std::cout, std::ostream& err = std::cerr);

  Data(const Data&) = delete;
  // operator= is not generated since there are const members
  virtual ~Data() = default;

  [[nodiscard]] auto& ucd() const { return _ucd; }
  [[nodiscard]] auto findUcd(const std::string& kanjiName) const {
    return _ucd.find(kanjiName);
  }

  // functions used by 'Kanji' classes during construction, each take kanji name
  [[nodiscard]] virtual Kanji::OptSize frequency(const std::string&) const = 0;
  [[nodiscard]] virtual JlptLevels level(const std::string&) const = 0;
  [[nodiscard]] virtual KenteiKyus kyu(const std::string&) const = 0;
  [[nodiscard]] virtual const Radical& ucdRadical(const std::string& kanjiName,
                                                  const Ucd* u) const {
    if (u) return _radicals.find(u->radical());
    // 'throw' should never happen - every 'Kanji' class instance should have
    // also exist in the data loaded from Unicode.
    throw std::domain_error("UCD entry not found: " + kanjiName);
  }

  // 'getRadicalByName' is used by 'ExtraKanji' classes during construction. It
  // returns the Radical for the given 'radicalName' (like 二, 木, 言, etc.).
  [[nodiscard]] virtual const Radical&
  getRadicalByName(const std::string& radicalName) const {
    return _radicals.find(radicalName);
  }

  // 'getPinyin' returns 'optional' since not all Kanji have a Pinyin reading
  [[nodiscard]] auto getPinyin(const Ucd* u) const {
    return u && !u->pinyin().empty() ? Kanji::OptString(u->pinyin())
                                     : std::nullopt;
  }

  // 'getMorohashiId' returns an optional 'Dai Kan-Wa Jiten' index number (see
  // comments in scripts/parseUcdAllFlat.sh)
  [[nodiscard]] auto getMorohashiId(const Ucd* u) const {
    return u && !u->morohashiId().empty() ? Kanji::OptString(u->morohashiId())
                                          : std::nullopt;
  }

  // 'getNelsonIds' returns a vector of 0 or more 'Classic Nelson' ids
  Kanji::NelsonIds getNelsonIds(const Ucd*) const;

  // 'getCompatibilityName' returns the UCD compatibility code for the given
  // 'kanjiName' if it exists (_ucd.find method takes care of checking whether
  // kanjiName has a variation selector).
  [[nodiscard]] auto getCompatibilityName(const std::string& kanjiName) const {
    const auto u{_ucd.find(kanjiName)};
    return u && u->name() != kanjiName ? Kanji::OptString(u->name())
                                       : std::nullopt;
  }

  [[nodiscard]] auto getStrokes(const std::string& kanjiName, const Ucd* u,
                                bool variant = false,
                                bool onlyUcd = false) const {
    if (!onlyUcd) {
      if (const auto i{_strokes.find(kanjiName)}; i != _strokes.end())
        return i->second;
    }
    return u ? u->getStrokes(variant) : 0U;
  }
  [[nodiscard]] auto getStrokes(const std::string& kanjiName) const {
    return getStrokes(kanjiName, findUcd(kanjiName));
  }

  // get list by KanjiType
  [[nodiscard]] auto& types(KanjiTypes t) const { return _types[t]; }
  [[nodiscard]] auto typeSize(KanjiTypes t) const { return types(t).size(); }

  // get list by KanjiGrade
  [[nodiscard]] auto& grades(KanjiGrades g) const { return _grades[g]; }
  [[nodiscard]] auto gradeSize(KanjiGrades g) const { return grades(g).size(); }

  // get list by JLPT Level
  [[nodiscard]] auto& levels(JlptLevels l) const { return _levels[l]; }
  [[nodiscard]] auto levelSize(JlptLevels l) const { return levels(l).size(); }

  // get list by Kentei Kyu
  [[nodiscard]] auto& kyus(KenteiKyus k) const { return _kyus[k]; }
  [[nodiscard]] auto kyuSize(KenteiKyus k) const { return kyus(k).size(); }

  // See comment for '_frequencies' private data member for more details about
  // frequency lists
  enum Values { FrequencyBuckets = 5, FrequencyBucketEntries = 500 };
  [[nodiscard]] auto& frequencies(size_t f) const {
    return f < FrequencyBuckets ? _frequencies[f] : BaseEnumMap<List>::Empty;
  }
  [[nodiscard]] auto frequencySize(size_t f) const {
    return frequencies(f).size();
  }

  [[nodiscard]] KanjiTypes getType(const std::string& name) const;

  // 'findKanjiByName' supports finding a Kanji by UTF-8 string including
  // 'variation selectors', i.e., the same result is returned for '侮︀ [4FAE
  // FE00]' and '侮 [FA30]' (a single UTF-8 compatibility kanji).
  [[nodiscard]] OptEntry findKanjiByName(const std::string& s) const {
    const auto i{_compatibilityMap.find(s)};
    if (const auto j{
          _kanjiNameMap.find(i != _compatibilityMap.end() ? i->second : s)};
        j != _kanjiNameMap.end())
      return j->second;
    return {};
  }

  // 'findKanjiByFrequency' returns the Kanji with the given 'frequency'
  // (should be a value from 1 to 2501)
  [[nodiscard]] OptEntry findKanjiByFrequency(size_t frequency) const {
    if (frequency < 1 || frequency >= _maxFrequency) return {};
    auto bucket{--frequency / FrequencyBucketEntries};
    if (bucket == FrequencyBuckets)
      --bucket; // last bucket contains FrequencyBucketEntries + 1
    return _frequencies[bucket][frequency - bucket * FrequencyBucketEntries];
  }

  // 'findKanjisByMorohashiId' can return more than one entry. The ids are
  // usually plain just numeric, but they can also be an index number followed
  // by a 'P'. For example, '4138' maps to 嗩 and '4138P' maps to 嘆.
  [[nodiscard]] auto& findKanjisByMorohashiId(const std::string& id) const {
    const auto i{_morohashiMap.find(id)};
    return i != _morohashiMap.end() ? i->second : BaseEnumMap<List>::Empty;
  }

  // 'findKanjisByNelsonId' can return more than one entry. For example, 1491
  // maps to 㡡, 幮 and 𢅥.
  [[nodiscard]] auto& findKanjisByNelsonId(size_t id) const {
    const auto i{_nelsonMap.find(id)};
    return i != _nelsonMap.end() ? i->second : BaseEnumMap<List>::Empty;
  }

  void printError(const std::string&) const;

  [[nodiscard]] auto debug() const { return _debugMode != DebugMode::None; }
  [[nodiscard]] auto fullDebug() const { return _debugMode == DebugMode::Full; }
  [[nodiscard]] auto infoDebug() const { return _debugMode == DebugMode::Info; }

  [[nodiscard]] auto& out() const { return _out; }
  [[nodiscard]] auto& err() const { return _err; }
  [[nodiscard]] auto& dataDir() const { return _dataDir; }
  [[nodiscard]] auto dataDir(const std::filesystem::path& dir,
                             const std::string& file) const {
    return _dataDir / dir / file;
  }
  [[nodiscard]] auto& kanjiNameMap() const { return _kanjiNameMap; }

  // 'log' can be used for putting a standard prefix to output messages (used
  // for some debug messages)
  [[nodiscard]] auto& log(bool heading = false) const {
    return heading ? _out << ">>>\n>>> " : _out << ">>> ";
  }

  [[nodiscard]] static auto maxFrequency() { return _maxFrequency; }

  // 'nextArg' will return 'currentArg + 1' if argv[currentArg + 1] is not
  // used by this class (ie getDataDir or getDebug). If currentArg + 1 is used
  // by this class then a larger increment is returned to 'skip over' the
  // args, for example:
  //   for (auto i{Data::nextArg(argc, argv)}; i < argc;
  //        i = Data::nextArg(argc, argv, i))
  [[nodiscard]] static size_t nextArg(size_t argc, const char* const* argv,
                                      size_t currentArg = 0);
protected:
  // 'getDataDir' looks for a directory called 'data' containing 'jouyou.txt'
  // based on checking directories starting at 'argv[0]' (the program name)
  // and working up parent directories. Therefore argc must be at least 1.
  // '-data' followed by a directory name can also be used as an override.
  [[nodiscard]] static std::filesystem::path getDataDir(size_t argc,
                                                        const char** argv);

  // 'getDebugMode' looks for '-debug' or '-info' flags in 'argv' list (see
  // 'DebugMode' above)
  [[nodiscard]] static DebugMode getDebugMode(size_t argc, const char** argv);

  // 'loadStrokes' and 'loadFrequencyReadings' must be called before calling
  // 'populate Lists' functions
  void loadStrokes(const std::filesystem::path&, bool checkDuplicates = true);
  void loadFrequencyReadings(const std::filesystem::path&);

  // populate Lists (_types datastructure)
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const DataFile&);
  void processUcd(); // should be called after processing all other types

  // 'checkStrokes' should be called after all lists are populated. If debug
  // is enabled (-debug) then this function will print any entries in _strokes
  // that are 'Frequency' type or not found. It also compares strokes that
  // were loaded from other files to strokes in 'ucd.txt'
  void checkStrokes() const;

  // '_radicals' holds the 214 official Kanji Radicals
  RadicalData _radicals;

  // '_ucd' is used to get Kanji attributes like radical, meaning and reading
  UcdData _ucd;

  // '_strokes' is populated from strokes.txt and supplements jinmei Kanji
  // (file doesn't have 'Strokes' column) as well as old Kanji from jouyou and
  // jinmei files. This file contains stroke counts followed by one or more
  // lines each with a single kanji that has the given number of strokes.
  std::map<std::string, size_t> _strokes;

  EnumList<KanjiTypes> _types;
private:
  // 'populateLinkedKanji' is called by 'populateJouyou' function. It reads
  // data from 'linked-jinmei.txt' and creates either a LinkedJinmei or a
  // LinkedOld kanji for each entry.
  void populateLinkedKanji();

  // helper functions for checking and inserting into '_kanjiNameMap'
  bool checkInsert(const Entry&);
  bool checkInsert(List&, const Entry&);
  void insertSanityChecks(const Entry&) const;

  const std::filesystem::path _dataDir;
  const DebugMode _debugMode;
  std::ostream& _out;
  std::ostream& _err;

  // '_compatibilityMap' maps from a UCD 'compatibility' code name to a
  // 'variation selector' style name. This map only has entries for recognized
  // kanji that were loaded with a selector.
  std::map<std::string, std::string> _compatibilityMap;

  // '_frequencyReadings' holds readings loaded from frequency-readings.txt -
  // these are for Top Frequency kanji that aren't part of any other group (so
  // not Jouyou or Jinmei).
  std::map<std::string, std::string> _frequencyReadings;

  // lists of kanji per Level, Grade and Kyu (excluding the 'None' enum
  // values)
  EnumList<JlptLevels> _levels;
  EnumList<KanjiGrades> _grades;
  EnumList<KenteiKyus> _kyus;

  // Lists of kanji grouped into 5 frequency ranges: 1-500, 501-1000,
  // 1001-1500, 1501-2000, 2001-2501. The last list is one longer in order to
  // hold the full frequency list (of 2501 kanji).
  std::array<List, FrequencyBuckets> _frequencies;

  Map _kanjiNameMap;                         // lookup by UTF-8 name
  std::map<std::string, List> _morohashiMap; // lookup by Dai Kan-Wa Jiten ID
  std::map<size_t, List> _nelsonMap;         // lookup by Nelson ID

  // 'maxFrequency' is set to 1 larger than the highest frequency of any kanji
  // put into '_kanjiNameMap'
  inline static constinit size_t _maxFrequency;

  inline static const std::string dataArg{"-data"}, debugArg{"-debug"},
    infoArg{"-info"};
  inline static const Kanji::NelsonIds _emptyNelsonIds;
};

using DataPtr = std::shared_ptr<const Data>;

} // namespace kanji_tools
