#ifndef KANJI_TOOLS_UTILS_MBCHAR_H
#define KANJI_TOOLS_UTILS_MBCHAR_H

#include <filesystem>
#include <map>
#include <optional>
#include <regex>
#include <string>

namespace kanji_tools {

// MBChar is a helper class for working with UTF-8 strings. Create an MBChar from a string
// and then call 'getNext' to get one 'character' at a time. 'getNext' will return false
// once the end of the original string is reached. Use 'reset' if you want to iterate again.
//
// Note on UTF-8 structure:
// - UTF-8 uses 1 to 4 bytes per character, depending on the Unicode symbol
// - if high bit is 0 then it's a single byte value (so normal case)
// - if two high bits are 10 then it's a continuation byte (of a multi-byte sequence)
// - Otherwise it's the first byte of a multi-byte sequence. The number of leading '1's indicates
//   how many bytes are in the sequence, i.e.: 110 means 2 bytes, 1110 means 3, etc.
class MBChar {
public:
  enum Values : unsigned char {
    Bit5 = 0b00'00'10'00,
    Bit4 = 0b00'01'00'00,
    Bit3 = 0b00'10'00'00,
    Bit2 = 0b01'00'00'00,
    Bit1 = 0b10'00'00'00, // continuation pattern
    Mask = 0b11'00'00'00  // mask for first two bits
  };

  // 'isVariationSelector' returns true if s points to a UTF-8 variation selector, this
  // method is used by 'length', 'next' and 'doPeek'.
  static auto isVariationSelector(const unsigned char* s) {
    // Checking for variation selectors would be easier if 'i' was wchar_t, but that would involve
    // calling more expensive conversion functions (like fromUtf8). Note, variation selectors are
    // range 'fe00' to 'fe0f' in Unicode which is '0xef 0xb8 0x80' to '0xef 0xb8 0x8f' in UTF-8.
    return s && *s++ == 0xef && *s++ == 0xb8 && *s >= 0x80 && *s <= 0x8f;
  }
  static auto isVariationSelector(const char* s) {
    return isVariationSelector(reinterpret_cast<const unsigned char*>(s));
  }
  static auto isVariationSelector(const std::string& s) { return isVariationSelector(s.c_str()); }

  // 'length' with onlyMB=true only counts multi-byte 'sequence start' bytes, otherwise length
  // includes both multi-byte sequence starts as well as regular single byte values, i.e.,
  // simply don't add 'continuation' bytes to length (this done by using '11 00 00 00' to grab the
  // first two bits and only adding to length if the result is not binary '10 00 00 00'). Examples:
  // - length("abc") = 0
  // - length("abc", false) = 3
  // - length("大blue空") = 2
  // - length("大blue空", false) = 6
  // Note: some Kanji can be followed by a 'variation selector' - these are not counted by default
  // since they are considered part of the previous 'MB character' (as a modifier).
  static auto length(const char* s, bool onlyMB = true, bool skipVariationSelectors = true) {
    int len = 0;
    // doing one 'reinterpret_cast' at the beginning saves doing a bunch of static_casts when checking
    // if the next 3 bytes represent a 'variation selector'
    if (auto i = reinterpret_cast<const unsigned char*>(s); i) {
      while (*i)
        if (skipVariationSelectors && isVariationSelector(i))
          i += 3;
        else if (onlyMB)
          len += (*i++ & Mask) == Mask;
        else
          len += (*i++ & Mask) != Bit1;
    }
    return len;
  }
  static auto length(const std::string& s, bool onlyMB = true, bool skipVariationSelectors = true) {
    return length(s.c_str(), onlyMB, skipVariationSelectors);
  }

  // 'isMBCharWithVariationSelector' returns true if 's' is a single MBChar (so len 2-4) followed
  // by a variation selector (which are always len 3).
  static auto isMBCharWithVariationSelector(const std::string& s) {
    return s.length() > 4 && s.length() < 8 && isVariationSelector(s.substr(s.length() - 3));
  }
  static auto withoutVariationSelector(const std::string& s) {
    return isMBCharWithVariationSelector(s) ? s.substr(0, s.length() - 3) : s;
  }
  static auto optionalWithoutVariationSelector(const std::string& s) {
    return isMBCharWithVariationSelector(s) ? std::optional(s.substr(0, s.length() - 3)) : std::nullopt;
  }

