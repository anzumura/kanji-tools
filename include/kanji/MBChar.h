#ifndef KANJI_MBCHAR_H
#define KANJI_MBCHAR_H

#include <filesystem>
#include <map>
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
  bool getNext(std::string& result, bool onlyMB = true);
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
  MBCharCount() {}
  // 'add' adds all the 'MBChars' from the given string 's' and returns the number added
  size_t add(const std::string& s) {
    MBChar c(s);
    size_t added = 0;
    for (std::string token; c.getNext(token); ++added)
      ++_map[token];
    return added;
  }
  // 'addFile' adds strings from given 'file' or from all files in directory (if file is 'directory').
  // 'recurse' determines if subdirectories are also searched.
  size_t addFile(const std::filesystem::path& file, bool recurse = true);
  // return count for given string or 0 if not found
  size_t count(const std::string& s) const {
    auto i = _map.find(s);
    return i != _map.end() ? i->second : 0;
  }
  size_t uniqueEntries() const { return _map.size(); }
  const Map& map() const { return _map; }
private:
  Map _map;
};

// Helper methods to print binary or hex versions of an unsigned char
inline std::string to_binary(unsigned char x) {
  std::string result;
  for (; x > 0; x >>= 1)
    result.insert(result.begin(), '0' + x % 2);
  return result;
}
inline std::string to_hex(unsigned char x) {
  std::string result;
  for (; x > 0; x >>= 4) {
    const auto i = x % 16;
    result.insert(result.begin(), (i < 10 ? '0' + i : 'a' + i - 10));
  }
  return result;
}

} // namespace kanji

#endif
