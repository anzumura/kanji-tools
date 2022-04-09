#include <kanji_tools/kanji/UcdFileKanji.h>

namespace kanji_tools {

const Kanji::LinkNames& UcdFileKanji::oldNames() const {
  return _hasOldLinks ? _linkNames : EmptyLinkNames;
}

Kanji::OptString UcdFileKanji::newName() const {
  return _linkNames.empty() || _hasOldLinks ? std::nullopt
                                            : OptString{_linkNames[0]};
}

UcdFileKanji::UcdFileKanji(const Data& d, const std::string& name,
    const std::string& reading, const Ucd* u)
    : NonLinkedKanji{d, name, d.ucdRadical(name, u), reading,
          d.ucdStrokes(name, u), u},
      _hasOldLinks{u && u->hasTraditionalLinks()}, _linkNames{linkNames(u)},
      _linkedReadings{u && u->linkedReadings()} {}

UcdFileKanji::UcdFileKanji(const Data& d, const std::string& name, const Ucd* u)
    : UcdFileKanji{d, name, d.ucd().getReadingsAsKana(u), u} {}

StandardKanji::StandardKanji(
    const Data& d, const std::string& name, const std::string& reading)
    : UcdFileKanji{d, name, reading, d.findUcd(name)}, _kyu{d.kyu(name)} {}

StandardKanji::StandardKanji(const Data& d, const std::string& name)
    : StandardKanji{d, name, d.kyu(name)} {}

StandardKanji::StandardKanji(
    const Data& d, const std::string& name, KenteiKyus kyu)
    : UcdFileKanji{d, name, d.findUcd(name)}, _kyu{kyu} {}

FrequencyKanji::FrequencyKanji(
    const Data& d, const std::string& name, Frequency frequency)
    : StandardKanji{d, name}, _frequency{frequency} {}

FrequencyKanji::FrequencyKanji(const Data& d, const std::string& name,
    const std::string& reading, Frequency frequency)
    : StandardKanji{d, name, reading}, _frequency{frequency} {}

KenteiKanji::KenteiKanji(const Data& d, const std::string& name, KenteiKyus kyu)
    : StandardKanji{d, name, kyu} {}

UcdKanji::UcdKanji(const Data& d, const Ucd& u)
    : UcdFileKanji{d, u.name(), &u} {}

} // namespace kanji_tools
