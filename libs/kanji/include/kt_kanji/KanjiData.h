#pragma once

#include <kt_kanji/Kanji.h>
#include <kt_kanji/KanjiListFile.h>
#include <kt_kanji/RadicalData.h>
#include <kt_kanji/UcdData.h>
#include <kt_utils/Args.h>
#include <kt_utils/EnumMap.h>

namespace kanji_tools { /// \kanji_group{KanjiData}
/// KanjiData class used for loading and finding Kanji

/// abstract class used for loading and finding Kanji \kanji{KanjiData}
///
/// This class also provides some generic functionality finding 'data' directory
/// processing some command-line arguments related to debugging.
class KanjiData {
public:
  /// type of debug output to print, can be set by command-line args
  enum class DebugMode {
    Full, ///< print all debug info and then exit
    Info, ///< print summary info and then exit
    None  ///< run normally
  };

  using List = std::vector<KanjiPtr>;
  using Map = std::map<String, KanjiPtr>;
  using Path = KanjiListFile::Path;

  inline static const String DataArg{"-data"}, ///< arg to specify 'data' dir
      DebugArg{"-debug"},                      ///< arg for 'Full' #DebugMode
      InfoArg{"-info"};                        ///< arg for 'Info' #DebugMode

  /// top 2501 frequency (most commonly occurring) Kanji are grouped into 10
  /// 'buckets', each having 250 entries except the last which has 251 @{
  static constexpr Kanji::Frequency FrequencyBuckets{10}, FrequencyEntries{250};
  ///@}

  /// get the next arg that would not be used by KanjiData class
  /// \details this function is meant to be used by other classes that process
  /// command-line options, but also have a KanjiData class (like Quiz and Stats
  /// programs) - it returns `current + 1` if `args[currentArg + 1]` is not by
  /// this class. If `current + 1` would be used (like '-data', '-info', etc.)
  /// then a larger value is returned to 'skip over' the args. For example:
  /// \code
  ///   for (auto i{KanjiData::nextArg(args)}; i < args.size();
  ///       i = KanjiData::nextArg(args)) { /* do something with args[i] */ }
  /// \endcode
  [[nodiscard]] static Args::Size nextArg(const Args&, Args::Size current = 0);

  /// called for fatal problems with command-line args or loading initial data
  /// \param msg the error message
  /// \throw DomainError containing `msg`
  static void usage(const String& msg);

  /// lambda to sort shared Kanji pointers by 'qualified name' (see Kanji.h)
  static constexpr auto OrderByQualifiedName{
      [](const KanjiPtr& a, const KanjiPtr& b) {
        return a->orderByQualifiedName(*b);
      }};

  /// lambda to sort shared Kanji pointers by 'strokes' (see Kanji.h)
  static constexpr auto OrderByStrokes{
      [](const KanjiPtr& a, const KanjiPtr& b) {
        return a->orderByStrokes(*b);
      }};

  /// return `highest frequency + 1` out of all the currently loaded Kanji
  /// \details 'frequency' numbers start at `1` which means 'the most frequent'
  [[nodiscard]] static Kanji::Frequency maxFrequency() noexcept;

  /// return `u->pinyin()` or an empty value if `u` is null
  [[nodiscard]] static const Pinyin& getPinyin(UcdPtr u) noexcept;

  /// return `u->morohashiId()' or an empty id if `u` is null
  [[nodiscard]] static const MorohashiId& getMorohashiId(UcdPtr) noexcept;

  /// return list of 'Classic Nelson' ids from `u` or empty list if `u` is null
  [[nodiscard]] static Kanji::NelsonIds getNelsonIds(UcdPtr u);

  KanjiData(const KanjiData&) = delete; ///< deleted copy ctor
  virtual ~KanjiData() = default;       ///< default dtor

  /// return const ref to the UcdData object
  [[nodiscard]] auto& ucd() const noexcept { return _ucd; }

  /// return a pointer to a Ucd object for `kanjiName` or nullptr if not found
  [[nodiscard]] UcdPtr findUcd(const String& kanjiName) const;

  /// used by Kanji class ctors, each takes a Kanji name String @{
  [[nodiscard]] virtual Kanji::Frequency frequency(const String&) const = 0;
  [[nodiscard]] virtual JlptLevels level(const String&) const = 0;
  [[nodiscard]] virtual KenteiKyus kyu(const String&) const = 0;
  [[nodiscard]] virtual RadicalRef ucdRadical(const String&, UcdPtr) const;
  [[nodiscard]] virtual Strokes ucdStrokes(const String&, UcdPtr) const;
  ///@}

  /// used by NumberedKanji ctors to get a Radical for the given String
  [[nodiscard]] virtual RadicalRef getRadicalByName(const String&) const;

  /// return the UCD compatibility code for `kanji` if it exists
  [[nodiscard]] Kanji::OptString getCompatibilityName(
      const String& kanji) const;

  [[nodiscard]] auto& types() const { return _types; }
  [[nodiscard]] auto& grades() const { return _grades; }
  [[nodiscard]] auto& levels() const { return _levels; }
  [[nodiscard]] auto& kyus() const { return _kyus; }

  /// get list of Kanji for `bucket` see for #FrequencyBuckets for more details
  [[nodiscard]] const List& frequencyList(size_t bucket) const;

  [[nodiscard]] KanjiTypes getType(const String& name) const;

  /// find Kanji by name including 'variation selectors', i.e., same value is
  /// returned for '侮︀ [4FAE FE00]' and '侮 [FA30]' (compatibility Kanji).
  [[nodiscard]] KanjiPtr findByName(const String&) const;

