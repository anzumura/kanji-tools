#pragma once

#include <kanji_tools/kanji/NonLinkedKanji.h>
#include <kanji_tools/utils/ColumnFile.h>

namespace kanji_tools {

// 'CustomFileKanji' is the base class for ExtraKanji and OfficialKanji and
// supports loading data from column based customized local files.
// - Each file contains: 'Number', 'Name', 'Radical' and 'Reading'
// - Jouyou and Extra files contain 'Strokes', whereas Jinmei strokes come from
//   'strokes.txt' or 'ucd.txt'
class CustomFileKanji : public NonLinkedKanji {
public:
  using File = const ColumnFile&;
  using Number = u_int16_t;

  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] OptString extraTypeInfo() const override;
  [[nodiscard]] OldNames oldNames() const override { return _oldNames; }

  [[nodiscard]] auto number() const { return _number; }

  // 'fromFile' is a factory method that creates a list of Kanji of type 'T':
  // - 'T' must have a public 'RequiredColumns' and be constructable from 'data'
  //   and a custom file (currently JouyouKanji, JinmeiKanji and ExtraKanji)
  // - 'f' must have tab separated lines that have the right number of columns
  //   for the given Kanji type 'T' (and the first line must have header names
  //   that match the static 'Column' instances below)
  template<typename T>
  [[nodiscard]] static Data::KanjiList fromFile(
      DataRef data, const Data::Path& f) {
    // all 'CustomFileKanji' files must have at least the following columns
    ColumnFile::Columns columns{NumberCol, NameCol, RadicalCol, ReadingCol};
    for (auto& i : T::RequiredColumns) columns.emplace_back(i);
    Data::KanjiList results;
    for (ColumnFile file{f, columns}; file.nextRow();)
      results.emplace_back(std::make_shared<T>(data, file));
    return results;
  }
protected:
  static Name name(File);

  inline static const ColumnFile::Column NumberCol{"Number"}, NameCol{"Name"},
      RadicalCol{"Radical"}, OldNamesCol{"OldNames"}, YearCol{"Year"},
      StrokesCol{"Strokes"}, GradeCol{"Grade"}, MeaningCol{"Meaning"},
      ReadingCol{"Reading"}, ReasonCol{"Reason"};

  // ctor used by 'CustomFileKanji' and 'ExtraKanji': has a 'meaning' field
  CustomFileKanji(DataRef, File, Name, Strokes, Meaning, OldNames, UcdPtr);

  // ctor used by 'OfficialKanji': 'strokes' and 'meaning' loaded from 'ucd.txt'
  CustomFileKanji(DataRef, File, Name, OldNames, UcdPtr);
private:
  const KenteiKyus _kyu;
  const Number _number;
  const LinkNames _oldNames;
};

// 'OfficialKanji' contains attributes shared by Jouyou and Jinmei kanji, i.e.,
// 'level' (can be None) plus optional 'Frequency' and 'Year' values
class OfficialKanji : public CustomFileKanji {
public:
  [[nodiscard]] OptString extraTypeInfo() const override;
  [[nodiscard]] OptFreq frequency() const override { return _frequency; }
  [[nodiscard]] JlptLevels level() const override { return _level; }
  [[nodiscard]] Year year() const override { return _year; }
protected:
  // ctor used by 'JinmeiKanji'
  OfficialKanji(DataRef, File, Name, UcdPtr);

  // ctor used by 'JouyouKanji' calls base with 'strokes' and 'meaning' fields
  OfficialKanji(DataRef, File, Name, Strokes, Meaning);
private:
  [[nodiscard]] static LinkNames getOldNames(File);

  const OptFreq _frequency;
  const JlptLevels _level;
  const Year _year;
};

class JinmeiKanji : public OfficialKanji {
public:
  JinmeiKanji(DataRef, File);

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Jinmei; }
  [[nodiscard]] JinmeiReasons reason() const override { return _reason; }
  [[nodiscard]] OptString extraTypeInfo() const override;

  inline static const std::array RequiredColumns{
      OldNamesCol, YearCol, ReasonCol};
private:
  const JinmeiReasons _reason;
};

class JouyouKanji : public OfficialKanji {
public:
  JouyouKanji(DataRef, File);
  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Jouyou; }
  [[nodiscard]] KanjiGrades grade() const override { return _grade; }

  inline static const std::array RequiredColumns{
      OldNamesCol, YearCol, StrokesCol, GradeCol, MeaningCol};
private:
  [[nodiscard]] static KanjiGrades getGrade(const std::string&);
  const KanjiGrades _grade;
};

// 'ExtraKanji' is used for Kanji loaded from 'extra.txt' which is meant to hold
// 'fairly common' Kanji that aren't in official lists (Jouyou, Jinmei and their
// links). They should also not be in 'frequency.txt' or have a JLPT level.
class ExtraKanji : public CustomFileKanji {
public:
  ExtraKanji(DataRef, File);

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Extra; }
  [[nodiscard]] OptString newName() const override { return _newName; }

  inline static const std::array RequiredColumns{StrokesCol, MeaningCol};
private:
  ExtraKanji(DataRef, File, Name);
  ExtraKanji(DataRef, File, Name, UcdPtr);

  const OptString _newName;
};

} // namespace kanji_tools
