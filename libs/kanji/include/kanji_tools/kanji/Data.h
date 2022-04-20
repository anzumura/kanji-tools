#pragma once

#include <kanji_tools/kanji/Kanji.h>
#include <kanji_tools/kanji/RadicalData.h>
#include <kanji_tools/kanji/UcdData.h>
#include <kanji_tools/utils/Args.h>
#include <kanji_tools/utils/DataFile.h>
#include <kanji_tools/utils/EnumMap.h>

namespace kanji_tools {

// 'Data' provides methods used by 'Kanji' classes during loading and is the
// base class for 'KanjiData'
class Data {
public:
  using KanjiList = std::vector<KanjiPtr>;
  using KanjiMap = std::map<std::string, KanjiPtr>;
  template<typename T> using EnumList = EnumMap<T, KanjiList>;
  using Path = DataFile::Path;

  // DataArg (to specify location of 'data' dir), DebugArg and InfoArg are
  // command line options that can be passed to apps using this 'Data' class
  inline static const std::string DataArg{"-data"}, DebugArg{"-debug"},
      InfoArg{"-info"};

  // 'nextArg' is meant to be used by other classes that process command line
  // options, but also have a 'Data' class (like Quiz and Stats programs) - it
  // returns 'current + 1' if args[currentArg + 1] is not used used by this
  // 'Data' class (ie getDataDir or getDebug). If 'current + 1' is used then a
  // larger increment is returned to 'skip over' the args, for example:
  //   for (auto i{Data::nextArg(args)}; i < args.size();
  //       i = Data::nextArg(args)) { /* do something with args[i] */ }
  [[nodiscard]] static Args::Size nextArg(const Args&, Args::Size current = 0);

  // 'DebugMode' is controlled by debug/info command-line options (see above):
  //   DebugArg: sets '_debugMode' to 'Full' to print all debug output
  //   InfoArg:  sets '_debugMode' to 'Info' to print some summary debug output
  enum class DebugMode { Full, Info, None };

  static void usage(const std::string& msg) { DataFile::usage(msg); }
  static constexpr auto OrderByQualifiedName{
      [](KanjiPtr& a, KanjiPtr& b) { return a->orderByQualifiedName(*b); }};

  [[nodiscard]] static Kanji::Frequency maxFrequency();

  Data(const Path& dataDir, DebugMode, std::ostream& out = std::cout,
      std::ostream& err = std::cerr);

  Data(const Data&) = delete;
  virtual ~Data() = default;

  [[nodiscard]] auto& ucd() const { return _ucd; }
  [[nodiscard]] UcdPtr findUcd(const std::string& kanjiName) const;

  // functions used by 'Kanji' class ctors, each takes a Kanji name string
  [[nodiscard]] virtual Kanji::Frequency frequency(const std::string&) const = 0;
  [[nodiscard]] virtual JlptLevels level(const std::string&) const = 0;
  [[nodiscard]] virtual KenteiKyus kyu(const std::string&) const = 0;
  [[nodiscard]] virtual RadicalRef ucdRadical(const std::string&, UcdPtr) const;
  [[nodiscard]] virtual Ucd::Strokes ucdStrokes(
      const std::string&, UcdPtr) const;

  // 'getRadicalByName' is used by 'CustomFileKanji' ctors. It returns the
  // official Radical for the given 'radicalName' (like 二, 木, 言, etc.).
  [[nodiscard]] virtual RadicalRef getRadicalByName(
      const std::string& radicalName) const;

  [[nodiscard]] const Pinyin& getPinyin(UcdPtr) const;

  // 'getMorohashiId' returns an optional 'Dai Kan-Wa Jiten' index number (see
  // comments in scripts/parseUcdAllFlat.sh)
  [[nodiscard]] Kanji::OptString getMorohashiId(UcdPtr) const;

  // 'getNelsonIds' returns a vector of 0 or more 'Classic Nelson' ids
  Kanji::NelsonIds getNelsonIds(UcdPtr) const;

  // 'getCompatibilityName' returns the UCD compatibility code for the given
  // 'kanji' if it exists (_ucd.find method takes care of checking whether
  // 'kanji' has a variation selector).
  [[nodiscard]] Kanji::OptString getCompatibilityName(const std::string&) const;

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

  // See comment for '_frequencies' private data member for more details
  static constexpr Kanji::Frequency FrequencyBuckets{5}, FrequencyEntries{500};
  [[nodiscard]] const KanjiList& frequencies(size_t) const;
  [[nodiscard]] size_t frequencySize(size_t) const;

