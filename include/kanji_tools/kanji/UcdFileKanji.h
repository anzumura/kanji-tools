#ifndef KANJI_TOOLS_KANJI_UCD_FILE_KANJI_H
#define KANJI_TOOLS_KANJI_UCD_FILE_KANJI_H

#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

// 'UcdFileKanji' is for kanji with attributes mainly loaded from 'data/ucd.txt' (as opposed to
// kanji loaded from jouyou.txt, jinmei.txt, linked-jinmei.txt and extra.txt). There is an optional
// 'newName' field for supporting ucd links and is the base class for FrequencyKanji, KenteiKanji
// and UcdKanji. 'UcdFileKanji' should not have a JLPT level and are meant to hold less common kanji
// that haven't already been loaded from a custom file (see CustomFileKanji.h).
class UcdFileKanji : public NonLinkedKanji {
public:
  OptString newName() const override { return _newName; }
protected:
  UcdFileKanji(const Data& d, int number, const std::string& name, const std::string& reading, const Ucd* u,
               bool findFrequency = true, bool findKyu = true)
    : NonLinkedKanji(d, number, name, d.ucdRadical(name, u), reading, d.getStrokes(name, u), u, findFrequency, false,
                     findKyu),
      _newName(u && u->hasLink() ? std::optional(u->linkName()) : std::nullopt) {}
  UcdFileKanji(const Data& d, int number, const std::string& name, const Ucd* u, bool findFrequency = true,
               bool findKyu = true)
    : UcdFileKanji(d, number, name, d.ucd().getReadingsAsKana(u), u, findFrequency, findKyu) {}
private:
  // Use an OptString for 'newName' instead of trying to hold an OptEntry (shared pointer to another
  // loaded kanji) since 'ucd links' are more arbitrary than the standard 'official' jinmei and jouyou
  // linked kanji (ie official variants). Ucd links can potentially even be circular depending on how
  // the source data is parsed and there are also cases of links to another ucd kanji with a link.
  const OptString _newName;
};

// 'FrequencyKanji' is for kanji from 'frequency.txt' that aren't already loaded from jouyou or jinmei files
class FrequencyKanji : public UcdFileKanji {
public:
  // constructor used for 'FrequencyKanji' with readings from 'frequency-readings.txt'
  FrequencyKanji(const Data& d, int number, const std::string& name, const std::string& reading)
    : UcdFileKanji(d, number, name, reading, d.findUcd(name)) {}
  // constructor used for 'FrequencyKanji' without a reading
  FrequencyKanji(const Data& d, int number, const std::string& name) : UcdFileKanji(d, number, name, d.findUcd(name)) {}

  KanjiTypes type() const override { return KanjiTypes::Frequency; }
};

// 'KenteiKanji' is for kanji in 'kentei/k*.txt' files that aren't already pulled in from other files
class KenteiKanji : public UcdFileKanji {
public:
  KenteiKanji(const Data& d, int number, const std::string& name)
    : UcdFileKanji(d, number, name, d.findUcd(name), false) {}

  KanjiTypes type() const override { return KanjiTypes::Kentei; }
};

// 'UcdKanji' is for kanji in 'ucd.txt' file that aren't already pulled in from any other files
class UcdKanji : public UcdFileKanji {
public:
  UcdKanji(const Data& d, int number, const Ucd& u) : UcdFileKanji(d, number, u.name(), &u, false, false) {}

  KanjiTypes type() const override { return KanjiTypes::Ucd; }
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_UCD_FILE_KANJI_H
