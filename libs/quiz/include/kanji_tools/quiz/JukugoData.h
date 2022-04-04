#pragma once

#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/quiz/Jukugo.h>

namespace kanji_tools {

class JukugoData {
public:
  using Entry = std::shared_ptr<Jukugo>;
  using List = std::vector<Entry>;

  // if 'dir' is provided it will be used intead of 'data->dataDir()/jukugo'
  // when looking for jukugo files (to help with testing)
  JukugoData(DataPtr, const Data::Path* dir = {});

  JukugoData(const JukugoData&) = delete;
  JukugoData& operator=(const JukugoData&) = delete;

  [[nodiscard]] const List& find(const std::string& kanji) const;
private:
  inline static const List EmptyList;

  static size_t findOpenBracket(const std::string&, bool onePerLine);
  static size_t findCloseBracket(const std::string&, bool onePerLine);
  static void error(const std::string&);

  void createJukugo(const std::string& line, KanjiGrades);

  using JukugoKey = std::pair<std::string, std::string>;
  [[nodiscard]] size_t loadFile(const Data::Path&, KanjiGrades);

  std::map<JukugoKey, Entry> _uniqueJukugo;
  std::map<std::string, List> _kanjiToJukugo;
};

using JukugoDataPtr = std::shared_ptr<const JukugoData>;

} // namespace kanji_tools
