#ifndef KANJI_TOOLS_KANJI_OTHER_KANJI_H
#define KANJI_TOOLS_KANJI_OTHER_KANJI_H

#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

// 'OtherKanji' is for kanji from 'frequency.txt' that aren't already loaded from jouyou or jinmei files
class OtherKanji : public NonLinkedKanji {
public:
  // constructor used for 'Other' kanji with readings from 'other-readings.txt'
  OtherKanji(const Data& d, int number, const std::string& name, const std::string& reading)
    : NonLinkedKanji(d, number, name, reading, d.findUcd(name)) {}
  // constructor used for 'Other' kanji without a reading
  OtherKanji(const Data& d, int number, const std::string& name) : NonLinkedKanji(d, number, name, d.findUcd(name)) {}

  KanjiTypes type() const override { return KanjiTypes::Other; }
};

// 'KenteiKanji' is for kanji in 'kentei/k*.txt' files that aren't already pulled in from other files
class KenteiKanji : public NonLinkedKanji {
public:
  KenteiKanji(const Data& d, int number, const std::string& name)
    : NonLinkedKanji(d, number, name, d.findUcd(name), false) {}

  KanjiTypes type() const override { return KanjiTypes::Kentei; }
};

// 'UcdKanji' is for kanji in 'ucd.txt' file that aren't already pulled in from any other files
class UcdKanji : public NonLinkedKanji {
public:
  UcdKanji(const Data& d, int number, const Ucd& u) : NonLinkedKanji(d, number, u.name(), &u, false, false) {}

  KanjiTypes type() const override { return KanjiTypes::Ucd; }
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_OTHER_KANJI_H
