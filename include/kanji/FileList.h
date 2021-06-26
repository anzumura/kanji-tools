#ifndef KANJI_FILE_LIST_H
#define KANJI_FILE_LIST_H

#include <array>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace kanji {

// JLPT Levels, None=not a JLPT kanji
enum class Levels { N5, N4, N3, N2, N1, None };
constexpr std::array AllLevels{Levels::N5, Levels::N4, Levels::N3, Levels::N2, Levels::N1, Levels::None};
const char* toString(Levels);
inline std::ostream& operator<<(std::ostream& os, const Levels& x) { return os << toString(x); }

class FileList {
public:
  using List = std::vector<std::string>;
  using Map = std::map<std::string, int>;
  using Set = std::set<std::string>;
  // 'getFile' checks that 'file' exists in 'dir' and is a regular type file and then returns the full path
  static std::filesystem::path getFile(const std::filesystem::path& dir, const std::filesystem::path& file);
  static void print(const List&, const std::string& type, const std::string& group = "", bool isError = false);
  static void usage(const std::string& msg) { throw std::domain_error(msg); }
  // should be called after loading all lists to clean up unneeded static data
  static void clearUniqueCheckData() {
    UniqueNames.clear();
    UniqueLevelNames.clear();
  }

  FileList(const std::filesystem::path&, Levels, bool onePerLine);
  // Constructor for kana and punctuation files (allows multiple space separated entries per line)
  FileList(const std::filesystem::path& p) : FileList(p, Levels::None, false) {}
  // Constructor for JLPT n1-n5 and frequency files (which have one entry per line and name is based on level)
  FileList(const std::filesystem::path& p, Levels level) : FileList(p, level, true) {}
  FileList(const FileList&) = delete;

  bool exists(const std::string& s) const { return _map.find(s) != _map.end(); }
  // return 0 for 'not found'
  int get(const std::string& name) const {
    auto i = _map.find(name);
    return i != _map.end() ? i->second : 0;
  }
  const std::string& name() const { return _name; }
  Levels level() const { return _level; }
  const List& list() const { return _list; }
private:
  // Static sets used to ensure uniqueness across multiple FileList instances:
  //   'UniqueNames': ensures uniqueness across non-JLPT lists (like frequency and kana lists)
  //   'UniqueLevelNames': ensures the same kanji isn't included in more than one JLPT level
  // Two sets are needed because most kanji in JLPT 'level' files are also in the 'frequency' file
  static Set UniqueNames;
  static Set UniqueLevelNames;

  const std::string _name;
  const Levels _level;
  List _list;
  Map _map;
};

inline std::string capitalize(const std::string& s) {
  if (s.length() && std::islower(s[0])) {
    std::string result(s);
    result[0] = std::toupper(result[0]);
    return result;
  }
  return s;
}

} // namespace kanji

#endif // KANJI_FILE_LIST_H
