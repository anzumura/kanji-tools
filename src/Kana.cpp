#include <kanji/Kana.h>
#include <kanji/KanaConvert.h>
#include <kanji/MBUtils.h>

namespace kanji {

const std::string& Kana::getRomaji(int flags) const {
  return (flags & KanaConvert::Hepburn) && _hepburn.has_value() ? *_hepburn
    : (flags & KanaConvert::Kunrei) && _kunreiVariant           ? _variants[0]
    : (flags & KanaConvert::Kunrei) && _kunrei.has_value()      ? *_kunrei
                                                                : _romaji;
}

const std::string& Kana::get(CharType t, int flags) const {
  switch (t) {
  case CharType::Romaji: return getRomaji(flags);
  case CharType::Hiragana: return _hiragana;
  case CharType::Katakana: return _katakana;
  }
}

void Kana::validate() const {
  for (auto& i : _variants)
    assert(!i.empty() && i.length() < 4);                           // must be 1 to 3 chars
  assert(!_romaji.empty() && _romaji.length() < 4);           // must be 1 to 3 chars
  assert(_hiragana.length() == 3 || _hiragana.length() == 6); // 3 bytes per character
  assert(_katakana.length() == 3 || _katakana.length() == 6); // 3 bytes per characer
  assert(isAllSingleByte(_romaji));
  assert(isAllHiragana(_hiragana));
  assert(isAllKatakana(_katakana));
}

} // namespace kanji
