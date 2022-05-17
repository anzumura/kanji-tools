#pragma once

#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/KanjiListFile.h>
#include <kanji_tools/kanji/RadicalData.h>
#include <kanji_tools/kanji/UcdData.h>
#include <kanji_tools/utils/Args.h>
#include <kanji_tools/utils/EnumMap.h>

namespace kanji_tools {

// 'KanjiData' provides helper functions used during 'Kanji' loading and holds
// data such as Kanji radicals, 'UCD' data and maps of enum values to lists of
// Kanji. This is the base class of 'RealKanjiData' (as well as 'MockKanjiData'
// and 'TestKanjiData').
class KanjiData {
public:
  using List = std::vector<KanjiPtr>;
  using Map = std::map<String, KanjiPtr>;
  template<typename T> using KanjiEnumMap = EnumMap<T, List>;
  using Path = KanjiListFile::Path;

  // DataArg (specify location of 'data' dir), DebugArg and InfoArg are command
  // line options that can be passed to apps using this 'KanjiData' class
  inline static const String DataArg{"-data"}, DebugArg{"-debug"},
      InfoArg{"-info"};

  // 'nextArg' is meant to be used by other classes that process command line
  // options, but also have a 'KanjiData' class (like Quiz and Stats programs) -
  // it returns 'current + 1' if args[currentArg + 1] is not used used by this
  // class. If 'current + 1' would be used by this class (like '-data', '-info',
  // etc.) then a larger value is returned to 'skip over' the args, for example:
  //   for (auto i{KanjiData::nextArg(args)}; i < args.size();
  //       i = KanjiData::nextArg(args)) { /* do something with args[i] */ }
  [[nodiscard]] static Args::Size nextArg(const Args&, Args::Size current = 0);

  // 'DebugMode' is controlled by debug/info command-line options (see above):
  //   DebugArg: sets '_debugMode' to 'Full' to print all debug output
  //   InfoArg:  sets '_debugMode' to 'Info' to print some summary debug output
  enum class DebugMode { Full, Info, None };

  static void usage(const String& msg) { KanjiListFile::usage(msg); }
  static constexpr auto OrderByQualifiedName{
      [](KanjiPtr& a, KanjiPtr& b) { return a->orderByQualifiedName(*b); }};

  [[nodiscard]] static Kanji::Frequency maxFrequency();

  // 'getPinyin' is a helper function to return pinyin value from the UcdPtr
  // if the pointer is non-null, otherwise an empty value is returned.
  [[nodiscard]] static const Pinyin& getPinyin(UcdPtr);

  // 'getMorohashiId' returns a 'Dai Kan-Wa Jiten' index number (see comments in
  // scripts/parseUcdAllFlat.sh)
  [[nodiscard]] static const MorohashiId& getMorohashiId(UcdPtr);

  // 'getNelsonIds' returns a vector of 0 or more 'Classic Nelson' ids
  [[nodiscard]] static Kanji::NelsonIds getNelsonIds(UcdPtr);

  KanjiData(const Path& dataDir, DebugMode, std::ostream& = std::cout,
      std::ostream& = std::cerr);

  KanjiData(const KanjiData&) = delete;
  virtual ~KanjiData() = default;

  [[nodiscard]] auto& ucd() const { return _ucd; }
  [[nodiscard]] UcdPtr findUcd(const String& kanjiName) const;

  // functions used by 'Kanji' class ctors, each takes a Kanji name string
  [[nodiscard]] virtual Kanji::Frequency frequency(const String&) const = 0;
  [[nodiscard]] virtual JlptLevels level(const String&) const = 0;
  [[nodiscard]] virtual KenteiKyus kyu(const String&) const = 0;
  [[nodiscard]] virtual RadicalRef ucdRadical(const String&, UcdPtr) const;
  [[nodiscard]] virtual Strokes ucdStrokes(const String&, UcdPtr) const;

  // 'getRadicalByName' is used by 'CustomFileKanji' ctors. It returns the
  // official Radical for the given string (like 二, 木, 言, etc.).
  [[nodiscard]] virtual RadicalRef getRadicalByName(const String&) const;

  // 'getCompatibilityName' returns the UCD compatibility code for the given
  // 'kanji' if it exists (_ucd.find method takes care of checking whether
  // 'kanji' has a variation selector).
  [[nodiscard]] Kanji::OptString getCompatibilityName(const String&) const;

  [[nodiscard]] auto& types() const { return _types; }
  [[nodiscard]] auto& grades() const { return _grades; }
  [[nodiscard]] auto& levels() const { return _levels; }
  [[nodiscard]] auto& kyus() const { return _kyus; }

  // See comment for '_frequencies' private data member for more details
  [[nodiscard]] const List& frequencyList(size_t) const;

  [[nodiscard]] KanjiTypes getType(const String& name) const;

  // 'findKanjiByName' supports finding a Kanji by UTF-8 string including
  // 'variation selectors', i.e., the same result is returned for '侮︀ [4FAE
  // FE00]' and '侮 [FA30]' (a single UTF-8 compatibility kanji).
  [[nodiscard]] KanjiPtr findKanjiByName(const String&) const;

  // 'findKanjiByFrequency' returns the Kanji with the given 'freq' (should be a
  // value from 1 to 2501)
  [[nodiscard]] KanjiPtr findKanjiByFrequency(Kanji::Frequency freq) const;

  // 'findByMorohashiId' can return more than one Kanji. Ids are usually just
  // numeric, but they can also be a number followed by a 'P'. For example,
  // '4138' maps to 嗩 and '4138P' maps to 嘆.
  [[nodiscard]] const List& findByMorohashiId(const MorohashiId&) const;
  [[nodiscard]] const List& findByMorohashiId(const String&) const;

