#ifndef KANJI_TOOLS_UTILS_DISPLAY_LENGTH_H
#define KANJI_TOOLS_UTILS_DISPLAY_LENGTH_H

#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

// --- begin generated code from 'parseEastAsiaWidth.sh' ---
constexpr std::array WideBlocks = {
  UnicodeBlock{0x1100, 0x115F},   UnicodeBlock{0x231A, 0x231B},   UnicodeBlock{0x2329, 0x232A},
  UnicodeBlock{0x23E9, 0x23EC},   UnicodeBlock{0x23F0},           UnicodeBlock{0x23F3},
  UnicodeBlock{0x25FD, 0x25FE},   UnicodeBlock{0x2614, 0x2615},   UnicodeBlock{0x2648, 0x2653},
  UnicodeBlock{0x267F},           UnicodeBlock{0x2693},           UnicodeBlock{0x26A1},
  UnicodeBlock{0x26AA, 0x26AB},   UnicodeBlock{0x26BD, 0x26BE},   UnicodeBlock{0x26C4, 0x26C5},
  UnicodeBlock{0x26CE},           UnicodeBlock{0x26D4},           UnicodeBlock{0x26EA},
  UnicodeBlock{0x26F2, 0x26F3},   UnicodeBlock{0x26F5},           UnicodeBlock{0x26FA},
  UnicodeBlock{0x26FD},           UnicodeBlock{0x2705},           UnicodeBlock{0x270A, 0x270B},
  UnicodeBlock{0x2728},           UnicodeBlock{0x274C},           UnicodeBlock{0x274E},
  UnicodeBlock{0x2753, 0x2755},   UnicodeBlock{0x2757},           UnicodeBlock{0x2795, 0x2797},
  UnicodeBlock{0x27B0},           UnicodeBlock{0x27BF},           UnicodeBlock{0x2B1B, 0x2B1C},
  UnicodeBlock{0x2B50},           UnicodeBlock{0x2B55},           UnicodeBlock{0x2E80, 0x2E99},
  UnicodeBlock{0x2E9B, 0x2EF3},   UnicodeBlock{0x2F00, 0x2FD5},   UnicodeBlock{0x2FF0, 0x2FFB},
  UnicodeBlock{0x3000, 0x303E},   UnicodeBlock{0x3041, 0x3096},   UnicodeBlock{0x3099, 0x30FF},
  UnicodeBlock{0x3105, 0x312F},   UnicodeBlock{0x3131, 0x318E},   UnicodeBlock{0x3190, 0x31E3},
  UnicodeBlock{0x31F0, 0x321E},   UnicodeBlock{0x3220, 0x3247},   UnicodeBlock{0x3250, 0x4DBF},
  UnicodeBlock{0x4E00, 0xA48C},   UnicodeBlock{0xA490, 0xA4C6},   UnicodeBlock{0xA960, 0xA97C},
  UnicodeBlock{0xAC00, 0xD7A3},   UnicodeBlock{0xF900, 0xFAFF},   UnicodeBlock{0xFE10, 0xFE19},
  UnicodeBlock{0xFE30, 0xFE52},   UnicodeBlock{0xFE54, 0xFE66},   UnicodeBlock{0xFE68, 0xFE6B},
  UnicodeBlock{0xFF01, 0xFF60},   UnicodeBlock{0xFFE0, 0xFFE6},   UnicodeBlock{0x16FE0, 0x16FE4},
  UnicodeBlock{0x16FF0, 0x16FF1}, UnicodeBlock{0x17000, 0x187F7}, UnicodeBlock{0x18800, 0x18CD5},
  UnicodeBlock{0x18D00, 0x18D08}, UnicodeBlock{0x1AFF0, 0x1AFF3}, UnicodeBlock{0x1AFF5, 0x1AFFB},
  UnicodeBlock{0x1AFFD, 0x1AFFE}, UnicodeBlock{0x1B000, 0x1B122}, UnicodeBlock{0x1B150, 0x1B152},
  UnicodeBlock{0x1B164, 0x1B167}, UnicodeBlock{0x1B170, 0x1B2FB}, UnicodeBlock{0x1F004},
  UnicodeBlock{0x1F0CF},          UnicodeBlock{0x1F18E},          UnicodeBlock{0x1F191, 0x1F19A},
  UnicodeBlock{0x1F200, 0x1F202}, UnicodeBlock{0x1F210, 0x1F23B}, UnicodeBlock{0x1F240, 0x1F248},
  UnicodeBlock{0x1F250, 0x1F251}, UnicodeBlock{0x1F260, 0x1F265}, UnicodeBlock{0x1F300, 0x1F320},
  UnicodeBlock{0x1F32D, 0x1F335}, UnicodeBlock{0x1F337, 0x1F37C}, UnicodeBlock{0x1F37E, 0x1F393},
  UnicodeBlock{0x1F3A0, 0x1F3CA}, UnicodeBlock{0x1F3CF, 0x1F3D3}, UnicodeBlock{0x1F3E0, 0x1F3F0},
  UnicodeBlock{0x1F3F4},          UnicodeBlock{0x1F3F8, 0x1F43E}, UnicodeBlock{0x1F440},
  UnicodeBlock{0x1F442, 0x1F4FC}, UnicodeBlock{0x1F4FF, 0x1F53D}, UnicodeBlock{0x1F54B, 0x1F54E},
  UnicodeBlock{0x1F550, 0x1F567}, UnicodeBlock{0x1F57A},          UnicodeBlock{0x1F595, 0x1F596},
  UnicodeBlock{0x1F5A4},          UnicodeBlock{0x1F5FB, 0x1F64F}, UnicodeBlock{0x1F680, 0x1F6C5},
  UnicodeBlock{0x1F6CC},          UnicodeBlock{0x1F6D0, 0x1F6D2}, UnicodeBlock{0x1F6D5, 0x1F6D7},
  UnicodeBlock{0x1F6DD, 0x1F6DF}, UnicodeBlock{0x1F6EB, 0x1F6EC}, UnicodeBlock{0x1F6F4, 0x1F6FC},
  UnicodeBlock{0x1F7E0, 0x1F7EB}, UnicodeBlock{0x1F7F0},          UnicodeBlock{0x1F90C, 0x1F93A},
  UnicodeBlock{0x1F93C, 0x1F945}, UnicodeBlock{0x1F947, 0x1F9FF}, UnicodeBlock{0x1FA70, 0x1FA74},
  UnicodeBlock{0x1FA78, 0x1FA7C}, UnicodeBlock{0x1FA80, 0x1FA86}, UnicodeBlock{0x1FA90, 0x1FAAC},
  UnicodeBlock{0x1FAB0, 0x1FABA}, UnicodeBlock{0x1FAC0, 0x1FAC5}, UnicodeBlock{0x1FAD0, 0x1FAD9},
  UnicodeBlock{0x1FAE0, 0x1FAE7}, UnicodeBlock{0x1FAF0, 0x1FAF6}, UnicodeBlock{0x20000, 0x2FFFD},
  UnicodeBlock{0x30000, 0x3FFFD}};
// --- end generated code from 'parseEastAsiaWidth.sh' ---

// 'displayLength' returns the length of 's' in terms of how many columns would be required for
// display on a terminal, i.e, 1 column for a normal sized character and 2 for a wide character.
inline size_t displayLength(const std::string& s) {
  auto w = fromUtf8(s);
  size_t result = 0;
  for (auto i : w)
    if (inRange(i, WideBlocks))
      result += 2;
    else if (!inRange(i, NonSpacingBlocks))
      ++result;
  return result;
}

// 'wideSetw' returns a value that works for 'std::setw' when 's' contains wide chars. For
// example, if 's' contains 1 wide char that is 3 bytes then calling 'os << std::setw(4) << s'
// will not result in expected padding of 2 (the wide char plus 2 to get 4). Instead it will
// add 1 space since std::setw only looks at bytes and s already has 3 bytes. However, using
// this function, i.e., 'os << std::setw(wideSetw(s, 6)) << s' will correctly fill with 2 by
// returning '5' (5 is 2 more than the 3 byte length of 's').
inline int wideSetw(const std::string& s, int setwLen) { return setwLen + s.length() - displayLength(s); }

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_DISPLAY_LENGTH_H
