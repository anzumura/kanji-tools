#pragma once

#include <kanji_tools/kanji/KanjiData.h>

namespace kanji_tools {

// 'Stats' counts all multi-byte characters in 'top' file and if 'top' is a
// directory than all the regulars under top will be processed (recursively).
// The 'count' for each unique kanji (frequency) will be displayed (non-kanji
// are not included).
class Stats {
public:
  // Command line options must specify one or more files and 'data' class is
  // used to lookup kanji found in files - see HelpMessage in Stats.cpp for more
  // details on command line options.
  Stats(const Args&, const KanjiDataPtr&);

  Stats(const Stats&) = delete;
private:
  [[nodiscard]] auto& log(bool heading = false) const {
    return _data->log(heading);
  }
  [[nodiscard]] auto& out() const { return _data->out(); }

  void countKanji(
      const KanjiData::Path& top, bool showBreakdown, bool verbose) const;

  const KanjiDataPtr _data;
};

} // namespace kanji_tools
