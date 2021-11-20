#ifndef KANJI_TOOLS_QUIZ_JUKUGO_DATA_H
#define KANJI_TOOLS_QUIZ_JUKUGO_DATA_H

#include <kanji_tools/quiz/Jukugo.h>
#include <kanji_tools/kanji/Data.h>

#include <map>
#include <vector>

namespace kanji_tools {

class JukugoData {
public:
  using Entry = std::shared_ptr<Jukugo>;
  using List = std::vector<Entry>;
  using Map = std::map<std::string, List>;
  JukugoData(const Data&);
  JukugoData(const JukugoData&) = delete;

  const List& find(const std::string& kanji) const {
    auto i = _map.find(kanji);
    return i != _map.end() ? i->second : _emptyList;
  }
private:
  int loadFile(const std::filesystem::path&, KanjiGrades);

  Map _map;
  inline static const List _emptyList;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_JUKUGO_DATA_H
