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

// Kanji Kentei (漢字検定) Kyū (級), K = Kanken (漢検), J=Jun (準), None=not a Kentei kanji
enum class Kyus { K10, K9, K8, K7, K6, K5, K4, K3, KJ2, K2, KJ1, K1, None };
constexpr std::array AllKyus{Kyus::K10, Kyus::K9,  Kyus::K8, Kyus::K7,  Kyus::K6, Kyus::K5,  Kyus::K4,
                             Kyus::K3,  Kyus::KJ2, Kyus::K2, Kyus::KJ1, Kyus::K1, Kyus::None};
const char* toString(Kyus);
inline std::ostream& operator<<(std::ostream& os, const Kyus& x) { return os << toString(x); }

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
    for (auto i : OtherUniqueNames)
      i->clear();
  }

  enum class FileType { MultiplePerLine, OnePerLine };
  FileList(const std::filesystem::path& p, FileType fileType, bool createNewUniqueFile = false)
    : FileList(p, fileType, createNewUniqueFile, nullptr) {}
  FileList(const std::filesystem::path& p, bool createNewUniqueFile = false)
    : FileList(p, FileType::OnePerLine, createNewUniqueFile, nullptr) {}
  FileList(const FileList&) = delete;

  bool exists(const std::string& s) const { return _map.find(s) != _map.end(); }
  // return 0 for 'not found'
  int get(const std::string& name) const {
    auto i = _map.find(name);
    return i != _map.end() ? i->second : 0;
  }
  const std::string& name() const { return _name; }
  virtual Levels level() const { return Levels::None; }
  virtual Kyus kyu() const { return Kyus::None; }
  const List& list() const { return _list; }
  size_t size() const { return _list.size(); }
  // 'toString' returns the full contents of this list into a string (with no separates)
  std::string toString() const {
    std::string result;
    // reserve for efficiency - make a guess that each entry in the list is a 3 byte utf8 character
    result.reserve(_list.size() * 3);
    for (const auto& i : _list)
      result += i;
    return result;
  }
protected:
  FileList(const std::filesystem::path& p, FileType fileType, bool, Set*, const std::string& name = "");
private:
  // 'UniqueNames': ensures uniqueness across non-typed FileLists (currently only frequency.txt)
  inline static Set UniqueNames;
  inline static std::set<Set*> OtherUniqueNames;

  const std::string _name;
  List _list;
  Map _map;
};

// For now there are TypeFileLists for 'Levels' and 'Kyus'
template<typename T> class TypeFileList : public FileList {
protected:
  TypeFileList(const std::filesystem::path& p, T type, bool createNewUniqueFile = false)
    : FileList(p, FileType::MultiplePerLine, createNewUniqueFile, &UniqueTypeNames, kanji::toString(type)), _type(type) {}
protected:
  const T _type;
private:
  inline static Set UniqueTypeNames;
};

class LevelFileList : public TypeFileList<Levels> {
public:
  LevelFileList(const std::filesystem::path& p, Levels level, bool createNewUniqueFile = false)
    : TypeFileList(p, level, createNewUniqueFile) {}

  Levels level() const override { return _type; }
};

class KyuFileList : public TypeFileList<Kyus> {
public:
  KyuFileList(const std::filesystem::path& p, Kyus kyu, bool createNewUniqueFile = false)
    : TypeFileList(p, kyu, createNewUniqueFile) {}

  Kyus kyu() const override { return _type; }
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
