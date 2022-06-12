#pragma once

#include <kt_kanji/KanjiEnums.h>

#include <memory>

namespace kanji_tools { /// \quiz_group{Jukugo}
/// Jukugo (Japanese compound word) class

/// class for holding a Japanese Jukugo (熟語) \quiz{Jukugo}
class Jukugo final {
public:
  /// create a Jukugo object
  /// \param name Jukugo name in Kanji (can also include Kana)
  /// \param reading Jukugo reading in Hiragana
  /// \param grade Jukugo grade level
  /// \throw DomainError if `name` has less than 2 Kanji or `reading` is not all
  ///     Hiragana (prolong mark, ー, is also allowed)
  Jukugo(const String& name, const String& reading, KanjiGrades grade);

  Jukugo(const Jukugo&) = delete;

  [[nodiscard]] auto& name() const { return _name; }
  [[nodiscard]] auto& reading() const { return _reading; }
  [[nodiscard]] auto grade() const { return _grade; }

  /// return 'name' plus 'reading' in wide brackets, e.g., "朝日 (あさひ)"
  [[nodiscard]] String nameAndReading() const;
private:
  void error(const String&) const;

  const String _name;
  const String _reading;
  const KanjiGrades _grade;
};

using JukugoPtr = std::shared_ptr<Jukugo>;

/// \end_group
} // namespace kanji_tools
