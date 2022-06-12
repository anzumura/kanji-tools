#pragma once

#include <kt_kanji/KanjiData.h>
#include <kt_utils/ColumnFile.h>

namespace kanji_tools { /// \kanji_group{OfficialKanji}
/// NumberedKanji, OfficialKanji and OfficialLinkedKanji class hierarchies

/// has 'kyu', 'number' and 'oldNames' fields and includes shared functionality
/// for loading from customized local files \kanji{OfficialKanji}
class NumberedKanji : public LoadedKanji {
public:
  using File = const ColumnFile&;
  using Number = uint16_t;

  [[nodiscard]] KenteiKyus kyu() const final { return _kyu; }
  [[nodiscard]] OptString extraTypeInfo() const override;
  [[nodiscard]] OldNames oldNames() const final { return _oldNames; }

  /// return a unique number starting at `1` for this Kanji (matches the row
  /// number of the source '.txt' file)
  [[nodiscard]] auto number() const { return _number; }

  /// factory method that creates a list of Kanji of type `T`
  /// \details all files must have 'Number', 'Name', 'Radical' and 'Reading'
  ///     columns plus the columns listed in T::RequiredColumns
  /// \tparam T must have public RequiredColumns and a ctor taking KanjiDataRef
  ///     and a custom file (currently JouyouKanji, JinmeiKanji and ExtraKanji)
  /// \param d KanjiData instance that is used for constructing Kanji
  /// \param f path to file that must have tab separated data with the right
  ///     number of columns for the given Kanji type `T` (and the first line
  ///     must have header names that match the static `Column` instance names)
  /// \return a list of Kanji instance of type `T`
  /// \throw DomainError if `f` is missing or has malformed data
  template<typename T>
  [[nodiscard]] static auto fromFile(KanjiDataRef d, const KanjiData::Path& f);
protected:
  static Name name(File);

  inline static const ColumnFile::Column NumberCol{"Number"}, NameCol{"Name"},
      RadicalCol{"Radical"}, OldNamesCol{"OldNames"}, YearCol{"Year"},
      StrokesCol{"Strokes"}, GradeCol{"Grade"}, MeaningCol{"Meaning"},
      ReadingCol{"Reading"}, ReasonCol{"Reason"};

  /// ctor used by NumberedKanji and ExtraKanji: has a 'meaning' param
  NumberedKanji(KanjiDataRef, File, Name, Strokes, Meaning, OldNames, UcdPtr);

  /// ctor used by OfficialKanji: 'strokes' and 'meaning' loaded from `UcdPtr`
  NumberedKanji(KanjiDataRef, File, Name, OldNames, UcdPtr);
private:
  const KenteiKyus _kyu;
  const Number _number;
  const LinkNames _oldNames;
};

/// has attributes shared by derived classes including 'level' (can be `None`)
/// and optional 'frequency' and 'year' values \kanji{OfficialKanji}
class OfficialKanji : public NumberedKanji {
public:
  [[nodiscard]] OptString extraTypeInfo() const override;
  [[nodiscard]] Frequency frequency() const final { return _frequency; }
  [[nodiscard]] JlptLevels level() const final { return _level; }
  [[nodiscard]] Year year() const final { return _year; }
protected:
  /// ctor used by JinmeiKanji
  OfficialKanji(KanjiDataRef, File, Name, UcdPtr);

  /// ctor used by JouyouKanji, calls base with 'strokes' and 'meaning' params
  OfficialKanji(KanjiDataRef, File, Name, Strokes, Meaning);
private:
  [[nodiscard]] static LinkNames getOldNames(File);

  const Frequency _frequency;
  const JlptLevels _level;
  const Year _year;
};

/// class representing the 633 official Jinmeiyō Kanji \kanji{OfficialKanji}
class JinmeiKanji final : public OfficialKanji {
public:
  /// ctor called by fromFile() method
  JinmeiKanji(KanjiDataRef, File);

  [[nodiscard]] KanjiTypes type() const final { return KanjiTypes::Jinmei; }
  [[nodiscard]] JinmeiReasons reason() const final { return _reason; }
  [[nodiscard]] OptString extraTypeInfo() const final;

  /// additional columns required by JinmeiKanji, see fromFile()
  inline static const std::array RequiredColumns{
      OldNamesCol, YearCol, ReasonCol};
private:
  const JinmeiReasons _reason;
};

/// class representing the 2,136 official Jōyō Kanji \kanji{OfficialKanji}
class JouyouKanji final : public OfficialKanji {
public:
  /// ctor called by fromFile() method
  JouyouKanji(KanjiDataRef, File);

