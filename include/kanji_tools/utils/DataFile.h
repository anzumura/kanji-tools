#ifndef KANJI_TOOLS_UTILS_DATA_FILE_H
#define KANJI_TOOLS_UTILS_DATA_FILE_H

#include <kanji_tools/utils/JlptLevels.h>
#include <kanji_tools/utils/KenteiKyus.h>

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace kanji_tools {

// 'secondLast' is a helper function to get the second last value of an array (useful for AllKanjiTypes,
// AllKanjiGrades, etc. where the final entry is 'None' and don't want to include in loops for example).
template<typename T, size_t S> constexpr inline auto secondLast(const std::array<T, S>& x) {
  static_assert(S > 1);
  return x[S - 2];
}

// 'DataFile' holds data loaded from text files that contain unique 'string' entries (either one per line
// or multiple per line separated by space). Uniqueness is verified when data is loaded and entries are
// stored in order in a list. There are derived classes for specific data types, i.e., where all entries
// are for a 'JLPT Level' or a 'Kentei Kyu'.
class DataFile {
public:
  using List = std::vector<std::string>;
  using Set = std::set<std::string>;
  // 'getFile' checks that 'file' exists in 'dir' and is a regular type file and then returns the full path
  static std::filesystem::path getFile(const std::filesystem::path& dir, const std::filesystem::path& file);
  static void print(const List&, const std::string& type, const std::string& group = "", bool isError = false);
  static void usage(const std::string& msg) { throw std::domain_error(msg); }
  // should be called after loading all lists to clean up unneeded static data
  static void clearUniqueCheckData() {
    UniqueNames.clear();
    for (auto i : OtherUniqueNames) i->clear();
  }

  enum class FileType { MultiplePerLine, OnePerLine };
  DataFile(const std::filesystem::path& p, FileType fileType, bool createNewUniqueFile = false)
    : DataFile(p, fileType, createNewUniqueFile, nullptr) {}
  DataFile(const std::filesystem::path& p, bool createNewUniqueFile = false)
    : DataFile(p, FileType::OnePerLine, createNewUniqueFile, nullptr) {}
  DataFile(const DataFile&) = delete;

  auto exists(const std::string& s) const { return _map.find(s) != _map.end(); }
  auto get(const std::string& name) const {
    auto i = _map.find(name);
    return i != _map.end() ? i->second : 0;
  }
  auto& name() const { return _name; }
  virtual JlptLevels level() const { return JlptLevels::None; }
  virtual KenteiKyus kyu() const { return KenteiKyus::None; }
  auto& list() const { return _list; }
  auto size() const { return _list.size(); }
  // 'toString' returns the full contents of this list into a string (with no separates)
  auto toString() const {
    std::string result;
    // reserve for efficiency - make a guess that each entry in the list is a 3 byte utf8 character
    result.reserve(_list.size() * 3);
    for (auto& i : _list) result += i;
    return result;
  }
protected:
  DataFile(const std::filesystem::path& p, FileType fileType, bool, Set*, const std::string& name = "");
private:
  using Map = std::map<std::string, int>;
  // 'UniqueNames': ensures uniqueness across non-typed DataFiles (currently only frequency.txt)
  inline static Set UniqueNames;
  inline static std::set<Set*> OtherUniqueNames;

  const std::string _name;
  List _list;
  Map _map;
};

// Currently there are TypedDataFile derived classes for 'JlptLevels' and 'KenteiKyus'
template<typename T> class TypedDataFile : public DataFile {
protected:
  TypedDataFile(const std::filesystem::path& p, T type, bool createNewUniqueFile = false)
    : DataFile(p, FileType::MultiplePerLine, createNewUniqueFile, &UniqueTypeNames, kanji_tools::toString(type)),
      _type(type) {}
protected:
  const T _type;
private:
  inline static Set UniqueTypeNames;
};

class LevelDataFile : public TypedDataFile<JlptLevels> {
public:
  LevelDataFile(const std::filesystem::path& p, JlptLevels level, bool createNewUniqueFile = false)
    : TypedDataFile(p, level, createNewUniqueFile) {}

  JlptLevels level() const override { return _type; }
};

class KyuDataFile : public TypedDataFile<KenteiKyus> {
public:
  KyuDataFile(const std::filesystem::path& p, KenteiKyus kyu, bool createNewUniqueFile = false)
    : TypedDataFile(p, kyu, createNewUniqueFile) {}

  KenteiKyus kyu() const override { return _type; }
};

inline auto capitalize(const std::string& s) {
  if (s.length() && std::islower(s[0])) {
    std::string result(s);
    result[0] = std::toupper(result[0]);
    return result;
  }
  return s;
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_DATA_FILE_H
