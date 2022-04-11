#include <kanji_tools/kanji/NonLinkedKanji.h>

namespace kanji_tools {

Kanji::LinkNames NonLinkedKanji::linkNames(UcdPtr u) {
  LinkNames result;
  if (u && u->hasLinks())
    std::transform(u->links().begin(), u->links().end(),
        std::back_inserter(result), [](const auto& i) { return i.name(); });
  return result;
}

} // namespace kanji_tools
