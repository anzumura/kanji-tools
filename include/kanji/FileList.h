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

// Helper functions for working with UTF-8 strings. Note on UTF-8 structure:
// - if high bit is 0 then it's a single byte value (so normal case)
// - if two high bits are 10 then it's a continuation byte (of a multi-byte sequence)
// - Otherwise it's the first byte of a multi-byte sequence. The number of leading '1's indicates
//   how many bytes are in the sequence, i.e.: 110 means 2 bytes, 1110 means 3, etc.

// 'length' works for both normal and UTF-8 encoded strings
inline size_t length(const char* s) {
  size_t len = 0;
  // don't add 'continuation' bytes to length, i.e.: 0xc0 is '11 00 00 00' in binary form so use
  // it to grab the first two bits and only add to length is these bits are not 0x80 (10 00 00 00)
  while (*s)
    len += (*s++ & 0xc0) != 0x80;
  return len;
}

inline size_t length(const std::string& s) { return length(s.c_str()); }

inline std::string to_binary(unsigned char x) {
  std::string result;
  for (; x > 0; x >>= 1)
    result.insert(result.begin(), '0' + x % 2);
  return result;
}

inline std::string to_hex(unsigned char x) {
  std::string result;
  for (; x > 0; x >>= 4) { 
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : 'a' + i - 10));
  }
  return result;
}

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
