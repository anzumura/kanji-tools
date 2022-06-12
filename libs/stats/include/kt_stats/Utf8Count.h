#pragma once

#include <kt_utils/Utf8.h>

#include <filesystem>
#include <map>
#include <optional>
#include <regex>

namespace kanji_tools { /// \stats_group{Utf8Count}
/// Utf8Count class hierarchy

/// counts multi-byte characters in strings passed to add() \stats{Utf8Count}
class Utf8Count {
public:
  using Map = std::map<String, size_t>;
  using TagMap = std::map<String, Map>;
  using OptRegex = std::optional<std::wregex>;
  using OptString = std::optional<String>;

  /// regex to remove Furigana from text files (can be passed to Utf8Count ctor)
  /// \details Furigana is usually a Kanji followed by one or more Kana inside
  /// wide brackets. This regex matches a Kanji (or wide letter) followed by
  /// bracketed Kana (and #DefaultReplace replaces it with just just the Kanji
  /// match part). See Utf8CharTest.cpp for examples of how the regex works.
  /// \note almost all Furigana is Hiragana, but sometimes Katakana can also be
  /// used like: 護謨製（ゴムせい）.
  static const std::wregex RemoveFurigana;

  /// used as the default replacement string in Utf8Count ctor to replace the
  /// contents in brackets with itself (and get rid of the rest of the string).
  /// \details can be used in combination with #RemoveFurigana
  static const std::wstring DefaultReplace;

  /// if `find` is provided it's applied before processing for counting
  explicit Utf8Count(const OptRegex& find = {},
      const std::wstring& replace = DefaultReplace, std::ostream* debug = {});

  Utf8Count(const Utf8Count&) = delete;
  virtual ~Utf8Count() = default;

  /// adds all UTF-8 characters from `s`
  /// \param s UTF-8 string
  /// \param tag if provided then #_tags is updated (with count per `tag`)
  /// \return number of characters added
  size_t add(const String& s, const OptString& tag = {});

  /// add all UTF-8 characters from `file` if it's a regular file or all files
  /// contained in `file` if it's a directory
  /// \param file regular file or directory
  /// \param addTag true means use file names as tags when calling add()
  /// \param fileNames true means also count characters used in the file names
  /// \param recurse true means add files from directories found in `file` (if
  ///     `file` is a directory) and keep recursing. Otherwise don't process any
  ///     deeper than one level, i.e., just add first level children of `file`.
  /// \return number of characters added
  size_t addFile(const std::filesystem::path& file, bool addTag = true,
      bool fileNames = true, bool recurse = true);

  /// return count (number of occurrences) for `s` or `0` if not found
  [[nodiscard]] size_t count(const String& s) const;

  /// return an optional Map of 'tag to count' for tag `s`
  [[nodiscard]] const Map* tags(const String& s) const;

  [[nodiscard]] auto uniqueEntries() const { return _map.size(); }
  [[nodiscard]] auto files() const { return _files; }
  [[nodiscard]] auto directories() const { return _directories; }

  /// return number of lines changed by regex passed into the ctor
  [[nodiscard]] auto replacements() const { return _replacements; }

  /// return last tag (file name) that had a line replaced (if 'addTag' is used)
  [[nodiscard]] auto& lastReplaceTag() const { return _lastReplaceTag; }

  [[nodiscard]] auto errors() const { return _errors; }
  [[nodiscard]] auto variants() const { return _variants; }
  [[nodiscard]] auto combiningMarks() const { return _combiningMarks; }
  [[nodiscard]] auto& map() const { return _map; }
  [[nodiscard]] auto debug() const { return _debug; }
private:
  /// return true if `line` has an open bracket without a closing bracket when
  /// searching back from the end
  [[nodiscard]] static bool hasUnclosedBrackets(const String& line);

  /// used to process parts of two lines together to help with cases of open and
  /// close brackets spanning across lines (for Furigana replacement)
  /// \param[in,out] prevLine pass in unprocessed part of previous line, this is
  ///     then set to unprocessed part of `line` (ie from `pos` + close bracket)
  /// \param line current line read from file
  /// \param pos location of start of first close bracket in `line`
  /// \param tag if provided then #_tags is updated (with count per `tag`)
  /// \return count of multi-byte characters found in the joined lines
  [[nodiscard]] size_t processJoinedLine(
      String& prevLine, const String& line, size_t pos, const OptString& tag);

  /// process one file (when no regex has been set) line by line
  /// \param file regular file
  /// \param tag if provided then #_tags is updated (with count per `tag`)
  /// \return number of multi-byte characters found in `file`
  [[nodiscard]] size_t processFile(
      const std::filesystem::path& file, const OptString& tag);

  /// process one file and apply the regex passed to the ctor before counting
  /// \details hasUnclosedBrackets() and processJoinedLines() are used to help
  /// the regex match larger sets of data. The focus on brackets is to help
  /// removing Furigana which is in brackets after Kanji and can potentially
  /// span across lines of a text file.
  /// \param file regular file
  /// \param tag if provided then #_tags is updated (with count per `tag`)
  /// \return number of multi-byte characters found in `file`
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

/// count based on a predicate `T` \stats{Utf8Count}
template<typename T> class Utf8CountIf final : public Utf8Count {
public:
  explicit Utf8CountIf(T pred, const OptRegex& find = {},
      const std::wstring& replace = DefaultReplace, std::ostream* debug = {})
      : Utf8Count{find, replace, debug}, _pred{pred} {}
private:
  [[nodiscard]] bool allowAdd(const String& token) const final {
    return _pred(token);
  }
  const T _pred;
};

/// \end_group
} // namespace kanji_tools
