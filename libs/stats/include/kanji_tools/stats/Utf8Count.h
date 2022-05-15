#pragma once

#include <kanji_tools/utils/Utf8.h>

#include <filesystem>
#include <map>
#include <optional>
#include <regex>

namespace kanji_tools {

// 'Utf8Count' counts multi-byte characters in strings passed to 'add' functions
class Utf8Count {
public:
  using Map = std::map<String, size_t>;
  using TagMap = std::map<String, Map>;
  using OptRegex = std::optional<std::wregex>;
  using OptString = std::optional<String>;

  // 'RemoveFurigana' is a regex for removing Furigana from text files. It can
  // be passed to Utf8Count ctor. Furigana is usually a Kanji followed by one or
  // more Kana characters inside wide brackets. This regex matches a Kanji (or
  // wide letter) followed by bracketed Kana (and 'DefaultReplace' replaces it
  // with just just the Kanji match part). See Utf8CharTest.cpp for examples of
  // how the regex works. Note, almost all Furigana is Hiragana, but sometimes
  // Katakana can also be used like: 護謨製（ゴムせい）.
  static const std::wregex RemoveFurigana;

  // 'DefaultReplace' is used as the default replacement string in below ctor to
  // replace the contents in brackets with itself (and get rid of the rest of
  // the string). It can be used in combination with 'RemoveFurigana' regex.
  static const std::wstring DefaultReplace;

  // if 'find' regex is provided it's applied before processing for counting
  explicit Utf8Count(const OptRegex& find = {},
      const std::wstring& replace = DefaultReplace, std::ostream* debug = {});

  Utf8Count(const Utf8Count&) = delete;
  virtual ~Utf8Count() = default;

  // 'add' adds all the 'Utf8Chars' from the given string 's' and returns the
  // number added. If 'tag' is provided then '_tags' will be updated (which
  // contains a count per tag per unique token).
  size_t add(const String& s, const OptString& tag = {});

  // 'addFile' adds strings from given 'file' or from all files in directory (if
  // file is 'directory'). 'fileNames' controls whether the name of the file (or
  // directory) should also be included in the count and 'recurse' determines if
  // subdirectories are also searched. By default, file names are used as 'tag'
  // values when calling 'add'.
  size_t addFile(const std::filesystem::path& file, bool addTag = true,
      bool fileNames = true, bool recurse = true);

  // return count for given string or 0 if not found
  [[nodiscard]] size_t count(const String& s) const;

  // return an optional Map of 'tag to count' for the given Utf8Char 's'
  [[nodiscard]] const Map* tags(const String& s) const;

  [[nodiscard]] auto uniqueEntries() const { return _map.size(); }
  [[nodiscard]] auto files() const { return _files; }
  [[nodiscard]] auto directories() const { return _directories; }

  // returns number of lines changed due to 'replace' regex
  [[nodiscard]] auto replacements() const { return _replacements; }

  // returns last tag (file name) that had line replaced (if 'addTag' is used)
  [[nodiscard]] auto& lastReplaceTag() const { return _lastReplaceTag; }

  [[nodiscard]] auto errors() const { return _errors; }
  [[nodiscard]] auto variants() const { return _variants; }
  [[nodiscard]] auto combiningMarks() const { return _combiningMarks; }
  [[nodiscard]] auto& map() const { return _map; }
  [[nodiscard]] auto debug() const { return _debug; }
private:
  // 'hasUnclosedBrackets' returns true if 'line' has an open bracket without a
  // closing bracket (searching back from the end), otherwise it returns false.
  [[nodiscard]] static bool hasUnclosedBrackets(const String& line);

  // 'processJoinedLine' returns count from processing 'prevline' plus 'line' up
  // until 'pos' (plus size of close bracket) and sets 'prevLine' to the
  // unprocessed remainder of 'line'.
  [[nodiscard]] size_t processJoinedLine(
      String& prevLine, const String& line, size_t pos, const OptString& tag);

  // 'processFile' returns the Utf8Char count from 'file'. If '_find' is not set
  // then each line is processed independently, otherwise 'hasUnclosedBrackets'
  // and 'processJoinedLine' are used to join up to two lines together before
  // calling 'add' to help '_find' match against larger sets of data. The focus
  // on brackets is to help removing furigana which is in brackets after a kanji
  // and can potentially span across lines of a text file.
  [[nodiscard]] size_t processFile(
      const std::filesystem::path& file, const OptString& tag);
  [[nodiscard]] size_t processFileWithRegex(
      const std::filesystem::path& file, const OptString& tag);

  [[nodiscard]] virtual bool allowAdd(const String&) const { return true; }
  [[nodiscard]] size_t doAddFile(const std::filesystem::path& file, bool addTag,
      bool fileNames, bool recurse = true);

  Map _map;
  TagMap _tags;
  String _lastReplaceTag;

  // count files and directories processed
  size_t _files{}, _directories{};

  // count errors, variants, combining marks and replacements during processing
  size_t _errors{}, _variants{}, _combiningMarks{}, _replacements{};

  const OptRegex _find;
  const std::wstring _replace;
  std::ostream* const _debug;
};

template<typename Pred> class Utf8CountIf : public Utf8Count {
public:
  explicit Utf8CountIf(Pred pred, const OptRegex& find = {},
      const std::wstring& replace = DefaultReplace, std::ostream* debug = {})
      : Utf8Count{find, replace, debug}, _pred{pred} {}
private:
  [[nodiscard]] bool allowAdd(const String& token) const override {
    return _pred(token);
  }
  const Pred _pred;
};

} // namespace kanji_tools