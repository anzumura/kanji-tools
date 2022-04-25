#include <kanji_tools/kanji/UcdFileKanji.h>

namespace kanji_tools {

const Kanji::LinkNames& UcdFileKanji::oldNames() const {
  return _hasOldLinks ? _linkNames : EmptyLinkNames;
}

Kanji::OptString UcdFileKanji::newName() const {
  return _linkNames.empty() || _hasOldLinks ? std::nullopt
                                            : OptString{_linkNames[0]};
}

UcdFileKanji::UcdFileKanji(DataRef data, Name name, Reading reading, UcdPtr u)
    : NonLinkedKanji{data, name, data.ucdRadical(name, u), reading, u},
      _hasOldLinks{u && u->hasTraditionalLinks()}, _linkNames{linkNames(u)},
      _linkedReadings{u && u->linkedReadings()} {}

UcdFileKanji::UcdFileKanji(DataRef data, Name name, UcdPtr u) // LCOV_EXCL_LINE
    : UcdFileKanji{data, name, data.ucd().getReadingsAsKana(u), u} {}

StandardKanji::StandardKanji(DataRef data, Name name, Reading reading)
    : UcdFileKanji{data, name, reading, data.findUcd(name)}, _kyu{data.kyu(
                                                                 name)} {}

StandardKanji::StandardKanji(DataRef data, Name name)
    : StandardKanji{data, name, data.kyu(name)} {}

StandardKanji::StandardKanji(DataRef data, Name name, KenteiKyus kyu)
    : UcdFileKanji{data, name, data.findUcd(name)}, _kyu{kyu} {}

FrequencyKanji::FrequencyKanji(DataRef data, Name name, Frequency frequency)
    : StandardKanji{data, name}, _frequency{frequency} {}

FrequencyKanji::FrequencyKanji(
    DataRef data, const std::string& name, Reading reading, Frequency frequency)
    : StandardKanji{data, name, reading}, _frequency{frequency} {}

KenteiKanji::KenteiKanji(DataRef data, Name name, KenteiKyus kyu)
    : StandardKanji{data, name, kyu} {}

UcdKanji::UcdKanji(DataRef data, const Ucd& u)
    : UcdFileKanji{data, u.name(), &u} {}

} // namespace kanji_tools
