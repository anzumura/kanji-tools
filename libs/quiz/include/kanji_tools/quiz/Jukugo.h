#pragma once

#include <kanji_tools/kanji/KanjiEnums.h>

#include <memory>

namespace kanji_tools {

class Jukugo {
public:
  Jukugo(const String& name, const String& reading, KanjiGrades);

  Jukugo(const Jukugo&) = delete;

  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto& reading() const { return _reading; }
  [[nodiscard]] auto grade() const { return _grade; }
  [[nodiscard]] String nameAndReading() const;
private:
  void error(const String&) const;

  const String _name;
  const String _reading;
  const KanjiGrades _grade;
};

using JukugoPtr = std::shared_ptr<Jukugo>;

} // namespace kanji_tools
