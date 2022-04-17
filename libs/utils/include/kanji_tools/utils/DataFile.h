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
  using Path = std::filesystem::path;
  using StringList = std::vector<std::string>;
  using StringSet = std::set<std::string>;

  inline static const std::string TextFileExtension{".txt"};

  // 'getFile' checks that 'file' exists in 'dir' and is a regular type file and
  // then returns the full path. It will also try adding '.txt' extension if
  // 'file' isn't found and doesn't already have an extension.
  [[nodiscard]] static Path getFile(const Path& dir, const Path& file);

  static void print(std::ostream&, const StringList&, const std::string& type,
      const std::string& group);

  static void usage(const std::string& msg) { throw std::domain_error(msg); }

  // should be called after loading all lists to clean up unneeded static data
  static void clearUniqueCheckData();

  enum class FileType { MultiplePerLine, OnePerLine };

  DataFile(const Path&, FileType);
  explicit DataFile(const Path&);

  DataFile(const DataFile&) = delete;

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
  DataFile(const Path&, FileType, StringSet*, const std::string& name = {});
private:
  using Map = std::map<std::string, size_t>;

  // 'UniqueNames': ensures uniqueness across non-typed DataFiles (currently
  // only frequency.txt)
  inline static StringSet UniqueNames;
  inline static std::set<StringSet*> OtherUniqueNames;

  const std::string _name;
  StringList _list;
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
  [[nodiscard]] T type() const { return _type; }
private:
  const T _type;

  inline static StringSet UniqueTypeNames;
};

class LevelDataFile : public TypedDataFile<JlptLevels> {
public:
  LevelDataFile(const Path& p, JlptLevels level) : TypedDataFile{p, level} {}

  [[nodiscard]] JlptLevels level() const override { return type(); }
};

class KyuDataFile : public TypedDataFile<KenteiKyus> {
public:
  KyuDataFile(const Path& p, KenteiKyus kyu) : TypedDataFile{p, kyu} {}

  [[nodiscard]] KenteiKyus kyu() const override { return type(); }
};

} // namespace kanji_tools
