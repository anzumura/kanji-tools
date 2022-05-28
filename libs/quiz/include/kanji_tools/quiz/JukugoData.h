#pragma once

#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/Jukugo.h>

namespace kanji_tools {

class JukugoData {
public:
  using List = std::vector<JukugoPtr>;

  // if 'dir' is provided it will be used instead of 'data->dataDir()/jukugo'
  // when looking for jukugo files (to help with testing)
  explicit JukugoData(const KanjiDataPtr&, const KanjiData::Path* dir = {});

  JukugoData(const JukugoData&) = delete;
  auto operator=(const JukugoData&) = delete;

  [[nodiscard]] const List& find(const String& kanji) const;
private:
  inline static const List EmptyList;

  static size_t findOpenBracket(const String&, bool onePerLine);
  static size_t findCloseBracket(const String&, bool onePerLine);
  static void error(const String&);

  void createJukugo(const String& line, KanjiGrades, bool onePerLine);

  using JukugoKey = std::pair<String, String>;
  [[nodiscard]] size_t loadFile(const KanjiData::Path&, KanjiGrades);

  std::map<JukugoKey, JukugoPtr> _uniqueJukugo;
  std::map<String, List> _kanjiToJukugo;
};

using JukugoDataPtr = std::shared_ptr<const JukugoData>;

} // namespace kanji_tools
