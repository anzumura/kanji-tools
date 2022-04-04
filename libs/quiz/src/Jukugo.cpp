#include <kanji_tools/kana/Kana.h>
#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/quiz/Jukugo.h>
#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

Jukugo::Jukugo(
    const std::string& name, const std::string& reading, KanjiGrades grade)
    : _name{name}, _reading{reading}, _grade{grade} {
  MBChar nameChars{name}, readingChars{reading};
  size_t count{};
  for (std::string c; nameChars.next(c);)
    if (isKanji(c)) ++count;
  if (!count) error("contains no Kanji");
  if (count < 2) error("must contain two or more Kanji");
  for (std::string c; readingChars.next(c);)
    if (!isHiragana(c) && c != Kana::ProlongMark)
      error("reading must be all Hiragana");
}

std::string Jukugo::nameAndReading() const {
  return _name + "（" + _reading + "）";
}

void Jukugo::error(const std::string& msg) const {
  throw std::domain_error{"jukugo '" + _name + "' " + msg};
}

} // namespace kanji_tools
