#pragma once

#include <kt_kanji/KanjiData.h>
#include <kt_quiz/Jukugo.h>

namespace kanji_tools { /// \quiz_group{JukugoData}
/// JukugoData class for loading data from 'jukugo/*.txt' files

/// load, store and find Jukugo objects \quiz{JukugoData}
class JukugoData final {
public:
  using List = std::vector<JukugoPtr>;

  /// ctor loads data from 'jukugo/*.txt' files and prints a summary or all data
  /// loaded depending on the value of KanjiData::DebugMode
  /// \param data used for validating Kanji and printing
  /// \param dir can override using `data->dataDir()` (to help testing)
  /// \throw DomainError if there are problems reading in data like formatting,
  ///     Jukugo is repeated within a file or Jukugo ctor throws and error
  explicit JukugoData(
      const KanjiDataPtr& data, const KanjiData::Path* dir = {});

  JukugoData(const JukugoData&) = delete;     ///< deleted copy ctor
  auto operator=(const JukugoData&) = delete; ///< deleted operator=

  /// return a list of Jukugo containing `kanji`
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

/// \end_group
} // namespace kanji_tools