  // 'findKanjisByNelsonId' can return more than one Kanji. For example, 1491
  // maps to 㡡, 幮 and 𢅥.
  [[nodiscard]] const List& findByNelsonId(Kanji::NelsonId) const;

  void printError(const String&) const;

  [[nodiscard]] auto debug() const { return _debugMode != DebugMode::None; }
  [[nodiscard]] auto fullDebug() const { return _debugMode == DebugMode::Full; }
  [[nodiscard]] auto infoDebug() const { return _debugMode == DebugMode::Info; }

  [[nodiscard]] auto& out() const { return _out; }
  [[nodiscard]] auto& err() const { return _err; }
  [[nodiscard]] auto& dataDir() const { return _dataDir; }
  [[nodiscard]] auto& kanjiNameMap() const { return _nameMap; }

  // 'log' can be used for putting a standard prefix to output messages (used
  // for some debug messages)
  [[nodiscard]] std::ostream& log(bool heading = false) const;
protected:
  // 'getDataDir' looks for a directory called 'data' containing expected number
  // of .txt files based on checking directories starting at 'current dir' and
  // working up parent directories (if this fails and 'args' is non-empty then
  // search up based on args[0]). '-data' followed by a directory name can also
  // be used as an override.
  [[nodiscard]] static Path getDataDir(const Args&);

  // 'getDebugMode' looks for 'DebugArg' or 'InfoArg' flags in 'args' list (see
  // 'DebugMode' above)
  [[nodiscard]] static DebugMode getDebugMode(const Args&);

  // 'loadStrokes' and 'loadFrequencyReadings' must be called before calling
  // 'populate Lists' functions
  void loadStrokes(const Path&, bool checkDuplicates = true);
  void loadFrequencyReadings(const Path&);

  void populateJouyou();
  // 'populateLinkedKanji' should be called immediately after 'populateJouyou'.
  // This function creates a LinkedJinmei Kanji for each entry in the file (each
  // line should start with a Jouyou Kanji). It then creates LinkedOld Kanji for
  // all Jouyou Kanji 'oldNames' that aren't already LinkedJinmei.
  void populateLinkedKanji(const Path&);
  void populateJinmei();
  void populateExtra();
  void processList(const KanjiListFile&);
  void processUcd(); // should be called after processing all other types

  // 'checkStrokes' should be called after all lists are populated. It compares
  // stroke values loaded from other files to strokes in 'ucd.txt' and prints
  // the results (if -debug is specified)
  void checkStrokes() const;

  // used by derived classes
  [[nodiscard]] auto& radicals() { return _radicals; }
  [[nodiscard]] auto& ucd() { return _ucd; }
  [[nodiscard]] auto& types() { return _types; }

  // checkInsert is non-private to help support testing
  bool checkInsert(const KanjiPtr&, UcdPtr = {});
private:
  using OptPath = std::optional<Path>;

  [[nodiscard]] static OptPath searchUpForDataDir(Path);
  [[nodiscard]] static bool isValidDataDir(const Path&);

  // '_radicals' holds the 214 official Kanji Radicals
  RadicalData _radicals;

  // '_ucd' is used by 'Kanji' class ctors to get 'pinyin', 'morohashiId' and
  // 'nelsonIds' attributes. It also provides 'radical', 'strokes', 'meaning'
  // and 'reading' when needed (mostly by non-CustomFileKanji classes).
  UcdData _ucd;

  // helper functions for checking and inserting into '_nameMap'
  bool checkInsert(List&, const KanjiPtr&);
  void insertSanityChecks(const Kanji&, UcdPtr) const;

  const Path _dataDir;
  const DebugMode _debugMode;
  std::ostream& _out;
  std::ostream& _err;

  // '_compatibilityMap' maps from a UCD 'compatibility' name to a 'variation
  // selector' style name. This map only has entries for recognized Kanji that
  // were loaded with a selector.
  std::map<String, String> _compatibilityMap;

  // '_frequencyReadings' holds readings loaded from frequency-readings.txt -
  // these are for Top Frequency kanji that aren't part of any other group (so
  // not Jouyou or Jinmei).
  std::map<String, String> _frequencyReadings;

  // each 'EnumMap' has a Kanji list per enum value (excluding 'None' values)
  KanjiEnumMap<KanjiTypes> _types;
  KanjiEnumMap<KanjiGrades> _grades;
  KanjiEnumMap<JlptLevels> _levels;
  KanjiEnumMap<KenteiKyus> _kyus;

  // '_frequencies' holds lists of Kanji grouped into 5 frequency ranges: each
  // list has 500 entries except the last which has 501 (for a total of 2501)
  static constexpr Kanji::Frequency FrequencyBuckets{5}, FrequencyEntries{500};
  std::array<List, FrequencyBuckets> _frequencies;

  Map _nameMap;                               // UTF-8 name map to one Kanji
  std::map<MorohashiId, List> _morohashiMap;  // Dai Kan-Wa Jiten ID lookup
  std::map<Kanji::NelsonId, List> _nelsonMap; // Nelson ID lookup

  // 'maxFrequency' is set to 1 larger than the 'frequency' of any Kanji added
  // to '_nameMap' (should end being '2502' after all Kanji have been loaded)
  inline static constinit Kanji::Frequency _maxFrequency;
};

using KanjiDataPtr = std::shared_ptr<const KanjiData>;

} // namespace kanji_tools
