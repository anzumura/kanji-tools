#pragma once

#include <kanji_tools/utils/MBUtils.h>

#include <filesystem>
#include <map>
#include <optional>
#include <regex>

namespace kanji_tools {

// 'MBCount' counts multi-byte characters in strings passed to 'add' functions
class MBCount {
public:
  using Map = std::map<std::string, size_t>;
  using TagMap = std::map<std::string, Map>;
  using OptRegex = std::optional<std::wregex>;
  using OptString = std::optional<std::string>;

  // 'RemoveFurigana' is a regex for removing Furigana from text files. It can
  // be passed to MBCount ctor. Furigana is usually a Kanji followed by one or
  // more Kana characters inside wide brackets. This regex matches a Kanji (or
  // wide letter) followed by bracketed Kana (and 'DefaultReplace' replaces it
  // with just just the Kanji match part). See MBCharTest.cpp for examples of
  // how the regex works. Note, almost all Furigana is Hiragana, but sometimes
  // Katakana can also be used like: 護謨製（ゴムせい）.
  static const std::wregex RemoveFurigana;

  // 'DefaultReplace' is used as the default replacement string in below ctor to
  // replace the contents in brackets with itself (and get rid of the rest of
  // the string). It can be used in combination with 'RemoveFurigana' regex.
  static const std::wstring DefaultReplace;

  // if 'find' regex is provided it's applied before processing for counting
  explicit MBCount(OptRegex find = {},
      const std::wstring& replace = DefaultReplace, std::ostream* debug = {});

  MBCount(const MBCount&) = delete;
  virtual ~MBCount() = default;

  // 'add' adds all the 'MBChars' from the given string 's' and returns the
  // number added. If 'tag' is provided then '_tags' will be updated (which
  // contains a count per tag per unique token).
  size_t add(const std::string& s, const OptString& tag = {});

  // 'addFile' adds strings from given 'file' or from all files in directory (if
  // file is 'directory'). 'fileNames' controls whether the name of the file (or
  // directory) should also be included in the count and 'recurse' determines if
  // subdirectories are also searched. By default, file names are used as 'tag'
  // values when calling 'add'.
  size_t addFile(const std::filesystem::path& file, bool addTag = true,
      bool fileNames = true, bool recurse = true);

  // return count for given string or 0 if not found
  [[nodiscard]] size_t count(const std::string& s) const;

  // return an optional Map of 'tag to count' for the given MBChar 's'
  [[nodiscard]] const Map* tags(const std::string& s) const;

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
  [[nodiscard]] static bool hasUnclosedBrackets(const std::string& line);

  // 'processJoinedLine' returns count from processing 'prevline' plus 'line' up
  // until 'pos' (plus size of close bracket) and sets 'prevLine' to the
  // unprocessed remainder of 'line'.
  [[nodiscard]] size_t processJoinedLine(std::string& prevLine,
      const std::string& line, size_t pos, const OptString& tag);

  // 'processFile' returns the MBChar count from 'file'. If '_find' is not set
  // then each line is processed independently, otherwise 'hasUnclosedBrackets'
  // and 'processJoinedLine' are used to join up to two lines together before
  // calling 'add' to help '_find' match against larger sets of data. The focus
  // on brackets is to help removing furigana which is in brackets after a kanji
  // and can potentially span across lines of a text file.
  [[nodiscard]] size_t processFile(
      const std::filesystem::path& file, const OptString& tag);

  [[nodiscard]] virtual bool allowAdd(const std::string&) const { return true; }
  [[nodiscard]] size_t doAddFile(const std::filesystem::path& file, bool addTag,
      bool fileNames, bool recurse = true);

  Map _map;
  TagMap _tags;
  std::string _lastReplaceTag;

  // count files and directories processed
  size_t _files{}, _directories{};

  // count errors, variants, combining marks and replacements during processing
  size_t _errors{}, _variants{}, _combiningMarks{}, _replacements{};

  const OptRegex _find;
  const std::wstring _replace;
  std::ostream* const _debug;
};

template<typename Pred> class MBCountIf : public MBCount {
public:
  MBCountIf(Pred pred, OptRegex find = {},
      const std::wstring& replace = DefaultReplace, std::ostream* debug = {})
      : MBCount{find, replace, debug}, _pred{pred} {}
private:
  [[nodiscard]] bool allowAdd(const std::string& token) const override {
    return _pred(token);
  }
  const Pred _pred;
};

} // namespace kanji_tools
