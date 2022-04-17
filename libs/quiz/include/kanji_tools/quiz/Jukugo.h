#pragma once

#include <kanji_tools/kanji/KanjiGrades.h>

#include <memory>

namespace kanji_tools {

class Jukugo {
public:
  Jukugo(const std::string& name, const std::string& reading, KanjiGrades);

  Jukugo(const Jukugo&) = delete;

  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto& reading() const { return _reading; }
  [[nodiscard]] auto grade() const { return _grade; }
  [[nodiscard]] std::string nameAndReading() const;
private:
  void error(const std::string&) const;

  const std::string _name;
  const std::string _reading;
  const KanjiGrades _grade;
};

using JukugoPtr = std::shared_ptr<Jukugo>;

} // namespace kanji_tools
