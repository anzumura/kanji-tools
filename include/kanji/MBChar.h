#ifndef KANJI_MBCHAR_H
#define KANJI_MBCHAR_H

#include <array>
#include <codecvt> // for codecvt_utf8
#include <filesystem>
#include <locale> // for wstring_convert
#include <map>
#include <optional>
#include <regex>
#include <string>

namespace kanji {

struct UnicodeBlock {
  const wchar_t start;
  const wchar_t end;
  bool has(wchar_t x) const { return x >= start && x <= end; }
};

constexpr UnicodeBlock WidePunctuation{L'\u3000', L'\u303f'};
// 'OtherMBPunctuation' contains other common multi-byte punctuation values not included in WidePunctuation
constexpr std::array OtherMBPunctuation{L'…', L'─', L'“', L'”', L'‥', L'℃', L'·', L'×', L'→', L'–', L'—'};
constexpr UnicodeBlock Hiragana{L'\u3040', L'\u309f'};
constexpr UnicodeBlock Katakana{L'\u30a0', L'\u30ff'};
constexpr UnicodeBlock KatakanaExtended{L'\u31f0', L'\u31ff'}; // things like ㇱ (small letter)
// WideLetters includes 'full-width roman letters' as well as 'half-width katakana'
constexpr UnicodeBlock WideLetter{L'\uff00', L'\uffef'};
// Latin Supplement and Extended blocks includes letters with accents, etc. like ā, é, etc.
constexpr UnicodeBlock LatinSupplement{L'\u0080', L'\u00ff'};
constexpr UnicodeBlock LatinExtended{L'\u0100', L'\u017f'};
// KanjiRange includes both the 'common range' and the 'rare range'
constexpr auto KanjiRange = L"\u4e00-\u9faf\u3400-\u4dbf";
constexpr auto HiraganaRange = L"\u3040-\u309f";

inline std::wstring fromUtf8(const std::string& s) {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return conv.from_bytes(s);
}

inline std::string toUtf8(wchar_t c) {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return conv.to_bytes(c);
}

inline std::string toUtf8(const std::wstring& s) {
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  return conv.to_bytes(s);
}

inline bool inWCharList(wchar_t c, int count, const wchar_t* list) {
  for (int i = 0; i < count; ++i)
    if (c == list[i]) return true;
  return false;
}

inline bool inWCharRange(const std::string& s, const UnicodeBlock& b, int count = 0, const wchar_t* list = nullptr) {
  if (s.length() < 2 || s.length() > 4) return false;
  auto w = fromUtf8(s);
  return w.length() == 1 && (b.has(w[0]) || inWCharList(w[0], count, list));
}

inline bool inWCharRange(const std::string& s, const UnicodeBlock& b1, const UnicodeBlock& b2, int count = 0,
                         const wchar_t* list = nullptr) {
  if (s.length() < 2 || s.length() > 4) return false;
  auto w = fromUtf8(s);
  return w.length() == 1 && (b1.has(w[0]) || b2.has(w[0]) || inWCharList(w[0], count, list));
}

inline bool inWCharRange(const std::string& s, const UnicodeBlock& b1, const UnicodeBlock& b2, const UnicodeBlock& b3,
                         int count = 0, const wchar_t* list = nullptr) {
  if (s.length() < 2 || s.length() > 4) return false;
  auto w = fromUtf8(s);
  return w.length() == 1 && (b1.has(w[0]) || b2.has(w[0]) || b3.has(w[0]) || inWCharList(w[0], count, list));
}

// functions for classifying 'recognized' utf-8 encoded characters:
//   's' should be a single wide character (so 2-4 bytes)
inline bool isHiragana(const std::string& s) { return inWCharRange(s, Hiragana); }
inline bool isKatakana(const std::string& s) { return inWCharRange(s, Katakana, KatakanaExtended); }
inline bool isKana(const std::string& s) { return inWCharRange(s, Hiragana, Katakana, KatakanaExtended); }
// 'isMBPunctuation' tests for wide space by default, but also allows not including spaces.
inline bool isMBPunctuation(const std::string& s, bool includeSpace = true) {
  return s == "　" ? includeSpace
                   : inWCharRange(s, WidePunctuation, OtherMBPunctuation.size(), OtherMBPunctuation.data());
}
inline bool isMBLetter(const std::string& s) { return inWCharRange(s, LatinSupplement, LatinExtended, WideLetter); }
inline bool isKanji(const std::string& s) { return inWCharRange(s, {L'\u4e00', L'\u9faf'}, {L'\u3400', L'\u4dbf'}); }
// 'isRecognizedWide' returns true if 's' is Kanji, Kana, Wide Punctuation (including wide space) or Wide Letter
inline bool isRecognizedWide(const std::string& s) {
  return isKanji(s) || isKana(s) || isMBLetter(s) || isMBPunctuation(s);
}

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
  // 'valid' returns true if string contains one proper multi-byte sequence, i.e., a single
  // well-formed 'multi-byte symbol'. Examples:
  // - valid("") = false
  // - valid("a") = false
  // - valid("a猫") = false
  // - valid("雪") = true
  // - valid("雪s") = false
  // - valid("吹雪") = false
  // Note, the last two cases can be considered 'valid' if checkLengthOne is set to false
  static bool valid(const char* s, bool checkLengthOne = true) {
    if (s) {
      if (const unsigned char x = *s; (x & Mask) == Mask) { // first two bits must be '11' to start a sequence
        if ((*++s & Mask) != Bit1) return false;            // second byte didn't start with '10'
        if (x & Bit3) {
          if ((*++s & Mask) != Bit1) return false; // third byte didn't start with '10'
          if (x & Bit4) {
            if (x & Bit5) return false;              // UTF-8 can only have up to 4 bytes
            if ((*++s & Mask) != Bit1) return false; // fourth byte didn't start with '10'
          }
        }
        return !checkLengthOne || !*++s;
      }
    }
    return false;
  }
  static bool valid(const std::string& s, bool checkLengthOne = true) { return valid(s.c_str(), checkLengthOne); }

