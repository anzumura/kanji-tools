#pragma once

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// 'Stats' counts all multi-byte characters in 'top' file and if 'top' is a
// directroy than all the regulars under top will be processed (recursively).
// The 'count' for each unique kanji (frequency) will be displayed (non-kanji
// are not included).
class Stats {
public:
  // Command line options must specify one or more files and 'data' class is
  // used to lookup kanji found in files - see HelpMessage in Stats.cpp for more
  // details on command line options.
  Stats(Data::ArgCount argc, const char** argv, DataPtr data);

  Stats(const Stats&) = delete;
  // operator= is not generated since there are const members
private:
  [[nodiscard]] auto& log(bool heading = false) const {
    return _data->log(heading);
  }
  [[nodiscard]] auto& out() const { return _data->out(); }

  void countKanji(
      const Data::Path& top, bool showBreakdown, bool verbose) const;

  const DataPtr _data;
};

} // namespace kanji_tools