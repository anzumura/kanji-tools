#pragma once

#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/utils/ColumnFile.h>

namespace kanji_tools { /// \kanji_group{CustomFileKanji}
/// CustomFileKanji and LinkedKanji class hierarchies

/// base class for ExtraKanji and OfficialKanji and supports loading data from
/// column based customized local files \kanji{CustomFileKanji}
class CustomFileKanji : public NonLinkedKanji {
public:
  using File = const ColumnFile&;
  using Number = uint16_t;

  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] OptString extraTypeInfo() const override;
  [[nodiscard]] OldNames oldNames() const override { return _oldNames; }

  [[nodiscard]] auto number() const { return _number; }

  /// factory method that creates a list of Kanji of type `T`:
  /// \tparam T must have public RequiredColumns and a ctor taking KanjiDataRef
  ///     and a custom file (currently JouyouKanji, JinmeiKanji and ExtraKanji)
  /// \param d KanjiData instance that is used for constructing Kanji
  /// \param f path to file that must have tab separated data with the right
  ///     number of columns for the given Kanji type `T` (and the first line
  ///     must have header names that match the static `Column` instance names)
  /// \return a list of Kanji instance of type `T`
  template<typename T>
  [[nodiscard]] static auto fromFile(KanjiDataRef d, const KanjiData::Path& f);
protected:
  static Name name(File);

  inline static const ColumnFile::Column NumberCol{"Number"}, NameCol{"Name"},
      RadicalCol{"Radical"}, OldNamesCol{"OldNames"}, YearCol{"Year"},
      StrokesCol{"Strokes"}, GradeCol{"Grade"}, MeaningCol{"Meaning"},
      ReadingCol{"Reading"}, ReasonCol{"Reason"};

  /// ctor used by CustomFileKanji and ExtraKanji: has a 'meaning' param
  CustomFileKanji(KanjiDataRef, File, Name, Strokes, Meaning, OldNames, UcdPtr);

  /// ctor used by OfficialKanji: 'strokes' and 'meaning' loaded from `UcdPtr`
  CustomFileKanji(KanjiDataRef, File, Name, OldNames, UcdPtr);
private:
  const KenteiKyus _kyu;
  const Number _number;
  const LinkNames _oldNames;
};

/// has attributes shared by Jouyou and Jinmei kanji like 'level' (can be None)
/// plus optional 'Frequency' and 'Year' values \kanji{CustomFileKanji}
class OfficialKanji : public CustomFileKanji {
public:
  [[nodiscard]] OptString extraTypeInfo() const override;
  [[nodiscard]] Frequency frequency() const override { return _frequency; }
  [[nodiscard]] JlptLevels level() const override { return _level; }
  [[nodiscard]] Year year() const override { return _year; }
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

/// class representing the 633 official Jinmeiyō Kanji \kanji{CustomFileKanji}
class JinmeiKanji : public OfficialKanji {
public:
  JinmeiKanji(KanjiDataRef, File);

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Jinmei; }
  [[nodiscard]] JinmeiReasons reason() const override { return _reason; }
  [[nodiscard]] OptString extraTypeInfo() const override;

  inline static const std::array RequiredColumns{
      OldNamesCol, YearCol, ReasonCol};
private:
  const JinmeiReasons _reason;
};

/// class representing the 2,136 official Jōyō Kanji \kanji{CustomFileKanji}
class JouyouKanji : public OfficialKanji {
public:
  JouyouKanji(KanjiDataRef, File);
  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Jouyou; }
  [[nodiscard]] KanjiGrades grade() const override { return _grade; }

  inline static const std::array RequiredColumns{
      OldNamesCol, YearCol, StrokesCol, GradeCol, MeaningCol};
private:
  [[nodiscard]] static KanjiGrades getGrade(const String&);
  const KanjiGrades _grade;
};

/// class for Kanji loaded from 'extra.txt' \kanji{CustomFileKanji}
///
/// This group contains manually selected 'fairly common' Kanji that aren't in
/// official Jōyō or Jinmeiyō lists (or their official old/alternative forms).
/// These Kanji should also not be in 'frequency.txt'.
class ExtraKanji : public CustomFileKanji {
public:
  ExtraKanji(KanjiDataRef, File);

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Extra; }
  [[nodiscard]] OptString newName() const override { return _newName; }

  inline static const std::array RequiredColumns{StrokesCol, MeaningCol};
private:
  ExtraKanji(KanjiDataRef, File, Name);
  ExtraKanji(KanjiDataRef, File, Name, UcdPtr);

  const OptString _newName;
};

/// base class Jōyō and Jinmeiyō 'linked' Kanji \kanji{CustomFileKanji}
///
/// Some of these Kanji are in the top 2,501 frequency list and almost all of
/// them are in Kentei KJ1 or K1 kyus. However, none of them have a JLPT level.
class LinkedKanji : public Kanji {
public:
  [[nodiscard]] Meaning meaning() const override;
  [[nodiscard]] Reading reading() const override;

  [[nodiscard]] Frequency frequency() const override { return _frequency; }
  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] KanjiPtr link() const override { return _link; }

  [[nodiscard]] bool linkedReadings() const override { return true; }
  [[nodiscard]] OptString newName() const override;
protected:
  LinkedKanji(KanjiDataRef, Name, const KanjiPtr&, UcdPtr);

  /// return %Kanji name passed in
  /// \throw DomainError if linkedOldKanji doesn't link to a JouyouKanji or
  /// LinkedJinmeiKanji doesn't link to either a JouyouKanji or a JinmeiKanji
  [[nodiscard]] static Name linkType(Name, const Kanji&, bool isJouyou = true);
private:
  const Frequency _frequency;
  const KenteiKyus _kyu;
  const KanjiPtr _link;
};

/// official set of 230 Jinmeiyō Kanji that are old or alternative forms of
/// JouyouKanji or JinmeiKanji \kanji{CustomFileKanji}
///
/// There are 230 of these Kanji:
/// \li 204 are part of the 365 JouyouKanji 'old names' set
/// \li 8 are different alternate forms of JouyouKanji (薗 駈 嶋 盃 冨 峯 埜 凉)
/// \li 18 are alternate forms of standard JinmeiKanji
class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(KanjiDataRef, Name, const KanjiPtr&);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedJinmei;
  }
};

/// official set of 163 Kanji that link to a JouyouKanji \kanji{CustomFileKanji}
///
/// These are the published Jōyō variants that aren't already included in the
/// 230 Jinmeiyō 'official variants'.
class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(KanjiDataRef, Name, const KanjiPtr&);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedOld;
  }
};

template<typename T>
auto CustomFileKanji::fromFile(KanjiDataRef data, const KanjiData::Path& f) {
  // all 'CustomFileKanji' files must have at least the following columns
  ColumnFile::Columns columns{NumberCol, NameCol, RadicalCol, ReadingCol};
  for (auto& i : T::RequiredColumns) columns.emplace_back(i);
  KanjiData::List results;
  for (ColumnFile file{f, columns}; file.nextRow();)
    results.emplace_back(std::make_shared<T>(data, file));
  return results;
}

/// \end_group
} // namespace kanji_tools
