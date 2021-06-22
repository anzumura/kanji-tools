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

inline void usage(const std::string& msg) {
  std::cerr << msg << '\n';
  exit(1);
}

// JLPT Levels, None=not a JLPT kanji
enum class Levels { N5, N4, N3, N2, N1, None };
constexpr std::array AllLevels = {Levels::N5, Levels::N4, Levels::N3, Levels::N2, Levels::N1, Levels::None};
const char* toString(Levels);
inline std::ostream& operator<<(std::ostream& os, const Levels& x) { return os << toString(x); }

// helper functions to get the string length in encoded charaters instead of bytes
inline size_t length(const char* s) {
  size_t len = 0;
  while (*s)
    len += (*s++ & 0xc0) != 0x80;
  return len;
}
inline size_t length(const std::string& s) { return length(s.c_str()); }

// only works for single byte strings
inline std::string capitalize(const std::string& s) {
  if (s.length()) {
    std::string result(s);
    result[0] = std::toupper(result[0]);
    return result;
  }
  return s;
}

class FileList {
public:
  using List = std::vector<std::string>;
  using Map = std::map<std::string, int>;
  using Set = std::set<std::string>;
  static std::filesystem::path getRegularFile(const std::filesystem::path& dir, const std::filesystem::path& file);
  static void print(const List&, const std::string& type, const std::string& group = "", bool isError = false);

  FileList(const std::filesystem::path&, Levels, bool onePerLine);
  // Constructor for kana and punctuation files (the have multiple space separated entries per line)
  FileList(const std::filesystem::path& p) : FileList(p, Levels::None, false) {}
  // Constructor for JLPT n1-n5 and frequency files (which have one entry per line and name is based on level)
  FileList(const std::filesystem::path& p, Levels level) : FileList(p, level, true) {}

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
  static Set UniqueNames; // populated and used by lists that specify a non-None level

  const std::string _name;
  const Levels _level;
  List _list;
  Map _map;
};

} // namespace kanji

#endif // KANJI_FILE_LIST_H
