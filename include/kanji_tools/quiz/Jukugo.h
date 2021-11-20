#ifndef KANJI_TOOLS_QUIZ_JUKUGO_H
#define KANJI_TOOLS_QUIZ_JUKUGO_H

#include <kanji_tools/utils/KanjiGrades.h>

namespace kanji_tools {

class Jukugo {
public:
  Jukugo(const std::string& name, const std::string& reading, KanjiGrades grade) : _name(name), _reading(reading), _grade(grade) {}

  const std::string& name() const { return _name; }
  const std::string& reading() const { return _reading; }
  std::string nameAndReading() const { return _name + "（" + _reading + "）"; }
  KanjiGrades grade() const { return _grade; }
private:
  const std::string _name;
  const std::string _reading;
  const KanjiGrades _grade;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_JUKUGO_H
