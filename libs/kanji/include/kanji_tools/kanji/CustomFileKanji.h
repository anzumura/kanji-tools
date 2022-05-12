#pragma once

#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/kanji/KanjiGrades.h>
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
  using Number = uint16_t;

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
  [[nodiscard]] static KanjiData::KanjiList fromFile(
      KanjiDataRef data, const KanjiData::Path& f) {
    // all 'CustomFileKanji' files must have at least the following columns
    ColumnFile::Columns columns{NumberCol, NameCol, RadicalCol, ReadingCol};
    for (auto& i : T::RequiredColumns) columns.emplace_back(i);
    KanjiData::KanjiList results;
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
  CustomFileKanji(KanjiDataRef, File, Name, Strokes, Meaning, OldNames, UcdPtr);

  // ctor used by 'OfficialKanji': 'strokes' and 'meaning' loaded from 'ucd.txt'
  CustomFileKanji(KanjiDataRef, File, Name, OldNames, UcdPtr);
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
  [[nodiscard]] Frequency frequency() const override { return _frequency; }
  [[nodiscard]] JlptLevels level() const override { return _level; }
  [[nodiscard]] Year year() const override { return _year; }
protected:
  // ctor used by 'JinmeiKanji'
  OfficialKanji(KanjiDataRef, File, Name, UcdPtr);

  // ctor used by 'JouyouKanji' calls base with 'strokes' and 'meaning' fields
  OfficialKanji(KanjiDataRef, File, Name, Strokes, Meaning);
private:
  [[nodiscard]] static LinkNames getOldNames(File);

  const Frequency _frequency;
  const JlptLevels _level;
  const Year _year;
};

// JinmeiReasons represents reason kanji was added to Jinmei list:
// - Names: for use in names
// - Print: for use in publications
// - Variant: allowed variant form (異体字)
// - Moved: moved out of Jouyou into Jinmei
// - Simple: simplified form (表外漢字字体表の簡易慣用字体)
// - Other: reason listed as その他
// - None: all JinmeiKanji have one of the above reasons, None is used for base
//   class virtual function return value (similar to other Kanji related enums)
enum class JinmeiReasons : EnumContainer::Size {
  Names,
  Print,
  Variant,
  Moved,
  Simple,
  Other,
  None
};

template<> inline constexpr auto is_enumlist_with_none<JinmeiReasons>{true};

inline const auto AllJinmeiReasons{BaseEnumList<JinmeiReasons>::create(
    "Names", "Print", "Variant", "Moved", "Simple", "Other")};

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

// 'ExtraKanji' is used for Kanji loaded from 'extra.txt' which is meant to hold
// 'fairly common' Kanji that aren't in official lists (Jouyou, Jinmei and their
// links). They should also not be in 'frequency.txt' or have a JLPT level.
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

// 'LinkedKanji' are official variants of Jouyou or Jinmei Kanji. Some are in
// the top 2501 frequency list and they are generally in Kentei KJ1 or K1 kyus.
// However, none of them are in a JLPT level and 'meaning' and 'reading' methods
// return the values from the 'link' (ie original Jouyou or Jinmei) Kanji.
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

  // linkedOldKanji must link back to Jouyou and LinkedJinmeiKanji can link to
  // either Jouyou or Jinmei
  [[nodiscard]] static Name linkType(Name, const Kanji&, bool isJouyou = true);
private:
  const Frequency _frequency;
  const KenteiKyus _kyu;
  const KanjiPtr _link;
};

// There are 230 of these Kanji. 212 have a 'link' pointing to a JinmeiKanji and
// 18 have a 'link' pointing to a or JouyouKanji.
class LinkedJinmeiKanji : public LinkedKanji {
public:
  LinkedJinmeiKanji(KanjiDataRef, Name, const KanjiPtr&);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedJinmei;
  }
};

// There are 163 of these Kanji. 'link' points to a JouyouKanji - these are the
// published Jōyō variants that aren't already included as Jinmeiyō variants.
class LinkedOldKanji : public LinkedKanji {
public:
  LinkedOldKanji(KanjiDataRef, Name, const KanjiPtr&);

  [[nodiscard]] KanjiTypes type() const override {
    return KanjiTypes::LinkedOld;
  }
};

} // namespace kanji_tools
