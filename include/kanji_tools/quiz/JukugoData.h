#pragma once

#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/quiz/Jukugo.h>

namespace kanji_tools {

class JukugoData {
public:
  using Entry = std::shared_ptr<Jukugo>;
  using List = std::vector<Entry>;

  JukugoData(DataPtr);

  JukugoData(const JukugoData&) = delete;
  JukugoData& operator=(const JukugoData&) = delete;

  [[nodiscard]] auto& find(const std::string& kanji) const {
    const auto i{_kanjiToJukugo.find(kanji)};
    return i != _kanjiToJukugo.end() ? i->second : EmptyList;
  }
private:
  inline static const List EmptyList;

  template<typename T>
  void createJukugo(T& error, KanjiGrades, const std::string& name,
                    const std::string& reading);

  using JukugoKey = std::pair<std::string, std::string>;
  [[nodiscard]] size_t loadFile(const std::filesystem::path&, KanjiGrades);

  std::map<JukugoKey, Entry> _uniqueJukugo;
  std::map<std::string, List> _kanjiToJukugo;
};

using JukugoDataPtr = std::shared_ptr<const JukugoData>;

} // namespace kanji_tools
