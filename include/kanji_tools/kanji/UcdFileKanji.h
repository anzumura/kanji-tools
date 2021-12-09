#ifndef KANJI_TOOLS_KANJI_UCD_FILE_KANJI_H
#define KANJI_TOOLS_KANJI_UCD_FILE_KANJI_H

#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

// 'UcdFileKanji' is for kanji with attributes mainly loaded from 'data/ucd.txt' (as opposed to kanji
// loaded from jouyou.txt, jinmei.txt, linked-jinmei.txt and extra.txt). There are '_hasOldLinks' and
// '_linkNames' field for supporting ucd links as well '_linkedReadings' (see Kanji.h for details).
// StandardKanji and UcdKanji derive from this class. 'UcdFileKanji' are not in JLPT and are meant
// for less common kanji that haven't already been loaded from a custom file (see CustomFileKanji.h).
class UcdFileKanji : public NonLinkedKanji {
public:
  const LinkNames& oldNames() const override { return _hasOldLinks ? _linkNames : EmptyLinkNames; }
  OptString newName() const override {
    return _linkNames.empty() || _hasOldLinks ? std::nullopt : OptString(_linkNames[0]);
  }
  bool linkedReadings() const override { return _linkedReadings; }
protected:
  UcdFileKanji(const Data& d, const std::string& name, const std::string& reading, const Ucd* u)
    : NonLinkedKanji(d, name, d.ucdRadical(name, u), reading, d.getStrokes(name, u), u),
      _hasOldLinks(u && u->hasTraditionalLinks()), _linkNames(getLinkNames(u)),
      _linkedReadings(u && u->linkedReadings()) {}
  UcdFileKanji(const Data& d, const std::string& name, const Ucd* u)
    : UcdFileKanji(d, name, d.ucd().getReadingsAsKana(u), u) {}
private:
  // Use 'LinkNames' instead of trying to hold an OptEntry (shared pointer to another loaded kanji) since
  // 'ucd links' are more arbitrary than the standard 'official' jinmei and jouyou linked kanji (ie official
  // variants). Ucd links can potentially even be circular depending on how the source data is parsed and
  // there are also cases of links to another ucd kanji with a link.
  const bool _hasOldLinks;
  const LinkNames _linkNames;
  const bool _linkedReadings;
};

// 'StandardKanji' is the base class for FrequencyKanji and KenteiKanji and has a '_kyu' field. In
// addition to OfficialKanji, these kanji are included in the 'kanjiQuiz' program and are generally
// recognized as standard Japanese characters.
class StandardKanji : public UcdFileKanji {
public:
  KenteiKyus kyu() const override { return _kyu; }
protected:
  // constructors used by FrequencyKanji
  StandardKanji(const Data& d, const std::string& name) : StandardKanji(d, name, d.getKyu(name)) {}
  StandardKanji(const Data& d, const std::string& name, const std::string& reading)
    : UcdFileKanji(d, name, reading, d.findUcd(name)), _kyu(d.getKyu(name)) {}
  // constructor used by KenteiKanji
  StandardKanji(const Data& d, const std::string& name, KenteiKyus kyu)
    : UcdFileKanji(d, name, d.findUcd(name)), _kyu(kyu) {}
private:
  const KenteiKyus _kyu;
};

// 'FrequencyKanji' is for kanji from 'frequency.txt' that aren't already loaded from jouyou or jinmei files
class FrequencyKanji : public StandardKanji {
public:
  // constructor used for 'FrequencyKanji' without a reading
  FrequencyKanji(const Data& d, const std::string& name, int frequency)
    : StandardKanji(d, name), _frequency(frequency) {}
  // constructor used for 'FrequencyKanji' with readings from 'frequency-readings.txt'
  FrequencyKanji(const Data& d, const std::string& name, const std::string& reading, int frequency)
    : StandardKanji(d, name, reading), _frequency(frequency) {}

  KanjiTypes type() const override { return KanjiTypes::Frequency; }
  OptInt frequency() const override { return OptInt(_frequency); }
private:
  const int _frequency;
};

// 'KenteiKanji' is for kanji in 'kentei/k*.txt' files that aren't already pulled in from other files
class KenteiKanji : public StandardKanji {
public:
  KenteiKanji(const Data& d, const std::string& name, KenteiKyus kyu) : StandardKanji(d, name, kyu) {}

  KanjiTypes type() const override { return KanjiTypes::Kentei; }
};

// 'UcdKanji' is for kanji in 'ucd.txt' file that aren't already included in any other 'types'. Many
// of these kanji are in 'Dai Kan-Wa Jiten' (ie, they have a Morohashi ID), but others are pulled in
// via links and may not even have a Japanese reading.
class UcdKanji : public UcdFileKanji {
public:
  UcdKanji(const Data& d, const Ucd& u) : UcdFileKanji(d, u.name(), &u) {}

  KanjiTypes type() const override { return KanjiTypes::Ucd; }
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_UCD_FILE_KANJI_H
