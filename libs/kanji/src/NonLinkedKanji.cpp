#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

Kanji::LinkNames NonLinkedKanji::linkNames(const Ucd* u) {
  LinkNames result;
  if (u && u->hasLinks())
    std::transform(u->links().begin(), u->links().end(),
        std::back_inserter(result), [](const auto& i) { return i.name(); });
  return result;
}

NonLinkedKanji::NonLinkedKanji(const Data& d, const std::string& name,
    // LCOV_EXCL_START
    const Radical& rad, const std::string& meaning, const std::string& reading,
    // LCOV_EXCL_STOP
    Strokes s, const Ucd* u)
    : Kanji{name, d.getCompatibilityName(name), rad, s, d.getMorohashiId(u),
          d.getNelsonIds(u), d.getPinyin(u)},
      _meaning{meaning}, _reading{reading} {}

NonLinkedKanji::NonLinkedKanji(const Data& d, const std::string& name,
    const Radical& rad, const std::string& reading, Strokes s, const Ucd* u)
    : NonLinkedKanji{d, name, rad, d.ucd().getMeaning(u), reading, s, u} {}

} // namespace kanji_tools
