#ifndef KANJI_FILE_LIST_H
#define KANJI_FILE_LIST_H

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

class FileList {
public:
  using List = std::vector<std::string>;
  using Map = std::map<std::string, int>;
  using Set = std::set<std::string>;

  FileList(const std::filesystem::path&, Levels = Levels::None);
  bool exists(const std::string& s) const { return _map.find(s) != _map.end(); }
  // return 0 for 'not found'
  int get(const std::string& name) const {
    auto i = _map.find(name);
    return i != _map.end() ? i->second : 0;
  }
  const List& list() const { return _list; }
  const std::string name;
  const Levels level;
  static void print(const List&, const std::string& type, const std::string& group = "", bool isError = false);
private:
  static Set uniqueNames; // populated and used by lists that specify a non-None level
  List _list;
  Map _map;
};

} // namespace kanji

#endif // KANJI_FILE_LIST_H