  explicit MBChar(const std::string& data) : _data(data), _location(_data.c_str()) {}
  // call reset in order to loop over the string again
  void reset() { _location = _data.c_str(); }
  // 'next' populates 'result' with the full multi-byte character (so could be more than one byte)
  // returns true if result was populated.
  bool next(std::string& result, bool onlyMB = true);
  size_t length(bool onlyMB = true) const { return length(_data, onlyMB); }
  bool valid(bool checkLengthOne = true) const { return valid(_data, checkLengthOne); }
private:
  const std::string _data;
  const char* _location;
};

// 'MBCharCount' counts unique multi-byte characters in strings passed to the 'add' functions
class MBCharCount {
public:
  using Map = std::map<std::string, int>;
  using TagMap = std::map<std::string, Map>;
  using OptRegex = std::optional<std::wregex>;

  // if 'regex' is provided it will be applied to strings before they are processed to remove data
  MBCharCount(OptRegex find = std::nullopt, const std::string& replace = "", bool debug = false)
    : _files(0), _directories(0), _find(find), _replace(fromUtf8(replace)), _debug(debug) {}
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
  size_t files() const { return _files; }
  size_t directories() const { return _directories; }
  const Map& map() const { return _map; }
private:
  virtual bool allowAdd(const std::string&) const { return true; }
  size_t doAddFile(const std::filesystem::path& file, bool addTag, bool fileNames, bool recurse = true);

  Map _map;
  TagMap _tags;
  // keep a count of number of files and directories processed
  size_t _files;
  size_t _directories;
  const OptRegex _find;
  const std::wstring _replace;
  const bool _debug;
};

template<typename Pred> class MBCharCountIf : public MBCharCount {
public:
  MBCharCountIf(Pred pred, OptRegex find = std::nullopt, const std::string& replace = "", bool debug = false)
    : MBCharCount(find, replace, debug), _pred(pred) {}
private:
  bool allowAdd(const std::string& token) const override { return _pred(token); }
  const Pred _pred;
};

// Helper methods to print binary or hex versions of an unsigned char
inline std::string toBinary(unsigned char x) {
  std::string result;
  for (; x > 0; x >>= 1)
    result.insert(result.begin(), '0' + x % 2);
  return result;
}

inline std::string toHex(unsigned char x) {
  std::string result;
  for (; x > 0; x >>= 4) {
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : 'a' + i - 10));
  }
  return result;
}

} // namespace kanji

#endif
