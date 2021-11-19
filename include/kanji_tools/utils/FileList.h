#ifndef KANJI_TOOLS_UTILS_FILE_LIST_H
#define KANJI_TOOLS_UTILS_FILE_LIST_H

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
template<typename T, size_t S> constexpr inline T secondLast(const std::array<T, S>& x) {
  static_assert(S > 1);
  return x[S - 2];
}

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
  virtual JlptLevels level() const { return JlptLevels::None; }
  virtual KenteiKyus kyu() const { return KenteiKyus::None; }
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

// For now there are TypeFileLists for 'JlptLevels' and 'KenteiKyus'
template<typename T> class TypeFileList : public FileList {
protected:
  TypeFileList(const std::filesystem::path& p, T type, bool createNewUniqueFile = false)
    : FileList(p, FileType::MultiplePerLine, createNewUniqueFile, &UniqueTypeNames, kanji_tools::toString(type)),
      _type(type) {}
protected:
  const T _type;
private:
  inline static Set UniqueTypeNames;
};

class LevelFileList : public TypeFileList<JlptLevels> {
public:
  LevelFileList(const std::filesystem::path& p, JlptLevels level, bool createNewUniqueFile = false)
    : TypeFileList(p, level, createNewUniqueFile) {}

  JlptLevels level() const override { return _type; }
};

class KyuFileList : public TypeFileList<KenteiKyus> {
public:
  KyuFileList(const std::filesystem::path& p, KenteiKyus kyu, bool createNewUniqueFile = false)
    : TypeFileList(p, kyu, createNewUniqueFile) {}

  KenteiKyus kyu() const override { return _type; }
};

inline std::string capitalize(const std::string& s) {
  if (s.length() && std::islower(s[0])) {
    std::string result(s);
    result[0] = std::toupper(result[0]);
    return result;
  }
  return s;
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_FILE_LIST_H
