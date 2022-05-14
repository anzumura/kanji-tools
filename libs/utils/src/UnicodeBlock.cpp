#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

std::ostream& operator<<(std::ostream& os, const UnicodeVersion& v) {
  // simplify the following once 'std::format' is available (in Clang and GCC)
  const unsigned m{v.date().month()};
  const int y{v.date().year()};
  return os << 'v' << v.version() << ": " << m << ", " << y;
}

std::ostream& operator<<(std::ostream& os, const UnicodeBlock& b) {
  if (b.name().empty())
    os << "start=" << toUnicode(b.start()) << ", end=" << toUnicode(b.end());
  else
    os << b.name();
  if (b.version()) os << " (" << *b.version() << ')';
  return os;
}

// Kana

bool isHiragana(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, HiraganaBlocks);
}

bool isAllHiragana(const String& s) { return inWCharRange(s, HiraganaBlocks); }

bool isKatakana(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, KatakanaBlocks);
}

bool isAllKatakana(const String& s) { return inWCharRange(s, KatakanaBlocks); }

bool isKana(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, HiraganaBlocks, KatakanaBlocks);
}

bool isAllKana(const String& s) {
  return inWCharRange(s, HiraganaBlocks, KatakanaBlocks);
}

// Kanji

bool isCommonKanji(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, CommonKanjiBlocks);
}

bool isAllCommonKanji(const String& s) {
  return inWCharRange(s, CommonKanjiBlocks);
}

bool isRareKanji(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, RareKanjiBlocks);
}

bool isAllRareKanji(const String& s) {
  return inWCharRange(s, RareKanjiBlocks);
}

bool isKanji(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, CommonKanjiBlocks, RareKanjiBlocks);
}

bool isAllKanji(const String& s) {
  return inWCharRange(s, CommonKanjiBlocks, RareKanjiBlocks);
}

// other multi-byte characters

bool isMBPunctuation(const String& s, bool includeSpace, bool sizeOne) {
  return s.starts_with("　") ? (includeSpace && (s.size() < 4 || !sizeOne))
                             : inWCharRange(s, sizeOne, PunctuationBlocks);
}

bool isAllMBPunctuation(const String& s) {
  return inWCharRange(s, PunctuationBlocks);
}

bool isMBSymbol(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, SymbolBlocks);
}

bool isAllMBSymbol(const String& s) { return inWCharRange(s, SymbolBlocks); }

bool isMBLetter(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, LetterBlocks);
}

bool isAllMBLetter(const String& s) { return inWCharRange(s, LetterBlocks); }

bool isRecognizedUtf8(const String& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, HiraganaBlocks, CommonKanjiBlocks,
      RareKanjiBlocks, KatakanaBlocks, PunctuationBlocks, SymbolBlocks,
      LetterBlocks);
}

bool isAllRecognizedUtf8(const String& s) {
  return inWCharRange(s, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks,
      KatakanaBlocks, PunctuationBlocks, SymbolBlocks, LetterBlocks);
}

} // namespace kanji_tools
