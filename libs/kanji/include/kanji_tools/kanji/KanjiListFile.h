#pragma once

#include <kanji_tools/kanji/JlptLevels.h>
#include <kanji_tools/kanji/KenteiKyus.h>

#include <filesystem>
#include <set>
#include <vector>

namespace kanji_tools {

// 'KanjiListFile' holds data loaded from text files that contain unique 'kanji'
// string entries (either one per line or multiple per line separated by space).
// Uniqueness is verified when data is loaded and entries are stored in order in
// a list. There are derived classes for specific data types, i.e., where all
// entries are for a 'JLPT Level' or a 'Kentei Kyu'.
class KanjiListFile {
public:
  using Index = uint16_t; // support up to 65K entries
  using Path = std::filesystem::path;
  using StringList = std::vector<String>;
  using StringSet = std::set<String>;

  inline static const String TextFileExtension{".txt"};

  static constexpr Index MaxEntries{std::numeric_limits<Index>::max() - 1};

  // 'getFile' checks that 'file' exists in 'dir' and is a regular type file and
  // then returns the full path. It will also try adding '.txt' extension if
  // 'file' isn't found and doesn't already have an extension.
  [[nodiscard]] static Path getFile(const Path& dir, const Path& file);

  static void print(std::ostream&, const StringList&, const String& type,
      const String& group);

  static void usage(const String& msg) { throw std::domain_error(msg); }

  // should be called after loading all lists to clean up unneeded static data
  static void clearUniqueCheckData();

  enum class FileType { MultiplePerLine, OnePerLine };

  explicit KanjiListFile(const Path&, FileType = FileType::OnePerLine);

  KanjiListFile(const KanjiListFile&) = delete;

  virtual ~KanjiListFile() = default;

  // return index for 'name' starting at '1' or return '0' for not found.
  [[nodiscard]] Index getIndex(const String& name) const;

  [[nodiscard]] bool exists(const String&) const;
  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] virtual JlptLevels level() const { return JlptLevels::None; }
  [[nodiscard]] virtual KenteiKyus kyu() const { return KenteiKyus::None; }
  [[nodiscard]] auto& list() const { return _list; }
  [[nodiscard]] auto size() const { return _list.size(); }

  // return the full contents of this list in a string (with no separates)
  [[nodiscard]] String toString() const;
protected:
  KanjiListFile(const Path&, FileType, StringSet*, const String& name = {});
private:
  using Map = std::map<String, Index>;

  // 'UniqueNames': ensures uniqueness across non-typed KanjiListFiles
  // (currently only frequency.txt)
  inline static StringSet UniqueNames;
  inline static std::set<StringSet*> OtherUniqueNames;

  // 'load' is called by ctor to load contents of 'file'
  void load(const Path& file, FileType, StringSet*);

  // 'validate' is called by 'load' for each token. The lambda function (in 'T')
  // is called for errors (which results in an exception being thrown) and false
  // is returned if the token already exists in optional 'StringSet'.
  template<typename T> bool validate(const T&, StringSet*, const String&);

  // 'addEntry' returns false if adding another entry would exceed 'MaxEntries'
  // otherwise, it adds the given token to _list and _map and returns true.
  [[nodiscard]] bool addEntry(const String& token);

  const String _name;
  StringList _list;
  Map _map;
};

// there are TypedListFile classes for 'JlptLevels' and 'KenteiKyus'
template<typename T> class TypedListFile : public KanjiListFile {
protected:
  TypedListFile(const Path& p, T type)
      : KanjiListFile{p, FileType::MultiplePerLine, &UniqueTypeNames,
            kanji_tools::toString(type)},
        _type{type} {}

  [[nodiscard]] T type() const { return _type; }
private:
  const T _type;

  inline static StringSet UniqueTypeNames;
};

class LevelListFile : public TypedListFile<JlptLevels> {
public:
  LevelListFile(const Path& p, JlptLevels level) : TypedListFile{p, level} {}

  [[nodiscard]] JlptLevels level() const override { return type(); }
};

class KyuListFile : public TypedListFile<KenteiKyus> {
public:
  KyuListFile(const Path& p, KenteiKyus kyu) : TypedListFile{p, kyu} {}

  [[nodiscard]] KenteiKyus kyu() const override { return type(); }
};

} // namespace kanji_tools