  // 'getFirst' returns the first MBChar from 's' (including any variation selector that might follow).
  // If 's' doesn't start with a multi-byte sequence then empty string is returned.
  static auto getFirst(const std::string& s) {
    std::string result;
    MBChar c(s);
    c.next(result);
    return result;
  }

  // 'Results' is used for the return value of the 'valid' method - see comments below for more details.
  enum class Results {
    Valid,
    NotMBChar,
    StringTooLong,
    ContinuationByte,   // returned when the first byte is a continuation byte, i.e., starts with '10'
    MBCharTooLong,      // returned when the first byte starts with more than 4 1's (so too long for UTF-8)
    MBCharMissingBytes, // returned when there are not enough continuation bytes
    // 'Overlong' is when a character is 'UTF-8' encoded with more bytes than the minimum required, i.e.,
    // if a characer can be encoded in two bytes, but instead is encoded using three or four bytes (with
    // extra leading zero bits - see https://en.wikipedia.org/wiki/UTF-8#Overlong_encodings).
    Overlong
  };

  // 'validateUtf8' returns 'Valid' if string contains one proper multi-byte sequence, i.e., a single
  // well-formed 'multi-byte symbol'. Examples:
  // - valid("") = NotMBChar
  // - valid("a") = NotMBChar
  // - valid("a猫") = NotMBChar
  // - valid("雪") = Valid
  // - valid("雪s") = StringTooLong
  // - valid("吹雪") = StringTooLong
  // Note, the last two cases can be considered 'valid' if checkLengthOne is set to false
  static Results validateUtf8(const char* s, bool checkLengthOne = true);
  static auto validateUtf8(const std::string& s, bool checkLengthOne = true) {
    return validateUtf8(s.c_str(), checkLengthOne);
  }

  static auto isValid(const std::string& s, bool checkLengthOne = true) {
    return validateUtf8(s, checkLengthOne) == Results::Valid;
  }

  explicit MBChar(const std::string& data) : _data(data) {}

  // call reset in order to loop over the string again
  void reset() {
    _location = _data.c_str();
    _errors = 0;
    _variants = 0;
  }

  // 'next' populates 'result' with the full multi-byte character (so could be more than one byte)
  // returns true if result was populated. This function also supports 'variation selectors', i.e.,
  // when a multi-byte character is added to 'result' the next character is also inspected and if
  // it's a variation selector it will be added as well.
  bool next(std::string& result, bool onlyMB = true);

  // 'peek' works the same as 'next', but it doesn't update state (like _location or _errors).
  auto peek(std::string& result, bool onlyMB = true) const { return doPeek(result, onlyMB, _location); }
  auto errors() const { return _errors; }
  auto variants() const { return _variants; }
  auto length(bool onlyMB = true) const { return length(_data, onlyMB); }
  auto valid(bool checkLengthOne = true) const { return validateUtf8(_data, checkLengthOne); }
  auto isValid(bool checkLengthOne = true) const { return valid(checkLengthOne) == Results::Valid; }
private:
  // 'doPeek' can skip some logic if it knows it was called from 'next' or called recursively since
  // in these cases it only matters if the following value is a 'variation selector'.
  bool doPeek(std::string& result, bool onlyMB, const char* location, bool internalCall = false) const;

  const std::string _data;
  const char* _location = _data.c_str();
  int _errors = 0;   // '_errors' keeps track of how many invalid bytes were encountered during iteration
  int _variants = 0; // '_variants' keeps track of how many 'Variation Selector's were found
};

// 'MBCharCount' counts unique multi-byte characters in strings passed to the 'add' functions
class MBCharCount {
public:
  using Map = std::map<std::string, int>;
  using TagMap = std::map<std::string, Map>;
  using OptRegex = std::optional<std::wregex>;
  using OptString = std::optional<std::string>;

  // 'RemoveFurigana' is a regex for removing furigana from text files - it can be passed
  // to MBCharCount constructor. Furigana in a .txt file is usually a Kanji followed by one
  // or more Kana characters inside wide brackets. This regex matches a Kanji followed
  // by bracketed Kana (and 'DefaultReplace' will replace it with just the Kanji match
  // part). See MBCharTest.cpp for examples of how the regex works. Note, almost all furigana
  // is hiragana, but very occasionally katakana can also be included like: 護謨製（ゴムせい）
  static const std::wregex RemoveFurigana;
  // 'DefaultReplace' is used as the default replacement string in below constructor to
  // replace the contents in brackets with itself (and get rid of the rest of the string). It
  // can be used in combination with 'RemoveFurigana' regex.
  static const std::wstring DefaultReplace;

