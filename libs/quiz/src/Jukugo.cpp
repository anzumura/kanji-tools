#include <kanji_tools/kana/Kana.h>
#include <kanji_tools/kana/Utf8Char.h>
#include <kanji_tools/quiz/Jukugo.h>
#include <kanji_tools/utils/UnicodeBlock.h>

namespace kanji_tools {

Jukugo::Jukugo(const String& name, const String& reading, KanjiGrades grade)
    : _name{name}, _reading{reading}, _grade{grade} {
  Utf8Char nameChars{name}, readingChars{reading};
  size_t count{};
  for (String c; nameChars.next(c);)
    if (isKanji(c)) ++count;
  if (!count) error("contains no Kanji");
  if (count < 2) error("must contain two or more Kanji");
  for (String c; readingChars.next(c);)
    if (!isHiragana(c) && c != Kana::ProlongMark)
      error("reading must be all Hiragana");
}

String Jukugo::nameAndReading() const { return _name + "（" + _reading + "）"; }

void Jukugo::error(const String& msg) const {
  throw DomainError{"jukugo '" + _name + "' " + msg};
}

} // namespace kanji_tools