  [[nodiscard]] KanjiTypes getType(const std::string& name) const;

  // 'findKanjiByName' supports finding a Kanji by UTF-8 string including
  // 'variation selectors', i.e., the same result is returned for '侮︀ [4FAE
  // FE00]' and '侮 [FA30]' (a single UTF-8 compatibility kanji).
  [[nodiscard]] KanjiPtr findKanjiByName(const std::string&) const;

  // 'findKanjiByFrequency' returns the Kanji with the given 'freq' (should be a
  // value from 1 to 2501)
  [[nodiscard]] KanjiPtr findKanjiByFrequency(Kanji::Frequency freq) const;

  // 'findByMorohashiId' can return more than one Kanji. Ids are usually just
  // numeric, but they can also be a number followed by a 'P'. For example,
  // '4138' maps to 嗩 and '4138P' maps to 嘆.
  [[nodiscard]] const KanjiList& findByMorohashiId(const std::string&) const;

  // 'findKanjisByNelsonId' can return more than one Kanji. For example, 1491
  // maps to 㡡, 幮 and 𢅥.
  [[nodiscard]] const KanjiList& findByNelsonId(Kanji::NelsonId) const;

  void printError(const std::string&) const;

  [[nodiscard]] auto debug() const { return _debugMode != DebugMode::None; }
  [[nodiscard]] auto fullDebug() const { return _debugMode == DebugMode::Full; }
  [[nodiscard]] auto infoDebug() const { return _debugMode == DebugMode::Info; }

  [[nodiscard]] auto& out() const { return _out; }
  [[nodiscard]] auto& err() const { return _err; }
  [[nodiscard]] auto& dataDir() const { return _dataDir; }
  [[nodiscard]] auto& kanjiNameMap() const { return _kanjiNameMap; }

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
  void processList(const DataFile&);
  void processUcd(); // should be called after processing all other types

  // 'checkStrokes' should be called after all lists are populated. It compares
  // stroke values loaded from other files to strokes in 'ucd.txt' and prints
  // the results (if -debug is specified)
  void checkStrokes() const;

  // used by derived classes
  auto& radicals() { return _radicals; }
  auto& ucd() { return _ucd; }
  auto& types() { return _types; }
  auto& types() const { return _types; }

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

  EnumList<KanjiTypes> _types;

  // helper functions for checking and inserting into '_kanjiNameMap'
  bool checkInsert(KanjiList&, const KanjiPtr&);
  void insertSanityChecks(const Kanji&, UcdPtr) const;

  const Path _dataDir;
  const DebugMode _debugMode;
  std::ostream& _out;
  std::ostream& _err;

  // '_compatibilityMap' maps from a UCD 'compatibility' name to a 'variation
  // selector' style name. This map only has entries for recognized Kanji that
  // were loaded with a selector.
  std::map<std::string, std::string> _compatibilityMap;

  // '_frequencyReadings' holds readings loaded from frequency-readings.txt -
  // these are for Top Frequency kanji that aren't part of any other group (so
  // not Jouyou or Jinmei).
  std::map<std::string, std::string> _frequencyReadings;

  // lists of kanji per Level, Grade and Kyu (excluding the 'None' enum values)
  EnumList<JlptLevels> _levels;
  EnumList<KanjiGrades> _grades;
  EnumList<KenteiKyus> _kyus;

  // Lists of kanji grouped into 5 frequency ranges: 1-500, 501-1000, 1001-1500,
  // 1501-2000, 2001-2501. The last list is one longer in order to hold the full
  // frequency list (of 2501 kanji).
  std::array<KanjiList, FrequencyBuckets> _frequencies;

  KanjiMap _kanjiNameMap;                          // UTF-8 name to Kanji
  std::map<std::string, KanjiList> _morohashiMap;  // Dai Kan-Wa Jiten ID lookup
  std::map<Kanji::NelsonId, KanjiList> _nelsonMap; // Nelson ID lookup

  // 'maxFrequency' is set to 1 larger than the highest frequency of any Kanji
  // put into '_kanjiNameMap'
  inline static constinit Kanji::Frequency _maxFrequency;

  inline static const Kanji::NelsonIds EmptyNelsonIds;

  inline static const Path DataDir{"data"};
};

using DataPtr = std::shared_ptr<const Data>;
using DataRef = const Data&;

} // namespace kanji_tools
