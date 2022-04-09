#pragma once

#include <kanji_tools/kanji/JinmeiKanjiReasons.h>
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
  using Number = u_int16_t;

  [[nodiscard]] KenteiKyus kyu() const override { return _kyu; }
  [[nodiscard]] OptString extraTypeInfo() const override;
  [[nodiscard]] const LinkNames& oldNames() const override { return _oldNames; }

  [[nodiscard]] auto number() const { return _number; }

  // 'fromFile' is a factory method that creates a list of Kanji of type 'T':
  // - 'T' must have a public 'RequiredColumns' and be constructable from 'data'
  //   and a custom file (currently JouyouKanji, JinmeiKanji and ExtraKanji)
  // - 'f' must have tab separated lines that have the right number of columns
  //   for the given Kanji type 'T' (and the first line must have header names
  //   that match the static 'Column' instances below)
  template<typename T>
  [[nodiscard]] static Data::List fromFile(const Data& d, const Data::Path& f) {
    // all 'CustomFileKanji' files must have at least the following columns
    ColumnFile::Columns columns{NumberCol, NameCol, RadicalCol, ReadingCol};
    for (auto& i : T::RequiredColumns) columns.emplace_back(i);
    Data::List results;
    for (ColumnFile file{f, columns}; file.nextRow();)
      results.emplace_back(std::make_shared<T>(d, file));
    return results;
  }
protected:
  inline static const ColumnFile::Column NumberCol{"Number"}, NameCol{"Name"},
      RadicalCol{"Radical"}, OldNamesCol{"OldNames"}, YearCol{"Year"},
      StrokesCol{"Strokes"}, GradeCol{"Grade"}, MeaningCol{"Meaning"},
      ReadingCol{"Reading"}, ReasonCol{"Reason"};

  [[nodiscard]] static const Ucd* findUcd(const Data&, const std::string& name);

  // ctor used by 'CustomFileKanji' and 'ExtraKanji': has a 'meaning' field
  CustomFileKanji(const Data&, const ColumnFile&, const std::string& name,
      Strokes, const std::string& meaning, const LinkNames&, const Ucd*);

  // ctor used by 'OfficialKanji': 'meaning' gets looked up from 'ucd.txt'
  CustomFileKanji(const Data&, const ColumnFile&, const std::string& name,
      Strokes, const LinkNames&, const Ucd*);
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

  [[nodiscard]] auto year() const { return _year; }
protected:
  // ctor used by 'JinmeiKanji' calls base without 'meaning' field
  OfficialKanji(
      const Data&, const ColumnFile&, const std::string& name, const Ucd*);

  // ctor used by 'JouyouKanji' calls base with 'meaning' field
  OfficialKanji(const Data&, const ColumnFile&, const std::string& name,
      Strokes, const std::string& meaning);
private:
  [[nodiscard]] static LinkNames getOldNames(const ColumnFile&);

  const OptFreq _frequency;
  const JlptLevels _level;
  const OptFreq _year;
};

class JinmeiKanji : public OfficialKanji {
public:
  JinmeiKanji(const Data&, const ColumnFile&);

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Jinmei; }
  [[nodiscard]] OptString extraTypeInfo() const override;
  [[nodiscard]] auto reason() const { return _reason; }

  inline static const std::array RequiredColumns{
      OldNamesCol, YearCol, ReasonCol};
private:
  const JinmeiKanjiReasons _reason;
};

class JouyouKanji : public OfficialKanji {
public:
  JouyouKanji(const Data&, const ColumnFile&);
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
  ExtraKanji(const Data&, const ColumnFile&);

  [[nodiscard]] KanjiTypes type() const override { return KanjiTypes::Extra; }
  [[nodiscard]] OptString newName() const override { return _newName; }

  inline static const std::array RequiredColumns{StrokesCol, MeaningCol};
private:
  ExtraKanji(const Data&, const ColumnFile&, const std::string&);
  ExtraKanji(const Data&, const ColumnFile&, const std::string&, const Ucd*);

  const OptString _newName;
};

} // namespace kanji_tools
