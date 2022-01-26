#ifndef KANJI_TOOLS_QUIZ_JUKUGO_DATA_H
#define KANJI_TOOLS_QUIZ_JUKUGO_DATA_H

#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/quiz/Jukugo.h>

#include <map>
#include <vector>

namespace kanji_tools {

class JukugoData {
public:
  using Entry = std::shared_ptr<Jukugo>;
  using List = std::vector<Entry>;
  JukugoData(const Data&);
  JukugoData(const JukugoData&) = delete;

  auto& find(const std::string& kanji) const {
    auto i = _kanjiToJukugo.find(kanji);
    return i != _kanjiToJukugo.end() ? i->second : _emptyList;
  }
private:
  template<typename T> void createJukugo(T& error, KanjiGrades, const std::string& name, const std::string& reading);

  using JukugoKey = std::pair<std::string, std::string>;
  int loadFile(const std::filesystem::path&, KanjiGrades);

  std::map<JukugoKey, Entry> _uniqueJukugo;
  std::map<std::string, List> _kanjiToJukugo;
  inline static const List _emptyList;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_JUKUGO_DATA_H
