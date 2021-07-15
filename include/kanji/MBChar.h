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

  // 'RemoveFurigana' is a regex for removing furigana from text files - it can be passed
  // to MBCharCount constructor. Furigana in a .txt file is usually a Kanji followed by one
  // or more Hiragana characters inside wide brackets. This regex matches a Kanji followed
  // by bracketed Hiragana (and 'DefaultReplace' will replace it with just the Kanji match
  // part). See MBCharTest.cpp for examples of how the regex works.
  static const std::wregex RemoveFurigana;
  // 'DefaultReplace' is used as the default replacement string in below constructor to
  // replace the contents in brackets with itself (and get rid of the rest of the string). It
  // can be used in combination with 'RemoveFurigana' regex.
  static const std::wstring DefaultReplace;

  // if 'regex' is provided it will be applied to strings before they are processed for counting
  MBCharCount(OptRegex find = std::nullopt, const std::wstring& replace = DefaultReplace, bool debug = false)
    : _files(0), _directories(0), _find(find), _replace(replace), _debug(debug) {}
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
  MBCharCountIf(Pred pred, OptRegex find = std::nullopt, const std::wstring& replace = DefaultReplace,
                bool debug = false)
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

struct UnicodeBlock {
  const wchar_t start;
  const wchar_t end;
  // 'range' returns the number of code points in the block (inclusive of start and end)
  size_t range() const { return end - start + 1; }
  // 'opterator()' returns true if the given character is in this block
  bool operator()(wchar_t x) const { return x >= start && x <= end; }
  bool operator<(const UnicodeBlock& rhs) const { return start < rhs.start; }
  bool operator==(const UnicodeBlock& rhs) const { return start == rhs.start && end == rhs.end; }
};

constexpr std::array HiraganaBlocks = {UnicodeBlock{L'\u3040', L'\u309f'}};
// Second block is 'Katakana Extended' and contains things like ㇱ (small letter)
constexpr std::array KatakanaBlocks = {UnicodeBlock{L'\u30a0', L'\u30ff'}, UnicodeBlock{L'\u31f0', L'\u31ff'}};
// There are ~20K common kanji in one block and several more CJK extension blocks. For now just
// include 'Extension A' (which has ~6K kanji) in 'RareKanjiBlocks' and maybe add more extensions
// later if needed. Note: the test/sample-data files don't contain any 'rare' kanji so far, but
// they do contain more the 2600 unique kanji (out of over 75K total kanji).
constexpr std::array CommonKanjiBlocks = {UnicodeBlock{L'\u4e00', L'\u9ffc'}};
constexpr std::array RareKanjiBlocks = {UnicodeBlock{L'\u3400', L'\u4dbf'}};
constexpr std::array PunctuationBlocks = {
  UnicodeBlock{L'\u2000', L'\u206f'}, // General MB Punctuation: —, ‥, ”, “
  UnicodeBlock{L'\u3000', L'\u303f'}  // Wide Punctuation: 、, 。, （
};
constexpr std::array SymbolBlocks = {
  UnicodeBlock{L'\u2100', L'\u2145'}, // Letterlike Symbols: ℃
  UnicodeBlock{L'\u2190', L'\u21ff'}, // Arrows: →
  UnicodeBlock{L'\u2200', L'\u22ff'}, // Math Symbols: ∀
  UnicodeBlock{L'\u2500', L'\u257f'}, // Box Drawing: ─
  UnicodeBlock{L'\u25A0', L'\u25ff'}, // Geometric Shapes: ○
  UnicodeBlock{L'\u2600', L'\u26ff'}  // Misc Symbols: ☆
};
constexpr std::array LetterBlocks = {
  UnicodeBlock{L'\u0080', L'\u00ff'}, // Latin Supplement: ·, ×
  UnicodeBlock{L'\u0100', L'\u017f'}, // Latin Extended
  UnicodeBlock{L'\u2150', L'\u2185'}, // Number Forms: Roman Numerals, etc.
  UnicodeBlock{L'\u2460', L'\u24ff'}, // Enclosed Alphanumeic: ⑦
  UnicodeBlock{L'\uff00', L'\uffef'}  // Wide Letters: full width Roman letters and half-width Katakana
};

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

template<typename T> inline bool inRange(wchar_t c, const T& t) {
  for (auto& i : t)
    if (i(c)) return true;
  return false;
}

template<typename T, typename... Ts> inline bool inRange(wchar_t c, const T& t, Ts... args) {
  for (auto& i : t)
    if (i(c)) return true;
  return inRange(c, args...);
}

template<typename... T> inline bool inWCharRange(const std::string& s, T... t) {
  if (s.length() > 1 && s.length() < 5) {
    auto w = fromUtf8(s);
    if (w.length() == 1) return inRange(w[0], t...);
  }
  return false;
}

// functions for classifying 'recognized' utf-8 encoded characters: 's' should contain one MB character (so 2-4 bytes)

// kana
inline bool isHiragana(const std::string& s) { return inWCharRange(s, HiraganaBlocks); }
inline bool isKatakana(const std::string& s) { return inWCharRange(s, KatakanaBlocks); }
inline bool isKana(const std::string& s) { return inWCharRange(s, HiraganaBlocks, KatakanaBlocks); }
// kanji
inline bool isCommonKanji(const std::string& s) { return inWCharRange(s, CommonKanjiBlocks); }
inline bool isRareKanji(const std::string& s) { return inWCharRange(s, RareKanjiBlocks); }
inline bool isKanji(const std::string& s) { return inWCharRange(s, CommonKanjiBlocks, RareKanjiBlocks); }
// 'isMBPunctuation' tests for wide space by default, but also allows not including spaces.
inline bool isMBPunctuation(const std::string& s, bool includeSpace = true) {
  return s == "　" ? includeSpace : inWCharRange(s, PunctuationBlocks);
}
inline bool isMBSymbol(const std::string& s) { return inWCharRange(s, SymbolBlocks); }
inline bool isMBLetter(const std::string& s) { return inWCharRange(s, LetterBlocks); }
// 'isRecognizedMB' returns true if 's' is in any UnicodeBlock defined in this header file (including wide space)
inline bool isRecognizedMB(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks, KatakanaBlocks, PunctuationBlocks,
                      SymbolBlocks, LetterBlocks);
}

// KanjiRange includes both the 'rare block' and the 'common block' defined above
constexpr wchar_t KanjiRange[] = L"\u3400-\u4dbf\u4e00-\u9ffc";
constexpr wchar_t HiraganaRange[] = L"\u3040-\u309f";

} // namespace kanji

#endif