  /// find Kanji with the given `freq` (should be a value from 1 to 2501)
  [[nodiscard]] KanjiPtr findByFrequency(Kanji::Frequency freq) const;

  /// return a list of Kanji for Morohashi ID `id`
  /// \details Ids are usually just numeric, but can also be a number followed
  /// by a 'P'. For example, '4138' maps to 嗩 and '4138P' maps to 嘆. @{
  [[nodiscard]] const List& findByMorohashiId(const MorohashiId& id) const;
  [[nodiscard]] const List& findByMorohashiId(const String& id) const; ///@}

  /// return a list of Kanji for Classic Nelson ID `id`
  /// \details a few Ids map to multiple Kanji (ie '1491' maps to 㡡, 幮 and 𢅥)
  [[nodiscard]] const List& findByNelsonId(Kanji::NelsonId id) const;

  /// print "ERROR[#] --- " followed `msg` to err(), '#' is total error count
  void printError(const String& msg) const;

  [[nodiscard]] auto debug() const { return _debugMode != DebugMode::None; }
  [[nodiscard]] auto fullDebug() const { return _debugMode == DebugMode::Full; }
  [[nodiscard]] auto infoDebug() const { return _debugMode == DebugMode::Info; }

  [[nodiscard]] auto& out() const { return _out; }
  [[nodiscard]] auto& err() const { return _err; }
  [[nodiscard]] auto& dataDir() const { return _dataDir; }
  [[nodiscard]] auto& nameMap() const { return _nameMap; }

  /// used for putting a standard prefix on output messages when needed
  [[nodiscard]] std::ostream& log(bool heading = false) const;
protected:
  /// get path to a directory containing '.txt' files required by this program
  /// \details This function first looks in `args` for #DataArg followed by a
  /// directory name and returns that value if found, otherwise it searches up
  /// the parent directories starting at 'current dir'. Finally if `args[0]` is
  /// a valid path, its parent directories will be searched.
  /// \param args command-line args
  /// \return a directory with the expected number of .txt files
  /// \throw DomainError if an appropriate data directory isn't found
  [[nodiscard]] static Path getDataDir(const Args& args);

  /// return #DebugMode by looking for #DebugArg or #InfoArg flags in `args`
  [[nodiscard]] static DebugMode getDebugMode(const Args& args);

  /// ctor called by derived classes
  /// \param dataDir directory containing '.txt' files with Kanji related data
  /// \param debugMode if not None then print info after loading and then exit
  /// \param out stream to write standard output
  /// \param err stream to write error output
  KanjiData(const Path& dataDir, DebugMode debugMode,
      std::ostream& out = std::cout, std::ostream& err = std::cerr);

  /// create UcdKanji for any entries in #_ucd that don't already have a Kanji
  /// created already (should be called after creating all other types)
  void processUcd();

  /// compares stroke values loaded from other files to strokes in 'ucd.txt' and
  /// prints results (if -debug was specified), call after calling processUcd()
  void checkStrokes() const;

  [[nodiscard]] auto& radicals() { return _radicals; }
  [[nodiscard]] auto& getUcd() { return _ucd; }
  [[nodiscard]] auto& getTypes() { return _types; }

  bool checkInsert(const KanjiPtr&, UcdPtr = {});
  bool checkInsert(List&, const KanjiPtr&);
  void addToKyus(const KanjiPtr&);
  void addToLevels(const KanjiPtr&);
  void addToFrequencies(const KanjiPtr&);
private:
  template <typename T> using KanjiEnumMap = EnumMap<T, List>;
  using OptPath = std::optional<Path>;

  [[nodiscard]] static OptPath searchUpForDataDir(Path);
  [[nodiscard]] static bool isValidDataDir(const Path&);

  /// holds the 214 official Kanji Radicals
  RadicalData _radicals;

  /// used by Kanji class ctors to get 'pinyin', 'morohashiId' and 'nelsonIds'
  /// attributes. It also provides 'radical', 'strokes', 'meaning' and 'reading'
  /// when needed (mostly by non-NumberedKanji classes).
  UcdData _ucd;

  void insertSanityChecks(const Kanji&, UcdPtr) const;

  const Path _dataDir;
  const DebugMode _debugMode;
  std::ostream& _out;
  std::ostream& _err;

  /// maps from a UCD 'compatibility' name to a 'variation selector' style name.
  /// This map only has entries for Kanji that were loaded with a selector.
  std::map<String, String> _compatibilityMap;

  /// each EnumMap has a Kanji list per enum value (excluding 'None' values) @{
  KanjiEnumMap<KanjiTypes> _types;
  KanjiEnumMap<KanjiGrades> _grades;
  KanjiEnumMap<JlptLevels> _levels;
  KanjiEnumMap<KenteiKyus> _kyus; ///@}

  std::array<List, FrequencyBuckets> _frequencies;

  Map _nameMap;                               ///< UTF-8 name map to one Kanji
  std::map<MorohashiId, List> _morohashiMap;  ///< Dai Kan-Wa Jiten ID lookup
  std::map<Kanji::NelsonId, List> _nelsonMap; ///< Nelson ID lookup

  /// set to 1 larger than the 'frequency' of any Kanji added to #_nameMap
  /// \note should end up being '2502' after all Kanji have been loaded
  inline static constinit Kanji::Frequency _maxFrequency;
};

using KanjiDataPtr = std::shared_ptr<const KanjiData>;

/// \end_group
} // namespace kanji_tools