  // if 'regex' is provided it will be applied to strings before they are processed for counting
  MBCharCount(OptRegex find = std::nullopt, const std::wstring& replace = DefaultReplace, bool debug = false)
    : _find(find), _replace(replace), _debug(debug) {}
  virtual ~MBCharCount() = default;

  // 'add' adds all the 'MBChars' from the given string 's' and returns the number added. If 'tag'
  // is provided then '_tags' will be updated (which contains a count per tag per unique token).
  int add(const std::string& s, const OptString& tag = {});

  // 'addFile' adds strings from given 'file' or from all files in directory (if file is 'directory').
  // 'fileNames' controls whether the name of the file (or directory) should also be included
  // in the count and 'recurse' determines if subdirectories are also searched. By default, file names
  // are used as 'tag' values when calling 'add'.
  int addFile(const std::filesystem::path& file, bool addTag = true, bool fileNames = true, bool recurse = true) {
    if (!std::filesystem::exists(file)) throw std::domain_error("file not found: " + file.string());
    return doAddFile(file, addTag, fileNames, recurse);
  }

  // return count for given string or 0 if not found
  auto count(const std::string& s) const {
    const auto i = _map.find(s);
    return i != _map.end() ? i->second : 0;
  }

  // return an optional Map of 'tag to count' for the given MBChar 's'
  auto tags(const std::string& s) const {
    const auto i = _tags.find(s);
    return i != _tags.end() ? &i->second : nullptr;
  }

  auto uniqueEntries() const { return _map.size(); }
  auto files() const { return _files; }
  auto directories() const { return _directories; }
  // 'replaceCount' returns number of lines that were changed due to 'replace' regex
  auto replaceCount() const { return _replaceCount; }
  // 'lastReplaceTag' returns last tag (file name) that had line replaced (if 'addTag' is used)
  auto& lastReplaceTag() const { return _lastReplaceTag; }
  auto errors() const { return _errors; }
  auto variants() const { return _variants; }
  auto& map() const { return _map; }
  auto debug() const { return _debug; }
private:
  // 'hasUnclosedBrackets' returns true if 'line' has an open bracket without a closing
  // bracket (searching back from the end), otherwise it returns false.
  static bool hasUnclosedBrackets(const std::string& line);

  // 'processJoinedLine' returns count from processing 'prevline' plus 'line' up until 'pos'
  // (plus size of close bracket) and sets 'prevLine' to the unprocessed remainder of 'line'.
  int processJoinedLine(std::string& prevLine, const std::string& line, int pos, const OptString& tag);

  // 'processFile' returns the MBChar count from 'file'. If '_find' is not set then each line
  // is processed independently, otherwise 'hasUnclosedBrackets' and 'processJoinedLine' are
  // used to join up to two lines together before calling 'add' to help '_find' match against
  // larger sets of data. The focus on brackets is to help the use case of removing furigana
  // which is in brackets after a kanji and can potentially span across lines of a text file.
  int processFile(const std::filesystem::path& file, const OptString& tag);

  virtual bool allowAdd(const std::string&) const { return true; }
  int doAddFile(const std::filesystem::path& file, bool addTag, bool fileNames, bool recurse = true);

  Map _map;
  TagMap _tags;
  // keep counts of number of files and directories processed
  int _files = 0;
  int _directories = 0;
  int _errors = 0;
  int _variants = 0;
  std::string _lastReplaceTag;
  int _replaceCount = 0;
  const OptRegex _find;
  const std::wstring _replace;
  const bool _debug;
};

template<typename Pred> class MBCharCountIf : public MBCharCount {
public:
  MBCharCountIf(Pred pred, OptRegex find = std::nullopt, const std::wstring& replace = DefaultReplace,
                bool debug = false)
    : MBCharCount(find, replace, debug), _pred(pred) {}
private:
  bool allowAdd(const std::string& token) const override { return _pred(token); }
  const Pred _pred;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_MBCHAR_H
