#pragma once

#include <kanji_tools/utils/JlptLevels.h>
#include <kanji_tools/utils/KenteiKyus.h>

#include <filesystem>
#include <set>
#include <vector>

namespace kanji_tools {

// 'DataFile' holds data loaded from text files that contain unique 'string'
// entries (either one per line or multiple per line separated by space).
// Uniqueness is verified when data is loaded and entries are stored in order in
// a list. There are derived classes for specific data types, i.e., where all
// entries are for a 'JLPT Level' or a 'Kentei Kyu'.
class DataFile {
public:
  using List = std::vector<std::string>;
  using Path = std::filesystem::path;
  using Set = std::set<std::string>;

  inline static const std::string TextFileExtension{".txt"};

  // 'getFile' checks that 'file' exists in 'dir' and is a regular type file and
  // then returns the full path. It will also try adding '.txt' extension if
  // 'file' isn't found and doesn't already have an extension.
  [[nodiscard]] static Path getFile(const Path& dir, const Path& file);

  static void print(std::ostream&, const List&, const std::string& type,
      const std::string& group, bool isError = false);
  static void print(std::ostream&, const List&, const std::string& type);

  static void usage(const std::string& msg) { throw std::domain_error(msg); }

  // should be called after loading all lists to clean up unneeded static data
  static void clearUniqueCheckData();

  enum class FileType { MultiplePerLine, OnePerLine };

  DataFile(const Path&, FileType);
  DataFile(const Path&);

  DataFile(const DataFile&) = delete;
  // operator= is not generated since there are const members

  virtual ~DataFile() = default;

  [[nodiscard]] bool exists(const std::string& name) const;
  [[nodiscard]] size_t getIndex(const std::string& name) const;
  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] virtual JlptLevels level() const { return JlptLevels::None; }
  [[nodiscard]] virtual KenteiKyus kyu() const { return KenteiKyus::None; }
  [[nodiscard]] auto& list() const { return _list; }
  [[nodiscard]] auto size() const { return _list.size(); }

  // return the full contents of this list in a string (with no separates)
  [[nodiscard]] std::string toString() const;
protected:
  DataFile(const Path&, FileType, Set*, const std::string& name = {});
private:
  using Map = std::map<std::string, size_t>;

  // 'UniqueNames': ensures uniqueness across non-typed DataFiles (currently
  // only frequency.txt)
  inline static Set UniqueNames;
  inline static std::set<Set*> OtherUniqueNames;

  const std::string _name;
  List _list;
  Map _map;
};

// there are TypedDataFile classes for 'JlptLevels' and 'KenteiKyus'
template<typename T> class TypedDataFile : public DataFile {
protected:
  TypedDataFile(const Path& p, T type)
      : DataFile{p, FileType::MultiplePerLine, &UniqueTypeNames,
            kanji_tools::toString(type)},
        _type{type} {}
protected:
  const T _type;
private:
  inline static Set UniqueTypeNames;
};

class LevelDataFile : public TypedDataFile<JlptLevels> {
public:
  LevelDataFile(const Path& p, JlptLevels level) : TypedDataFile{p, level} {}

  [[nodiscard]] JlptLevels level() const override { return _type; }
};

class KyuDataFile : public TypedDataFile<KenteiKyus> {
public:
  KyuDataFile(const Path& p, KenteiKyus kyu) : TypedDataFile{p, kyu} {}

  [[nodiscard]] KenteiKyus kyu() const override { return _type; }
};

} // namespace kanji_tools
