#pragma once

#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

// 'UcdFileKanji' is for kanji with attributes mainly loaded from 'data/ucd.txt'
// as opposed to Kanji loaded from 'jouyou.txt', 'jinmei.txt', and 'extra.txt'.
// There are '_hasOldLinks' and '_linkNames' field for supporting 'ucd links' as
// well as '_linkedReadings' (see Kanji.h). 'UcdFileKanji' are not in JLPT and
// are meant for less common kanji that haven't already been loaded from a
// custom file (see CustomFileKanji.h).
class UcdFileKanji : public NonLinkedKanji {
public:
  [[nodiscard]] const LinkNames& oldNames() const override;
  [[nodiscard]] OptString newName() const override;
  [[nodiscard]] bool linkedReadings() const override { return _linkedReadings; }
protected:
  // ctor used by 'StandardKanji': has 'reading'
  UcdFileKanji(const Data&, const std::string& name, const std::string& reading,
      const Ucd*);
  // ctor used by 'StandardKanji' and 'UcdKanji': looks up 'reading'
  UcdFileKanji(const Data&, const std::string& name, const Ucd*);
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

// 'StandardKanji' is the base class for 'FrequencyKanji' and 'KenteiKanji' and
// has a '_kyu' field. In addition to 'OfficialKanji', these Kanji are included
// in 'kanjiQuiz' and are generally recognized as standard Japanese characters.
class StandardKanji : public UcdFileKanji {
public:
  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
protected:
  // ctor used by 'FrequencyKanji': has 'reading' and looks up 'kyu'
  StandardKanji(const Data&, const std::string&, const std::string& reading);

  // ctor used by 'FrequencyKanji': looks up 'kyu'
  StandardKanji(const Data&, const std::string&);

  // ctor used by 'KenteiKanji': has 'kyu'
  StandardKanji(const Data&, const std::string&, KenteiKyus);
private:
  const KenteiKyus _kyu;
};

// 'FrequencyKanji' is for kanji from 'frequency.txt' that aren't already loaded
// from jouyou or jinmei files
class FrequencyKanji : public StandardKanji {
public:
  // ctor used for 'FrequencyKanji' without a reading
  FrequencyKanji(const Data& d, const std::string& name, Frequency);

  // ctor used for 'FrequencyKanji' with readings from 'frequency-readings.txt'
  FrequencyKanji(const Data&, const std::string& name,
      const std::string& reading, Frequency);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::Frequency;
  }
  [[nodiscard]] OptFreq frequency() const override { return _frequency; }
private:
  const Frequency _frequency;
};

// 'KenteiKanji' is for kanji in 'kentei/k*.txt' files that aren't already
// pulled in from other files
class KenteiKanji : public StandardKanji {
public:
  KenteiKanji(const Data&, const std::string& name, KenteiKyus);

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Kentei; }
};

// 'UcdKanji' is for Kanji in 'ucd.txt' file that aren't already included in any
// other 'types'. Many of these Kanji are in 'Dai Kan-Wa Jiten' (ie, they have a
// Morohashi ID), but others are pulled in via links and may not even have a
// Japanese reading.
class UcdKanji : public UcdFileKanji {
public:
  UcdKanji(const Data&, const Ucd&);

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Ucd; }
};

} // namespace kanji_tools
