#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

// Kana

bool isHiragana(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, HiraganaBlocks);
}

bool isAllHiragana(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks);
}

bool isKatakana(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, KatakanaBlocks);
}

bool isAllKatakana(const std::string& s) {
  return inWCharRange(s, KatakanaBlocks);
}

bool isKana(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, HiraganaBlocks, KatakanaBlocks);
}

bool isAllKana(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks, KatakanaBlocks);
}

// Kanji

bool isCommonKanji(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, CommonKanjiBlocks);
}

bool isAllCommonKanji(const std::string& s) {
  return inWCharRange(s, CommonKanjiBlocks);
}

bool isRareKanji(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, RareKanjiBlocks);
}

bool isAllRareKanji(const std::string& s) {
  return inWCharRange(s, RareKanjiBlocks);
}

bool isKanji(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, CommonKanjiBlocks, RareKanjiBlocks);
}

bool isAllKanji(const std::string& s) {
  return inWCharRange(s, CommonKanjiBlocks, RareKanjiBlocks);
}

// other multi-byte characters

bool isMBPunctuation(const std::string& s, bool includeSpace, bool sizeOne) {
  return s.starts_with("　") ? (includeSpace && (s.size() < 4 || !sizeOne))
                             : inWCharRange(s, sizeOne, PunctuationBlocks);
}

bool isAllMBPunctuation(const std::string& s) {
  return inWCharRange(s, PunctuationBlocks);
}

bool isMBSymbol(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, SymbolBlocks);
}

bool isAllMBSymbol(const std::string& s) {
  return inWCharRange(s, SymbolBlocks);
}

bool isMBLetter(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, LetterBlocks);
}

bool isAllMBLetter(const std::string& s) {
  return inWCharRange(s, LetterBlocks);
}

bool isRecognizedMBChar(const std::string& s, bool sizeOne) {
  return inWCharRange(s, sizeOne, HiraganaBlocks, CommonKanjiBlocks,
      RareKanjiBlocks, KatakanaBlocks, PunctuationBlocks, SymbolBlocks,
      LetterBlocks);
}

bool isAllRecognizedCharacters(const std::string& s) {
  return inWCharRange(s, HiraganaBlocks, CommonKanjiBlocks, RareKanjiBlocks,
      KatakanaBlocks, PunctuationBlocks, SymbolBlocks, LetterBlocks);
}

} // namespace kanji_tools
