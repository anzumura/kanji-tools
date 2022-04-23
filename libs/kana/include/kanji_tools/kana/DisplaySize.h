#pragma once

#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

// --- begin generated code from 'parseEastAsiaWidth.sh' ---
inline constexpr std::array WideBlocks{makeBlock<0x1100, 0x115F>(),
    makeBlock<0x231A, 0x231B>(), makeBlock<0x2329, 0x232A>(),
    makeBlock<0x23E9, 0x23EC>(), makeBlock<0x23F0>(), makeBlock<0x23F3>(),
    makeBlock<0x25FD, 0x25FE>(), makeBlock<0x2614, 0x2615>(),
    makeBlock<0x2648, 0x2653>(), makeBlock<0x267F>(), makeBlock<0x2693>(),
    makeBlock<0x26A1>(), makeBlock<0x26AA, 0x26AB>(),
    makeBlock<0x26BD, 0x26BE>(), makeBlock<0x26C4, 0x26C5>(),
    makeBlock<0x26CE>(), makeBlock<0x26D4>(), makeBlock<0x26EA>(),
    makeBlock<0x26F2, 0x26F3>(), makeBlock<0x26F5>(), makeBlock<0x26FA>(),
    makeBlock<0x26FD>(), makeBlock<0x2705>(), makeBlock<0x270A, 0x270B>(),
    makeBlock<0x2728>(), makeBlock<0x274C>(), makeBlock<0x274E>(),
    makeBlock<0x2753, 0x2755>(), makeBlock<0x2757>(),
    makeBlock<0x2795, 0x2797>(), makeBlock<0x27B0>(), makeBlock<0x27BF>(),
    makeBlock<0x2B1B, 0x2B1C>(), makeBlock<0x2B50>(), makeBlock<0x2B55>(),
    makeBlock<0x2E80, 0x2E99>(), makeBlock<0x2E9B, 0x2EF3>(),
    makeBlock<0x2F00, 0x2FD5>(), makeBlock<0x2FF0, 0x2FFB>(),
    makeBlock<0x3000, 0x303E>(), makeBlock<0x3041, 0x3096>(),
    makeBlock<0x3099, 0x30FF>(), makeBlock<0x3105, 0x312F>(),
    makeBlock<0x3131, 0x318E>(), makeBlock<0x3190, 0x31E3>(),
    makeBlock<0x31F0, 0x321E>(), makeBlock<0x3220, 0x3247>(),
    makeBlock<0x3250, 0x4DBF>(), makeBlock<0x4E00, 0xA48C>(),
    makeBlock<0xA490, 0xA4C6>(), makeBlock<0xA960, 0xA97C>(),
    makeBlock<0xAC00, 0xD7A3>(), makeBlock<0xF900, 0xFAFF>(),
    makeBlock<0xFE10, 0xFE19>(), makeBlock<0xFE30, 0xFE52>(),
    makeBlock<0xFE54, 0xFE66>(), makeBlock<0xFE68, 0xFE6B>(),
    makeBlock<0xFF01, 0xFF60>(), makeBlock<0xFFE0, 0xFFE6>(),
    makeBlock<0x16FE0, 0x16FE4>(), makeBlock<0x16FF0, 0x16FF1>(),
    makeBlock<0x17000, 0x187F7>(), makeBlock<0x18800, 0x18CD5>(),
    makeBlock<0x18D00, 0x18D08>(), makeBlock<0x1AFF0, 0x1AFF3>(),
    makeBlock<0x1AFF5, 0x1AFFB>(), makeBlock<0x1AFFD, 0x1AFFE>(),
    makeBlock<0x1B000, 0x1B122>(), makeBlock<0x1B150, 0x1B152>(),
    makeBlock<0x1B164, 0x1B167>(), makeBlock<0x1B170, 0x1B2FB>(),
    makeBlock<0x1F004>(), makeBlock<0x1F0CF>(), makeBlock<0x1F18E>(),
    makeBlock<0x1F191, 0x1F19A>(), makeBlock<0x1F200, 0x1F202>(),
    makeBlock<0x1F210, 0x1F23B>(), makeBlock<0x1F240, 0x1F248>(),
    makeBlock<0x1F250, 0x1F251>(), makeBlock<0x1F260, 0x1F265>(),
    makeBlock<0x1F300, 0x1F320>(), makeBlock<0x1F32D, 0x1F335>(),
    makeBlock<0x1F337, 0x1F37C>(), makeBlock<0x1F37E, 0x1F393>(),
    makeBlock<0x1F3A0, 0x1F3CA>(), makeBlock<0x1F3CF, 0x1F3D3>(),
    makeBlock<0x1F3E0, 0x1F3F0>(), makeBlock<0x1F3F4>(),
    makeBlock<0x1F3F8, 0x1F43E>(), makeBlock<0x1F440>(),
    makeBlock<0x1F442, 0x1F4FC>(), makeBlock<0x1F4FF, 0x1F53D>(),
    makeBlock<0x1F54B, 0x1F54E>(), makeBlock<0x1F550, 0x1F567>(),
    makeBlock<0x1F57A>(), makeBlock<0x1F595, 0x1F596>(), makeBlock<0x1F5A4>(),
    makeBlock<0x1F5FB, 0x1F64F>(), makeBlock<0x1F680, 0x1F6C5>(),
    makeBlock<0x1F6CC>(), makeBlock<0x1F6D0, 0x1F6D2>(),
    makeBlock<0x1F6D5, 0x1F6D7>(), makeBlock<0x1F6DD, 0x1F6DF>(),
    makeBlock<0x1F6EB, 0x1F6EC>(), makeBlock<0x1F6F4, 0x1F6FC>(),
    makeBlock<0x1F7E0, 0x1F7EB>(), makeBlock<0x1F7F0>(),
    makeBlock<0x1F90C, 0x1F93A>(), makeBlock<0x1F93C, 0x1F945>(),
    makeBlock<0x1F947, 0x1F9FF>(), makeBlock<0x1FA70, 0x1FA74>(),
    makeBlock<0x1FA78, 0x1FA7C>(), makeBlock<0x1FA80, 0x1FA86>(),
    makeBlock<0x1FA90, 0x1FAAC>(), makeBlock<0x1FAB0, 0x1FABA>(),
    makeBlock<0x1FAC0, 0x1FAC5>(), makeBlock<0x1FAD0, 0x1FAD9>(),
    makeBlock<0x1FAE0, 0x1FAE7>(), makeBlock<0x1FAF0, 0x1FAF6>(),
    makeBlock<0x20000, 0x2FFFD>(), makeBlock<0x30000, 0x3FFFD>()};
// --- end generated code from 'parseEastAsiaWidth.sh' ---

// return size in terms of how many columns would be required for display on a
// terminal, i.e, 1 for a normal character and 2 for a wide character
[[nodiscard]] size_t displaySize(const std::u32string&);
[[nodiscard]] size_t displaySize(const char*);
[[nodiscard]] size_t displaySize(const std::string&);

// 'wideSetw' returns a value that works for 'std::setw' when 's' contains wide
// chars. For example, if 's' contains 1 wide char that is 3 bytes then calling
// 'os << std::setw(4) << s' will not result in expected padding of 2 (the wide
// char plus 2 to get 4). Instead it will add 1 space since std::setw only looks
// at bytes and s already has 3 bytes. However, using this function, i.e., 'os
// << std::setw(wideSetw(s, 6)) << s' will correctly fill with 2 by returning
// '5' (5 is 2 more than the 3 byte size of 's').
[[nodiscard]] int wideSetw(const std::string& s, size_t setwLen);

} // namespace kanji_tools
