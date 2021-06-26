#ifndef KANJI_DATA_H
#define KANJI_DATA_H

#include <kanji/FileList.h>
#include <kanji/Radical.h>

#include <memory>
#include <optional>

namespace kanji {

// forward declares
class Kanji;
class Group;
enum class GroupType;

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
  static void usage(const std::string& msg) { FileList::usage(msg); }
  // 'Linked' and 'Other' types don't have radicals right now
  static bool hasRadical(Types t) { return t == Types::Jouyou || t == Types::Jinmei || t == Types::Extra; }

  using Entry = std::shared_ptr<Kanji>;
  using List = std::vector<Entry>;
  using Map = std::map<std::string, Entry>;
  using RadicalMap = std::map<std::string, Radical>;

  Data(const std::filesystem::path& dataDir, bool debug) : _dataDir(dataDir), _debug(debug) {}
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
  const List& jouyouKanji() const { return _lists.at(Types::Jouyou); }
  const List& jinmeiKanji() const { return _lists.at(Types::Jinmei); }
  const List& linkedJinmeiKanji() const { return _lists.at(Types::LinkedJinmei); }
  const List& linkedOldKanji() const { return _lists.at(Types::LinkedOld); }
  const List& otherKanji() const { return _lists.at(Types::Other); }
  const List& extraKanji() const { return _lists.at(Types::Extra); }
  const RadicalMap& radicals() const { return _radicals; }
  std::optional<const Entry> findKanji(const std::string& s) const {
    auto i = _map.find(s);
    if (i == _map.end()) return {};
    return i->second;
  }
  Types getType(const std::string& s) const;
  bool isOldJouyou(const std::string& s) const { return _jouyouOldSet.find(s) != _jouyouOldSet.end(); }
  bool isOldJinmei(const std::string& s) const { return _jinmeiOldSet.find(s) != _jinmeiOldSet.end(); }
  bool isOldName(const std::string& s) const { return isOldJouyou(s) || isOldJinmei(s); }

  using GroupEntry = std::shared_ptr<Group>;
  using GroupMap = std::map<std::string, GroupEntry>;
  using GroupList = std::vector<GroupEntry>;
protected:
  const std::filesystem::path _dataDir;
  const bool _debug;
  // helper functions for getting command line options
  static std::filesystem::path getDataDir(int, const char**);
  static bool getDebug(int, const char**);
  // helper functions for checking and inserting into collection
  static void checkInsert(const std::string&, GroupMap&, const GroupEntry&);
  static void checkInsert(FileList::Set&, const std::string&);
  static void checkNotFound(const FileList::Set&, const std::string&);
  static void printError(const std::string&);
  bool checkInsert(const Entry&);
  void checkInsert(List&, const Entry&);
  void checkNotFound(const Entry&) const;
  // 'loadRadicals' and 'loadStrokes' must be called before calling the 'populate Lists' functions
  void loadRadicals(const std::filesystem::path&);
  void loadStrokes(const std::filesystem::path&);
  // populate Lists (_lists datastructure)
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const FileList&);
  // 'checkStrokes' should be called after all lists are populated. This function makes sure
  // that there isn't any entries in _strokes that are 'Other' or 'None' type, but instead of
  // asserting, print lists to help find any problems.
  void checkStrokes() const;
  // 'loadGroups' loads from '-groups.txt' files
  void loadGroup(const std::filesystem::path&, GroupMap&, GroupList&, GroupType);
  // the following print functions are called after loading all data if -debug flag is specified
  void printStats() const;
  void printGrades() const;
  void printLevels() const;
  void printRadicals() const;
  void printGroups(const GroupMap&, const GroupList&) const;
  template<typename T> void printCount(const std::string& name, T pred) const;

  // '_meaningGroups' and '_meaningGroupList' are populated from 'meaning-groups.txt' and
  // '_patternGroups' and '_patternGroupList' are populated from 'pattern-groups.txt. The
  // maps have an entry for each kanji to its group so currently a kanji can't be in more
  // than one group per group type.
  GroupMap _meaningGroups;
  GroupMap _patternGroups;
  GroupList _meaningGroupList;
  GroupList _patternGroupList;
  // '_radicals' is populated from radicals.txt
  RadicalMap _radicals;
  // '_strokes' is populated from strokes.txt and is meant to supplement jinmei kanji (file doesn't
  // have a 'Strokes' column) as well as old kanjis from both jouyou and jinmei files. This file
  // contains stroke counts followed by one or more lines each with a single kanji that has the given
  // number of strokes.
  std::map<std::string, int> _strokes;
  // lists of kanjis corresponding to 'Types' enum
  std::map<Types, List> _lists;
  // allow lookup by name
  Map _map;
  // sets to help during loading (detecting duplicates, print diagnostics, etc.)
  FileList::Set _jouyouOldSet;
  FileList::Set _jinmeiOldSet;
};

} // namespace kanji

#endif // KANJI_DATA_H
