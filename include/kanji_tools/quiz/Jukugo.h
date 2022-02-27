#pragma once

#include <kanji_tools/kanji/KanjiGrades.h>

namespace kanji_tools {

class Jukugo {
public:
  Jukugo(const std::string& name, const std::string& reading, KanjiGrades grade)
      : _name(name), _reading(reading), _grade(grade) {}

  Jukugo(const Jukugo&) = delete;
  // operator= is not generated since there are const members

  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto& reading() const { return _reading; }
  [[nodiscard]] auto nameAndReading() const {
    return _name + "（" + _reading + "）";
  }
  [[nodiscard]] auto grade() const { return _grade; }
private:
  const std::string _name;
  const std::string _reading;
  const KanjiGrades _grade;
};

} // namespace kanji_tools
