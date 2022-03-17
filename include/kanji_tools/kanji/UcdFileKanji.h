#pragma once

#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

// 'UcdFileKanji' is for kanji with attributes mainly loaded from 'data/ucd.txt'
// as opposed to kanji loaded from jouyou.txt, jinmei.txt, linked-jinmei.txt
// and extra.txt. There are '_hasOldLinks' and '_linkNames' field for supporting
// ucd links as well '_linkedReadings' (see Kanji.h for details). StandardKanji
// and UcdKanji derive from this class. 'UcdFileKanji' are not in JLPT and are
// meant for less common kanji that haven't already been loaded from a custom
// file (see CustomFileKanji.h).
class UcdFileKanji : public NonLinkedKanji {
public:
  [[nodiscard]] const LinkNames& oldNames() const override {
    return _hasOldLinks ? _linkNames : EmptyLinkNames;
  }
  [[nodiscard]] OptString newName() const override {
    return _linkNames.empty() || _hasOldLinks ? std::nullopt
                                              : OptString{_linkNames[0]};
  }
  [[nodiscard]] bool linkedReadings() const override { return _linkedReadings; }
protected:
  // constructor used by 'StandardKanji': has 'reading'
  UcdFileKanji(const Data& d, const std::string& name,
      const std::string& reading, const Ucd* u)
      : NonLinkedKanji{d, name, d.ucdRadical(name, u), reading,
            d.getStrokes(name, u), u},
        _hasOldLinks{u && u->hasTraditionalLinks()}, _linkNames{linkNames(u)},
        _linkedReadings{u && u->linkedReadings()} {}

  // constructor used by 'StandardKanji' and 'UcdKanji': looks up 'reading'
  UcdFileKanji(const Data& d, const std::string& name, const Ucd* u)
      : UcdFileKanji{d, name, d.ucd().getReadingsAsKana(u), u} {}
private:
  const bool _hasOldLinks;

  // Use 'LinkNames' instead of trying to hold an OptEntry (shared pointer to
  // another kanji) since 'ucd links' are more arbitrary than the standard
  // 'official' jinmei and jouyou linked kanji (ie official variants). Ucd links
  // can potentially even be circular depending on how the source data is parsed
  // and there are also cases of links to another ucd kanji with a link.
  const LinkNames _linkNames;

  const bool _linkedReadings;
};

// 'StandardKanji' is the base class for FrequencyKanji and KenteiKanji and has
// a '_kyu' field. In addition to OfficialKanji, these kanji are included in
// 'kanjiQuiz' and are generally recognized as standard Japanese characters.
class StandardKanji : public UcdFileKanji {
public:
  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
protected:
  // constructor used by 'FrequencyKanji': has 'reading' and looks up 'kyu'
  StandardKanji(
      const Data& d, const std::string& name, const std::string& reading)
      : UcdFileKanji{d, name, reading, d.findUcd(name)}, _kyu{d.kyu(name)} {}

  // constructor used by 'FrequencyKanji': looks up 'kyu'
  StandardKanji(const Data& d, const std::string& name)
      : StandardKanji{d, name, d.kyu(name)} {}

  // constructor used by 'KenteiKanji': has 'kyu'
  StandardKanji(const Data& d, const std::string& name, KenteiKyus kyu)
      : UcdFileKanji{d, name, d.findUcd(name)}, _kyu{kyu} {}
private:
  const KenteiKyus _kyu;
};

// 'FrequencyKanji' is for kanji from 'frequency.txt' that aren't already loaded
// from jouyou or jinmei files
class FrequencyKanji : public StandardKanji {
public:
  // constructor used for 'FrequencyKanji' without a reading
  FrequencyKanji(const Data& d, const std::string& name, u_int16_t frequency)
      : StandardKanji{d, name}, _frequency{frequency} {}

  // constructor used for 'FrequencyKanji' with readings from
  // 'frequency-readings.txt'
  FrequencyKanji(const Data& d, const std::string& name,
      const std::string& reading, u_int16_t frequency)
      : StandardKanji{d, name, reading}, _frequency{frequency} {}

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::Frequency;
  }
  [[nodiscard]] OptU16 frequency() const override { return _frequency; }
private:
  const u_int16_t _frequency;
};

// 'KenteiKanji' is for kanji in 'kentei/k*.txt' files that aren't already
// pulled in from other files
class KenteiKanji : public StandardKanji {
public:
  KenteiKanji(const Data& d, const std::string& name, KenteiKyus kyu)
      : StandardKanji{d, name, kyu} {}

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Kentei; }
};

// 'UcdKanji' is for kanji in 'ucd.txt' file that aren't already included in any
// other 'types'. Many of these kanji are in 'Dai Kan-Wa Jiten' (ie, they have a
// Morohashi ID), but others are pulled in via links and may not even have a
// Japanese reading.
class UcdKanji : public UcdFileKanji {
public:
  UcdKanji(const Data& d, const Ucd& u) : UcdFileKanji{d, u.name(), &u} {}

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Ucd; }
};

} // namespace kanji_tools
