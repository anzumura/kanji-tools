#ifndef KANJI_MBCHAR_H
#define KANJI_MBCHAR_H

#include <string>

namespace kanji {

// MBChar is a helper class for working with UTF-8 strings. Create an MBChar from a string
// and then call 'getNext' to get one 'character' at a time. 'getNext' will return false
// once the end of the original string is reached. Use 'reset' if you want to iterate again.
//
// Note on UTF-8 structure:
// - if high bit is 0 then it's a single byte value (so normal case)
// - if two high bits are 10 then it's a continuation byte (of a multi-byte sequence)
// - Otherwise it's the first byte of a multi-byte sequence. The number of leading '1's indicates
//   how many bytes are in the sequence, i.e.: 110 means 2 bytes, 1110 means 3, etc.
class MBChar {
public:
  enum Values { HighSecondBit = 0b01'00'00'00, ContinuationPattern = 0b10'00'00'00, HighTwoBitsMask = 0b11'00'00'00 };
  explicit MBChar(const std::string& data) : _data(data), _location(_data.c_str()) {}
  // 'length' works for both normal and UTF-8 encoded strings
  size_t length() const {
    const char* s = _data.c_str();
    size_t len = 0;
    // don't add 'continuation' bytes to length, i.e.: use '11 00 00 00' to grab the first
    // two bits and only add to length if the resulting number is not binary '10 00 00 00'
    while (*s)
      len += (*s++ & HighTwoBitsMask) != ContinuationPattern;
    return len;
  }
  // call reset in order to loop over the string again
  void reset() { _location = _data.c_str(); }
  // 'next' populates 'result' with the full multi-byte character (so could be more than one byte)
  // returns true if result was populated.
  bool getNext(std::string& result, bool onlyMB = true);
private:
  const std::string _data;
  const char* _location;
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
