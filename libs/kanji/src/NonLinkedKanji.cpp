#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

Kanji::LinkNames NonLinkedKanji::linkNames(const Ucd* u) {
  LinkNames result;
  if (u && u->hasLinks())
    std::transform(u->links().begin(), u->links().end(),
        std::back_inserter(result), [](const auto& i) { return i.name(); });
  return result;
}

NonLinkedKanji::NonLinkedKanji(const Data& data, const std::string& name,
    // LCOV_EXCL_START
    const Radical& radical, Strokes strokes, const std::string& meaning,
    // LCOV_EXCL_STOP
    const std::string& reading, const Ucd* u)
    : Kanji{name, data.getCompatibilityName(name), radical, strokes,
          data.getMorohashiId(u), data.getNelsonIds(u), data.getPinyin(u)},
      _meaning{meaning}, _reading{reading} {}

NonLinkedKanji::NonLinkedKanji(const Data& data, const std::string& name,
    const Radical& radical, Strokes strokes, const std::string& reading,
    const Ucd* u) // LCOV_EXCL_LINE
    : NonLinkedKanji{
          data, name, radical, strokes, data.ucd().getMeaning(u), reading, u} {}

} // namespace kanji_tools
