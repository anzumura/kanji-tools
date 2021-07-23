#ifndef KANJI_MBCHAR_H
#define KANJI_MBCHAR_H

#include <filesystem>
#include <map>
#include <optional>
#include <regex>
#include <string>

namespace kanji {

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
  // 'length' with onlyMB=true only counts multi-byte 'sequence start' bytes, otherwise length
  // includes both multi-byte sequence starts as well as regular single byte values, i.e.,
  // simply don't add 'continuation' bytes to length (this done by using '11 00 00 00' to grab the
  // first two bits and only adding to length if the result is not binary '10 00 00 00'). Examples:
  // - length("abc") = 0
  // - length("abc", false) = 3
  // - length("大blue空") = 2
  // - length("大blue空", false) = 6
  static size_t length(const char* s, bool onlyMB = true) {
    size_t len = 0;
    if (s) {
      if (onlyMB)
        while (*s)
          len += (*s++ & Mask) == Mask;
      else
        while (*s)
          len += (*s++ & Mask) != Bit1;
    }
    return len;
  }
  static size_t length(const std::string& s, bool onlyMB = true) { return length(s.c_str(), onlyMB); }
  // 'Results' is used for the return value of the 'valid' method - see comments below for more details.
  enum class Results {
    Valid,
    NotMBChar,
    StringTooLong,
    ContinuationByte,  // returned when the first byte is a continuation byte, i.e., starts with '10'
    MBCharTooLong,     // returned when the first byte starts with more than 4 1's (so too long for UTF-8)
    MBCharMissingBytes // returned when there are not enough continuation bytes
  };
  // 'valid' returns 'Valid' if string contains one proper multi-byte sequence, i.e., a single
  // well-formed 'multi-byte symbol'. Examples:
  // - valid("") = NotMBChar
  // - valid("a") = NotMBChar
  // - valid("a猫") = NotMBChar
  // - valid("雪") = Valid
  // - valid("雪s") = StringTooLong
  // - valid("吹雪") = StringTooLong
  // Note, the last two cases can be considered 'valid' if checkLengthOne is set to false
  static Results valid(const char* s, bool checkLengthOne = true) {
    if (s) {
      if (const unsigned char x = *s; (x & Mask) == Mask) { // first two bits must be '11' to start a sequence
        if ((*++s & Mask) != Bit1) return Results::MBCharMissingBytes; // second byte didn't start with '10'
        if (x & Bit3) {
          if ((*++s & Mask) != Bit1) return Results::MBCharMissingBytes; // third byte didn't start with '10'
          if (x & Bit4) {
            if (x & Bit5) return Results::MBCharTooLong;                   // UTF-8 can only have up to 4 bytes
            if ((*++s & Mask) != Bit1) return Results::MBCharMissingBytes; // fourth byte didn't start with '10'
          }
        }
        return (!checkLengthOne || !*++s ? Results::Valid : Results::StringTooLong);
      } else if ((x & Mask) == Bit1)
        return Results::ContinuationByte;
    }
    return Results::NotMBChar;
  }
  static Results valid(const std::string& s, bool checkLengthOne = true) { return valid(s.c_str(), checkLengthOne); }
  static bool isValid(const std::string& s, bool checkLengthOne = true) {
    return valid(s, checkLengthOne) == Results::Valid;
  }

  explicit MBChar(const std::string& data) : _data(data), _location(_data.c_str()), _errors(0) {}

  // call reset in order to loop over the string again
  void reset() {
    _location = _data.c_str();
    _errors = 0;
  }
  // 'next' populates 'result' with the full multi-byte character (so could be more than one byte)
  // returns true if result was populated.
  bool next(std::string& result, bool onlyMB = true);
  int errors() const { return _errors; }
  size_t length(bool onlyMB = true) const { return length(_data, onlyMB); }
  Results valid(bool checkLengthOne = true) const { return valid(_data, checkLengthOne); }
  bool isValid(bool checkLengthOne = true) const { return valid(checkLengthOne) == Results::Valid; }
private:
  const std::string _data;
  const char* _location;
  // '_errors' keeps track of how many invalid bytes were encountered during iteration
  int _errors;
};

// 'MBCharCount' counts unique multi-byte characters in strings passed to the 'add' functions
class MBCharCount {
public:
  using Map = std::map<std::string, int>;
  using TagMap = std::map<std::string, Map>;
  using OptRegex = std::optional<std::wregex>;

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
    : _files(0), _directories(0), _errors(0), _find(find), _replace(replace), _debug(debug) {}
  virtual ~MBCharCount() = default;

  // 'add' adds all the 'MBChars' from the given string 's' and returns the number added. If 'tag'
  // is non-empty then '_tags' will be updated (which contains a count per tag per unique token).
  size_t add(const std::string& s, const std::string& tag = "");

  // 'addFile' adds strings from given 'file' or from all files in directory (if file is 'directory').
  // 'fileNames' controls whether the name of the file (or directory) should also be included
  // in the count and 'recurse' determines if subdirectories are also searched. By default, file names
  // are used as 'tag' values when calling 'add'.
  size_t addFile(const std::filesystem::path& file, bool addTag = true, bool fileNames = true, bool recurse = true) {
    if (!std::filesystem::exists(file)) throw std::domain_error("file not found: " + file.string());
    return doAddFile(file, addTag, fileNames, recurse);
  }

  // return count for given string or 0 if not found
  size_t count(const std::string& s) const {
    auto i = _map.find(s);
    return i != _map.end() ? i->second : 0;
  }

  // return an optional Map of 'tag to count' for the given MBChar 's'
  const Map* tags(const std::string& s) const {
    auto i = _tags.find(s);
    if (i != _tags.end()) return &i->second;
    return nullptr;
  }

  size_t uniqueEntries() const { return _map.size(); }
  int files() const { return _files; }
  int directories() const { return _directories; }
  int errors() const { return _errors; }
  const Map& map() const { return _map; }
  bool debug() const { return _debug; }
private:
  virtual bool allowAdd(const std::string&) const { return true; }
  size_t doAddFile(const std::filesystem::path& file, bool addTag, bool fileNames, bool recurse = true);

  Map _map;
  TagMap _tags;
  // keep counts of number of files and directories processed
  int _files;
  int _directories;
  int _errors;
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

} // namespace kanji

#endif // KANJI_MBCHAR_H
