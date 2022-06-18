#pragma once

#include <kt_kanji/KanjiEnums.h>

#include <filesystem>
#include <set>
#include <vector>

namespace kanji_tools { /// \kanji_group{KanjiListFile}
/// KanjiListFile class

/// holds data loaded from files with Kanji string entries \kanji{KanjiListFile}
///
/// Kanji can be specified either one per line or multiple per line separated by
/// space. Uniqueness is verified when data is loaded and entries are stored in
/// order in a list. There are derived classes for specific data types, i.e.,
/// where all entries are for a 'JLPT Level' or a 'Kentei Kyu'.
class KanjiListFile {
public:
  using Index = uint16_t; ///< support up to 65K entries
  using Path = std::filesystem::path;
  using StringList = std::vector<String>;
  using StringSet = std::set<String>;

  inline static const String TextFileExtension{".txt"};

  static constexpr Index MaxEntries{std::numeric_limits<Index>::max() - 1};

  /// check that `file` exists in `dir` and is a regular type file
  /// \details will also try adding '.txt' extension if `file` isn't found and
  /// doesn't already have an extension
  /// \return full path
  /// \throw DomainError if `dir` is not a directory or if `file` is not found
  ///     or is not a regular file
  [[nodiscard]] static Path getFile(const Path& dir, const Path& file);

  static void print(std::ostream&, const StringList&, const String& type,
      const String& group);

  /// report a fatal 'usage' error (like bad command-line args, missing files,
  /// invalid static data, etc.)
  /// \param msg usage error message
  /// \throw DomainError is thrown with 'what' string set to `msg`
  static void usage(const String& msg);

  /// can be called after loading is complete to clear up static data used for
  /// checking uniqueness
  static void clearUniqueCheckData();

  /// indicates how data is stored in the text file
  enum class FileType {
    MultiplePerLine, ///< a line can have more than one (space separated) Kanji
    OnePerLine       ///< each line can only have a single Kanji
  };

  /// public ctor used for a non-typed KanjiListFile (calls protected ctor)
  /// \param p path to text file to load
  /// \param fileType how data is stored in `p`
  /// \throw DomainError if `p` is not found or is not a regular file
  explicit KanjiListFile(
      const Path& p, FileType fileType = FileType::OnePerLine);

  KanjiListFile(const KanjiListFile&) = delete; ///< deleted copy ctor

  virtual ~KanjiListFile() = default; ///< default dtor

  /// return index for `name` starting at `1` or return `0` for not found
  [[nodiscard]] Index getIndex(const String& name) const;

  [[nodiscard]] bool exists(const String&) const;
  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] virtual JlptLevels level() const { return JlptLevels::None; }
  [[nodiscard]] virtual KenteiKyus kyu() const { return KenteiKyus::None; }
  [[nodiscard]] auto& list() const { return _list; }
  [[nodiscard]] auto size() const { return _list.size(); }

  /// return the full contents of this list in a string (with no separates)
  [[nodiscard]] String toString() const;
protected:
  /// create a KanjiListFile object and call load()
  /// \param p path to text file to load
  /// \param fileType how data is stored in `p`
  /// \param uniqueTypeNames optional set to use for making sure entries in `p`
  ///     are unique (instead of using a global set)
  /// \param name optional name, if empty then capitalized file name is used
  /// \throw DomainError if `p` is not found or is not a regular file 
  KanjiListFile(const Path& p, FileType fileType, StringSet* uniqueTypeNames,
      const String& name = {});
private:
  using Map = std::map<String, Index>;

  /// ensure uniqueness across non-typed KanjiListFile instances (currently only
  /// applies to 'frequency.txt')
  inline static StringSet _uniqueNames;

  /// pointers to `_uniqueTypeNames` sets (from derived classes) and is used to
  /// facilitate clearing data once everything is loaded
  inline static std::set<StringSet*> _otherUniqueNames;

  /// called by ctor to load contents of `file`
  void load(const Path& file, FileType, StringSet*);

  /// called by load() for each kanji string
  /// \tparam T type of `error`
  /// \param error function to call when an error is found
  /// \param uniqueTypeNames `token` is inserted into this set if provided
  /// \param token the kanji string to validate
  /// \return result of insert into `uniqueTypeNames` or true set not provided
  /// \throw DomainError if `token` isn't a valid multi-byte UTF-8 character or
  ///     it already exists in the same file
  /// \throw DomainError if `uniqueTypeNames` is not provided and `token` has
  ///     already been loaded in another file
  template<typename T>
  bool validate(
      const T& error, StringSet* uniqueTypeNames, const String& token);

  /// return false if adding another entry would exceed #MaxEntries, otherwise
  /// add the given token to #_list and #_map and returns true
  [[nodiscard]] bool addEntry(const String& token);

  const String _name;
  StringList _list;
  Map _map;
};

/// template for KanjiListFile that loads a type of Kanji \kanji{KAnjiListFile}
template<typename T> class TypedListFile : public KanjiListFile {
protected:
  TypedListFile(const Path& p, T type)
      : KanjiListFile{p, FileType::MultiplePerLine, &_uniqueTypeNames,
            kanji_tools::toString(type)},
        _type{type} {}

  [[nodiscard]] T type() const { return _type; }
private:
  const T _type;

  inline static StringSet _uniqueTypeNames;
};

/// KanjiListFile for loading Kanji per JLPT Level \kanji{KanjiListFile}
class LevelListFile final : public TypedListFile<JlptLevels> {
public:
  LevelListFile(const Path& p, JlptLevels level) : TypedListFile{p, level} {}

  [[nodiscard]] JlptLevels level() const final { return type(); }
};

/// KanjiListFile for loading Kanji per Kentei Kyu \kanji{KanjiListFile}
class KyuListFile final : public TypedListFile<KenteiKyus> {
public:
  KyuListFile(const Path& p, KenteiKyus kyu) : TypedListFile{p, kyu} {}

  [[nodiscard]] KenteiKyus kyu() const final { return type(); }
};

/// \end_group
} // namespace kanji_tools
