#include <kanji_tools/kanji/UcdFileKanji.h>

namespace kanji_tools {

const Kanji::LinkNames& UcdFileKanji::oldNames() const {
  return _hasOldLinks ? _linkNames : EmptyLinkNames;
}

Kanji::OptString UcdFileKanji::newName() const {
  return _linkNames.empty() || _hasOldLinks ? std::nullopt
                                            : OptString{_linkNames[0]};
}

UcdFileKanji::UcdFileKanji(const Data& data, const std::string& name,
    const std::string& reading, const Ucd* u)
    : NonLinkedKanji{data, name, data.ucdRadical(name, u),
          data.ucdStrokes(name, u), reading, u},
      _hasOldLinks{u && u->hasTraditionalLinks()}, _linkNames{linkNames(u)},
      _linkedReadings{u && u->linkedReadings()} {}

UcdFileKanji::UcdFileKanji(
    const Data& data, const std::string& name, const Ucd* u) // LCOV_EXCL_LINE
    : UcdFileKanji{data, name, data.ucd().getReadingsAsKana(u), u} {}

StandardKanji::StandardKanji(
    const Data& data, const std::string& name, const std::string& reading)
    : UcdFileKanji{data, name, reading, data.findUcd(name)}, _kyu{data.kyu(
                                                                 name)} {}

StandardKanji::StandardKanji(const Data& data, const std::string& name)
    : StandardKanji{data, name, data.kyu(name)} {}

StandardKanji::StandardKanji(
    const Data& data, const std::string& name, KenteiKyus kyu)
    : UcdFileKanji{data, name, data.findUcd(name)}, _kyu{kyu} {}

FrequencyKanji::FrequencyKanji(
    const Data& data, const std::string& name, Frequency frequency)
    : StandardKanji{data, name}, _frequency{frequency} {}

FrequencyKanji::FrequencyKanji(const Data& data, const std::string& name,
    const std::string& reading, Frequency frequency)
    : StandardKanji{data, name, reading}, _frequency{frequency} {}

KenteiKanji::KenteiKanji(
    const Data& data, const std::string& name, KenteiKyus kyu)
    : StandardKanji{data, name, kyu} {}

UcdKanji::UcdKanji(const Data& data, const Ucd& u)
    : UcdFileKanji{data, u.name(), &u} {}

} // namespace kanji_tools