  [[nodiscard]] KanjiTypes type() const final { return KanjiTypes::Jouyou; }
  [[nodiscard]] KanjiGrades grade() const final { return _grade; }

  /// additional columns required by JouyouKanji, see fromFile()
  inline static const std::array RequiredColumns{
      OldNamesCol, YearCol, StrokesCol, GradeCol, MeaningCol};
private:
  [[nodiscard]] static KanjiGrades getGrade(const String&);
  const KanjiGrades _grade;
};

/// class for Kanji loaded from 'extra.txt' \kanji{OfficialKanji}
///
/// This group contains manually selected 'fairly common' Kanji that aren't in
/// official Jōyō or Jinmeiyō lists (or their official old/alternative forms).
/// These Kanji should also not be in 'frequency.txt'.
class ExtraKanji final : public NumberedKanji {
public:
  /// ctor called by fromFile() method
  ExtraKanji(KanjiDataRef, File);

  [[nodiscard]] KanjiTypes type() const final { return KanjiTypes::Extra; }
  [[nodiscard]] OptString newName() const final { return _newName; }

  /// additional columns required by ExtraKanji, see fromFile()
  inline static const std::array RequiredColumns{StrokesCol, MeaningCol};
private:
  ExtraKanji(KanjiDataRef, File, Name);
  ExtraKanji(KanjiDataRef, File, Name, UcdPtr);

  const OptString _newName;
};

/// base class Jōyō and Jinmeiyō 'linked' Kanji \kanji{OfficialKanji}
///
/// Some of these Kanji are in the top 2,501 frequency list and almost all of
/// them are in Kentei KJ1 or K1 kyus. However, none of them have a JLPT level.
class OfficialLinkedKanji : public Kanji {
public:
  [[nodiscard]] Meaning meaning() const final;
  [[nodiscard]] Reading reading() const final;

  [[nodiscard]] Frequency frequency() const final { return _frequency; }
  [[nodiscard]] KenteiKyus kyu() const final { return _kyu; }
  [[nodiscard]] Link link() const final { return _link; }

  [[nodiscard]] bool linkedReadings() const final { return true; }
  [[nodiscard]] OptString newName() const final;
protected:
  OfficialLinkedKanji(KanjiDataRef, Name, Link, UcdPtr);

  /// used by ctor to ensure `link` has expected type
  /// \param name String name of instance being constructed
  /// \param link Kanji to use as the link
  /// \param isOld true if called from LinkedOld ctor
  /// \return `name` (the same value passed into the function)
  /// \throw DomainError if `link` type is not Jouyou and (`isOld` is true or
  ///     `link` type is not Jinmei)
  [[nodiscard]] static Name check(Name name, Link link, bool isOld);
private:
  const Frequency _frequency;
  const KenteiKyus _kyu;
  const KanjiPtr _link;
};

/// official set of 230 Jinmeiyō Kanji that are old or alternative forms of
/// JouyouKanji or JinmeiKanji \kanji{OfficialKanji}
///
/// There are 230 of these Kanji:
/// \li 204 are part of the 365 JouyouKanji 'old names' set
/// \li 8 are different alternate forms of JouyouKanji (薗 駈 嶋 盃 冨 峯 埜 凉)
/// \li 18 are alternate forms of standard JinmeiKanji
class LinkedJinmeiKanji final : public OfficialLinkedKanji {
public:
  /// ctor called by KanjiData
  LinkedJinmeiKanji(KanjiDataRef, Name, Link);

  [[nodiscard]] KanjiTypes type() const final {
    return KanjiTypes::LinkedJinmei;
  }
};

/// official set of 163 Kanji that link to a JouyouKanji \kanji{OfficialKanji}
///
/// These are the published Jōyō variants that aren't already included in the
/// 230 Jinmeiyō 'official variants'.
class LinkedOldKanji final : public OfficialLinkedKanji {
public:
  /// ctor called by KanjiData (after creating all LinkedJinmeiKanji)
  LinkedOldKanji(KanjiDataRef, Name, Link);

  [[nodiscard]] KanjiTypes type() const final { return KanjiTypes::LinkedOld; }
};

template<typename T>
auto NumberedKanji::fromFile(KanjiDataRef d, const KanjiData::Path& f) {
  ColumnFile::Columns columns{NumberCol, NameCol, RadicalCol, ReadingCol};
  for (auto& i : T::RequiredColumns) columns.emplace_back(i);
  KanjiData::List results;
  for (ColumnFile file{f, columns}; file.nextRow();)
    results.emplace_back(std::make_shared<T>(d, file));
  return results;
}

/// \end_group
} // namespace kanji_tools
