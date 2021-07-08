#ifndef KANJI_DATA_H
#define KANJI_DATA_H

#include <kanji/FileList.h>
#include <kanji/Radical.h>

#include <memory>
#include <optional>

namespace kanji {

// forward declares
class Kanji;

// Official Grades for Jouyou kanji
enum class Grades { G1, G2, G3, G4, G5, G6, S, None }; // S=secondary school, None=not jouyou
constexpr std::array AllGrades{Grades::G1, Grades::G2, Grades::G3, Grades::G4,
                               Grades::G5, Grades::G6, Grades::S,  Grades::None};
const char* toString(Grades);
inline std::ostream& operator<<(std::ostream& os, const Grades& x) { return os << toString(x); }

// Types represents the type of Kanji:
// - Jouyou: 2136 official Jouyou kanji
// - Jinmei: 633 official Jinmei kanji
// - LinkedJinmei: 230 more Jinmei kanji that are old/variant forms of Jouyou (212) or Jinmei (18)
// - LinkedOld: old/variant Jouyou kanji that aren't in 'LinkedJinmei'
// - Other: kanji that are in the top 2501 frequency list, but not one of the first 4 types
// - Extra: kanji loaded from 'extra.txt' - shouldn't be any of the above types
// - None: used as a type for a kanji that hasn't been loaded
enum class Types { Jouyou, Jinmei, LinkedJinmei, LinkedOld, Other, Extra, None };
constexpr std::array AllTypes{Types::Jouyou, Types::Jinmei, Types::LinkedJinmei, Types::LinkedOld,
                              Types::Other,  Types::Extra,  Types::None};
const char* toString(Types);
inline std::ostream& operator<<(std::ostream& os, const Types& x) { return os << toString(x); }

// 'Data': provides methods used by 'Kanji' classes during loading and is the base class for KanjiData
class Data {
public:
  using Entry = std::shared_ptr<Kanji>;
  using OptEntry = std::optional<const Entry>;
  using List = std::vector<Entry>;
  using Map = std::map<std::string, Entry>;
  using RadicalMap = std::map<std::string, Radical>;

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

  // functions used by 'Kanji' classes during construction
  virtual int getFrequency(const std::string&) const = 0;
  virtual Levels getLevel(const std::string&) const = 0;
  Radical getRadical(const std::string& name) const {
    auto i = _radicals.find(name);
    if (i == _radicals.end()) throw std::domain_error("name not found: " + name);
    return i->second;
  }
  int getStrokes(const std::string& s) const {
    auto i = _strokes.find(s);
    return i == _strokes.end() ? 0 : i->second;
  }

  // get kanji lists
  const List& jouyouKanji() const { return _types.at(Types::Jouyou); }
  const List& jinmeiKanji() const { return _types.at(Types::Jinmei); }
  const List& linkedJinmeiKanji() const { return _types.at(Types::LinkedJinmei); }
  const List& linkedOldKanji() const { return _types.at(Types::LinkedOld); }
  const List& otherKanji() const { return _types.at(Types::Other); }
  const List& extraKanji() const { return _types.at(Types::Extra); }
  const RadicalMap& radicals() const { return _radicals; }
  OptEntry findKanji(const std::string& s) const {
    auto i = _map.find(s);
    if (i == _map.end()) return {};
    return i->second;
  }
  Types getType(const std::string& s) const;
  bool isOldJouyou(const std::string& s) const { return _jouyouOldSet.find(s) != _jouyouOldSet.end(); }
  bool isOldJinmei(const std::string& s) const { return _jinmeiOldSet.find(s) != _jinmeiOldSet.end(); }
  bool isOldName(const std::string& s) const { return isOldJouyou(s) || isOldJinmei(s); }

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
  // See comment for '_frequencies' private data member for more details about frequency lists
  enum Values { FrequencyBuckets = 5 };
  const List& frequencyList(int range) const {
    return range >= 0 && range < FrequencyBuckets ? _frequencies[range] : _emptyList;
  }
  size_t frequencyTotal(int range) const { return frequencyList(range).size(); }
  void printError(const std::string&) const;

  std::ostream& out() const {return _out; }
  std::ostream& err() const {return _err; }
  const std::filesystem::path& dataDir() const { return _dataDir; }
  bool debug() const { return _debug; }
  // 'log' can be used for putting a standard prefix to output messages (used for some debug messages)
  std::ostream& log(bool heading = false) const { return heading ? _out << ">>>\n>>> " : _out << ">>> "; }
  static int maxFrequency() { return _maxFrequency; }
protected:
  // helper functions for getting command line options
  static std::filesystem::path getDataDir(int, const char**);
  static bool getDebug(int, const char**);
  // helper functions for checking and inserting into collection
  bool checkInsert(FileList::Set&, const std::string&) const;
  bool checkNotFound(const FileList::Set&, const std::string&) const;
  bool checkInsert(const Entry&);
  bool checkInsert(List&, const Entry&);
  bool checkNotFound(const Entry&) const;
  // 'loadRadicals', 'loadStrokes' and 'loadOtherReadings' must be called before calling 'populate Lists' functions
  void loadRadicals(const std::filesystem::path&);
  void loadStrokes(const std::filesystem::path&, bool checkDuplicates = true);
  void loadOtherReadings(const std::filesystem::path&);
  // populate Lists (_types datastructure)
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const FileList&);
  // 'checkStrokes' should be called after all lists are populated. If debug is enabled (-debug)
  // then this function will print any entries in _strokes that are 'Other' type or not found.
  void checkStrokes() const;

  std::ostream& _out;
  std::ostream& _err;
  const std::filesystem::path _dataDir;
  const bool _debug;
  // '_radicals' is populated from radicals.txt
  RadicalMap _radicals;
  // 'otherReadings' holds readings loaded from other-readings.txt - these are for Top Frequency kanji
  // that aren't part of any other group (so not Jouyou or Jinmei).
  std::map<std::string, std::string> _otherReadings;
  // '_strokes' is populated from strokes.txt and is meant to supplement jinmei kanji (file doesn't
  // have a 'Strokes' column) as well as old kanjis from both jouyou and jinmei files. This file
  // contains stroke counts followed by one or more lines each with a single kanji that has the given
  // number of strokes.
  std::map<std::string, int> _strokes;
  // lists of kanji corresponding to 'Types', 'Grades' and 'Levels' (excluding the 'None' enum values)
  std::map<Types, List> _types;
  std::map<Grades, List> _grades;
  std::map<Levels, List> _levels;
  // Lists of kanji grouped into 5 frequency ranges: 1-500, 501-1000, 1001-1500, 1501-2000, 2001-2501.
  // The last list is one longer in order to hold the full frequency list (of 2501 kanji).
  std::array<List, FrequencyBuckets> _frequencies;
  // allow lookup by name
  Map _map;
  // sets to help during loading (detecting duplicates, print diagnostics, etc.)
  FileList::Set _jouyouOldSet;
  FileList::Set _jinmeiOldSet;
  // 'maxFrequency' is set to 1 larger than the highest frequency of any kanji put into '_map'
  static int _maxFrequency;
  static const List _emptyList;
};

using DataPtr = std::shared_ptr<const Data>;

} // namespace kanji

#endif // KANJI_DATA_H
