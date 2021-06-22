#ifndef KANJI_DATA_H
#define KANJI_DATA_H

#include <kanji/FileList.h>
#include <kanji/Radical.h>

#include <memory>
#include <optional>

namespace kanji {

// Official Grades for Jouyou kanji
enum class Grades { G1, G2, G3, G4, G5, G6, S, None }; // S=secondary school, None=not jouyou
constexpr std::array AllGrades = {Grades::G1, Grades::G2, Grades::G3, Grades::G4,
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
constexpr std::array AllTypes = {Types::Jouyou, Types::Jinmei, Types::LinkedJinmei, Types::LinkedOld,
                                 Types::Other,  Types::Extra,  Types::None};
const char* toString(Types);
inline std::ostream& operator<<(std::ostream& os, const Types& x) { return os << toString(x); }

class Data {
public:
  Data(int argc, char** argv);
  using Entry = std::shared_ptr<class Kanji>;
  using List = std::vector<Entry>;
  using Map = std::map<std::string, Entry>;
  using RadicalMap = std::map<std::string, Radical>;

  class Group {
  public:
    // 'peers' should be false for most groups, but should be true for a group where 'name'
    // is just one of the 'members' (instead of 'name' being a logical parent). For example,
    // a 'peers=false' group could have name: '太' and members: '駄' and '汰' whereas a
    // 'peers=true' group could have name: '粋' and members: '枠' and '砕'
    Group(int number, const Entry& name, const Data::List& members, bool peers)
      : _number(number), _name(name), _members(members), _peers(peers) {}
    Group(const Group&) = default;

    int number() const { return _number; }
    const Entry& name() const { return _name; }
    const Data::List& members() const { return _members; }
    bool peers() const { return _peers; }
    std::string toString() const;
  private:
    int _number;
    const Entry _name;
    Data::List _members;
    bool _peers;
  };
  using GroupMap = std::map<std::string, Group>;
  using GroupList = std::vector<Group>;

  const List& jouyouKanji() const { return _lists.at(Types::Jouyou); }
  const List& jinmeiKanji() const { return _lists.at(Types::Jinmei); }
  const List& linkedJinmeiKanji() const { return _lists.at(Types::LinkedJinmei); }
  const List& linkedOldKanji() const { return _lists.at(Types::LinkedOld); }
  const List& otherKanji() const { return _lists.at(Types::Other); }
  const List& extraKanji() const { return _lists.at(Types::Extra); }
  const RadicalMap& radicals() const { return _radicals; }
  // 'Linked' and 'Other' types don't have radicals right now
  static bool hasRadical(Types t) { return t == Types::Jouyou || t == Types::Jinmei || t == Types::Extra; }

  std::optional<const Entry> find(const std::string& name) const {
    auto i = _map.find(name);
    if (i == _map.end()) return {};
    return i->second;
  }
  Types getType(const std::string& name) const;
  bool isOldJouyou(const std::string& name) const { return _jouyouOldSet.find(name) != _jouyouOldSet.end(); }
  bool isOldJinmei(const std::string& name) const { return _jinmeiOldSet.find(name) != _jinmeiOldSet.end(); }
  bool isOldName(const std::string& name) const { return isOldJouyou(name) || isOldJinmei(name); }
  // helper functions during loading
  int getFrequency(const std::string& name) const { return _frequency.get(name); }
  Levels getLevel(const std::string&) const;
  Radical getRadical(const std::string& radical) const {
    auto i = _radicals.find(radical);
    if (i == _radicals.end()) throw std::domain_error("radical not found: " + radical);
    return i->second;
  }
  int getStrokes(const std::string& name) const {
    auto i = _strokes.find(name);
    return i == _strokes.end() ? 0 : i->second;
  }
private:
  // helper functions for getting command line options
  static std::filesystem::path getDataDir(int, char**);
  static bool getDebug(int, char**);

  bool checkInsert(const Entry&);
  void checkInsert(List&, const Entry&);
  void checkInsert(const std::string&, const Group&);
  void checkNotFound(const Entry&) const;
  static void checkInsert(FileList::Set&, const std::string&);
  static void checkNotFound(const FileList::Set&, const std::string&);
  static void printError(const std::string&);
  void loadRadicals();
  void loadStrokes();
  void populateJouyou();
  void populateJinmei();
  void populateExtra();
  void processList(const FileList&);
  void loadGroups();
  void checkStrokes() const;
  // the following print functions are called after loading all data if -debug flag is specified
  void printStats() const;
  void printGrades() const;
  void printLevels() const;
  void printRadicals() const;
  void printGroups() const;
  template<typename T> void printCount(const std::string& name, T pred) const;

  const std::filesystem::path _dataDir;
  const bool _debug;
  // 'n1-n5' and 'frequency' lists are loaded from simple files with one kanji per line
  const FileList _n5;
  const FileList _n4;
  const FileList _n3;
  const FileList _n2;
  const FileList _n1;
  const FileList _frequency;
  // '_groups' and '_groupList' are populated from groups.txt
  // '_groups' maps each kanji name to its group so currently a kanji can't be included in more than one group
  GroupMap _groups;
  GroupList _groupList;
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

#endif // KANJI_KANJI_LISTS_H
